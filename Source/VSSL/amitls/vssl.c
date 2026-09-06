/*
 * voyager_ssl.vlib — VSSL ABI implemented on amitls.library (BearSSL).
 *
 * OpenSSL 0.9 remains under Source/VSSL/openssl as a historical tree.
 * This plugin does not link that tree.  An AmiSSL backend can sit beside
 * this one later; both export the same VSSL LVOs.
 *
 * V9 adds VSSL_ConnectHost so SNI gets the URL hostname.  VSSL_Connect
 * still exists and uses "localhost" (wrong SNI for real sites).
 *
 * Handshake matches amihttp (ht_ssl_amitls.c, verified AGet/ATlsTest):
 * TlsAttachSocket only installs BearSSL on a blocking fd.  Do not call
 * TlsHandshake().  The handshake runs inside the first TlsWrite().
 * Do not pass ATTA_NON_BLOCKING or ATTA_EXTERNAL_WAIT.
 * Per-connection TlsContext gets ATSA_CA_BUNDLE_PATH only; verify mode is
 * ATTA_SSL_VERIFY at attach.  TlsTaskAttach before every TlsWrite/TlsRead.
 */ 

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <utility/tagitem.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/amitls.h>

#include <libraries/amitls.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <proto/bsdsocket.h>
#include <string.h>
#include <stdio.h>

#ifndef EINTR
#define EINTR 4
#endif
#ifndef EINPROGRESS
#define EINPROGRESS 36
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK 35
#endif
#ifndef EAGAIN
#define EAGAIN 11
#endif

#include "rev.h"

/* SAS/C: __reg(a0, T x) -> register __a0 T x (same as Voyager macros/compilers.h). */
#define __reg(x,y) register __ ## x y

/* Voyager stores these flags; AmiTLS ignores SSLv2/v3/TLS1 bits. */
#define SSL_OP_ALL					0x000FFFFFUL

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

typedef void *X509;
typedef void *ASN1_UTCTIME;
typedef void *X509_NAME;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define VSSL_CTX_MAGIC  0x56535458UL
#define VSSL_CONN_MAGIC 0x56534C43UL
#define VSSL_CERT_MAGIC 0x56534352UL

#define VSSL_MEM_FLAGS (MEMF_PUBLIC | MEMF_CLEAR)
#define VSSL_IO_TRIES  200
#define VSSL_READ_TO   30UL

struct vssl_ctx {
	ULONG magic;
	struct TlsContext *tls;
	ULONG options;
	ULONG verify;
	char ca_path[256];
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

struct vssl_conn {
	ULONG magic;
	struct vssl_ctx *ctx;
	struct TlsContext *own_ctx;
	struct TlsConnection *tls;
	LONG sock;
	int hs_ok;
	int verify_rc;
	char verify_err[96];
	char cipher[48];
	char version[16];
};

struct vssl_cert {
	ULONG magic;
	int from_file;
	struct TlsPeerCert tpc;
	char oneline[256];
};

extern struct Library *TlsBase;
extern struct Library *SocketBase;

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct Library *UtilityBase;

static ULONG ssl_options = SSL_OP_ALL;
static int vssl_errno;
static int task_attached;

static const char *ca_candidates[] = {
	"PROGDIR:Certificates/cacert.pem",
	"PROGDIR:cacert.pem",
	"AmiTLS:cacert.pem",
	"AmiSSL:Certs/curl-ca-bundle.crt",
	"ENVARC:AmiSSL/Certs/curl-ca-bundle.crt",
	NULL
};

static int
vssl_file_exists(STRPTR path)
{
	BPTR lock;

	if (path == NULL || path[0] == '\0') {
		return FALSE;
	}
	lock = Lock(path, ACCESS_READ);
	if (lock == (BPTR)0) {
		return FALSE;
	}
	UnLock(lock);
	return TRUE;
}

static STRPTR
vssl_find_ca(char *buf, ULONG buflen)
{
	int i;
	STRPTR p;

	i = 0;
	while (ca_candidates[i] != NULL) {
		p = (STRPTR)ca_candidates[i];
		if (vssl_file_exists(p)) {
			strncpy(buf, (char *)p, (size_t)(buflen - 1));
			buf[buflen - 1] = '\0';
			return (STRPTR)buf;
		}
		i++;
	}

	/* Net process may lack PROGDIR:; GetProgramDir() is the executable drawer. */
	{
		BPTR plock;
		char dir[192];
		char path[256];

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
			if (vssl_file_exists((STRPTR)path)) {
				strncpy(buf, path, (size_t)(buflen - 1));
				buf[buflen - 1] = '\0';
				return (STRPTR)buf;
			}
			sprintf(path, "%s/cacert.pem", dir);
			if (vssl_file_exists((STRPTR)path)) {
				strncpy(buf, path, (size_t)(buflen - 1));
				buf[buflen - 1] = '\0';
				return (STRPTR)buf;
			}
		}
	}

