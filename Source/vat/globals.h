#ifndef VAT_GLOBALS_H
#define VAT_GLOBALS_H
/*
 * $Id: globals.h,v 1.3 2001/09/20 13:48:36 neko Exp $
 */

#include <macros/compilers.h>
#include "vat_debug.h"

#ifdef __MORPHOS__

#include <exec/types.h>

// TOFIX: #include <proto/bla> should work but then gcc pukes about libbases... find out why

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include <ppcinline/exec.h>

#include <ppcinline/dos.h>
#include <ppcinline/intuition.h>
#include <ppcinline/rexxsyslib.h>
#include <ppcinline/muimaster.h>
#include <ppcinline/asl.h>
#include <ppcinline/datatypes.h>
#include <ppcinline/battclock.h>
#include <ppcinline/timer.h>
#include <ppcinline/openurl.h>

#endif /* __MORPHOS__ */

#endif /* !VAT_GLOBALS_H */
