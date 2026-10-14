/*
 * Copyright (C) 2026, amigazen project
 */

/*
 * voyager_ssl_amissl.vlib - VSSL ABI on AmiSSL 5.27 (OpenSSL 3.6).
 *
 * Same LVOs as Source/VSSL/amitls (fd/vssl.fd, library version 10).
 * Libinit fails if amisslmaster.library cannot open AmiSSL 5.27, so
 * Voyager falls back to voyager_ssl.vlib (amitls.library).
 *
 * Voyager still passes OpenSSL 0.9 option bits from Source/vssl.h.
 * Those are translated before SSL_CTX_set_options(); the low 20 bits
 * of the old SSL_OP_ALL are not passed through, because OpenSSL 3 uses
 * that range for unrelated flags.
 *
 * SAS/C has no 64-bit integers.  AmiSSL's amissl/types.h typedefs
 * int64_t and uint64_t to 32-bit types so the headers parse, and the
 * library still expects a 64-bit argument as D0 (high) and D1 (low).
 * The option bits we pass all sit below bit 32, so the libcall passes
 * 0 in D0 and the flags in D1.
 *
 * Handshake matches the AmiTLS plugin: ConnectHost only attaches the
 * socket and SNI.  The first VSSL_Write runs SSL_write, which completes
 * the handshake.  The verify callback always returns success so a bad
 * certificate still yields a peer cert; VSSL_GetVerifyResult then
 * returns the OpenSSL verify code and Voyager prompts when it is >= 2.
 *
 * SocketBase arrives later via VSSL_SetTCPMode.  InitAmiSSL is deferred
 * until the first OpenSSL call in a task, and repeated for each task:
 * the nethandler process and the prefs process do not share AmiSSL
 * task state.  The nethandler inits with SocketBase; certificate PEM
 * reads init without it.
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <proto/bsdsocket.h>

#include <string.h>
#include <stdio.h>

#include <proto/amisslmaster.h>
#include <libraries/amisslmaster.h>
#include <proto/amissl.h>

#include "rev.h"

struct VSSL_CacheInfo {
	int sess_number;
	int sess_connect;
	int sess_connect_good;
	int sess_accept;
	int sess_accept_good;
	int sess_hits;
	int sess_cb_hits;
	int sess_misses;
	int sess_timeouts;
};

/* SAS/C: __reg(a0, T x) -> register __a0 T x (same as Voyager macros/compilers.h). */
#define __reg(x,y) register __ ## x y

/* Bits Voyager stores (Source/vssl.h, OpenSSL 0.9). */
#define VSSL_OP_ALL      0x000FFFFFUL
#define VSSL_OP_NO_SSLV3 0x02000000UL
#define VSSL_OP_NO_TLSV1 0x04000000UL

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef EINTR
#define EINTR 4
#endif
#ifndef EAGAIN
#define EAGAIN 35
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif
#ifndef EINPROGRESS
#define EINPROGRESS 36
#endif

#define VSSL_IO_TRIES 200

extern int errno;

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct Library *UtilityBase;

#define VSSL_TASKS 4

struct vssl_taskinit {
	struct Task *task;
	int has_socket;
};

static ULONG ssl_options = VSSL_OP_ALL;
static int amissl_opened;
static struct vssl_taskinit amissl_tasks[VSSL_TASKS];
static int amissl_ntasks;

/*
 * OpenSSL calls this with stack arguments.  SCOPTIONS uses PARAMETERS=REGISTERS,
 * so the callback is forced to the standard stack convention.
 */
static int __stdargs
vssl_verify_cb(int preverify_ok, X509_STORE_CTX *store)
{
	(void)preverify_ok;
	(void)store;
	return 1;
}