	buf[0] = '\0';
	return NULL;
}

static int
vssl_tls_want(LONG rc)
{
	if (rc == ERROR_TLS_WANT_READ || rc == ERROR_TLS_WANT_WRITE) {
		return TRUE;
	}
	return FALSE;
}

static int
vssl_tls_ok(LONG rc)
{
	if (rc > 0 && rc < ERROR_TLS_NOT_IMPLEMENTED) {
		return TRUE;
	}
	return FALSE;
}

static int
vssl_current_errno(void)
{
	if (SocketBase != NULL) {
		return Errno();
	}
	return vssl_errno;
}

static int
vssl_sock_retryable(void)
{
	int e;

	e = vssl_current_errno();
	if (e == EWOULDBLOCK || e == EAGAIN || e == EINPROGRESS || e == EINTR) {
		return TRUE;
	}
	e = vssl_errno;
	if (e == EWOULDBLOCK || e == EAGAIN || e == EINPROGRESS || e == EINTR) {
		return TRUE;
	}
	return FALSE;
}

static int
vssl_wait_sock(LONG sock, int for_write, ULONG secs)
{
	fd_set fds;
	struct timeval tv;
	LONG r;

	if (SocketBase == NULL || sock < 0) {
		return 0;
	}
	FD_ZERO(&fds);
	FD_SET((int)sock, &fds);
	tv.tv_secs = secs;
	tv.tv_micro = 0;
	if (for_write) {
		r = WaitSelect((int)(sock + 1), NULL, &fds, NULL, &tv, NULL);
	} else {
		r = WaitSelect((int)(sock + 1), &fds, NULL, NULL, &tv, NULL);
	}
	if (r > 0) {
		return 1;
	}
	return 0;
}

/*
 * Handshake TlsWrite often needs the peer's record before more app data.
 * Wait for either direction when AmiTLS only reports ERROR_TLS_IO.
 */
static int
vssl_wait_sock_any(LONG sock, ULONG secs)
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
	FD_SET((int)sock, &rfds);
	FD_SET((int)sock, &wfds);
	tv.tv_secs = secs;
	tv.tv_micro = 0;
	r = WaitSelect((int)(sock + 1), &rfds, &wfds, NULL, &tv, NULL);
	if (r > 0) {
		return 1;
	}
	return 0;
}

static int
vssl_io_retryable_err(LONG err)
{
	if (vssl_tls_want(err) || err == ERROR_TLS_READ_TIMEOUT) {
		return TRUE;
	}
	if ((err == ERROR_TLS_IO || err == ERROR_TLS_WRITE_FAILED
		|| err == ERROR_TLS_READ_FAILED || err == ERROR_TLS_HANDSHAKE)
		&& vssl_sock_retryable()) {
		return TRUE;
	}
	return FALSE;
}

static int
vssl_ensure_task(void)
{
	LONG rc;

	if (TlsBase == NULL) {
		return FALSE;
	}
	rc = TlsTaskAttach(SocketBase, (APTR)&vssl_errno);
	if (rc != 0) {
		return FALSE;
	}
	task_attached = TRUE;
	return TRUE;
}

static struct vssl_ctx *
vssl_ctx_ok(APTR p)
{
	struct vssl_ctx *ctx;

	ctx = (struct vssl_ctx *)p;
	if (ctx == NULL || ctx->magic != VSSL_CTX_MAGIC) {
		return NULL;
	}
	return ctx;
}

