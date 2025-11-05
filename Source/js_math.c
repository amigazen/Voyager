/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2003 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


/*
** $Id: js_math.c,v 1.22 2003/08/10 21:27:53 olli Exp $
*/

#include "voyager.h"
#include "classes.h"
#include "js.h"
#include "mui_func.h"

#include <math.h>

struct Data {
	int dummy;
};

BEGINPTABLE
DPROP( E, 		real )
DPROP( LN10, 	real )
DPROP( LN2, 	real )
DPROP( LOG10E, 	real )
DPROP( LOG2E, 	real )
DPROP( PI, 		real )
DPROP( SQRT1_2, real )
DPROP( SQRT2, 	real )
DPROP( abs,		funcptr )
DPROP( acos,	funcptr )
DPROP( asin,	funcptr )
DPROP( atan,	funcptr )
DPROP( atan2,	funcptr )
DPROP( ceil,	funcptr )
DPROP( cos,		funcptr )
DPROP( exp,		funcptr )
DPROP( floor,	funcptr )
DPROP( log,		funcptr )
DPROP( max,		funcptr )
DPROP( min,		funcptr )
DPROP( pow,		funcptr )
DPROP( random,	funcptr )
DPROP( round,	funcptr )
DPROP( sin,		funcptr )
DPROP( sqrt,	funcptr )
DPROP( tan,		funcptr )
ENDPTABLE

DECNEW
{
	obj = DoSuperNew( cl, obj,
		MA_JS_ClassName, "Math",
		TAG_MORE, msg->ops_AttrList
	);
	return( (ULONG)obj );
}

DECSMETHOD( JS_GetProperty )
{
	struct propt *pt = findprop( ptable, msg->propname );

	if( !pt )
		return( DOSUPER );

	if( pt->type == expt_funcptr )
	{
		storefuncprop( msg, -pt->id );
		return( TRUE );
	}
	
	switch( pt->id )
	{
		case JSPID_E:
			storerealprop( msg, 2.718281828459045 );
			return( TRUE );

		case JSPID_LN10:
			storerealprop( msg, 2.302585092994046 );
			return( TRUE );

		case JSPID_LN2:
			storerealprop( msg, 0.6931471805599453 );
			return( TRUE );

		case JSPID_LOG10E:
			storerealprop( msg, 0.4342944819032518 );
			return( TRUE );

		case JSPID_LOG2E:
			storerealprop( msg, 1.4426950408889633 );
			return( TRUE );

		case JSPID_PI:
			storerealprop( msg, 3.141592653589793 );
			return( TRUE );

		case JSPID_SQRT1_2:
			storerealprop( msg, 0.7071067811865476 );
			return( TRUE );

		case JSPID_SQRT2:
			storerealprop( msg, 1.4142135623730951 );
			return( TRUE );
	}

	return( FALSE );
}

DECSMETHOD( JS_CallMethod )
{
	static double rval;

	switch( msg->pid )
	{
		case JSPID_abs:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = fabs( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_acos:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = acos( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_asin:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = asin( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_atan:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = atan( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_atan2:
			{
				double v1, v2;

				if( msg->argcnt != 2 )
					return( 0 );

				v1 = exprs_pop_as_real( msg->es );
				v2 = exprs_pop_as_real( msg->es );

				msg->argcnt = 0;

				rval = atan2( v1, v2 );
			}
			break;

		case JSPID_ceil:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = ceil( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_cos:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = cos( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_exp:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = exp( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_floor:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = floor( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_log:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = log( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_max:
			{
				double v1, v2;

				if( msg->argcnt != 2 )
					return( 0 );

				v1 = exprs_pop_as_real( msg->es );
				v2 = exprs_pop_as_real( msg->es );

				msg->argcnt = 0;

				// rval = v1 > v2 ? v1 : v2;
				rval = max( v1, v2 );
			}
			break;

		case JSPID_min:
			{
				double v1, v2;

				if( msg->argcnt != 2 )
					return( 0 );

				v1 = exprs_pop_as_real( msg->es );
				v2 = exprs_pop_as_real( msg->es );

				msg->argcnt = 0;

				//rval = v1 < v2 ? v1 : v2;
				rval = min( v1, v2 );
			}
			break;

		case JSPID_pow:
			{
				double v1, v2;

				if( msg->argcnt != 2 )
					return( 0 );

				v1 = exprs_pop_as_real( msg->es );
				v2 = exprs_pop_as_real( msg->es );

				msg->argcnt = 0;

				rval = pow( v1, v2 );
			}
			break;

		case JSPID_random:
			{
				if( msg->argcnt != 0 )
					return( 0 );
				rval = drand48();
			}
			break;

		case JSPID_round:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = exprs_pop_as_real( msg->es );
				//rval = floor( rval + ( ( rval > 0 ) ? 0.5 : - 0.5 ) );
				rval = floor( rval + 0.5 );
			}
			break;

		case JSPID_sin:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = sin( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_sqrt:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = sqrt( exprs_pop_as_real( msg->es ) );
			}
			break;

		case JSPID_tan:
			{
				if( msg->argcnt-- < 1 )
					return( 0 );
				rval = tan( exprs_pop_as_real( msg->es ) );
			}
			break;

		default:
			return( DOSUPER );
	}

	*msg->typeptr = expt_real;
	*msg->dataptr = &rval;

	return( TRUE );
}

DECSMETHOD( JS_HasProperty )
{
	struct propt *pt;

	if( pt = findprop( ptable, msg->propname ) )
		return( (ULONG)pt->type );

	return( DOSUPER );
}

BEGINMTABLE
DEFNEW
DEFSMETHOD( JS_GetProperty )
DEFSMETHOD( JS_CallMethod )
DEFSMETHOD( JS_HasProperty )
ENDMTABLE

static struct MUI_CustomClass *mcc;

int create_js_math( void )
{
	D( db_init, bug( "initializing..\n" ) );
	
	if( !( mcc = ( struct MUI_CustomClass * )MUI_CreateCustomClass( NULL, NULL, getjs_object_class(), sizeof( struct Data ), DISPATCHERREF ) ) )
		return( FALSE );

#ifdef VDEBUG
	if( MUIMasterBase->lib_Version >= 20 )
		mcc->mcc_Class->cl_ID = "JS_MathClass";
#endif

	srand48( (long)time( 0 ) );

	return( TRUE );
}

void delete_js_math( void )
{
	if( mcc )
		MUI_DeleteCustomClass( mcc );
}

APTR getjs_math( void )
{
	return( mcc->mcc_Class );
}