static int
vssl_file_exists(STRPTR path)
{
	struct Process *proc;
	APTR oldwin;
	BPTR lock;

	if (path == NULL || path[0] == '\0') {
		return FALSE;
	}
	/* A missing assign must not pop a "please insert volume" requester. */
	proc = (struct Process *)FindTask(NULL);
	oldwin = proc->pr_WindowPtr;
	proc->pr_WindowPtr = (APTR)-1;
	lock = Lock(path, ACCESS_READ);
	proc->pr_WindowPtr = oldwin;
	if (lock == (BPTR)0) {
		return FALSE;
	}
	UnLock(lock);
	return TRUE;
}

static struct vssl_taskinit *
vssl_find_task(struct Task *task)
{
	int i;

	for (i = 0; i < amissl_ntasks; i++) {
		if (amissl_tasks[i].task == task) {
			return &amissl_tasks[i];
		}
	}
	return NULL;
}

static int
vssl_amissl_init(void)
{
	struct TagItem tags[4];
	struct vssl_taskinit *slot;
	struct Task *me;
	LONG rc;
	int n;
	int with_sock;

	if (!amissl_opened || AmiSSLBase == NULL) {
		return FALSE;
	}

	me = FindTask(NULL);
	with_sock = (SocketBase != NULL) ? TRUE : FALSE;
	slot = vssl_find_task(me);
	if (slot != NULL) {
		if (slot->has_socket || !with_sock) {
			return TRUE;
		}
		/* This task inited before SetTCPMode provided a socket base. */
		CleanupAmiSSLA(NULL);
		slot->has_socket = FALSE;
	}

	n = 0;
	if (with_sock) {
		tags[n].ti_Tag = AmiSSL_SocketBase;
		tags[n].ti_Data = (ULONG)SocketBase;
		n++;
		tags[n].ti_Tag = AmiSSL_ErrNoPtr;
		tags[n].ti_Data = (ULONG)&errno;
		n++;
	}
	tags[n].ti_Tag = TAG_DONE;
	tags[n].ti_Data = 0;

	rc = InitAmiSSLA(tags);
	if (rc != 0) {
		if (slot != NULL) {
			slot->task = NULL;
		}
		return FALSE;
	}
	if (slot == NULL) {
		if (amissl_ntasks >= VSSL_TASKS) {
			CleanupAmiSSLA(NULL);
			return FALSE;
		}
		slot = &amissl_tasks[amissl_ntasks];
		amissl_ntasks++;
		slot->task = me;
	}
	slot->has_socket = with_sock;
	return TRUE;
}

static int
vssl_current_errno(void)
{
	if (SocketBase != NULL) {
		return Errno();
	}
	return errno;
}

static int
vssl_sock_retryable(void)
{
	int e;

	e = vssl_current_errno();
	if (e == EWOULDBLOCK || e == EAGAIN || e == EINPROGRESS || e == EINTR) {
		return TRUE;
	}
	e = errno;
	if (e == EWOULDBLOCK || e == EAGAIN || e == EINPROGRESS || e == EINTR) {
		return TRUE;
	}
	return FALSE;
}

static int
vssl_wait_sock(int sock, int for_write, ULONG secs)
{
	fd_set fds;
	struct timeval tv;
	LONG r;

	if (SocketBase == NULL || sock < 0) {
		return 0;
	}
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	tv.tv_secs = secs;
	tv.tv_micro = 0;
	if (for_write) {
		r = WaitSelect(sock + 1, NULL, &fds, NULL, &tv, NULL);
	} else {
		r = WaitSelect(sock + 1, &fds, NULL, NULL, &tv, NULL);
	}
	if (r > 0) {
		return 1;
	}
	return 0;
}

static int
vssl_wait_sock_any(int sock, ULONG secs)
{
	fd_set rfds;
	fd_set wfds;
	struct timeval tv;
	LONG r;

	if (SocketBase == NULL || sock < 0) {
		return 0;
	}
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(sock, &rfds);
	FD_SET(sock, &wfds);
	tv.tv_secs = secs;
	tv.tv_micro = 0;
	r = WaitSelect(sock + 1, &rfds, &wfds, NULL, &tv, NULL);
	if (r > 0) {
		return 1;
	}
	return 0;
}

