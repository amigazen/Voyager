/*
 * $Id: errorreq.c,v 1.1 2000/10/08 14:28:21 zapek Exp $
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>

ULONG errorreq( char *title, char *body, char *gadgets)
{
	ULONG idcmp = NULL;
	struct EasyStruct estr;

	estr.es_StructSize = sizeof(struct EasyStruct);
	estr.es_Flags = NULL;
	estr.es_Title = title;
	estr.es_TextFormat = body;
	estr.es_GadgetFormat = gadgets;

	return ((ULONG)EasyRequestArgs(NULL, &estr, &idcmp, NULL));
}
