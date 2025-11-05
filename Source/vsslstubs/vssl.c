
#ifndef MBX
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/execbase.h>
#include <cl/lists.h>
#else /* MBX */
#include "mbx.h"
#include <modules/inspiration/screens.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifndef MBX
#include <libraries/vat.h>
#endif

#include "openssl/buffer.h"
#include "openssl/ssl.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/rand.h"

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

int ssl_options = SSL_OP_ALL | SSL_OP_NO_TLSv1;

#ifndef MBX

struct ExecBase *SysBase;
struct Library *UtilityBase;
struct Library *VATBase;
//struct Library *SocketBase;

#else

#define RPSZ 1024
unsigned char randpool[RPSZ];
unsigned char *randwriteptr;
unsigned char *randreadptr;
void vssl_screenadd(void);
#endif /* MBX */

static struct MinList sesslist;

//#define BIO_s_file BIO_s_file_internal_w16
//BIO_METHOD *BIO_s_file_internal_w16( void );

int GetCurrentTask( void )
{
	return( (int)FindTask( NULL ) );
}

#ifndef MBX
struct Task *getpid( void )
{
	return( FindTask( NULL ) );
}
#else
/*pid_t getpid( void )
{
	return (pid_t) FindProcess(NULL);
}*/
#endif

int getuid( void )
{
	return( 0 );
}

int des_read_pw_string(char *buf,int length,const char *prompt,int verify)
{
	return( 0 );
}

void abort( void )
{

}

char *getenv( const char *dummy )
{
	return( "" );;
}

// RAND -> VAT wrapper

void RAND_cleanup(void )
{
	// Dummy
}

int RAND_bytes(unsigned char *buf,int num)
{
#ifndef MBX
	while( num-- > 0 )
		*buf++ = VAT_RandomByte();
#else
	while( num-- > 0 )
	{
		*buf++ = *randreadptr;
		if( ++randreadptr == randpool+RPSZ )
			randreadptr = randpool;
		if( randreadptr == randwriteptr )
			vssl_screenadd();
	}
#endif
	return 1;
}

int RAND_pseudo_bytes(unsigned char *buf, int num)
{
	while( num-- )
		*buf++ = rand()%255;
	return 1;
}

void RAND_seed(const void *buf,int num)
{
#ifndef MBX
	VAT_RandomStir();
#endif /* MBX */
}

void RAND_add(const void *buf,int num,double entropy)
{
}

int __stat(const char *fn, struct __stat *dummy)
{
#ifndef MBX
	BPTR l = Lock( fn, SHARED_LOCK );
#else
	DOSHandle_p l = LockHandle( (STRPTR)fn, OM_SHLOCK );
#endif
	if( l )
	{
#ifndef MBX
		UnLock( l );
#else
		UnLockHandle( l );
#endif
		return( 0 );
	}
	return( -1 );
}

//
// Libstuff
//

char * ASM VSSL_Id( void )
{
	return( "Voyager SSL  " VERTAG " adapted by Oliver Wagner <owagner@vapor.com>" );
}

#ifndef MBX
long SAVEDS ASM __UserLibInit(register __a6 struct Library *libbase)
{
	SysBase=*((struct ExecBase**)4);

	if( !( SysBase->AttnFlags & AFF_68020 ) )
		return( -1 );

	DOSBase=(struct DosLibrary*)OpenLibrary( "dos.library", 37 );
	if( !DOSBase )
		return( -1 );

	UtilityBase = OpenLibrary( "utility.library", 37 );

	VATBase = OpenLibrary( "vapor_toolkit.library", VAT_VERSION );
	if( !VATBase )
		return( -1 );

	/*SocketBase = OldOpenLibrary( "bsdsocket.library" );
	if( !SocketBase )
		return( -1 );*/

	libbase->lib_Node.ln_Pri = -128;

	NEWLIST( &sesslist );

	return( 0 );
}
#else
void vssl_screenadd(void)
{
	Screen_p    screen;
	RastPort_p  srp;
	BitMap_p    sbm;
	DWORD       w,h;
	UBYTE      *bmp;
	UDWORD      lop,mdl;
	UBYTE       md[16];

	screen = GetActiveScreen();
	srp = screen->sc_RastPort;
	sbm = (BitMap_p)GetRastPortAttr( srp, RPTAG_BITMAP );
        w = GetBitMapAttrs( sbm, BMAV_Width );
	h = GetBitMapAttrs( sbm, BMAV_Height );
	bmp = AllocVec( 3*w*4, 0 );
	for( lop=0; lop<h; lop+=4 )
	{
		ReadPixelArrayBM( bmp, 0, 0, w*3, sbm, 0, lop, w, 4, PIXFMT_RGB888 );
		MD5( bmp, w*3*4, md );
		for( mdl=0; mdl!=16; mdl++ )
		{
			*randwriteptr++ ^= md[mdl];

			if (randwriteptr==randpool+RPSZ)
				randwriteptr=randpool;

			if (randwriteptr==randreadptr)
				break;
		}
		if (mdl!=16)
			break;
	}
	FreeVec( bmp );
}