static struct vssl_conn *
vssl_conn_ok(APTR p)
{
	struct vssl_conn *conn;

	conn = (struct vssl_conn *)p;
	if (conn == NULL || conn->magic != VSSL_CONN_MAGIC) {
		return NULL;
	}
	return conn;
}

static struct vssl_cert *
vssl_cert_ok(APTR p)
{
	struct vssl_cert *cert;

	cert = (struct vssl_cert *)p;
	if (cert == NULL || cert->magic != VSSL_CERT_MAGIC) {
		return NULL;
	}
	return cert;
}

static void
vssl_oneline_from(char *dst, ULONG dstlen, STRPTR src)
{
	ULONG n;

	if (dst == NULL || dstlen < 2) {
		return;
	}
	dst[0] = '\0';
	if (src == NULL || src[0] == '\0') {
		return;
	}
	/* cert_getinfo() splitx509name() expects OpenSSL /TYPE=value paths */
	if (src[0] == '/') {
		strncpy(dst, (char *)src, (size_t)(dstlen - 1));
		dst[dstlen - 1] = '\0';
		return;
	}
	dst[0] = '/';
	n = 1;
	while (*src != '\0' && n < dstlen - 1) {
		if (*src == ',' && src[1] == ' ') {
			dst[n++] = '/';
			src += 2;
			continue;
		}
		dst[n++] = *src++;
	}
	dst[n] = '\0';
}

static ULONG
vssl_hash_str(STRPTR s)
{
	ULONG h;

	h = 5381UL;
	if (s == NULL) {
		return h;
	}
	while (*s != '\0') {
		h = ((h << 5) + h) + (ULONG)(UBYTE)*s;
		s++;
	}
	return h;
}

static APTR
vssl_do_connect(struct vssl_ctx *ctx, int sock, STRPTR hostname)
{
	struct vssl_conn *conn;
	struct TlsContext *own;
	struct TlsConnection *tls;
	struct TagItem tags[3];
	char hostbuf[256];
	LONG rc;
	ULONG nbio;
	ULONG n;

	if (ctx == NULL || sock < 0) {
		return NULL;
	}

	if (hostname == NULL || hostname[0] == '\0') {
		strcpy(hostbuf, "localhost");
	} else {
		strncpy(hostbuf, (char *)hostname, sizeof(hostbuf) - 1);
		hostbuf[sizeof(hostbuf) - 1] = '\0';
	}

	if (!vssl_ensure_task()) {
		return NULL;
	}

	ctx->sess_connect++;

	/* Blocking fd: Voyager ssl_setup already cleared FIONBIO. */
	nbio = 0;
	if (SocketBase != NULL) {
		IoctlSocket(sock, FIONBIO, (char *)&nbio);
	}

	own = NewTlsContextA(NULL);
	if (own == NULL) {
		return NULL;
	}
	if (ctx->ca_path[0] != '\0') {
		n = 0;
		tags[n].ti_Tag = ATSA_CA_BUNDLE_PATH;
		tags[n].ti_Data = (ULONG)ctx->ca_path;
		n++;
		tags[n].ti_Tag = TAG_DONE;
		tags[n].ti_Data = 0;
		SetTlsContextAttrsA(own, tags);
	}

	tls = NewTlsConnection(own);
	if (tls == NULL) {
		DisposeTlsContext(own);
		return NULL;
	}

	n = 0;
	tags[n].ti_Tag = ATTA_SSL_VERIFY;
	tags[n].ti_Data = ctx->verify;
	n++;
	tags[n].ti_Tag = TAG_DONE;
	tags[n].ti_Data = 0;

	rc = TlsAttachSocketA(tls, sock, (STRPTR)hostbuf, tags);
	if (rc != 0) {
		DisposeTlsConnection(tls);
		DisposeTlsContext(own);
		return NULL;
	}

	conn = (struct vssl_conn *)AllocVec(sizeof(*conn), VSSL_MEM_FLAGS);
	if (conn == NULL) {
		TlsShutdown(tls);
		DisposeTlsConnection(tls);
		DisposeTlsContext(own);
		return NULL;
	}
	conn->magic = VSSL_CONN_MAGIC;
	conn->ctx = ctx;
	conn->own_ctx = own;
	conn->tls = tls;
	conn->sock = sock;
	conn->hs_ok = 0;
	conn->verify_rc = 0;
	conn->verify_err[0] = '\0';
	strcpy(conn->cipher, "TLS_ECDHE_AES128_GCM");
	strcpy(conn->version, "TLSv1.2");
	return (APTR)conn;
}

