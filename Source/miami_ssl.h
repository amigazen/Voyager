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


#include <pragmas/miamissl_pragmas.h>

typedef APTR SSL;
typedef APTR SSL_CTX;

SSL_CTX *SSL_CTX_new(void );
void	SSL_CTX_free(SSL_CTX *);
char *	SSL_get_cipher(SSL *s);
SSL *	SSL_new(SSL_CTX *);
int	SSL_set_fd(SSL *s, int fd);
#define SSL_write(a,b,c)	SSL2_write(a,b,c)
#define SSL_read(a,b,c)		SSL2_read(a,b,c)
#define SSL_connect(a)		SSL2_connect(a)
int	SSL2_write(SSL *s, const char *buf, int len);
int	SSL2_read(SSL *s, char *buf, int len);
int	SSL2_connect(SSL *s);
void	SSL_free(SSL *s);


#if 0
/* ssl/ssl.h */
/* Copyright (C) 1995-1997 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@mincom.oz.au).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@mincom.oz.au)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@mincom.oz.au)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_SSL_H 
#define HEADER_SSL_H 

#ifdef  __cplusplus
extern "C" {
#endif

/* Protocol Version Codes */
#define SSL_CLIENT_VERSION	0x0002
#define SSL_SERVER_VERSION	0x0002

/* SSLeay version number for ASN.1 encoding of the session information */
/* Version 0 - initial version
 * Version 1 - added the optional peer certificate
 */
#define SSL_SESSION_ASN1_VERSION 0x0001

/* Protocol Message Codes */
#define SSL_MT_ERROR			0
#define SSL_MT_CLIENT_HELLO		1
#define SSL_MT_CLIENT_MASTER_KEY	2
#define SSL_MT_CLIENT_FINISHED		3
#define SSL_MT_SERVER_HELLO		4
#define SSL_MT_SERVER_VERIFY		5
#define SSL_MT_SERVER_FINISHED		6
#define SSL_MT_REQUEST_CERTIFICATE	7
#define SSL_MT_CLIENT_CERTIFICATE	8

/* Error Message Codes */
#define SSL_PE_UNDEFINED_ERROR		0x0000
#define SSL_PE_NO_CIPHER		0x0001
#define SSL_PE_NO_CERTIFICATE		0x0002
#define SSL_PE_BAD_CERTIFICATE		0x0004
#define SSL_PE_UNSUPPORTED_CERTIFICATE_TYPE 0x0006

/* Cipher Kind Values */
#define SSL_CK_NULL_WITH_MD5			0x00,0x00,0x00 /* v3 */
#define SSL_CK_RC4_128_WITH_MD5			0x01,0x00,0x80
#define SSL_CK_RC4_128_EXPORT40_WITH_MD5	0x02,0x00,0x80
#define SSL_CK_RC2_128_CBC_WITH_MD5		0x03,0x00,0x80
#define SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5	0x04,0x00,0x80
#define SSL_CK_IDEA_128_CBC_WITH_MD5		0x05,0x00,0x80
#define SSL_CK_DES_64_CBC_WITH_MD5		0x06,0x00,0x40
#define SSL_CK_DES_64_CBC_WITH_SHA		0x06,0x01,0x40 /* v3 */
#define SSL_CK_DES_192_EDE3_CBC_WITH_MD5	0x07,0x00,0xc0
#define SSL_CK_DES_192_EDE3_CBC_WITH_SHA	0x07,0x01,0xc0 /* v3 */

#define SSL_CK_DES_64_CFB64_WITH_MD5_1		0xff,0x08,0x00 /* SSLeay */
#define SSL_CK_NULL				0xff,0x08,0x10 /* SSLeay */

/* text strings for the ciphers */
#define SSL_TXT_NULL_WITH_MD5		SSL2_TXT_NULL_WITH_MD5			
#define SSL_TXT_RC4_128_WITH_MD5	SSL2_TXT_RC4_128_WITH_MD5		
#define SSL_TXT_RC4_128_EXPORT40_WITH_MD5 SSL2_TXT_RC4_128_EXPORT40_WITH_MD5	
#define SSL_TXT_RC2_128_CBC_WITH_MD5	SSL2_TXT_RC2_128_CBC_WITH_MD5		
#define SSL_TXT_RC2_128_CBC_EXPORT40_WITH_MD5 SSL2_TXT_RC2_128_CBC_EXPORT40_WITH_MD5	
#define SSL_TXT_IDEA_128_CBC_WITH_MD5	SSL2_TXT_IDEA_128_CBC_WITH_MD5		
#define SSL_TXT_DES_64_CBC_WITH_MD5	SSL2_TXT_DES_64_CBC_WITH_MD5		
#define SSL_TXT_DES_64_CBC_WITH_SHA	SSL2_TXT_DES_64_CBC_WITH_SHA		
#define SSL_TXT_DES_192_EDE3_CBC_WITH_MD5 SSL2_TXT_DES_192_EDE3_CBC_WITH_MD5	
#define SSL_TXT_DES_192_EDE3_CBC_WITH_SHA SSL2_TXT_DES_192_EDE3_CBC_WITH_SHA	

#define SSL2_TXT_DES_64_CFB64_WITH_MD5_1	"DES-CFB-M1"
#define SSL2_TXT_NULL				"NULL"
#define SSL2_TXT_NULL_WITH_MD5			"NULL-MD5"
#define SSL2_TXT_RC4_128_WITH_MD5		"RC4-MD5"
#define SSL2_TXT_RC4_128_EXPORT40_WITH_MD5	"EXP-RC4-MD5"
#define SSL2_TXT_RC2_128_CBC_WITH_MD5		"RC2-CBC-MD5"
#define SSL2_TXT_RC2_128_CBC_EXPORT40_WITH_MD5	"EXP-RC2-CBC-MD5"
#define SSL2_TXT_IDEA_128_CBC_WITH_MD5		"IDEA-CBC-MD5"
#define SSL2_TXT_DES_64_CBC_WITH_MD5		"DES-CBC-MD5"
#define SSL2_TXT_DES_64_CBC_WITH_SHA		"DES-CBC-SHA"
#define SSL2_TXT_DES_192_EDE3_CBC_WITH_MD5	"DES-CBC3-MD5"
#define SSL2_TXT_DES_192_EDE3_CBC_WITH_SHA	"DES-CBC3-SHA"