static int
vssl_io_retryable(SSL *ssl, int rc)
{
	int err;

	err = SSL_get_error(ssl, rc);
	if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE
		|| err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT) {
		return TRUE;
	}
	if (err == SSL_ERROR_SYSCALL && vssl_sock_retryable()) {
		return TRUE;
	}
	return FALSE;
}

static void
vssl_load_file(SSL_CTX *ctx, const char *path)
{
	if (path == NULL || path[0] == '\0') {
		return;
	}
	if (!vssl_file_exists((STRPTR)path)) {
		return;
	}
	SSL_CTX_load_verify_locations(ctx, path, NULL);
}

static void
vssl_load_dir(SSL_CTX *ctx, STRPTR dir)
{
	char path[256];

	if (dir == NULL || dir[0] == '\0') {
		return;
	}
	SSL_CTX_load_verify_locations(ctx, NULL, (const char *)dir);
	sprintf(path, "%s/cacert.pem", (char *)dir);
	vssl_load_file(ctx, path);
}

static void
vssl_load_default_cas(SSL_CTX *ctx)
{
	BPTR plock;
	char dir[192];
	char path[256];

	vssl_load_file(ctx, "PROGDIR:Certificates/cacert.pem");
	/* The net process may not have PROGDIR:. Same file via the program lock. */
	plock = GetProgramDir();
	dir[0] = '\0';
	if (plock != (BPTR)0) {
		plock = DupLock(plock);
	}
	if (plock != (BPTR)0) {
		NameFromLock(plock, dir, (LONG)sizeof(dir));
		UnLock(plock);
	}
	if (dir[0] != '\0') {
		sprintf(path, "%s/Certificates/cacert.pem", dir);
		vssl_load_file(ctx, path);
	}
}

/*
 * Map Voyager's OpenSSL 0.9 flags onto the OpenSSL 3 flags that still
 * mean the same thing.  SSLv2 is already gone (SSL_OP_NO_SSLv2 is 0).
 */
static unsigned long
vssl_map_options(ULONG in)
{
	unsigned long out;

	out = 0;
	if (in & VSSL_OP_ALL) {
		out |= SSL_OP_ALL;
	}
	if (in & VSSL_OP_NO_SSLV3) {
		out |= SSL_OP_NO_SSLv3;
	}
	if (in & VSSL_OP_NO_TLSV1) {
		out |= SSL_OP_NO_TLSv1;
	}
	return out;
}

/*
 * SSL_CTX_set_options is a 64-bit LVO.  The SAS/C pragma only names D0,
 * which is the high half.  This libcall passes the low half in D1 as well.
 * Mask: arg count 3, A0 at nibble 2, D0 at nibble 3, D1 at nibble 4.
 */
ULONG vssl_set_options(SSL_CTX *ctx, ULONG high, ULONG low);
#pragma libcall AmiSSLBase vssl_set_options 683a 10803

static void
vssl_apply_options(SSL_CTX *ctx, ULONG in)
{
	int ver;
	unsigned long opts;

	if (ctx == NULL) {
		return;
	}
	opts = vssl_map_options(in);
	vssl_set_options(ctx, 0, opts);
	if (in & VSSL_OP_NO_TLSV1) {
		ver = TLS1_2_VERSION;
	} else {
		ver = TLS1_VERSION;
	}
	SSL_CTX_set_min_proto_version(ctx, ver);
}

static int
vssl_time_digits(const unsigned char *v, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (v[i] < '0' || v[i] > '9') {
			return FALSE;
		}
	}
	return TRUE;
}

char * __asm __saveds VSSL_Id(void)
{
	return ("Voyager SSL  " VERTAG " AmiSSL/OpenSSL");
}

