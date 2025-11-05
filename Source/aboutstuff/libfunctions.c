/* Automatically generated file! Do not edit! */

#include "clib/v_about_protos.h"

#include <emul/emulregs.h>

STRPTR  LIB_VABOUT_GetAboutIbeta(void)
{
	return (STRPTR )VABOUT_GetAboutIbeta();
}

STRPTR  LIB_VABOUT_GetAboutPtr(void)
{
	return (STRPTR )VABOUT_GetAboutPtr((STRPTR )REG_A0, (STRPTR )REG_A1, (STRPTR )REG_A2);
}

APTR  LIB_VABOUT_GetV3Logo(void)
{
	return (APTR )VABOUT_GetV3Logo((int *)REG_A0);
}

APTR  LIB_VABOUT_GetSSLLogo(void)
{
	return (APTR )VABOUT_GetSSLLogo((int *)REG_A0);
}

APTR  LIB_VABOUT_GetPNGLogo(void)
{
	return (APTR )VABOUT_GetPNGLogo((int *)REG_A0);
}

APTR  LIB_VABOUT_GetVLogo(void)
{
	return (APTR )VABOUT_GetVLogo((int *)REG_A0);
}

APTR  LIB_VABOUT_GetFlashLogo(void)
{
	return (APTR )VABOUT_GetFlashLogo((int *)REG_A0);
}