#define SSL2_TXT_DES_64_CFB64_WITH_MD5_1	"DES-CFB-M1"
#define SSL2_TXT_NULL				"NULL"

/* Certificate Type Codes */
#define SSL_CT_X509_CERTIFICATE			0x01

/* Authentication Type Code */
#define SSL_AT_MD5_WITH_RSA_ENCRYPTION		0x01

/* Upper/Lower Bounds */
#define SSL_MAX_MASTER_KEY_LENGTH_IN_BITS	256
#define SSL2_MAX_SSL_SESSION_ID_LENGTH_IN_BYTES	16
#define SSL3_MAX_SSL_SESSION_ID_LENGTH_IN_BYTES	32
#define SSL_MAX_SSL_SESSION_ID_LENGTH_IN_BYTES	32
#define SSL_MIN_RSA_MODULUS_LENGTH_IN_BYTES	64
#define SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER	(unsigned int)32767 
#define SSL_MAX_RECORD_LENGTH_3_BYTE_HEADER	16383 /**/

#ifndef HEADER_X509_H
#include "x509.h"
#endif

#ifndef HEADER_SSL_LOCL_H
#define  CERT		char
#define  CIPHER		char
#define  SSL_NUM_CIPHERS	13
#endif

#ifdef HEADER_X509_H
#define SSL_FILETYPE_PEM	X509_FILETYPE_PEM
#define SSL_FILETYPE_ASN1	X509_FILETYPE_ASN1
#else
#define SSL_FILETYPE_PEM	1
#define SSL_FILETYPE_ASN1	2
#endif

typedef struct ssl_st *ssl_crock_st;

/* Lets make this into an ASN.1 type structure as follows
 * SSL_SESSION_ID ::= SEQUENCE {
 *	version 		INTEGER,	-- structure version number
 *	SSLversion 		INTEGER,	-- SSL version number
 *	Cipher 			OCTET_STRING,	-- the 3 byte cipher ID
 *	Session_ID 		OCTET_STRING,	-- the Session ID
 *	Master_key 		OCTET_STRING,	-- the master key
 *	Key_Arg [ 0 ] IMPLICIT	OCTET_STRING,	-- the optional Key argument
 *	Time [ 1 ] EXPLICIT	INTEGER,	-- optional Start Time
 *	Timeout [ 2 ] EXPLICIT	INTEGER,	-- optional Timeout ins seconds
 *	Peer [ 3 ] EXPLICIT	X509,		-- optional Peer Certificate
 *	}
 * Look in ssl/ssl_asn1.c for more details
 * I'm using EXPLICIT tags so I can read the damn things using asn1parse :-).
 */
typedef struct ssl_session_st
	{
	int num_ciphers;
	unsigned int key_arg_length;
	unsigned char *key_arg;
	unsigned int master_key_length;
	unsigned char *master_key;
	/* session_id - valid? */
	unsigned int session_id_length;
	unsigned char session_id[SSL_MAX_SSL_SESSION_ID_LENGTH_IN_BYTES];

	/* The cert is the certificate used to establish this connection */
	struct cert_st *cert;

	/* This is the cert for the other end.  On servers, it will be
	 * the same as cert->x509 */
	X509 *peer;

	int references;
	unsigned long timeout;
	unsigned long time;

	CIPHER *cipher;
	CIPHER *ciphers[SSL_NUM_CIPHERS+1];
	} SSL_SESSION;

typedef struct ssl_ctx_st
	{
	int num_cipher_list;
	char **cipher_list;

	/* This should be called something other than 'cert' to stop
	 * me mixing it up with SSL->cert.  Tim won't let me because
	 * is is externally visable now :-(. */
#ifdef HEADER_X509_H
	CERTIFICATE_CTX *cert;
#else
	char *cert;
#endif
#ifdef HEADER_LHASH_H
	LHASH *sessions;	/* a set of SSL_SESSION's */
#else
	char *sessions;
#endif
	/* This can have one of 2 values, ored together,
	 * SSL_SESS_CACHE_CLIENT,
	 * SSL_SESS_CACHE_SERVER,
	 * Default is SSL_SESSION_CACHE_SERVER, which means only
	 * SSL_accept which cache SSL_SESSIONS. */
	int session_cache_mode;

	/* If timeout is not 0, it is the default timeout value set
	 * when SSL_new() is called.  This has been put in to make
	 * life easier to set things up */
	unsigned long session_timeout;

	/* If this callback is not null, it will be called each
	 * time a session id is added to the cache.  If this function
	 * returns 1, it means that the callback will do a
	 * SSL_SESSION_free() when it has finished using it.  Otherwise,
	 * on 0, it means the callback has finished with it. */

#ifndef NOPROTO
	int (*new_session_cb)(struct ssl_st *ssl,SSL_SESSION *sess);
#else
	int (*new_session_cb)();
#endif

#ifndef NOPROTO
	SSL_SESSION *(*get_session_cb)(struct ssl_st *ssl,
		unsigned char *data,int len,int *copy);
#else
	SSL_SESSION *(*get_session_cb)();
#endif

	int sess_connect;	/* SSL new (expensive) connection - started */
	int sess_connect_good;	/* SSL new (expensive) connection - finished */
	int sess_accept;	/* SSL new (expensive) accept - started */
	int sess_accept_good;	/* SSL new (expensive) accept - finished */
	int sess_miss;		/* session lookup misses  */
	int sess_timeout;	/* session reuse attempt on timeouted session */
	int sess_hit;		/* session reuse actually done */
	int sess_cb_hit;	/* session-id that was not in the cache was
				 * passed back via the callback.  This
				 * indicates that the application is supplying
				 * session-id's from other processes -
				 * spooky :-) */

	int references;

	/* used to do 'reverse' ssl */
	int reverse;

	/* used by client (for 'reverse' ssl) */
	char *cipher;
	unsigned int challenge_length;
	unsigned char *challenge;
	unsigned int master_key_length;
	unsigned char *master_key;
	unsigned int key_arg_length;
	unsigned char *key_arg;
	/* used by server (for 'reverse' ssl) */
	unsigned int conn_id_length;
	unsigned char *conn_id;

	void (*info_callback)();

	/* if defined, these override the X509_cert_verify() calls */
	int (*app_verify_callback)();
	char *app_verify_arg;

	/* default values to use in SSL structures */
	CERT *default_cert;
	int default_read_ahead;
	int default_verify_mode;
	int (*default_verify_callback)();

	/* Default password callback. */
	int (*default_passwd_callback)();

	/* get client cert callback */
	int (*client_cert_cb)(/* SSL *ssl, X509 **x509, EVP_PKEY **pkey */);
	} SSL_CTX;