long init_vssl(void)
{
	NEWLIST( &sesslist );
	randreadptr = randwriteptr = randpool;
	vssl_screenadd();

	return( 0 );
}
#endif

#ifndef MBX
void SAVEDS ASM __UserLibCleanup(void)
{
	CloseLibrary( DOSBase );
	CloseLibrary( UtilityBase );
	CloseLibrary( VATBase );
	//CloseLibrary( SocketBase );
}
#else /* MBX */
void close_vssl(void)
{
}
#define __reg(x,y) y
#endif

APTR ASM SAVEDS VSSL_Create_CTX( void )
{
	APTR ctx;
	APTR meth;
	int rc;

	SSL_library_init();
	SSL_load_error_strings();

	//meth = SSLv2_client_method();
	meth = SSLv23_client_method();
	ctx = SSL_CTX_new( meth );

	if( ctx )
	{
	 	SSL_CTX_load_verify_locations( ctx, NULL, "PROGDIR:Certificates" );
		SSL_CTX_set_session_cache_mode( ctx, SSL_SESS_CACHE_CLIENT );
		SSL_CTX_set_options( ctx, ssl_options );
		SSL_CTX_set_cipher_list( ctx, "ALL:!ADH:!EDH:!DH:!SHA:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP" );
	}

	return( ctx );
}

void ASM SAVEDS VSSL_CTX_Set_Options( __reg( a0, APTR ctx ), __reg( d0, ULONG options ) )
{
	SSL_CTX_set_options( ctx, options );
}

void ASM SAVEDS VSSL_Free_CTX( __reg( a0, APTR ctx ) )
{
	if( ctx )
		SSL_CTX_free( ctx );
}

/*static void mycb( int a1, int a2, int a3 )
{
	kprintf( "cb: %lx %ld %ld\n", a1, a2, a3 );
}*/

extern ULONG tcp_peername( int s );

struct sessinfo {
	struct MinNode n;
	ULONG ip;
	SSL_SESSION *session;
};

static struct sessinfo *si_get( int sock )
{
	ULONG ip = tcp_peername( sock );
	struct sessinfo *si;

	for( si = FIRSTNODE( &sesslist ); NEXTNODE( si ); si = NEXTNODE( si ) )
	{
		if( si->ip == ip )
			return( si );
	}
	return( NULL );
}

APTR ASM SAVEDS VSSL_Connect( __reg( a0, APTR ctx ), __reg( d0, int sock ) )
{
	SSL *ssl = SSL_new( ctx );
	struct sessinfo *si = si_get( sock );

	if( ssl )
	{
		int try;
		SSL_SESSION *s;

		if( si )
		{
			si->session->not_resumable = FALSE;
			SSL_set_session( ssl, si->session );
		}

		//SSL_set_connect_state( ssl );
		SSL_set_fd( ssl, sock );

retry:
		if( SSL_connect( ssl ) == -1 )
		{
			if( SSL_want_read( ssl ) )
				goto retry;
			//kprintf( "rwstate = %ld\n", SSL_want( ssl ) );
			SSL_free( ssl );
			ssl = NULL;
		}
		else
		{
			if( si )
			{
				REMOVE( si );
				SSL_SESSION_free( si->session );
				free( si );
			}
			si = malloc( sizeof( *si ) );
			si->session = SSL_get_session( ssl );
			//kprintf( "session: %lx, ver %ld\n", si->session, *((int*)si->session) );
			if( si->session )
			{
				si->ip = tcp_peername( sock );
				si->session->references++;
				ADDHEAD( &sesslist, si );
			}
			else
			{
				free( si );
			}
		}

#if 0
		{
			int c;

			s = SSL_get_session( ssl );
			kprintf( "connect ok, session id =" );

			for( c = 0; c < s->session_id_length; c++ )
				kprintf( " %02lx", s->session_id[ c ] );

			kprintf( "\n" );
		}
#endif
	}
	return( ssl );
}