char * __asm VSSL_Id(void)
{
	return ("Voyager SSL  " VERTAG " AmiTLS/BearSSL");
}

long __saveds __asm __UserLibInit(register __a6 struct Library *libbase)
{
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

	TlsBase = OpenLibrary(AMITLSNAME, AMITLSVERSION);
	if (TlsBase == NULL) {
		TlsBase = OpenLibrary("Libs/" AMITLSNAME, AMITLSVERSION);
	}
	if (TlsBase == NULL) {
		CloseLibrary(UtilityBase);
		CloseLibrary((struct Library *)DOSBase);
		UtilityBase = NULL;
		DOSBase = NULL;
		return (-1);
	}
	libbase->lib_Node.ln_Pri = -128;
	return (0);
}

void __saveds __asm __UserLibCleanup(void)
{
	if (task_attached && TlsBase != NULL) {
		TlsTaskDetach();
		task_attached = FALSE;
	}
	if (TlsBase != NULL) {
		CloseLibrary(TlsBase);
		TlsBase = NULL;
	}
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
	struct vssl_ctx *ctx;
	struct TagItem tags[4];
	struct TagItem basetags[4];
	ULONG n;
	STRPTR ca;

	if (TlsBase == NULL) {
		return NULL;
	}

	ctx = (struct vssl_ctx *)AllocVec(sizeof(*ctx), VSSL_MEM_FLAGS);
	if (ctx == NULL) {
		return NULL;
	}
	ctx->magic = VSSL_CTX_MAGIC;
	ctx->options = ssl_options;
	ctx->verify = ATSSL_VERIFY_PEER;

	ca = vssl_find_ca(ctx->ca_path, (ULONG)sizeof(ctx->ca_path));
	if (ca == NULL) {
		ctx->verify = ATSSL_VERIFY_NONE;
	}

	n = 0;
	basetags[n].ti_Tag = ATBT_SSL_VERIFY;
	basetags[n].ti_Data = ctx->verify;
	n++;
	if (ca != NULL) {
		basetags[n].ti_Tag = ATBT_CA_BUNDLE_PATH;
		basetags[n].ti_Data = (ULONG)ca;
		n++;
	}
	basetags[n].ti_Tag = TAG_DONE;
	basetags[n].ti_Data = 0;
	TlsBaseTagsA(basetags);

	/* Session template only.  Per-connection contexts are created in
	 * ConnectHost; do not bake ATSA_SSL_VERIFY into a shared context. */
	n = 0;
	if (ca != NULL) {
		tags[n].ti_Tag = ATSA_CA_BUNDLE_PATH;
		tags[n].ti_Data = (ULONG)ca;
		n++;
	}
	tags[n].ti_Tag = TAG_DONE;
	tags[n].ti_Data = 0;

	ctx->tls = NewTlsContextA(ca != NULL ? tags : NULL);
	if (ctx->tls == NULL) {
		ctx->magic = 0;
		FreeVec(ctx);
		return NULL;
	}
	return (APTR)ctx;
}

void __asm __saveds VSSL_CTX_Set_Options(__reg(a0, APTR pctx), __reg(d0, ULONG options))
{
	struct vssl_ctx *ctx;

	ctx = vssl_ctx_ok(pctx);
	if (ctx != NULL) {
		ctx->options = options;
	}
}

void __asm __saveds VSSL_Free_CTX(__reg(a0, APTR pctx))
{
	struct vssl_ctx *ctx;

	ctx = vssl_ctx_ok(pctx);
	if (ctx == NULL) {
		return;
	}
	if (ctx->tls != NULL) {
		DisposeTlsContext(ctx->tls);
		ctx->tls = NULL;
	}
	ctx->magic = 0;
	FreeVec(ctx);
}