#define SSL_SESS_CACHE_OFF	0x00
#define SSL_SESS_CACHE_CLIENT	0x01
#define SSL_SESS_CACHE_SERVER	0x02
#define SSL_SESS_CACHE_BOTH	(SSL_SESS_CACHE_CLIENT|SSL_SESS_CACHE_SERVER)
#define SSL_SESS_CACHE_NO_AUTO_CLEAR	0x80


#define SSL_CTX_sessions(ctx)		((ctx)->sessions)
/* You will need to include lhash.h to access the following #define */
#define SSL_CTX_sess_number(ctx)	((ctx)->sessions->num_items)
#define SSL_CTX_sess_connect(ctx)	((ctx)->sess_connect)
#define SSL_CTX_sess_connect_good(ctx)	((ctx)->sess_connect_good)
#define SSL_CTX_sess_accept(ctx)	((ctx)->sess_accept)
#define SSL_CTX_sess_accept_good(ctx)	((ctx)->sess_accept_good)
#define SSL_CTX_sess_hits(ctx)		((ctx)->sess_hit)
#define SSL_CTX_sess_cb_hits(ctx)	((ctx)->sess_cb_hit)
#define SSL_CTX_sess_misses(ctx)	((ctx)->sess_miss)
#define SSL_CTX_sess_timeouts(ctx)	((ctx)->sess_timeout)

#define SSL_CTX_sess_set_new_cb(ctx,cb)	((ctx)->new_session_cb=(cb))
#define SSL_CTX_sess_get_new_cb(ctx)	((ctx)->new_session_cb)
#define SSL_CTX_sess_set_get_cb(ctx,cb)	((ctx)->get_session_cb=(cb))
#define SSL_CTX_sess_get_get_cb(ctx)	((ctx)->get_session_cb)
#define SSL_CTX_set_session_cache_mode(ctx,m)	((ctx)->session_cache_mode=(m))
#define SSL_CTX_get_session_cache_mode(ctx)	((ctx)->session_cache_mode)
#define SSL_CTX_set_timeout(ctx,t)	((ctx)->session_timeout=(t))
#define SSL_CTX_get_timeout(ctx)	((ctx)->session_timeout)

#define SSL_CTX_set_info_callback(ctx,cb)	((ctx)->info_callback=(cb))
#define SSL_CTX_get_info_callback(ctx)		((ctx)->info_callback)
#define SSL_CTX_set_default_read_ahead(ctx,m) (((ctx)->default_read_ahead)=(m))

#define SSL_CTX_set_client_cert_cb(ctx,cb)	((ctx)->client_cert_cb=(cb))
#define SSL_CTX_get_client_cert_cb(ctx)		((ctx)->client_cert_cb)

#define SSL_NOTHING	1
#define SSL_WRITING	2
#define SSL_READING	3
#define SSL_X509_LOOKUP	4

/* These will only be used when doing non-blocking IO */
#define SSL_want(s)		((s)->rwstate)
#define SSL_want_nothing(s)	((s)->rwstate == SSL_NOTHING)
#define SSL_want_read(s)	((s)->rwstate == SSL_READING)
#define SSL_want_write(s)	((s)->rwstate == SSL_WRITING)
#define SSL_want_x509_lookup(s)	((s)->rwstate == SSL_X509_LOOKUP)