void ASM SAVEDS VSSL_Close( __reg( a0, APTR ssl ) )
{
	SSL_free( ssl );
}

char * ASM SAVEDS VSSL_GetCipher( __reg( a0, APTR ssl ) )
{
	return( (char*)SSL_get_cipher( ssl ) );
}

int ASM SAVEDS VSSL_Write( __reg( a0, APTR ssl ), __reg( a1, APTR data ), __reg( d0, ULONG len ) )
{
	int rc;
	//kprintf( "writing %ld bytes\n", len );
	rc = SSL_write( ssl, data, len );
	//kprintf( "ssl_write(%ld) returned %ld, error %ld, want %ld\n", len, rc, SSL_get_error( ssl, rc ), SSL_want( ssl ) );
	return( rc );
}

int ASM SAVEDS VSSL_Read( __reg( a0, APTR ssl ), __reg( a1, APTR data ), __reg( d0, ULONG len ) )
{
	int rc;
	rc = SSL_read( ssl, data, len );
	//kprintf( "ssl_read(%ld) returned %ld, lasterror %ld, want %ld\n", len, rc, SSL_get_error( ssl, rc ), SSL_want( ssl ) );
	return( rc );
}

int ASM SAVEDS VSSL_GetVerifyResult( __reg( a0, SSL *ssl ), __reg( a1, char **error ) )
{
	int rc = SSL_get_verify_result( ssl );
	*error = (char*)X509_verify_cert_error_string( rc );
	return( rc );
}

X509 * ASM SAVEDS VSSL_GetPeerCertificate( __reg( a0, SSL *ssl ) )
{
	return( SSL_get_peer_certificate( ssl ) );
}

void ASM SAVEDS VSSL_X509_Free( __reg( a0, APTR obj ) )
{
	X509_free( obj );
}

STRPTR ASM SAVEDS VSSL_X509_NameOneline( __reg( a0, X509_NAME *name ) )
{
	static char namebuff[ 256 ];

	return( X509_NAME_oneline( name, namebuff, sizeof( namebuff ) ) );
}

void ASM SAVEDS VSSL_X509_FreeNameOneline( __reg( a0, STRPTR name ) )
{
//	free( name );
}

X509_NAME * ASM SAVEDS VSSL_X509_get_subject_name( __reg( a0, X509 *cert ) )
{
	return( X509_get_subject_name( cert ) );
}

X509_NAME * ASM SAVEDS VSSL_X509_get_issuer_name( __reg( __a0, X509 *cert ) )
{
	return( X509_get_issuer_name( cert ) );
}

ASN1_UTCTIME * ASM SAVEDS VSSL_X509_get_notBefore( __reg( __a0, X509 *cert ) )
{
	return( X509_get_notBefore( cert ) );
}

ASN1_UTCTIME * ASM SAVEDS VSSL_X509_get_notAfter( __reg( __a0, X509 *cert ) )
{
	return( X509_get_notAfter( cert ) );
}

void ASM SAVEDS VSSL_AddCertDir( __reg( a0, SSL_CTX *ctx ), __reg( a1, STRPTR dir ) )
{
	//X509_add_cert_dir( ctx->cert, dir, X509_FILETYPE_PEM );
}

int ASM SAVEDS VSSL_WriteCertPEM( __reg( a0, X509 *cert ), __reg( a1, STRPTR outfile ) )
{
	BIO *out = NULL;
	int rc;

	out = BIO_new( BIO_s_file() );
	if( !out )
		return( -1 );

	if( BIO_write_filename( out, outfile ) <= 0 )
	{
		BIO_free( out );
		return( -2 );
	}

	rc = PEM_write_bio_X509( out, cert );
	BIO_free( out );

	return( rc );
}

X509 * ASM SAVEDS VSSL_ReadCertPEM( __reg( a0, STRPTR filename ) )
{
	BIO *bio = BIO_new( BIO_s_file() );
	X509 *cert = NULL;

	if( bio )
	{
		if( BIO_read_filename( bio, filename ) > 0 )
		{
			cert = PEM_read_bio_X509( bio, NULL, NULL, NULL );
		}
		BIO_free( bio );
	}
	return( cert );
}

X509 * ASM SAVEDS VSSL_ReadCertASN1( __reg( a0, STRPTR filename ) )
{
	BIO *bio = BIO_new( BIO_s_file() );
	X509 *cert = NULL;

	if( bio )
	{
		if( BIO_read_filename( bio, filename ) > 0 )
		{
			cert = d2i_X509_bio( bio, NULL );
		}
		BIO_free( bio );
	}
	return( cert );
}