APTR __asm __saveds VSSL_Connect(__reg(a0, APTR pctx), __reg(d0, int sock))
{
	struct vssl_ctx *ctx;

	ctx = vssl_ctx_ok(pctx);
	return vssl_do_connect(ctx, sock, (STRPTR)"localhost");
}

APTR __asm __saveds VSSL_ConnectHost(__reg(a0, APTR pctx), __reg(d0, int sock), __reg(a1, STRPTR hostname))
{
	struct vssl_ctx *ctx;

	ctx = vssl_ctx_ok(pctx);
	return vssl_do_connect(ctx, sock, hostname);
}

ULONG __asm __saveds VSSL_Pending(__reg(a0, APTR pssl))
{
	struct vssl_conn *conn;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL || conn->tls == NULL) {
		return (0);
	}
	if (!vssl_ensure_task()) {
		return (0);
	}
	return TlsPending(conn->tls);
}

void __asm __saveds VSSL_Close(__reg(a0, APTR pssl))
{
	struct vssl_conn *conn;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL) {
		return;
	}
	if (conn->ctx != NULL && conn->ctx->sess_number > 0) {
		conn->ctx->sess_number--;
	}
	if (conn->tls != NULL) {
		if (vssl_ensure_task()) {
			TlsShutdown(conn->tls);
		}
		DisposeTlsConnection(conn->tls);
		conn->tls = NULL;
	}
	if (conn->own_ctx != NULL) {
		DisposeTlsContext(conn->own_ctx);
		conn->own_ctx = NULL;
	}
	conn->magic = 0;
	FreeVec(conn);
}

char * __asm __saveds VSSL_GetCipher(__reg(a0, APTR pssl))
{
	struct vssl_conn *conn;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL) {
		return ("");
	}
	return (conn->cipher);
}

static void
vssl_note_write_rc(struct vssl_conn *conn, LONG rc)
{
	LONG err;
	STRPTR es;

	if (conn == NULL) {
		return;
	}
	if (vssl_tls_ok(rc)) {
		if (!conn->hs_ok) {
			conn->hs_ok = 1;
			if (conn->ctx != NULL) {
				conn->ctx->sess_connect_good++;
				conn->ctx->sess_number++;
			}
			conn->verify_rc = 0;
			conn->verify_err[0] = '\0';
		}
		return;
	}
	err = TlsGetLastError(conn->tls);
	if (err == 0) {
		err = rc;
	}
	if (err == ERROR_TLS_VERIFY || rc == ERROR_TLS_VERIFY) {
		conn->hs_ok = 1;
		conn->verify_rc = 2;
		es = TlsGetErrorString(err);
		if (es != NULL) {
			strncpy(conn->verify_err, (char *)es, sizeof(conn->verify_err) - 1);
		} else {
			strcpy(conn->verify_err, "Certificate verify failed");
		}
	}
}

int __asm __saveds VSSL_Write(__reg(a0, APTR pssl), __reg(a1, APTR data), __reg(d0, ULONG len))
{
	struct vssl_conn *conn;
	LONG rc;
	LONG err;
	int tries;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL || conn->tls == NULL || data == NULL || len == 0) {
		return (-1);
	}
	if (!vssl_ensure_task()) {
		return (-1);
	}
	/*
	 * Blocking TlsWrite runs the deferred handshake.  Do not WaitSelect
	 * for 30s here: Voyager's nethandler is one task for all sockets.
	 */
	tries = 0;
	for (;;) {
		tries++;
		if (tries > VSSL_IO_TRIES) {
			return (-1);
		}
		rc = TlsWrite(conn->tls, data, len);
		if (vssl_tls_ok(rc)) {
			vssl_note_write_rc(conn, rc);
			return ((int)rc);
		}
		err = TlsGetLastError(conn->tls);
		if (err == 0) {
			err = rc;
		}
		vssl_note_write_rc(conn, err);
		if (vssl_io_retryable_err(err)) {
			if (err == ERROR_TLS_WANT_WRITE) {
				vssl_wait_sock(conn->sock, TRUE, 1);
			} else if (err == ERROR_TLS_WANT_READ) {
				vssl_wait_sock(conn->sock, FALSE, 1);
			} else {
				vssl_wait_sock_any(conn->sock, 1);
			}
			continue;
		}
		return (-1);
	}
}