long __saveds __asm __UserLibInit(register __a6 struct Library *libbase)
{
	struct TagItem tags[6];
	LONG rc;
	int n;

	SysBase = *((struct ExecBase **)4);

	if (!(SysBase->AttnFlags & AFF_68020)) {
		return (-1);
	}

	DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37);
	if (DOSBase == NULL) {
		return (-1);
	}

	UtilityBase = OpenLibrary("utility.library", 37);
	if (UtilityBase == NULL) {
		CloseLibrary((struct Library *)DOSBase);
		DOSBase = NULL;
		return (-1);
	}

	AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION);
	if (AmiSSLMasterBase == NULL) {
		CloseLibrary(UtilityBase);
		CloseLibrary((struct Library *)DOSBase);
		UtilityBase = NULL;
		DOSBase = NULL;
		return (-1);
	}

	n = 0;
	tags[n].ti_Tag = AmiSSL_UsesOpenSSLStructs;
	tags[n].ti_Data = TRUE;
	n++;
	tags[n].ti_Tag = AmiSSL_InitAmiSSL;
	tags[n].ti_Data = FALSE;
	n++;
	tags[n].ti_Tag = AmiSSL_GetAmiSSLBase;
	tags[n].ti_Data = (ULONG)&AmiSSLBase;
	n++;
	tags[n].ti_Tag = AmiSSL_GetAmiSSLExtBase;
	tags[n].ti_Data = (ULONG)&AmiSSLExtBase;
	n++;
	tags[n].ti_Tag = TAG_DONE;
	tags[n].ti_Data = 0;

	rc = OpenAmiSSLTagList(AMISSL_CURRENT_VERSION, tags);
	if (rc != 0 || AmiSSLBase == NULL) {
		/*
		 * Installed AmiSSL is older than this SDK's current version.
		 * Any OpenSSL 3 AmiSSL is enough; the master opens the newest
		 * build that is compatible with that request.
		 */
		AmiSSLBase = NULL;
		AmiSSLExtBase = NULL;
		rc = OpenAmiSSLTagList(AMISSL_V3xx, tags);
	}
	if (rc != 0 || AmiSSLBase == NULL) {
		CloseLibrary(AmiSSLMasterBase);
		AmiSSLMasterBase = NULL;
		CloseLibrary(UtilityBase);
		CloseLibrary((struct Library *)DOSBase);
		UtilityBase = NULL;
		DOSBase = NULL;
		return (-1);
	}

	amissl_opened = TRUE;
	libbase->lib_Node.ln_Pri = -128;
	return (0);
}

void __saveds __asm __UserLibCleanup(void)
{
	struct vssl_taskinit *slot;

	/*
	 * CleanupAmiSSL drops the calling task's state.  Other tasks that
	 * called InitAmiSSL lose that state when those processes exit; the
	 * last CloseLibrary runs in only one of them.
	 */
	slot = vssl_find_task(FindTask(NULL));
	if (slot != NULL) {
		CleanupAmiSSLA(NULL);
		slot->task = NULL;
		slot->has_socket = FALSE;
	}
	if (amissl_opened && AmiSSLMasterBase != NULL) {
		CloseAmiSSL();
		amissl_opened = FALSE;
	}
	if (AmiSSLMasterBase != NULL) {
		CloseLibrary(AmiSSLMasterBase);
		AmiSSLMasterBase = NULL;
	}
	AmiSSLBase = NULL;
	AmiSSLExtBase = NULL;
	if (UtilityBase != NULL) {
		CloseLibrary(UtilityBase);
		UtilityBase = NULL;
	}
	if (DOSBase != NULL) {
		CloseLibrary((struct Library *)DOSBase);
		DOSBase = NULL;
	}
}

APTR __asm __saveds VSSL_Create_CTX(void)
{
	SSL_CTX *ctx;
	const SSL_METHOD *meth;

	if (!vssl_amissl_init()) {
		return NULL;
	}

	meth = TLS_client_method();
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL) {
		return NULL;
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, (SSL_verify_cb)vssl_verify_cb);
	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_CLIENT);
	SSL_CTX_set_mode(ctx,
		SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	vssl_apply_options(ctx, ssl_options);
	vssl_load_default_cas(ctx);
	return (APTR)ctx;
}

