/* Automatically generated file! Do not edit! */

#include "include/clib/v_plugin_protos.h"

#include <emul/emulregs.h>

APTR  LIB_VPLUG_ProcessURLMethod(void)
{
	return (APTR )VPLUG_ProcessURLMethod((STRPTR )REG_A0);
}

struct TagItem * LIB_VPLUG_Query(void)
{
	return (struct TagItem *)VPLUG_Query();
}

APTR  LIB_VPLUG_GetClass(void)
{
	return (APTR )VPLUG_GetClass((STRPTR )REG_A0);
}

void  LIB_VPLUG_Cleanup(void)
{
	VPLUG_Cleanup();
}

int  LIB_VPLUG_GetURLDataSize(void)
{
	return (int )VPLUG_GetURLDataSize((APTR )REG_A0);
}

int  LIB_VPLUG_ProcessURLString(void)
{
	return (int )VPLUG_ProcessURLString((STRPTR )REG_A0);
}

void  LIB_VPLUG_FinalSetup(void)
{
	VPLUG_FinalSetup();
}

BOOL  LIB_VPLUG_GetInfo(void)
{
	return (BOOL )VPLUG_GetInfo((struct VPlugInfo *)REG_A0, (APTR )REG_A1);
}

STRPTR  LIB_VPLUG_GetURLMIMEType(void)
{
	return (STRPTR )VPLUG_GetURLMIMEType((APTR )REG_A0);
}

APTR  LIB_VPLUG_GetURLData(void)
{
	return (APTR )VPLUG_GetURLData((APTR )REG_A0);
}

void  LIB_VPLUG_FreeURLData(void)
{
	VPLUG_FreeURLData((APTR )REG_A0);
}

void  LIB_VPLUG_Hook_Prefs(void)
{
	VPLUG_Hook_Prefs((ULONG )REG_D0, (struct vplug_prefs *)REG_A0);
}

BOOL  LIB_VPLUG_Setup(void)
{
	return (BOOL )VPLUG_Setup((struct vplug_functable *)REG_A0);
}