typedef struct ssl_st
	{
	/* procol version
	 * 2 for SSLv2
	 * 3 for SSLv3
	 * -3 for SSLv3 but accept SSLv2 */
	int version;

	/* There are 2 BIO's even though they are normally both the
	 * same.  This is so data can be read and written to different
	 * handlers */

#ifdef HEADER_BUFFER_H
	BIO *rbio; /* used by SSL_read */
	BIO *wbio; /* used by SSL_write */
	BIO *bbio; /* used during session-id reuse to concatinate
		    * messages */
#else
	char *rbio; /* used by SSL_read */
	char *wbio; /* used by SSL_write */
	char *bbio;
#endif
	/* This holds a variable that indicates what we were doing
	 * when a 0 or -1 is returned.  This is needed for
	 * non-blocking IO so we know what request needs re-doing when
	 * in SSL_accept or SSL_connect */
	int rwstate;

	int state;	/* where we are */
	struct	{
		unsigned int conn_id_length;
		unsigned int cert_type;	
		unsigned int cert_length;
		unsigned int csl; 

		unsigned int clear;
		unsigned int enc; 

		unsigned char *ccl;

		unsigned int cipher_spec_length;
		unsigned int session_id_length;

		unsigned int clen;
		unsigned int rlen;
		} tmp;

	int rstate;	/* where we are when reading */

	struct	{
		unsigned char *init_buf;/* buffer used during init */
		int init_num;		/* amount read/written */
		int init_off;		/* amount read/written */

		int send;		/* direction of packet */
		int three_byte_header;
		int clear_text;		/* clear text */
		int escape;		/* not used in SSLv2 */

		/* non-blocking io info, used to make sure the same
		 * args were passwd */
		unsigned int wnum;	/* number of bytes sent so far */
		int wpend_tot;
		char *wpend_buf;

		int wpend_off;	/* offset to data to write */
		int wpend_len; 	/* number of bytes passwd to write */
		int wpend_ret; 	/* number of bytes to return to caller */

		int rpend_off;	/* offset to read position */
		int rpend_len;	/* number of bytes to read */

		/* buffer raw data */
		int rbuf_left;
		int rbuf_offs;
		unsigned char *rbuf;
		unsigned char *wbuf;

		/* used internally by ssl_read to talk to read_n */
		unsigned char *packet;
		unsigned int packet_length;

		unsigned char *write_ptr;/* used to point to the start due to
					  * 2/3 byte header. */
		} s2;

	int read_ahead;		/* Read as many input bytes as possible */
	int hit;		/* reusing a previous session */


	unsigned int padding;
	unsigned int rlength; /* passed to ssl_enc */
	int ract_data_length; /* Set when things are encrypted. */
	unsigned int wlength; /* passed to ssl_enc */
	int wact_data_length; /* Set when things are decrypted. */
	unsigned char *ract_data;
	unsigned char *wact_data;
	unsigned char *mac_data;
	unsigned char *pad_data;

	/* crypto */
	/* Should I rework this stuff?  I want to end up with a
	 * list of pointers to ciphers.... */
	int num_cipher_list;
	char **cipher_list;	/* char ** is malloced and the rest is
				 * on malloced block pointed to by
				 * cipher_list[0] */
	char *crypt_state;	/* cryptographic state */

	unsigned char *read_key;
	unsigned char *write_key;

	/* session info */

	CERT *cert;
	/* This can also be in the session once a session is established */
	SSL_SESSION *session;

	/* Stuff specifically to do with this SSL session */
	unsigned int challenge_length;
	unsigned char *challenge;
	unsigned int conn_id_length;
	unsigned char *conn_id;
	unsigned int key_material_length;
	unsigned char *key_material;

	unsigned long read_sequence;
	unsigned long write_sequence;

	/* special stuff */
	int trust_level;	/* not used yet */

	/* The following has been moved into the SSL_SESSION */
	/* X509 *peer; */ 

	/* Used in SSL2 and SSL3 */
	int verify_mode;	/* 0 don't care about verify failure.
				 * 1 fail if verify fails */
	int (*verify_callback)(); /* fail if callback returns 0 */
	void (*info_callback)(); /* optional informational callback */

	int error;		/* error bytes to be written */
	int error_code;		/* actual code */

	SSL_CTX *ctx;
	/* set this flag to 1 and a sleep(1) is put into all SSL_read()
	 * and SSL_write() calls, good for nbio debuging :-) */
	int debug;	

	/* extra application data */
	int verify_result;
	char *app_data;
	} SSL;

#define SSL_set_verify_result(s,arg)	((s)->verify_result=(long)arg)
#define SSL_get_verify_result(s)	((s)->verify_result)
#define SSL_set_app_data(s,arg)		((s)->app_data=(char *)arg)
#define SSL_get_app_data(s)		((s)->app_data)

/* The following are the possible values for ssl->state are are
 * used to indicate where we are upto in the SSL conection establishment.
 * The macros that follow are about the only things you should need to use
 * and even then, only when using non-blocking IO.
 * It can also be useful to work out where you were when the connection
 * failed */

#define SSL_state(a)			((a)->state)
#define SSL_ST_CONNECT			0x1000
#define SSL_ST_ACCEPT			0x2000
#define SSL_ST_INIT			(SSL_ST_CONNECT|SSL_ST_ACCEPT)
#define SSL_ST_BEFORE			0x01
#define SSL_ST_OK			0x03

/* SSL info callback functions */
#define SSL_set_info_callback(ssl,cb)	((ssl)->info_callback=(cb))
#define SSL_get_info_callback(ssl)	((ssl)->info_callback)

#define SSL_CB_LOOP			0x01
#define SSL_CB_EXIT			0x02
#define SSL_CB_ACCEPT_LOOP		(SSL_ST_ACCEPT|SSL_CB_LOOP)
#define SSL_CB_ACCEPT_EXIT		(SSL_ST_ACCEPT|SSL_CB_EXIT)
#define SSL_CB_CONNECT_LOOP		(SSL_ST_CONNECT|SSL_CB_LOOP)
#define SSL_CB_CONNECT_EXIT		(SSL_ST_CONNECT|SSL_CB_EXIT)

/* Define the initial state
 * These are used as flags to be checked by SSL_in_connect_init() and
 * SSL_in_accept_init() */
#define SSL_set_connect_state(a)	((a)->state=SSL_ST_CONNECT)
#define SSL_set_accept_state(a)		((a)->state=SSL_ST_ACCEPT)

/* Is the SSL_connection established? */
#define SSL_is_init_finished(a)		((a)->state == SSL_ST_OK)
#define SSL_in_init(a)			((a)->state&SSL_ST_INIT)
#define SSL_in_connect_init(a)		((a)->state&SSL_ST_CONNECT)
#define SSL_in_accept_init(a)		((a)->state&SSL_ST_ACCEPT)

