/* crypto/buffer/bss_sock.c */
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

#include <system/system.h>
#include <system/types.h>
#include <net_lib_defs.h>
#include <stdio.h>

#define USE_SOCKETS
#include "cryptlib.h"
#include "openssl/include/openssl/buffer.h"

extern NETBASE;

int last_socket_error;

static int sock_free( BIO *a);

int tcp_recv( int s, char *to, int len )
{
	int rc;

	//kprintf( "SSL: recv(%ld)\n", len );
	rc = recv( s, to, len, 0 );
	if( rc <= 0 )
		last_socket_error = Errno();

	//kprintf( "SSL: got %ld bytes, Errno = %ld\n", rc, Errno() );

	return( rc );
}

int tcp_send( int s, char *data, DWORD len )
{
	int rc;
	int got = 0;

	//kprintf( "SSL: send(%ld)\n", len );

	if( len < 0 )
		len = strlen( data );
	if( !len )
		return( 0 );

	while( len > 0 )
	{
		rc = send( s, data, len, 0 );
		if( rc <= 0 )
			last_socket_error = Errno();

		if( rc < 1 )
		{
			//kprintf( "SSL: send rc %ld\n", rc );
			return( rc );
		}
		data += rc;
		len -= rc;
		got += rc;
	}
	//kprintf( "SSL: send sent %ld\n", got );
	return( got );
}

UDWORD tcp_peername( int s )
{
	struct sockaddr_in sockadr = { 0 };
	UDWORD ss = sizeof( sockadr );
	getsockname( s, &sockadr, &ss );

	memcpy( &ss, &sockadr.sin_addr, 4 );
	return( ss );
}



#ifndef BIO_FD
static int sock_read(b,out,outl)
#else
static int fd_read(b,out,outl)
#endif
BIO *b;
char *out;
int outl;
	{
	int ret=0;

	if (out != NULL)
		{
#ifndef BIO_FD
		clear_socket_error();
		ret=tcp_recv(b->num,out,outl);
#else
		clear_sys_error();
		ret=read(b->num,out,outl);
#endif
		BIO_clear_retry_flags(b);
		if (ret <= 0)
			{
#ifndef BIO_FD
			if (BIO_sock_should_retry(ret))
#else
			if (BIO_fd_should_retry(ret))
#endif
				BIO_set_retry_read(b);
			}
		}
	return(ret);
	}

#ifndef BIO_FD
static int sock_write(b,in,inl)
#else
static int fd_write(b,in,inl)
#endif
BIO *b;
char *in;
int inl;
	{
	int ret;
	
#ifndef BIO_FD
	clear_socket_error();
	ret=tcp_send(b->num,in,inl);
#else
	clear_sys_error();
	ret=write(b->num,in,inl);
#endif
	BIO_clear_retry_flags(b);
	if (ret <= 0)
		{
#ifndef BIO_FD
		if (BIO_sock_should_retry(ret))
#else
		if (BIO_fd_should_retry(ret))
#endif
			BIO_set_retry_write(b);
		}
	return(ret);
	}

static int sock_puts(bp,str)
BIO *bp;
char *str;
	{
	int n,ret;

	n=strlen(str);
	ret=sock_write(bp,str,n);
	return(ret);
	}

#ifndef BIO_FD
static long sock_ctrl(b,cmd,num,ptr)
#else
static long fd_ctrl(b,cmd,num,ptr)
#endif
BIO *b;
int cmd;
long num;
char *ptr;
	{
	long ret=1;
	int *ip;

	switch (cmd)
		{
	case BIO_CTRL_RESET:
		num=0;
	case BIO_C_FILE_SEEK:
#ifdef BIO_FD
		ret=(long)lseek(b->num,num,0);
#else
		ret=0;
#endif
		break;
	case BIO_C_FILE_TELL:
	case BIO_CTRL_INFO:
#ifdef BIO_FD
		ret=(long)lseek(b->num,0,1);
#else
		ret=0;
#endif
		break;
	case BIO_C_SET_FD:
#ifndef BIO_FD
		sock_free(b);
#else
		fd_free(b);
#endif
		b->num= *((int *)ptr);
		b->shutdown=(int)num;
		b->init=1;
		break;
	case BIO_C_GET_FD:
		if (b->init)
			{
			ip=(int *)ptr;
			if (ip != NULL) *ip=b->num;
			ret=b->num;
			}
		else
			ret= -1;
		break;
	case BIO_CTRL_GET_CLOSE:
		ret=b->shutdown;
		break;
	case BIO_CTRL_SET_CLOSE:
		b->shutdown=(int)num;
		break;
	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
		ret=0;
		break;
	case BIO_CTRL_DUP:
	case BIO_CTRL_FLUSH:
		ret=1;
		break;
	default:
		ret=0;
		break;
		}
	return(ret);
	}

static int sock_new(bi)
BIO *bi;
	{
	bi->init=0;
	bi->num=0;
	bi->ptr=NULL;
	bi->flags=0;
	return(1);
	}

static int sock_free(a)
BIO *a;
	{
	if (a == NULL) return(0);
	if (a->shutdown)
		{
		if (a->init)
			{
			}
		a->init=0;
		a->flags=0;
		}
	return(1);
	}

#ifndef BIO_FD
int BIO_sock_should_retry(i)
#else
int BIO_fd_should_retry(i)
#endif
int i;
	{
	int err;

	if ((i == 0) || (i == -1))
		{
#ifndef BIO_FD

		err=last_socket_error;
#else
		err=get_last_sys_error();
#endif

#if defined(WINDOWS) && 0 /* more microsoft stupidity? perhaps not? Ben 4/1/99 */
		if ((i == -1) && (err == 0))
			return(1);
#endif

#ifndef BIO_FD
		return(BIO_sock_non_fatal_error(err));
#else
		return(BIO_fd_non_fatal_error(err));
#endif
		}
	return(0);
	}

#ifndef BIO_FD
int BIO_sock_non_fatal_error(err)
#else
int BIO_fd_non_fatal_error(err)
#endif
int err;
	{
	   //kprintf( "non-fatal: %ld\n", err );
	switch (err)
		{
//	case ENOTCONN:
	case EINTR:
	case EAGAIN:
//	case EINPROGRESS:
//	case EALREADY:
		return(1);
		/* break; */
	default:
		break;
		}
	return(0);
	}


static BIO_METHOD methods_sockp=
	{
	BIO_TYPE_MEM,"socket",
	sock_write,
	sock_read,
	sock_puts,
	NULL, /* sock_gets, */
	sock_ctrl,
	sock_new,
	sock_free,
	};

BIO_METHOD *BIO_s_socket()
	{
	return(&methods_sockp);
	}