void __asm __saveds VSSL_CTX_Set_Options(__reg(a0, APTR pctx), __reg(d0, ULONG options))
{
	ssl_options = options;
	if (!vssl_amissl_init()) {
		return;
	}
	vssl_apply_options((SSL_CTX *)pctx, options);
}

void __asm __saveds VSSL_Free_CTX(__reg(a0, APTR pctx))
{
	if (pctx == NULL) {
		return;
	}
	SSL_CTX_free((SSL_CTX *)pctx);
}

static APTR
vssl_do_connect(SSL_CTX *ctx, int sock, STRPTR hostname)
{
	SSL *ssl;
	char hostbuf[256];

	if (ctx == NULL || sock < 0) {
		return NULL;
	}
	if (!vssl_amissl_init()) {
		return NULL;
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		return NULL;
	}

	if (SSL_set_fd(ssl, sock) != 1) {
		SSL_free(ssl);
		return NULL;
	}

	SSL_set_connect_state(ssl);

	if (hostname != NULL && hostname[0] != '\0') {
		strncpy(hostbuf, (char *)hostname, sizeof(hostbuf) - 1);
		hostbuf[sizeof(hostbuf) - 1] = '\0';
		SSL_set_tlsext_host_name(ssl, hostbuf);
		SSL_set1_host(ssl, hostbuf);
	}

	return (APTR)ssl;
}

APTR __asm __saveds VSSL_Connect(__reg(a0, APTR pctx), __reg(d0, int sock))
{
	return vssl_do_connect((SSL_CTX *)pctx, sock, (STRPTR)"localhost");
}

APTR __asm __saveds VSSL_ConnectHost(__reg(a0, APTR pctx), __reg(d0, int sock), __reg(a1, STRPTR hostname))
{
	return vssl_do_connect((SSL_CTX *)pctx, sock, hostname);
}

ULONG __asm __saveds VSSL_Pending(__reg(a0, APTR pssl))
{
	int n;

	if (pssl == NULL || !vssl_amissl_init()) {
		return (0);
	}
	n = SSL_pending((SSL *)pssl);
	if (n < 0) {
		return (0);
	}
	return (ULONG)n;
}

void __asm __saveds VSSL_Close(__reg(a0, APTR pssl))
{
	if (pssl == NULL) {
		return;
	}
	SSL_free((SSL *)pssl);
}

char * __asm __saveds VSSL_GetCipher(__reg(a0, APTR pssl))
{
	const char *name;

	if (pssl == NULL || !vssl_amissl_init()) {
		return ("");
	}
	name = SSL_get_cipher((SSL *)pssl);
	if (name == NULL) {
		return ("");
	}
	return (char *)name;
}

int __asm __saveds VSSL_Write(__reg(a0, APTR pssl), __reg(a1, APTR data), __reg(d0, ULONG len))
{
	SSL *ssl;
	int rc;
	int err;
	int tries;
	int fd;

	ssl = (SSL *)pssl;
	if (ssl == NULL || data == NULL || len == 0 || !vssl_amissl_init()) {
		return (-1);
	}

	/*
	 * nethandler is one task for every socket.  Retry WANT_* with a
	 * short select instead of a blocking SSL_write.
	 */
	fd = SSL_get_fd(ssl);
	tries = 0;
	for (;;) {
		tries++;
		if (tries > VSSL_IO_TRIES) {
			return (-1);
		}
		rc = SSL_write(ssl, data, (int)len);
		if (rc > 0) {
			return (rc);
		}
		err = SSL_get_error(ssl, rc);
		if (err == SSL_ERROR_WANT_READ) {
			vssl_wait_sock(fd, FALSE, 1);
			continue;
		}
		if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_CONNECT) {
			vssl_wait_sock(fd, TRUE, 1);
			continue;
		}
		if (err == SSL_ERROR_SYSCALL && vssl_sock_retryable()) {
			vssl_wait_sock_any(fd, 1);
			continue;
		}
		return (-1);
	}
}