/* SSLv2 */
/* client */
#define SSL2_ST_SEND_CLIENT_HELLO_A		(0x10|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_HELLO_B		(0x11|SSL_ST_CONNECT)
#define SSL2_ST_GET_SERVER_HELLO_A		(0x20|SSL_ST_CONNECT)
#define SSL2_ST_GET_SERVER_HELLO_B		(0x21|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_MASTER_KEY_A	(0x30|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_MASTER_KEY_B	(0x31|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_FINISHED_A		(0x40|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_FINISHED_B		(0x41|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_CERTIFICATE_A	(0x50|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_CERTIFICATE_B	(0x51|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_CERTIFICATE_C	(0x52|SSL_ST_CONNECT)
#define SSL2_ST_SEND_CLIENT_CERTIFICATE_D	(0x53|SSL_ST_CONNECT)
#define SSL2_ST_GET_SERVER_VERIFY_A		(0x60|SSL_ST_CONNECT)
#define SSL2_ST_GET_SERVER_VERIFY_B		(0x61|SSL_ST_CONNECT)
#define SSL2_ST_GET_SERVER_FINISHED_A		(0x70|SSL_ST_CONNECT)
#define SSL2_ST_GET_SERVER_FINISHED_B		(0x71|SSL_ST_CONNECT)
#define SSL2_ST_CLIENT_START_ENCRYPTION		(0x80|SSL_ST_CONNECT)
#define SSL2_ST_X509_GET_CLIENT_CERTIFICATE	(0x90|SSL_ST_CONNECT)
/* server */
#define SSL2_ST_GET_CLIENT_HELLO_A		(0x10|SSL_ST_ACCEPT)
#define SSL2_ST_GET_CLIENT_HELLO_B		(0x11|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_HELLO_A		(0x20|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_HELLO_B		(0x21|SSL_ST_ACCEPT)
#define SSL2_ST_GET_CLIENT_MASTER_KEY_A		(0x30|SSL_ST_ACCEPT)
#define SSL2_ST_GET_CLIENT_MASTER_KEY_B		(0x31|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_VERIFY_A		(0x40|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_VERIFY_B		(0x41|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_VERIFY_C		(0x42|SSL_ST_ACCEPT)
#define SSL2_ST_GET_CLIENT_FINISHED_A		(0x50|SSL_ST_ACCEPT)
#define SSL2_ST_GET_CLIENT_FINISHED_B		(0x51|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_FINISHED_A		(0x60|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_SERVER_FINISHED_B		(0x61|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_REQUEST_CERTIFICATE_A	(0x70|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_REQUEST_CERTIFICATE_B	(0x71|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_REQUEST_CERTIFICATE_C	(0x72|SSL_ST_ACCEPT)
#define SSL2_ST_SEND_REQUEST_CERTIFICATE_D	(0x73|SSL_ST_ACCEPT)
#define SSL2_ST_SERVER_START_ENCRYPTION		(0x80|SSL_ST_ACCEPT)
#define SSL2_ST_X509_GET_SERVER_CERTIFICATE	(0x90|SSL_ST_ACCEPT)

/* SSLv3 */
/* client */
#define SSL3_ST_SEND_CLIENT_HELLO_A		(0x100|SSL_ST_CONNECT)
#define SSL3_ST_SEND_CLIENT_HELLO_B		(0x101|SSL_ST_CONNECT)
#define SSL3_ST_GET_SERVER_HELLO_A		(0x200|SSL_ST_CONNECT)
#define SSL3_ST_GET_SERVER_HELLO_B		(0x201|SSL_ST_CONNECT)

/* server */
#define SSL3_ST_GET_CLIENT_HELLO_A		(0x100|SSL_ST_ACCEPT)
#define SSL3_ST_GET_CLIENT_HELLO_B		(0x101|SSL_ST_ACCEPT)

/* The following 2 states are kept in ssl->rstate when reads fail,
 * you should not need these */
#define SSL_ST_READ_HEADER			0xF0
#define SSL_ST_READ_BODY			0xF1
#define SSL_ST_READ_DONE			0xF2

/* use either SSL_VERIFY_NONE or SSL_VERIFY_PEER, the last 2 options
 * are 'ored' with SSL_VERIFY_PEER if they are desired */
#define SSL_VERIFY_NONE			0x00
#define SSL_VERIFY_PEER			0x01
#define SSL_VERIFY_FAIL_IF_NO_PEER_CERT	0x02
#define SSL_VERIFY_CLIENT_ONCE		0x04

#define SSL_RWERR_BAD_WRITE_RETRY	(-2)
#define SSL_RWERR_BAD_MAC_DECODE	(-3)
#define SSL_RWERR_INTERNAL_ERROR	(-4) /* should not get this one */

#define SSL_set_default_verify_paths(ctx) \
		X509_set_default_verify_paths((ctx)->cert)
#define SSL_load_verify_locations(ctx,CAfile,CApath) \
		X509_load_verify_locations((ctx)->cert,(CAfile),(CApath))

#define SSL_get_session(s)	((s)->session)
#define SSL_get_certificate(s)	((s)->session->cert)
#define SSL_get_SSL_CTX(s)	((s)->ctx)

/* this is for backward compatablility */
#define SSL_get_pref_cipher(s,n)	SSL_get_cipher_list(s,n)
#define SSL_set_pref_cipher(c,n)	SSL_set_cipher_list(c,n)
/* VMS linker has a 31 char name limit */
#define SSL_CTX_set_cert_verify_callback(a,b,c) \
		SSL_CTX_set_cert_verify_cb((a),(b),(c))


#ifdef SSLSHARED

#define d2i_SSL_SESSION_bio(bp,s_id) (SSL_SESSION *)ASN1_d2i_bio( \
	(char *(*)())SSL_SESSION_new,(char *(*)())d2i_SSL_SESSION, \
	(bp),(unsigned char **)(s_id))
#define i2d_SSL_SESSION_bio(bp,s_id) ASN1_i2d_bio(i2d_SSL_SESSION, \
	bp,(unsigned char *)s_id)

#else

#define d2i_SSL_SESSION_bio(bp,s_id) (SSL_SESSION *)ASN1_d2i_bio( \
	(char *(*)())SSL_SESSION_new_call,(char *(*)())d2i_SSL_SESSION_call, \
	(bp),(unsigned char **)(s_id))
#define i2d_SSL_SESSION_bio(bp,s_id) ASN1_i2d_bio(i2d_SSL_SESSION_call, \
	bp,(unsigned char *)s_id)