void ASM SAVEDS VSSL_ASN1_UTCTIME_sprint( __reg( a0, char *to ), __reg( a1, ASN1_UTCTIME *tm ) )
{
	char *v;
	int gmt=0;
	static char *mon[12]={
		"Jan","Feb","Mar","Apr","May","Jun",
		"Jul","Aug","Sep","Oct","Nov","Dec"};
	int i;
	int y=0,M=0,d=0,h=0,m=0,s=0;

	i=tm->length;
	v=(char *)tm->data;

	if (i < 10) goto err;
	if (v[i-1] == 'Z') gmt=1;
	for (i=0; i<10; i++)
		if ((v[i] > '9') || (v[i] < '0')) goto err;
	y= (v[0]-'0')*10+(v[1]-'0');
	if (y < 70) y+=100;
	M= (v[2]-'0')*10+(v[3]-'0');
	if ((M > 12) || (M < 1)) goto err;
	d= (v[4]-'0')*10+(v[5]-'0');
	h= (v[6]-'0')*10+(v[7]-'0');
	m=  (v[8]-'0')*10+(v[9]-'0');
	if (	(v[10] >= '0') && (v[10] <= '9') &&
		(v[11] >= '0') && (v[11] <= '9'))
		s=  (v[10]-'0')*10+(v[11]-'0');

	sprintf(to,"%s %2d %02d:%02d:%02d %d%s",
		mon[M-1],d,h,m,s,y+1900,(gmt)?" GMT":"");
		return;
err:
	strcpy(to,"Bad time value");
	return;
}

ULONG ASM SAVEDS VSSL_X509_NameHash( __reg( a0, X509 *cert ) )
{
	return( X509_subject_name_hash( cert ) );
}

int ASM SAVEDS VSSL_X509_HaveSubjectCert( __reg( a0, SSL_CTX *ctx ), __reg( a1, X509 *cert ) )
{
#if 0
	X509 *xi;
	int rc = FALSE;

	xi=SSL_get_certificate(ctx->cert,X509_get_subject_name(cert),NULL);
	if( xi )
	{
		if((!X509_name_cmp(cert->cert_info->subject,xi->cert_info->subject)))
			rc = TRUE;
		X509_free( xi );
	}
	return( rc );
#endif
	return( 0 );
}

int ASM SAVEDS VSSL_X509_fingerprint( __reg( a0, X509 *cert ), __reg( a1, UBYTE *md5 ) )
{
	unsigned int n = 0;

	if(!X509_digest(cert,EVP_md5(),md5,&n))
		return( 0 );
	else
		return( (int)n );
}

static void my_ASN1_INTEGER( ASN1_INTEGER *a, char *to )
{
	int i;
	static char *h="0123456789ABCDEF";

	if (a->length == 0)
	{
		strcpy( to, "00" );
	}
	else
	{
		for (i=0; i<a->length; i++)
		{
			if ((i != 0) && (i%35 == 0))
			{
				strcpy( to, "\\\n" );
				to += 2;
			}
			*to++=h[((unsigned char)a->data[i]>>4)&0x0f];
			*to++=h[((unsigned char)a->data[i]   )&0x0f];
		}
		*to = 0;
	}
}

void ASM SAVEDS VSSL_X509_serialnumber( __reg( a0, X509 *cert ), __reg( a1, UBYTE *to ) )
{
	my_ASN1_INTEGER( cert->cert_info->serialNumber, to );
}

void ASM SAVEDS VSSL_SetRandSeed( __reg( a0, APTR buff ), __reg( d0, int len ) )
{
	RAND_seed( buff, len );
}


void ASM SAVEDS VSSL_GetStats(
	__reg( a0, APTR ctx ),
	__reg( a1, struct VSSL_CacheInfo *ci )
)
{

#define CS(x) ci->x = SSL_CTX_##x( ctx );

	CS( sess_number );
	CS( sess_connect );
	CS( sess_connect_good );
	CS( sess_accept );
	CS( sess_accept_good );
	CS( sess_hits );
	CS( sess_cb_hits );
	CS( sess_misses );
	CS( sess_timeouts );

}

STRPTR ASM SAVEDS VSSL_GetVersion( __reg( a0, APTR ssl ) )
{
	return( (STRPTR)SSL_get_version( ssl ) );
}

void ASM SAVEDS VSSL_SetDefaultOptions( __reg( d0, int options ) )
{
	ssl_options = options;
}

#ifndef MBX
void kprintf( void )
{
}
#endif