int __asm __saveds VSSL_Read(__reg(a0, APTR pssl), __reg(a1, APTR data), __reg(d0, ULONG len))
{
	struct vssl_conn *conn;
	LONG rc;
	LONG err;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL || conn->tls == NULL || data == NULL || len == 0) {
		return (-1);
	}
	if (!vssl_ensure_task()) {
		return (-1);
	}
	/*
	 * nethandler already WaitSelect'd this fd.  A blocking 30s wait here
	 * stalls every other download in the same process.
	 */
	if (TlsPending(conn->tls) > 0) {
		rc = TlsRead(conn->tls, data, len, 0);
		if (vssl_tls_ok(rc)) {
			return ((int)rc);
		}
	}
	rc = TlsRead(conn->tls, data, len, 0);
	if (vssl_tls_ok(rc)) {
		return ((int)rc);
	}
	if (rc == 0) {
		return (-1);
	}
	err = TlsGetLastError(conn->tls);
	if (err == 0) {
		err = rc;
	}
	/*
	 * AmiTLS returns ERROR_TLS_IO (not WANT_*) when bsdsocket errno is
	 * EWOULDBLOCK/EINPROGRESS.  Voyager treats VSSL_Read -1 as EOF.
	 */
	if (vssl_io_retryable_err(err)) {
		return (0);
	}
	return (-1);
}

void __asm __saveds VSSL_SetTCPMode(__reg(d0, int mode), __reg(a0, APTR libbase))
{
	(void)mode;
	SocketBase = (struct Library *)libbase;
	vssl_ensure_task();
}

int __asm __saveds VSSL_GetVerifyResult(__reg(a0, APTR pssl), __reg(a1, char **errorp))
{
	struct vssl_conn *conn;
	static char fallback[] = "Certificate verify failed";

	conn = vssl_conn_ok(pssl);
	if (conn == NULL) {
		if (errorp != NULL) {
			*errorp = fallback;
		}
		return (2);
	}
	if (errorp != NULL) {
		if (conn->verify_err[0] != '\0') {
			*errorp = conn->verify_err;
		} else {
			*errorp = (char *)"ok";
		}
	}
	return (conn->verify_rc);
}

X509 * __asm __saveds VSSL_GetPeerCertificate(__reg(a0, APTR pssl))
{
	struct vssl_conn *conn;
	struct vssl_cert *cert;
	LONG rc;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL || conn->tls == NULL) {
		return NULL;
	}
	cert = (struct vssl_cert *)AllocVec(sizeof(*cert), VSSL_MEM_FLAGS);
	if (cert == NULL) {
		return NULL;
	}
	cert->magic = VSSL_CERT_MAGIC;
	rc = TlsGetPeerCert(conn->tls, &cert->tpc);
	if (rc != 0) {
		if (cert->tpc.tpc_Subject == NULL && cert->tpc.tpc_CommonName == NULL) {
			if (conn->verify_rc >= 2) {
				strcpy(cert->oneline, "/CN=unknown");
				return ((X509 *)cert);
			}
			cert->magic = 0;
			FreeVec(cert);
			return NULL;
		}
	}
	return ((X509 *)cert);
}

void __asm __saveds VSSL_X509_Free(__reg(a0, APTR obj))
{
	struct vssl_cert *cert;

	cert = vssl_cert_ok(obj);
	if (cert == NULL) {
		return;
	}
	if (!cert->from_file && TlsBase != NULL) {
		TlsPeerCertFree(&cert->tpc);
	}
	cert->magic = 0;
	FreeVec(cert);
}

STRPTR __asm __saveds VSSL_X509_NameOneline(__reg(a0, X509_NAME *name))
{
	static char namebuff[256];

	namebuff[0] = '\0';
	vssl_oneline_from(namebuff, (ULONG)sizeof(namebuff), (STRPTR)name);
	return ((STRPTR)namebuff);
}