#endif


#define SSL_write(a,b,c)	SSL2_write(a,b,c)
#define SSL_read(a,b,c)		SSL2_read(a,b,c)
#define SSL_peek(a,b,c)		SSL2_peek(a,b,c)
#define SSL_accept(a)		SSL2_accept(a)
#define SSL_connect(a)		SSL2_connect(a)

#ifndef NOPROTO

int	SSL2_write(SSL *s, const char *buf, int len);
int	SSL2_read(SSL *s, char *buf, int len);
int	SSL2_peek(SSL *s, char *buf, int len);
int	SSL2_accept(SSL *s);
int	SSL2_connect(SSL *s);

#ifdef HEADER_BUFFER_H
BIO_METHOD *BIO_f_ssl(void);
#endif
int	SSL_CTX_set_cipher_list(SSL_CTX *,char *str);
SSL_CTX *SSL_CTX_new(void );
void	SSL_CTX_free(SSL_CTX *);
void	SSL_clear(SSL *s);
void	SSL_debug(char *file);
void	SSL_flush_sessions(SSL_CTX *ctx,long tm);
void	SSL_free(SSL *s);
char *	SSL_get_cipher(SSL *s);
int	SSL_get_cipher_bits(SSL *s,int *alg_bits);
int	SSL_get_fd(SSL *s);
char *	SSL_get_cipher_list(SSL *s, int n);
char *	SSL_get_shared_ciphers(SSL *s, char *buf, int len);
int	SSL_get_read_ahead(SSL * s);
SSL *	SSL_new(SSL_CTX *);
int	SSL_pending(SSL *s);
#ifndef NO_SOCK
int	SSL_set_fd(SSL *s, int fd);
int	SSL_set_rfd(SSL *s, int fd);
int	SSL_set_wfd(SSL *s, int fd);
#endif
#ifdef HEADER_BUFFER_H
void	SSL_set_bio(SSL *s, BIO *rbio,BIO *wbio);
BIO *	SSL_get_rbio(SSL *s);
BIO *	SSL_get_wbio(SSL *s);
#else
void	SSL_set_bio(SSL *s, char *rbio,char *wbio);
char *	SSL_get_rbio(SSL *s);
char *	SSL_get_wbio(SSL *s);
#endif
int	SSL_set_cipher_list(SSL *s, char *str);
void	SSL_set_read_ahead(SSL * s, int yes);
void	SSL_set_verify(SSL *s, int mode, int (*callback) ());
int	SSL_use_RSAPrivateKey(SSL *ssl, RSA *rsa);
int	SSL_use_RSAPrivateKey_ASN1(SSL *ssl, unsigned char *d, long len);
int	SSL_use_RSAPrivateKey_file(SSL *ssl, char *file, int type);
int	SSL_use_PrivateKey(SSL *ssl, EVP_PKEY *pkey);
int	SSL_use_PrivateKey_ASN1(int pk,SSL *ssl, unsigned char *d, long len);
int	SSL_use_PrivateKey_file(SSL *ssl, char *file, int type);
int	SSL_use_certificate(SSL *ssl, X509 *x);
int	SSL_use_certificate_ASN1(SSL *ssl, int len, unsigned char *d);
int	SSL_use_certificate_file(SSL *ssl, char *file, int type);
void	ERR_load_SSL_strings(void );
void	SSL_load_error_strings(void );
char * 	SSL_state_string(SSL *s);
char * 	SSL_rstate_string(SSL *s);
char * 	SSL_state_string_long(SSL *s);
char * 	SSL_rstate_string_long(SSL *s);
long	SSL_get_time(SSL_SESSION *s);
long	SSL_set_time(SSL_SESSION *s, long t);
long	SSL_get_timeout(SSL_SESSION *s);
long	SSL_set_timeout(SSL_SESSION *s, long t);
void	SSL_copy_session_id(SSL *to,SSL *from);

SSL_SESSION *SSL_SESSION_new(void);
SSL_SESSION *SSL_SESSION_new_call(void);
#ifndef WIN16
int	SSL_SESSION_print_fp(FILE *fp,SSL_SESSION *ses);
#endif
#ifdef HEADER_BUFFER_H
int	SSL_SESSION_print(BIO *fp,SSL_SESSION *ses);
#else
int	SSL_SESSION_print(char *fp,SSL_SESSION *ses);
#endif
void	SSL_SESSION_free(SSL_SESSION *ses);
int	i2d_SSL_SESSION(SSL_SESSION *in,unsigned char **pp);
int	i2d_SSL_SESSION_call(SSL_SESSION *in,unsigned char **pp);
int	SSL_set_session(SSL *to, SSL_SESSION *session);
int	SSL_add_session(SSL_CTX *s, SSL_SESSION *c);
void	SSL_remove_session(SSL_CTX *,SSL_SESSION *c);
SSL_SESSION *d2i_SSL_SESSION(SSL_SESSION **a,unsigned char **pp,long length);
SSL_SESSION *d2i_SSL_SESSION_call(SSL_SESSION **a,unsigned char **pp,long length);

#ifdef HEADER_X509_H
X509 *	SSL_get_peer_certificate(SSL *s);
#else
char *	SSL_get_peer_certificate(SSL *s);
#endif

/* old name */
#define SSL_CTX_set_default_verify(a,b,c) SSL_CTX_set_verify(a,b,c)

void SSL_CTX_set_verify(SSL_CTX *ctx,int mode,int (*callback)());
void SSL_CTX_set_cert_verify_cb(SSL_CTX *ctx, int (*cb)(),char *arg);
int SSL_CTX_use_RSAPrivateKey(SSL_CTX *ctx, RSA *rsa);
int SSL_CTX_use_RSAPrivateKey_ASN1(SSL_CTX *ctx, unsigned char *d, long len);
int SSL_CTX_use_RSAPrivateKey_file(SSL_CTX *ctx, char *file, int type);
int SSL_CTX_use_PrivateKey(SSL_CTX *ctx, EVP_PKEY *pkey);
int SSL_CTX_use_PrivateKey_ASN1(int pk,SSL_CTX *ctx,
	unsigned char *d, long len);
int SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, char *file, int type);
int SSL_CTX_use_certificate(SSL_CTX *ctx, X509 *x);
int SSL_CTX_use_certificate_ASN1(SSL_CTX *ctx, int len, unsigned char *d);
int SSL_CTX_use_certificate_file(SSL_CTX *ctx, char *file, int type);

void SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx,int (*cb)());

int SSL_CTX_check_private_key(SSL_CTX *ctx);
int SSL_check_private_key(SSL *ctx);

#else

int	SSL2_write();
int	SSL2_read();
int	SSL2_peek();
int	SSL2_accept();
int	SSL2_connect();

#ifdef HEADER_BUFFER_H
BIO_METHOD *BIO_f_ssl();
#endif
int	SSL_CTX_set_cipher_list();
SSL_CTX *SSL_CTX_new();
void	SSL_CTX_free();
void	SSL_clear();
void	SSL_debug();
void	SSL_flush_sessions();
void	SSL_free();
char *	SSL_get_cipher();
int	SSL_get_cipher_bits();
int	SSL_get_fd();
char *	SSL_get_cipher_list();
char *	SSL_get_shared_ciphers();
int	SSL_get_read_ahead();
SSL *	SSL_new();
int	SSL_pending();
#ifndef NO_SOCK
int	SSL_set_fd();
int	SSL_set_rfd();
int	SSL_set_wfd();
#endif
#ifdef HEADER_BUFFER_H
void	SSL_set_bio();
BIO *	SSL_get_rbio();
BIO *	SSL_get_wbio();
#else
void	SSL_set_bio();
char *	SSL_get_rbio();
char *	SSL_get_wbio();
#endif
int	SSL_set_cipher_list();
void	SSL_set_read_ahead();
void	SSL_set_verify();
int	SSL_use_RSAPrivateKey();
int	SSL_use_RSAPrivateKey_ASN1();
int	SSL_use_RSAPrivateKey_file();
int	SSL_use_PrivateKey();
int	SSL_use_PrivateKey_ASN1();
int	SSL_use_PrivateKey_file();
int	SSL_use_certificate();
int	SSL_use_certificate_ASN1();
int	SSL_use_certificate_file();
void	ERR_load_SSL_strings();
void	SSL_load_error_strings();
char * 	SSL_state_string();
char * 	SSL_rstate_string();
char * 	SSL_state_string_long();
char * 	SSL_rstate_string_long();
long	SSL_get_time();
long	SSL_set_time();
long	SSL_get_timeout();
long	SSL_set_timeout();
void	SSL_copy_session_id();

SSL_SESSION *SSL_SESSION_new();
SSL_SESSION *SSL_SESSION_new_call();
#ifndef WIN16
int	SSL_SESSION_print_fp();
#endif
#ifdef HEADER_BUFFER_H
int	SSL_SESSION_print();
#else
int	SSL_SESSION_print();
#endif
void	SSL_SESSION_free();
int	i2d_SSL_SESSION();
int	i2d_SSL_SESSION_call();
int	SSL_set_session();
int	SSL_add_session();
void	SSL_remove_session();
SSL_SESSION *d2i_SSL_SESSION();
SSL_SESSION *d2i_SSL_SESSION_call();

#ifdef HEADER_X509_H
X509 *	SSL_get_peer_certificate();
#else
char *	SSL_get_peer_certificate();
#endif

void SSL_CTX_set_verify();
void SSL_CTX_set_cert_verify_cb();
int SSL_CTX_use_RSAPrivateKey();
int SSL_CTX_use_RSAPrivateKey_ASN1();
int SSL_CTX_use_RSAPrivateKey_file();
int SSL_CTX_use_PrivateKey();
int SSL_CTX_use_PrivateKey_ASN1();
int SSL_CTX_use_PrivateKey_file();
int SSL_CTX_use_certificate();
int SSL_CTX_use_certificate_ASN1();
int SSL_CTX_use_certificate_file();

void SSL_CTX_set_default_passwd_cb();

int SSL_CTX_check_private_key();
int SSL_check_private_key();

#endif

/* tjh added these two dudes to enable external control
 * of debug and trace logging
 */
extern FILE *SSL_ERR;
extern FILE *SSL_LOG;

/* BEGIN ERROR CODES */
/* Error codes for the SSL functions. */