int __asm __saveds VSSL_Read(__reg(a0, APTR pssl), __reg(a1, APTR data), __reg(d0, ULONG len))
{
	SSL *ssl;
	int rc;

	ssl = (SSL *)pssl;
	if (ssl == NULL || data == NULL || len == 0 || !vssl_amissl_init()) {
		return (-1);
	}

	/*
	 * nethandler already WaitSelect'd this fd.  WANT_* is "no application
	 * data yet" (return 0).  A clean shutdown or a hard error is -1,
	 * which Voyager treats as end of stream.
	 */
	rc = SSL_read(ssl, data, (int)len);
	if (rc > 0) {
		return (rc);
	}
	if (vssl_io_retryable(ssl, rc)) {
		return (0);
	}
	return (-1);
}

void __asm __saveds VSSL_SetTCPMode(__reg(d0, int mode), __reg(a0, APTR libbase))
{
	(void)mode;
	SocketBase = (struct Library *)libbase;
	vssl_amissl_init();
}

int __asm __saveds VSSL_GetVerifyResult(__reg(a0, APTR pssl), __reg(a1, char **errorp))
{
	long rc;
	const char *es;
	static char fallback[] = "Certificate verify failed";

	if (pssl == NULL || !vssl_amissl_init()) {
		if (errorp != NULL) {
			*errorp = fallback;
		}
		return (2);
	}

	rc = SSL_get_verify_result((SSL *)pssl);
	if (errorp != NULL) {
		es = X509_verify_cert_error_string(rc);
		if (es != NULL) {
			*errorp = (char *)es;
		} else {
			*errorp = fallback;
		}
	}
	return ((int)rc);
}

