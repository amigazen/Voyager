/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE__H
#define _PPCINLINE__H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef _BASE_NAME
#define _BASE_NAME VSSLBase
#endif /* !_BASE_NAME */

#define VSSL_X509_NameHash(__p0) \
	LP1(168, ULONG , VSSL_X509_NameHash, \
		X509 *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_GetCipher(__p0) \
	LP1(60, char *, VSSL_GetCipher, \
		APTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_WriteCertPEM(__p0, __p1) \
	LP2(144, int , VSSL_WriteCertPEM, \
		X509 *, __p0, a0, \
		STRPTR , __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Read(__p0, __p1, __p2) \
	LP3(72, int , VSSL_Read, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		int , __p2, d0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_ReadCertASN1(__p0) \
	LP1(156, X509 *, VSSL_ReadCertASN1, \
		STRPTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_fingerprint(__p0, __p1) \
	LP2(180, int , VSSL_X509_fingerprint, \
		X509 *, __p0, a0, \
		char *, __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_SetDefaultOptions(__p0) \
	LP1NR(216, VSSL_SetDefaultOptions, \
		ULONG , __p0, d0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_serialnumber(__p0, __p1) \
	LP2NR(186, VSSL_X509_serialnumber, \
		X509 *, __p0, a0, \
		char *, __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_NameOneline(__p0) \
	LP1(102, STRPTR , VSSL_X509_NameOneline, \
		X509_NAME *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Create_CTX() \
	LP0(36, APTR , VSSL_Create_CTX, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_FreeNameOneline(__p0) \
	LP1NR(108, VSSL_X509_FreeNameOneline, \
		STRPTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_get_subject_name(__p0) \
	LP1(114, X509_NAME *, VSSL_X509_get_subject_name, \
		X509 *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_CTX_Set_Options(__p0, __p1) \
	LP2NR(204, VSSL_CTX_Set_Options, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_GetVersion(__p0) \
	LP1(210, STRPTR , VSSL_GetVersion, \
		APTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Connect(__p0, __p1) \
	LP2(48, APTR , VSSL_Connect, \
		APTR , __p0, a0, \
		int , __p1, d0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Write(__p0, __p1, __p2) \
	LP3(66, int , VSSL_Write, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		int , __p2, d0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_ReadCertPEM(__p0) \
	LP1(150, X509 *, VSSL_ReadCertPEM, \
		STRPTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_ASN1_UTCTIME_sprint(__p0, __p1) \
	LP2NR(162, VSSL_ASN1_UTCTIME_sprint, \
		char *, __p0, a0, \
		ASN1_UTCTIME *, __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Id() \
	LP0(30, char *, VSSL_Id, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_get_issuer_name(__p0) \
	LP1(120, X509_NAME *, VSSL_X509_get_issuer_name, \
		X509 *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_GetPeerCertificate(__p0) \
	LP1(84, X509 *, VSSL_GetPeerCertificate, \
		APTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_GetStats(__p0, __p1) \
	LP2NR(198, VSSL_GetStats, \
		APTR , __p0, a0, \
		struct VSSL_CacheInfo *, __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Free_CTX(__p0) \
	LP1NR(42, VSSL_Free_CTX, \
		APTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_SetRandSeed(__p0, __p1) \
	LP2NR(192, VSSL_SetRandSeed, \
		APTR , __p0, a0, \
		int , __p1, d0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_GetVerifyResult(__p0, __p1) \
	LP2(138, int , VSSL_GetVerifyResult, \
		APTR , __p0, a0, \
		char **, __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_AddCertDir(__p0, __p1) \
	LP2NR(90, VSSL_AddCertDir, \
		APTR , __p0, a0, \
		STRPTR , __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_Close(__p0) \
	LP1NR(54, VSSL_Close, \
		APTR , __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_HaveSubjectCert(__p0, __p1) \
	LP2(174, int , VSSL_X509_HaveSubjectCert, \
		APTR , __p0, a0, \
		X509 *, __p1, a1, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_get_notAfter(__p0) \
	LP1(132, ASN1_UTCTIME *, VSSL_X509_get_notAfter, \
		X509 *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_Free(__p0) \
	LP1NR(96, VSSL_X509_Free, \
		X509 *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_X509_get_notBefore(__p0) \
	LP1(126, ASN1_UTCTIME *, VSSL_X509_get_notBefore, \
		X509 *, __p0, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define VSSL_SetTCPMode(__p0, __p1) \
	LP2NR(78, VSSL_SetTCPMode, \
		int , __p0, d0, \
		APTR , __p1, a0, \
		, _BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE__H */