/* Function codes. */
#define SSL_F_CLIENT_CERTIFICATE			 100
#define SSL_F_CLIENT_HELLO				 101
#define SSL_F_CLIENT_MASTER_KEY				 102
#define SSL_F_D2I_SSL_SESSION				 103
#define SSL_F_GET_CLIENT_FINISHED			 104
#define SSL_F_GET_CLIENT_HELLO				 105
#define SSL_F_GET_CLIENT_MASTER_KEY			 106
#define SSL_F_GET_SERVER_FINISHED			 107
#define SSL_F_GET_SERVER_HELLO				 108
#define SSL_F_GET_SERVER_VERIFY				 109
#define SSL_F_I2D_SSL_SESSION				 110
#define SSL_F_READ_N					 111
#define SSL_F_REQUEST_CERTIFICATE			 112
#define SSL_F_SERVER_HELLO				 113
#define SSL_F_SSL2_ACCEPT				 114
#define SSL_F_SSL2_CONNECT				 115
#define SSL_F_SSL2_READ					 116
#define SSL_F_SSL_CERT_NEW				 117
#define SSL_F_SSL_CHECK_PRIVATE_KEY			 118
#define SSL_F_SSL_CTX_CHECK_PRIVATE_KEY			 119
#define SSL_F_SSL_CTX_USE_CERTIFICATE			 120
#define SSL_F_SSL_CTX_USE_CERTIFICATE_ASN1		 121
#define SSL_F_SSL_CTX_USE_CERTIFICATE_FILE		 122
#define SSL_F_SSL_CTX_USE_PRIVATEKEY			 123
#define SSL_F_SSL_CTX_USE_PRIVATEKEY_ASN1		 124
#define SSL_F_SSL_CTX_USE_PRIVATEKEY_FILE		 125
#define SSL_F_SSL_CTX_USE_RSAPRIVATEKEY			 126
#define SSL_F_SSL_CTX_USE_RSAPRIVATEKEY_ASN1		 127
#define SSL_F_SSL_CTX_USE_RSAPRIVATEKEY_FILE		 128
#define SSL_F_SSL_ENC_INIT				 129
#define SSL_F_SSL_MAKE_CIPHER_LIST			 130
#define SSL_F_SSL_NEW					 131
#define SSL_F_SSL_RSA_PRIVATE_DECRYPT			 132
#define SSL_F_SSL_RSA_PUBLIC_ENCRYPT			 133
#define SSL_F_SSL_SESSION_NEW				 134
#define SSL_F_SSL_SESSION_PRINT_FP			 135
#define SSL_F_SSL_SET_CERTIFICATE			 136
#define SSL_F_SSL_SET_FD				 137
#define SSL_F_SSL_SET_RFD				 138
#define SSL_F_SSL_SET_WFD				 139
#define SSL_F_SSL_STARTUP				 140
#define SSL_F_SSL_USE_CERTIFICATE			 141
#define SSL_F_SSL_USE_CERTIFICATE_ASN1			 142
#define SSL_F_SSL_USE_CERTIFICATE_FILE			 143
#define SSL_F_SSL_USE_PRIVATEKEY			 144
#define SSL_F_SSL_USE_PRIVATEKEY_ASN1			 145
#define SSL_F_SSL_USE_PRIVATEKEY_FILE			 146
#define SSL_F_SSL_USE_RSAPRIVATEKEY			 147
#define SSL_F_SSL_USE_RSAPRIVATEKEY_ASN1		 148
#define SSL_F_SSL_USE_RSAPRIVATEKEY_FILE		 149
#define SSL_F_WRITE_PENDING				 150

/* Reason codes. */
#define SSL_R_BAD_AUTHENTICATION_TYPE			 100
#define SSL_R_BAD_CHECKSUM				 101
#define SSL_R_BAD_DATA_RETURNED_BY_CALLBACK		 102
#define SSL_R_BAD_MAC_DECODE				 103
#define SSL_R_BAD_RESPONSE_ARGUMENT			 104
#define SSL_R_BAD_RSA_DECRYPT				 105
#define SSL_R_BAD_SSL_FILETYPE				 106
#define SSL_R_BAD_SSL_SESSION_ID_LENGTH			 107
#define SSL_R_BAD_STATE					 108
#define SSL_R_BAD_WRITE_RETRY				 109
#define SSL_R_CERTIFICATE_VERIFY_FAILED			 110
#define SSL_R_CHALLENGE_IS_DIFFERENT			 111
#define SSL_R_CIPHER_CODE_TOO_LONG			 112
#define SSL_R_CIPHER_TABLE_SRC_ERROR			 113
#define SSL_R_CONECTION_ID_IS_DIFFERENT			 114
#define SSL_R_INVALID_CHALLENGE_LENGTH			 115
#define SSL_R_NO_CERTIFICATE_ASIGNED			 116
#define SSL_R_NO_CERTIFICATE_SET			 117
#define SSL_R_NO_CERTIFICATE_SPECIFIED			 118
#define SSL_R_NO_CIPHER_LIST				 119
#define SSL_R_NO_CIPHER_MATCH				 120
#define SSL_R_NO_CIPHER_WE_TRUST			 121
#define SSL_R_NO_PRIVATEKEY				 122
#define SSL_R_NO_PRIVATE_KEY_ASIGNED			 123
#define SSL_R_NO_PUBLICKEY				 124
#define SSL_R_NO_READ_METHOD_SET			 125
#define SSL_R_NO_WRITE_METHOD_SET			 126
#define SSL_R_NULL_ARGUMENT				 127
#define SSL_R_NULL_SSL_CTX				 128
#define SSL_R_PEER_DID_NOT_RETURN_A_CERTIFICATE		 129
#define SSL_R_PEER_ERROR				 130
#define SSL_R_PEER_ERROR_CERTIFICATE			 131
#define SSL_R_PEER_ERROR_NO_CIPHER			 132
#define SSL_R_PEER_ERROR_UNSUPPORTED_CERTIFICATE_TYPE	 133
#define SSL_R_PERR_ERROR_NO_CERTIFICATE			 134
#define SSL_R_PUBLIC_KEY_ENCRYPT_ERROR			 135
#define SSL_R_PUBLIC_KEY_IS_NOT_RSA			 136
#define SSL_R_PUBLIC_KEY_NO_RSA				 137
#define SSL_R_READ_WRONG_PACKET_TYPE			 138
#define SSL_R_REVERSE_KEY_ARG_LENGTH_IS_WRONG		 139
#define SSL_R_REVERSE_MASTER_KEY_LENGTH_IS_WRONG	 140
#define SSL_R_REVERSE_SSL_SESSION_ID_LENGTH_IS_WRONG	 141
#define SSL_R_SHORT_READ				 142
#define SSL_R_SSL_SESSION_ID_IS_DIFFERENT		 143
#define SSL_R_UNABLE_TO_EXTRACT_PUBLIC_KEY		 144
#define SSL_R_UNDEFINED_INIT_STATE			 145
#define SSL_R_UNKNOWN_REMOTE_ERROR_TYPE			 146
#define SSL_R_UNKNOWN_STATE				 147
#define SSL_R_UNSUPORTED_CIPHER				 148
#define SSL_R_X509_LIB					 149

#ifdef  __cplusplus
}
#endif
#endif

#endif