X509 * __asm __saveds VSSL_GetPeerCertificate(__reg(a0, APTR pssl))
{
	if (pssl == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	return SSL_get_peer_certificate((SSL *)pssl);
}

void __asm __saveds VSSL_X509_Free(__reg(a0, APTR obj))
{
	if (obj == NULL || !vssl_amissl_init()) {
		return;
	}
	X509_free((X509 *)obj);
}

STRPTR __asm __saveds VSSL_X509_NameOneline(__reg(a0, X509_NAME *name))
{
	static char namebuff[256];

	namebuff[0] = '\0';
	if (name == NULL || !vssl_amissl_init()) {
		return ((STRPTR)namebuff);
	}
	if (X509_NAME_oneline(name, namebuff, (int)sizeof(namebuff)) == NULL) {
		namebuff[0] = '\0';
	}
	return ((STRPTR)namebuff);
}

void __asm __saveds VSSL_X509_FreeNameOneline(__reg(a0, STRPTR name))
{
	(void)name;
}

X509_NAME * __asm __saveds VSSL_X509_get_subject_name(__reg(a0, X509 *cert))
{
	if (cert == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	return X509_get_subject_name(cert);
}

X509_NAME * __asm __saveds VSSL_X509_get_issuer_name(__reg(a0, X509 *cert))
{
	if (cert == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	return X509_get_issuer_name(cert);
}

ASN1_UTCTIME * __asm __saveds VSSL_X509_get_notBefore(__reg(a0, X509 *cert))
{
	if (cert == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	return (ASN1_UTCTIME *)X509_get_notBefore(cert);
}

ASN1_UTCTIME * __asm __saveds VSSL_X509_get_notAfter(__reg(a0, X509 *cert))
{
	if (cert == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	return (ASN1_UTCTIME *)X509_get_notAfter(cert);
}

void __asm __saveds VSSL_AddCertDir(__reg(a0, APTR pctx), __reg(a1, STRPTR dir))
{
	if (pctx == NULL || !vssl_amissl_init()) {
		return;
	}
	vssl_load_dir((SSL_CTX *)pctx, dir);
}

int __asm __saveds VSSL_WriteCertPEM(__reg(a0, X509 *cert), __reg(a1, STRPTR outfile))
{
	BIO *out;
	int rc;

	if (cert == NULL || outfile == NULL || !vssl_amissl_init()) {
		return (-1);
	}
	out = BIO_new_file((const char *)outfile, "w");
	if (out == NULL) {
		return (-1);
	}
	rc = PEM_write_bio_X509(out, cert);
	BIO_free(out);
	if (rc != 1) {
		return (-2);
	}
	return (1);
}

X509 * __asm __saveds VSSL_ReadCertPEM(__reg(a0, STRPTR filename))
{
	BIO *bio;
	X509 *cert;

	if (filename == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	bio = BIO_new_file((const char *)filename, "r");
	if (bio == NULL) {
		return NULL;
	}
	cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
	BIO_free(bio);
	return cert;
}

X509 * __asm __saveds VSSL_ReadCertASN1(__reg(a0, STRPTR filename))
{
	BIO *bio;
	X509 *cert;

	if (filename == NULL || !vssl_amissl_init()) {
		return NULL;
	}
	bio = BIO_new_file((const char *)filename, "r");
	if (bio == NULL) {
		return NULL;
	}
	cert = d2i_X509_bio(bio, NULL);
	BIO_free(bio);
	return cert;
}

void __asm __saveds VSSL_ASN1_UTCTIME_sprint(__reg(a0, char *to), __reg(a1, ASN1_UTCTIME *tm))
{
	const unsigned char *v;
	int i;
	int gmt;
	int y;
	int M;
	int d;
	int h;
	int m;
	int s;
	int off;
	int generalized;
	static char *mon[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	if (to == NULL) {
		return;
	}
	if (tm == NULL || !vssl_amissl_init()) {
		strcpy(to, "Bad time value");
		return;
	}

	i = ASN1_STRING_length((const ASN1_STRING *)tm);
	v = ASN1_STRING_get0_data((const ASN1_STRING *)tm);
	generalized = (ASN1_STRING_type((const ASN1_STRING *)tm) == V_ASN1_GENERALIZEDTIME);
	gmt = 0;
	s = 0;

	if (v == NULL) {
		goto err;
	}
	if (generalized) {
		if (i < 14) {
			goto err;
		}
		if (!vssl_time_digits(v, 12)) {
			goto err;
		}
		y = (v[0] - '0') * 1000 + (v[1] - '0') * 100
			+ (v[2] - '0') * 10 + (v[3] - '0');
		off = 4;
	} else {
		if (i < 10) {
			goto err;
		}
		if (!vssl_time_digits(v, 10)) {
			goto err;
		}
		y = (v[0] - '0') * 10 + (v[1] - '0');
		if (y < 70) {
			y += 100;
		}
		y += 1900;
		off = 2;
	}
	if (v[i - 1] == 'Z') {
		gmt = 1;
	}
	M = (v[off] - '0') * 10 + (v[off + 1] - '0');
	if (M > 12 || M < 1) {
		goto err;
	}
	d = (v[off + 2] - '0') * 10 + (v[off + 3] - '0');
	h = (v[off + 4] - '0') * 10 + (v[off + 5] - '0');
	m = (v[off + 6] - '0') * 10 + (v[off + 7] - '0');
	if (i >= off + 10
		&& (v[off + 8] >= '0') && (v[off + 8] <= '9')
		&& (v[off + 9] >= '0') && (v[off + 9] <= '9')) {
		s = (v[off + 8] - '0') * 10 + (v[off + 9] - '0');
	}

	sprintf(to, "%s %2d %02d:%02d:%02d %d%s",
		mon[M - 1], d, h, m, s, y, gmt ? " GMT" : "");
	return;
err:
	strcpy(to, "Bad time value");
}

ULONG __asm __saveds VSSL_X509_NameHash(__reg(a0, X509 *cert))
{
	if (cert == NULL || !vssl_amissl_init()) {
		return 0;
	}
	return (ULONG)X509_subject_name_hash(cert);
}

int __asm __saveds VSSL_X509_HaveSubjectCert(__reg(a0, APTR pctx), __reg(a1, X509 *cert))
{
	(void)pctx;
	(void)cert;
	return (0);
}

int __asm __saveds VSSL_X509_fingerprint(__reg(a0, X509 *cert), __reg(a1, UBYTE *md5))
{
	unsigned int n;

	n = 0;
	if (cert == NULL || md5 == NULL || !vssl_amissl_init()) {
		return (0);
	}
	if (!X509_digest(cert, EVP_md5(), md5, &n)) {
		return (0);
	}
	return ((int)n);
}

void __asm __saveds VSSL_X509_serialnumber(__reg(a0, X509 *cert), __reg(a1, UBYTE *to))
{
	ASN1_INTEGER *a;
	const unsigned char *p;
	int len;
	int i;
	int n;
	static char *h = "0123456789ABCDEF";

	if (to == NULL) {
		return;
	}
	a = NULL;
	if (cert != NULL && vssl_amissl_init()) {
		a = X509_get_serialNumber(cert);
	}
	if (a == NULL) {
		strcpy((char *)to, "00");
		return;
	}
	len = ASN1_STRING_length((const ASN1_STRING *)a);
	p = ASN1_STRING_get0_data((const ASN1_STRING *)a);
	if (len <= 0 || p == NULL) {
		strcpy((char *)to, "00");
		return;
	}
	/* cert_getinfo() passes a 128-byte buffer.  32 octets is 64 hex digits. */
	if (len > 32) {
		len = 32;
	}
	n = 0;
	for (i = 0; i < len; i++) {
		to[n++] = (UBYTE)h[(p[i] >> 4) & 0x0f];
		to[n++] = (UBYTE)h[p[i] & 0x0f];
	}
	to[n] = 0;
}

void __asm __saveds VSSL_SetRandSeed(__reg(a0, APTR buff), __reg(d0, int len))
{
	if (buff == NULL || len <= 0 || !vssl_amissl_init()) {
		return;
	}
	RAND_seed(buff, len);
}

void __asm __saveds VSSL_GetStats(__reg(a0, APTR pctx), __reg(a1, struct VSSL_CacheInfo *ci))
{
	SSL_CTX *ctx;

	if (ci == NULL) {
		return;
	}
	memset(ci, 0, sizeof(*ci));
	ctx = (SSL_CTX *)pctx;
	if (ctx == NULL || !vssl_amissl_init()) {
		return;
	}
	ci->sess_number = (int)SSL_CTX_sess_number(ctx);
	ci->sess_connect = (int)SSL_CTX_sess_connect(ctx);
	ci->sess_connect_good = (int)SSL_CTX_sess_connect_good(ctx);
	ci->sess_accept = (int)SSL_CTX_sess_accept(ctx);
	ci->sess_accept_good = (int)SSL_CTX_sess_accept_good(ctx);
	ci->sess_hits = (int)SSL_CTX_sess_hits(ctx);
	ci->sess_cb_hits = (int)SSL_CTX_sess_cb_hits(ctx);
	ci->sess_misses = (int)SSL_CTX_sess_misses(ctx);
	ci->sess_timeouts = (int)SSL_CTX_sess_timeouts(ctx);
}

STRPTR __asm __saveds VSSL_GetVersion(__reg(a0, APTR pssl))
{
	const char *ver;

	if (pssl == NULL || !vssl_amissl_init()) {
		return ((STRPTR)"");
	}
	ver = SSL_get_version((SSL *)pssl);
	if (ver == NULL) {
		return ((STRPTR)"");
	}
	return ((STRPTR)ver);
}

void __asm __saveds VSSL_SetDefaultOptions(__reg(d0, int options))
{
	ssl_options = (ULONG)options;
}