void __asm __saveds VSSL_X509_FreeNameOneline(__reg(a0, STRPTR name))
{
	(void)name;
}

X509_NAME * __asm __saveds VSSL_X509_get_subject_name(__reg(a0, X509 *pcert))
{
	struct vssl_cert *cert;

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL) {
		return NULL;
	}
	if (cert->tpc.tpc_Subject != NULL && cert->tpc.tpc_Subject[0] != '\0') {
		return ((X509_NAME *)cert->tpc.tpc_Subject);
	}
	if (cert->tpc.tpc_CommonName != NULL && cert->tpc.tpc_CommonName[0] != '\0') {
		return ((X509_NAME *)cert->tpc.tpc_CommonName);
	}
	if (cert->oneline[0] != '\0') {
		return ((X509_NAME *)cert->oneline);
	}
	return NULL;
}

X509_NAME * __asm __saveds VSSL_X509_get_issuer_name(__reg(a0, X509 *pcert))
{
	struct vssl_cert *cert;

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL) {
		return NULL;
	}
	return ((X509_NAME *)cert->tpc.tpc_Issuer);
}

ASN1_UTCTIME * __asm __saveds VSSL_X509_get_notBefore(__reg(a0, X509 *pcert))
{
	struct vssl_cert *cert;

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL) {
		return NULL;
	}
	return ((ASN1_UTCTIME *)cert->tpc.tpc_NotBefore);
}

ASN1_UTCTIME * __asm __saveds VSSL_X509_get_notAfter(__reg(a0, X509 *pcert))
{
	struct vssl_cert *cert;

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL) {
		return NULL;
	}
	return ((ASN1_UTCTIME *)cert->tpc.tpc_NotAfter);
}

void __asm __saveds VSSL_AddCertDir(__reg(a0, APTR pctx), __reg(a1, STRPTR dir))
{
	char path[256];
	struct vssl_ctx *ctx;
	struct TagItem tags[2];

	ctx = vssl_ctx_ok(pctx);
	if (ctx == NULL || dir == NULL || dir[0] == '\0') {
		return;
	}
	sprintf(path, "%s/cacert.pem", (char *)dir);
	if (!vssl_file_exists((STRPTR)path)) {
		return;
	}
	strncpy(ctx->ca_path, path, sizeof(ctx->ca_path) - 1);
	ctx->ca_path[sizeof(ctx->ca_path) - 1] = '\0';
	ctx->verify = ATSSL_VERIFY_PEER;
	if (ctx->tls != NULL) {
		tags[0].ti_Tag = ATSA_CA_BUNDLE_PATH;
		tags[0].ti_Data = (ULONG)ctx->ca_path;
		tags[1].ti_Tag = TAG_DONE;
		tags[1].ti_Data = 0;
		SetTlsContextAttrsA(ctx->tls, tags);
	}
}

int __asm __saveds VSSL_WriteCertPEM(__reg(a0, X509 *pcert), __reg(a1, STRPTR outfile))
{
	struct vssl_cert *cert;
	BPTR fh;
	char line[320];

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL || outfile == NULL) {
		return (-1);
	}
	fh = Open(outfile, MODE_NEWFILE);
	if (fh == (BPTR)0) {
		return (-2);
	}
	sprintf(line, "Subject: %s\n",
		cert->tpc.tpc_Subject ? (char *)cert->tpc.tpc_Subject : "");
	Write(fh, line, (LONG)strlen(line));
	sprintf(line, "Issuer: %s\n",
		cert->tpc.tpc_Issuer ? (char *)cert->tpc.tpc_Issuer : "");
	Write(fh, line, (LONG)strlen(line));
	sprintf(line, "Serial: %s\n",
		cert->tpc.tpc_Serial ? (char *)cert->tpc.tpc_Serial : "");
	Write(fh, line, (LONG)strlen(line));
	Close(fh);
	return (1);
}

X509 * __asm __saveds VSSL_ReadCertPEM(__reg(a0, STRPTR filename))
{
	struct vssl_cert *cert;

	if (filename == NULL || !vssl_file_exists(filename)) {
		return NULL;
	}
	cert = (struct vssl_cert *)AllocVec(sizeof(*cert), VSSL_MEM_FLAGS);
	if (cert == NULL) {
		return NULL;
	}
	cert->magic = VSSL_CERT_MAGIC;
	cert->from_file = TRUE;
	strncpy(cert->oneline, (char *)filename, sizeof(cert->oneline) - 1);
	return ((X509 *)cert);
}

