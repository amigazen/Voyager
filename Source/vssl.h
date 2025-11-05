/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_VSSL_H
#define VOYAGER_VSSL_H

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

#if ( !defined(__GNUC__) && !defined(__DCC__) )

#pragma libcall VSSLBase VSSL_Id 1e 0
#pragma libcall VSSLBase VSSL_Create_CTX 24 0
#pragma libcall VSSLBase VSSL_Free_CTX 2a 801
#pragma libcall VSSLBase VSSL_Connect 30 0802
#pragma libcall VSSLBase VSSL_Close 36 801
#pragma libcall VSSLBase VSSL_GetCipher 3c 801
#pragma libcall VSSLBase VSSL_Write 42 09803
#pragma libcall VSSLBase VSSL_Read 48 09803
#pragma libcall VSSLBase VSSL_SetTCPMode 4e 8002
/* V2*/
#pragma libcall VSSLBase VSSL_GetPeerCertificate 54 801
#pragma libcall VSSLBase VSSL_AddCertDir 5a 9802
#pragma libcall VSSLBase VSSL_X509_Free 60 801
#pragma libcall VSSLBase VSSL_X509_NameOneline 66 801
#pragma libcall VSSLBase VSSL_X509_FreeNameOneline 6c 801
#pragma libcall VSSLBase VSSL_X509_get_subject_name 72 801
#pragma libcall VSSLBase VSSL_X509_get_issuer_name 78 801
#pragma libcall VSSLBase VSSL_X509_get_notBefore 7e 801
#pragma libcall VSSLBase VSSL_X509_get_notAfter 84 801
#pragma libcall VSSLBase VSSL_GetVerifyResult 8a 9802
#pragma libcall VSSLBase VSSL_WriteCertPEM 90 9802
#pragma libcall VSSLBase VSSL_ReadCertPEM 96 801
#pragma libcall VSSLBase VSSL_ReadCertASN1 9c 801
#pragma libcall VSSLBase VSSL_ASN1_UTCTIME_sprint a2 9802
#pragma libcall VSSLBase VSSL_X509_NameHash a8 801
#pragma libcall VSSLBase VSSL_X509_HaveSubjectCert ae 9802
/* V3*/
#pragma libcall VSSLBase VSSL_X509_fingerprint b4 9802
#pragma libcall VSSLBase VSSL_X509_serialnumber ba 9802
#pragma libcall VSSLBase VSSL_SetRandSeed c0 0802
/* V6*/
#pragma libcall VSSLBase VSSL_GetStats c6 9802
#pragma libcall VSSLBase VSSL_CTX_Set_Options cc 0802
#pragma libcall VSSLBase VSSL_GetVersion d2 801
/* V8*/
#pragma libcall VSSLBase VSSL_SetDefaultOptions d8 001

#endif

#ifdef MBX
#define VSSLBASE void *dummy
#define VSSLVAR  NULL
#else
#define VSSLBASE struct Library *VSSLBase
#define VSSLVAR VSSLBase
#endif

typedef void *X509;
typedef void *ASN1_UTCTIME;
typedef void *X509_NAME;

char *VSSL_Id( void );
APTR VSSL_Create_CTX( void );
void VSSL_Free_CTX( APTR ctx );
APTR VSSL_Connect( APTR ctx, int sock );
void VSSL_Close( APTR ssl );
char *VSSL_GetCipher( APTR ssl );
int VSSL_Write( APTR ssl, APTR buff, int len );
int VSSL_Read( APTR ssl, APTR buff, int len );
void VSSL_SetTCPMode( int mode, APTR );
void VSSL_AddCertDir( APTR ctx, STRPTR dir );
int VSSL_GetVerifyResult( APTR ssl, char **errorp );
X509 *VSSL_GetPeerCertificate( APTR ssl );
void VSSL_X509_Free( X509 *cert );
STRPTR VSSL_X509_NameOneline( X509_NAME *name );
void VSSL_X509_FreeNameOneline( STRPTR name );
X509_NAME * VSSL_X509_get_subject_name( X509 *cert );
X509_NAME * VSSL_X509_get_issuer_name( X509 *cert );
ASN1_UTCTIME * VSSL_X509_get_notBefore( X509 *cert );
ASN1_UTCTIME * VSSL_X509_get_notAfter( X509 *cert );
int VSSL_WriteCertPEM( X509 *cert, STRPTR outfile );
X509 *VSSL_ReadCertPEM( STRPTR filename );
X509 *VSSL_ReadCertASN1( STRPTR filename );
void VSSL_ASN1_UTCTIME_sprint( char *to, ASN1_UTCTIME *tm );
ULONG VSSL_X509_NameHash( X509 *cert );
int VSSL_X509_HaveSubjectCert( APTR ctx, X509 *cert );
int VSSL_X509_fingerprint( X509 *cert, char *to );
void VSSL_X509_serialnumber( X509 *cert, char *to );
void VSSL_SetRandSeed( APTR buff, int len );
void VSSL_GetStats( APTR ctx, struct VSSL_CacheInfo *ci );
void VSSL_CTX_Set_Options( APTR ctx, ULONG options );
STRPTR VSSL_GetVersion( APTR ssl );
void VSSL_SetDefaultOptions( ULONG options );

#define SSL_OP_MICROSOFT_SESS_ID_BUG			0x00000001L
#define SSL_OP_NETSCAPE_CHALLENGE_BUG			0x00000002L
#define SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG		0x00000008L
#define SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG		0x00000010L
#define SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER		0x00000020L
#define SSL_OP_MSIE_SSLV2_RSA_PADDING			0x00000040L
#define SSL_OP_SSLEAY_080_CLIENT_DH_BUG			0x00000080L
#define SSL_OP_TLS_D5_BUG				0x00000100L
#define SSL_OP_TLS_BLOCK_PADDING_BUG			0x00000200L
#define SSL_OP_TLS_ROLLBACK_BUG				0x00000400L

/* If set, only use tmp_dh parameters once */
#define SSL_OP_SINGLE_DH_USE				0x00100000L
/* Set to also use the tmp_rsa key when doing RSA operations. */
#define SSL_OP_EPHEMERAL_RSA				0x00200000L

/* The next flag deliberatly changes the ciphertest, this is a check
 * for the PKCS#1 attack */
#define SSL_OP_PKCS1_CHECK_1				0x08000000L
#define SSL_OP_PKCS1_CHECK_2				0x10000000L
#define SSL_OP_NETSCAPE_CA_DN_BUG			0x20000000L
#define SSL_OP_NON_EXPORT_FIRST 			0x40000000L
#define SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG		0x80000000L
#define SSL_OP_ALL					0x000FFFFFL

#define SSL_OP_NO_SSLv2					0x01000000L
#define SSL_OP_NO_SSLv3					0x02000000L
#define SSL_OP_NO_TLSv1					0x04000000L

#endif /* VOYAGER_VSSL_H */
