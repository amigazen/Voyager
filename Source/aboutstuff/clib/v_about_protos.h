#ifndef CLIB_V_ABOUT_PROTOS_H
#define CLIB_V_ABOUT_PROTOS_H

/*
 * Vapor About protos
 * ------------------
 *
 * © 2001 by VaporWare
 *
 * $Id: v_about_protos.h,v 1.2 2003/06/22 23:04:50 zapek Exp $
 *
 */

#include <exec/types.h>
#include <libraries/v_about.h>

STRPTR VABOUT_GetAboutPtr( STRPTR revid, STRPTR owner, STRPTR imgdeclib );
APTR VABOUT_GetVLogo( int *sizeptr );
APTR VABOUT_GetV3Logo( int *sizeptr );
APTR VABOUT_GetFlashLogo( int *sizeptr );
APTR VABOUT_GetSSLLogo( int *sizeptr );
APTR VABOUT_GetPNGLogo( int *sizeptr );
STRPTR VABOUT_GetAboutIbeta( void );

#endif /* !CLIB_V_ABOUT_PROTOS_H */
