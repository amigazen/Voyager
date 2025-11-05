#define VABOUT_NAME "PROGDIR:Plugins/voyager_about.vlib"
#define VABOUT_VERSION 3

#pragma libcall VAboutBase VABOUT_GetAboutPtr 1e A9803
#pragma libcall VAboutBase VABOUT_GetVLogo 24 801
#pragma libcall VAboutBase VABOUT_GetSSLLogo 2a 801
#pragma libcall VAboutBase VABOUT_GetPNGLogo 30 801
#pragma libcall VAboutBase VABOUT_GetAboutIbeta 36 0
#pragma libcall VAboutBase VABOUT_GetV3Logo 3c 801
#pragma libcall VAboutBase VABOUT_GetFlashLogo 42 801

STRPTR VABOUT_GetAboutPtr( STRPTR revid, STRPTR owner, STRPTR imgdeclib );
APTR VABOUT_GetVLogo( int *sizeptr );
APTR VABOUT_GetV3Logo( int *sizeptr );
APTR VABOUT_GetFlashLogo( int *sizeptr );
APTR VABOUT_GetSSLLogo( int *sizeptr );
APTR VABOUT_GetPNGLogo( int *sizeptr );
STRPTR VABOUT_GetAboutIbeta( void );