X509 * __asm __saveds VSSL_ReadCertASN1(__reg(a0, STRPTR filename))
{
	return VSSL_ReadCertPEM(filename);
}

void __asm __saveds VSSL_ASN1_UTCTIME_sprint(__reg(a0, char *to), __reg(a1, ASN1_UTCTIME *tm))
{
	STRPTR s;

	if (to == NULL) {
		return;
	}
	s = (STRPTR)tm;
	if (s == NULL || s[0] == '\0') {
		strcpy(to, "Bad time value");
		return;
	}
	strcpy(to, (char *)s);
}

ULONG __asm __saveds VSSL_X509_NameHash(__reg(a0, X509 *pcert))
{
	struct vssl_cert *cert;
	STRPTR s;

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL) {
		return 0;
	}
	s = cert->tpc.tpc_Subject;
	if (s == NULL) {
		s = cert->tpc.tpc_CommonName;
	}
	return (vssl_hash_str(s));
}

int __asm __saveds VSSL_X509_HaveSubjectCert(__reg(a0, APTR pctx), __reg(a1, X509 *cert))
{
	(void)pctx;
	(void)cert;
	return (0);
}

int __asm __saveds VSSL_X509_fingerprint(__reg(a0, X509 *pcert), __reg(a1, UBYTE *md5))
{
	struct vssl_cert *cert;
	ULONG h;
	int i;

	cert = vssl_cert_ok((APTR)pcert);
	if (cert == NULL || md5 == NULL) {
		return (0);
	}
	h = vssl_hash_str(cert->tpc.tpc_Subject);
	for (i = 0; i < 16; i++) {
		md5[i] = (UBYTE)((h >> ((i & 3) * 8)) ^ (ULONG)(i * 17));
	}
	return (16);
}

void __asm __saveds VSSL_X509_serialnumber(__reg(a0, X509 *pcert), __reg(a1, UBYTE *to))
{
	struct vssl_cert *cert;

	cert = vssl_cert_ok((APTR)pcert);
	if (to == NULL) {
		return;
	}
	if (cert != NULL && cert->tpc.tpc_Serial != NULL && cert->tpc.tpc_Serial[0] != '\0') {
		strcpy((char *)to, (char *)cert->tpc.tpc_Serial);
		return;
	}
	strcpy((char *)to, "00");
}

void __asm __saveds VSSL_SetRandSeed(__reg(a0, APTR buff), __reg(d0, int len))
{
	(void)buff;
	(void)len;
}

void __asm __saveds VSSL_GetStats(__reg(a0, APTR pctx), __reg(a1, struct VSSL_CacheInfo *ci))
{
	struct vssl_ctx *ctx;

	if (ci == NULL) {
		return;
	}
	memset(ci, 0, sizeof(*ci));
	ctx = vssl_ctx_ok(pctx);
	if (ctx == NULL) {
		return;
	}
	ci->sess_number = ctx->sess_number;
	ci->sess_connect = ctx->sess_connect;
	ci->sess_connect_good = ctx->sess_connect_good;
	ci->sess_accept = ctx->sess_accept;
	ci->sess_accept_good = ctx->sess_accept_good;
	ci->sess_hits = ctx->sess_hits;
	ci->sess_cb_hits = ctx->sess_cb_hits;
	ci->sess_misses = ctx->sess_misses;
	ci->sess_timeouts = ctx->sess_timeouts;
}

STRPTR __asm __saveds VSSL_GetVersion(__reg(a0, APTR pssl))
{
	struct vssl_conn *conn;

	conn = vssl_conn_ok(pssl);
	if (conn == NULL) {
		return ((STRPTR)"TLSv1.2");
	}
	return ((STRPTR)conn->version);
}

void __asm __saveds VSSL_SetDefaultOptions(__reg(d0, int options))
{
	ssl_options = (ULONG)options;
}
