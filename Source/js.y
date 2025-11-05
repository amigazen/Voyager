%{

/*
** $Id: js.y,v 1.70 2001/08/19 13:24:21 owagner Exp $
*/

#include "voyager.h"

#include "classes.h"
#include "js.h"
#include <math.h>

#ifdef __GNUC__
#ifndef __MORPHOS__
#define _STDIO_H_ 1
#endif /* !__MORPHOS__ */
#else
#define _STDIO_H 1
#endif

static void __yy_memcpy (char *,char *to,int);
int yyparse (void);
void yyerror(char *);

#define alloca js_alloca
extern void *js_alloca( int );

extern int lineno;

#define YYSTYPE struct yystype

#define YYPURE
#define YY_DECL int yylex ( YYSTYPE *yylval )

#define YYERROR_VERBOSE

// Fuck-o-Stdio trickery
#ifdef __SASC
typedef void *FILE;
#define EOF -1
#endif
#ifndef __MORPHOS__
#define stdin 0
#define stdout 0
#endif /* __MORPHOS__ */
#include "js_scanner.h"
%}

%token TOKEN_STRING TOKEN_REAL TOKEN_INTEGER TOKEN_SYMBOL TOKEN_BREAK TOKEN_CONTINUE
%token TOKEN_ELSE TOKEN_FALSE TOKEN_FOR TOKEN_FUNCTION TOKEN_IF TOKEN_IN TOKEN_NEW
%token TOKEN_NULL TOKEN_RETURN TOKEN_THIS TOKEN_TRUE TOKEN_VAR TOKEN_WHILE TOKEN_WITH
%token TOKEN_LEQ TOKEN_GEQ TOKEN_EQEQ TOKEN_PLUSEQ TOKEN_MINUSEQ TOKEN_TIMESEQ
%token TOKEN_DIVEQ TOKEN_NOTEQ TOKEN_MODEQ TOKEN_ANDEQ TOKEN_OREQ TOKEN_XOREQ
%token TOKEN_GGGEQ TOKEN_GGEQ TOKEN_LLEQ TOKEN_GGG TOKEN_GG TOKEN_LL TOKEN_PLUSPLUS
%token TOKEN_MINUSMINUS TOKEN_ANDAND TOKEN_OROR TOKEN_DEBUG TOKEN_INF TOKEN_NAN TOKEN_UNDEFINED
%token TOKEN_VOID TOKEN_TYPEOF TOKEN_DELETE TOKEN_EVAL TOKEN_DEBUGESTACK
%token TOKEN_ISNAN TOKEN_ISFINITE TOKEN_PARSEINT TOKEN_PARSEFLOAT TOKEN_NUMBER TOKEN_ESCAPE TOKEN_UNESCAPE
%token TOKEN_STRONGEQ TOKEN_STRONGNOTEQ TOKEN_DO TOKEN_STRINGFUNC
%token TOKEN_REGEXP TOKEN_SWITCH TOKEN_CASE TOKEN_DEFAULT

%expect 0
%nonassoc PREC_LOWER_THAN_ELSE
%nonassoc TOKEN_ELSE
%nonassoc TOKEN_NEW
%nonassoc TOKEN_DELETE
%nonassoc TOKEN_EVAL
%left ','
%left PREC_LESS_THAN_STATEMENT
%left PREC_STATEMENT
%right '=' TOKEN_PLUSEQ TOKEN_MINUSEQ TOKEN_TIMESEQ TOKEN_DIVEQ TOKEN_MODEQ TOKEN_GGEQ TOKEN_LLEQ TOKEN_GGGEQ TOKEN_ANDEQ TOKEN_OREQ TOKEN_XOREQ
%left ':'
%right '?'
%left TOKEN_OROR
%left TOKEN_ANDAND
%left '|'
%left '^'
%left '&'
%left TOKEN_EQEQ TOKEN_NOTEQ TOKEN_STRONGEQ TOKEN_STRONGNOTEQ
%left TOKEN_LEQ TOKEN_GEQ '<' '>'
%left TOKEN_GG TOKEN_LL TOKEN_GGG
%left '+' '-'
%left '%' '*' '/'
%nonassoc '~' '!' TOKEN_PLUSPLUS TOKEN_MINUSMINUS PREC_UMINUS TOKEN_TYPEOF
%left '.' 
%left PREC_HIGHEST
%%
program:    {  }
                | expr
                | outerstatementlist
                ;

outerstatementlist:     statementorblock
                    |   outerstatementlist statementorblock
                    ;

beginfunc:              TOKEN_FUNCTION TOKEN_SYMBOL '(' { SOPS( FUNC_BEGIN, yylval.text ); } 
                    ;

funcdecl:               beginfunc ')' { SOP( FUNC_PARMCLEANUP ); } statementblock
                        { SOP( FUNC_END ); }
                    |   beginfunc varlist ')' { SOP( FUNC_PARMCLEANUP ); } statementblock
                        { SOP( FUNC_END ); }
                    ;

beginfuncexpr:          TOKEN_FUNCTION '(' { SOP( FUNC_BEGIN_EXPR ); } 
                    ;

funcexpr:               beginfuncexpr ')' { SOP( FUNC_PARMCLEANUP ); } statementblock
                        { SOP( FUNC_END ); }
                    |   beginfuncexpr varlist ')' { SOP( FUNC_PARMCLEANUP ); } statementblock
                        { SOP( FUNC_END ); }
                    ;



varlist:                TOKEN_SYMBOL
                        { SOPS( PUSH_VARNAME, yylval.text ); SOP( FUNC_ASSIGNPARM ); }
                    |   varlist ',' TOKEN_SYMBOL
                        { SOPS( PUSH_VARNAME, yylval.text ); SOP( FUNC_ASSIGNPARM ); }
                    ;

statementblock:         '{' '}'
                    |   '{' statementlist '}'
                    |   '{' statementlist '}' ';'
                    ;

statementlist:          statementorblock
                    |   statementlist statementorblock
                    ;

switch:                 TOKEN_SWITCH '(' expr ')' { SOP( SWITCH_START ); } '{' caselist '}'
                        { SOP( EX_BREAK ); SOP( SWITCH_END ); }
                    ;

caselist:               case
                    |   caselist case
                    ;

case:                   TOKEN_CASE { SOP( CASE_START ); } expr ':' { SOP( OP_EQ ); SOP( CASE ); } casestatementorblock { SOP( CASE_END ); }
                    |   TOKEN_DEFAULT ':' { SOP( CASE_DEF ); } casestatementorblock { SOP( CASE_END ); }
                    ;

casestatementorblock:    statementlist
                    |   /* Nothing */
                    ;

beginif:            { SOP( EX_IF ); } statementorblock

beginfor:               TOKEN_FOR '('

forinit:                vardecl ';'
                    |    expr ';'
                    |    ';'
                    ;

forcond:                expr ';'
                    |    { int fv = 1; SOPD( PUSH_BOOL, &fv, sizeof( fv ) ); }
                    ;

forend:                 expr ')'
                        { SOP( OP_POPVAL ); }
                    |    ')'
                    ;

statement:              vardecl
                    |   funcdecl
                    |   beginfor forinit { SOP( FOR_PRECOND ); } forcond { SOP( FOR_COND ); } forend { SOP( FOR_BODY ); } statementorblock
                        { SOP( FOR_END  ); }
                    |   beginfor TOKEN_VAR TOKEN_SYMBOL TOKEN_IN expr ')' { SOPS( PUSH_VARNAME, $3.text ); SOP( FORIN_BUILDPROPLIST ); SOP( FORIN_SET ); } statementorblock { SOP( FORIN_END ); }
                    |   beginfor TOKEN_SYMBOL TOKEN_IN expr ')' { SOPS( PUSH_VARNAME, $2.text ); SOP( FORIN_BUILDPROPLIST ); SOP( FORIN_SET ); } statementorblock { SOP( FORIN_END ); }
                    |   switch
                    |   TOKEN_DO { SOP( EX_DOBEGIN ); } statementorblock TOKEN_WHILE '(' { SOP( EX_DOPRECOND ); } expr ')' { SOP( EX_DOEND ); }
                    |   TOKEN_WHILE { SOP( EX_WHILEBEGIN ); } '(' expr ')' { SOP( EX_WHILE ); } statementorblock
                        { SOP( EX_WHILEEND ); }
                    |   TOKEN_BREAK
                        { SOP( EX_BREAK ); }
                    |   TOKEN_CONTINUE
                        { SOP( EX_CONTINUE ); }
                    |   TOKEN_IF '(' expr ')' beginif TOKEN_ELSE { SOP( EX_ELSE ); } statementorblock
                        { SOP( EX_ENDIF ); }
                    |   TOKEN_IF '(' expr ')' beginif TOKEN_ELSE { /* nothing..., bug workaround */ } %prec PREC_LOWER_THAN_ELSE
                        { SOP( EX_ENDIF ); }
                    |   TOKEN_IF '(' expr ')' beginif %prec PREC_LOWER_THAN_ELSE
                        { SOP( EX_ENDIF ); }
                    |   TOKEN_ELSE statementorblock
                        { yyerror( "else without if" ); }
                    |   TOKEN_RETURN expr %prec PREC_STATEMENT
                        { SOP( FUNC_RETURN ); }
                    |   TOKEN_RETURN %prec PREC_LESS_THAN_STATEMENT
                        { SOP( FUNC_RETURN_NOVAL ); }
                    |   TOKEN_WITH '(' expr ')' { SOP( PUSH_OCONTEXT ); } statementorblock
                        { SOP( POP_OCONTEXT ); }
                    |   TOKEN_DEBUG '(' expr ')'
                        { SOP( EX_DEBUG ); }
                    |   TOKEN_DEBUGESTACK '(' ')'
                        { SOP( EX_DEBUGESTACK ); }
                    |   TOKEN_VOID expr
                        { SOP( OP_POPVAL ); }
                    |   TOKEN_DELETE lvalue
                        { SOP( DELETE ); }
                    |   expr %prec PREC_STATEMENT
                        { SOP( OP_POPVAL ); }
                    |   TOKEN_MINUSMINUS '>'
                        { /* Just a hack to deal with --> HTML comment delimiter in JS code */ }
                    ;

statementorblock:       statementblock
                    |   termstatement
                    ;

termstatement:          statement ';'
                    |   statement
                    |   ';'
                    ;

vardecl:                TOKEN_VAR vardecllist
                    ;

vardecllist:            vardecllist ',' vardeclitem
                    |   vardeclitem
                    ;

vardeclpart:            TOKEN_SYMBOL
                        { SOPS( PUSH_VARNAME, yylval.text ); }

vardeclitem:            vardeclpart
                        { SOP( SETIFUNSET ); }
                    |   vardeclpart '=' expr
                        { SOP( ASSIGNLOCAL ); SOP( OP_POPVAL ); }
                    |   TOKEN_UNDEFINED
                        { /* Ignore this crap */ }
                    ;

regexp:                 '/' { yy_push_state( REGEXP ); } TOKEN_REGEXP
                        {
                            { 
                                char *tmp = alloca( strlen( yylval.text ) + 3 );
                                tmp[ 0 ] = yylval.re_flags_i ? 'i' : ' ';
                                tmp[ 1 ] = yylval.re_flags_g ? 'g' : ' ';
                                strcpy( &tmp[ 2 ], yylval.text );
                                SOPS( CREATE_REGEXP, tmp ); 
                            }
                            yy_pop_state();
                        }

expr:                   '(' expr ')'
                    |   functioncall
                    |   funcexpr
                    |   lvalue
                        { SOP( EVAL_LVALUE ); }
                    |   TOKEN_UNDEFINED
                        { SOP( PUSH_UNDEFINED ); }
                    |   expr '+' expr
                        { SOP( OP_ADD ); }
                    |   expr '-' expr
                        { SOP( OP_SUB ); }
                    |   regexp
                    |   '-' expr %prec PREC_UMINUS
                        { SOP( OP_NEGATE ); }
                    |   '+' expr
                        { /* Unary + -- noop */ }
                    |   expr TOKEN_LEQ expr
                        { SOP( OP_LTEQ ); }
                    |   expr TOKEN_GEQ expr
                        { SOP( OP_GTEQ ); }
                    |   expr '<' expr
                        { SOP( OP_LT ); }
                    |   expr '>' expr
                        { SOP( OP_GT ); }
                    |   expr TOKEN_EQEQ expr
                        { SOP( OP_EQ ); }
                    |   expr TOKEN_STRONGEQ expr
                        { SOP( OP_STREQ ); }
                    |   expr TOKEN_NOTEQ expr
                        { SOP( OP_EQ ); SOP( OP_BOOLNEG ); }
                    |   expr TOKEN_STRONGNOTEQ expr
                        { SOP( OP_STREQ ); SOP( OP_BOOLNEG ); }
                    |   expr '*' expr
                        { SOP( OP_MUL ); }
                    |   expr '/' expr
                        { SOP( OP_DIV ); }
                    |   expr '%' expr
                        { SOP( OP_MOD ); }
                    |   expr TOKEN_OROR { SOP( OP_BOOLOR ); } expr
                        { SOP( OP_BOOLOR_END ); }
                    |   expr TOKEN_ANDAND { SOP( OP_BOOLAND ); } expr
                        { SOP( OP_BOOLAND_END ); }
                    |   expr '?' { SOP( OP_SELECT ); } expr ':' { SOP( OP_SELECT_SKIP ); } expr
                        { SOP( OP_SELECT_END ); }
                    |   '!' expr
                        { SOP( OP_BOOLNEG ); }
                    |   expr TOKEN_GG expr
                        { SOP( OP_GG ); }
                    |   expr TOKEN_LL expr
                        { SOP( OP_LL ); }
                    |   expr TOKEN_GGG expr
                        { SOP( OP_GGG ); }
                    |   expr '&' expr
                        { SOP( OP_BINAND ); }
                    |   expr '|' expr
                        { SOP( OP_BINOR ); }
                    |   expr '^' expr
                        { SOP( OP_BINEOR ); }
                    |   '~' expr
                        { SOP( OP_BINNEG ); }
                    |   TOKEN_INTEGER
                        { SOPD( PUSH_INT, &yylval.integer, sizeof( yylval.integer ) ); }
                    |   TOKEN_REAL
                        { SOPD( PUSH_REAL, &yylval.real, sizeof( yylval.real ) ); }
                    |   TOKEN_STRING
                        { SOPS( PUSH_STR, yylval.text ); }
                    |   TOKEN_INF
                        {
                            ULONG v[] = { 0x7ff00000, 0 };
                            { SOPD( PUSH_REAL, v, sizeof( v ) ); }
                        }
                    |   TOKEN_NAN
                        {
                            double v;
                            memset( &v, 0xff, sizeof( v ) );
                            { SOPD( PUSH_REAL, &v, sizeof( v ) ); }
                        }
                    |   TOKEN_FALSE
                        { int fv = 0; SOPD( PUSH_BOOL, &fv, sizeof( fv ) ); }
                    |   TOKEN_TRUE
                        { int fv = 1; SOPD( PUSH_BOOL, &fv, sizeof( fv ) ); }
                    |   TOKEN_NULL
                        { SOP( PUSH_NULL ); }
                    |   TOKEN_TYPEOF expr
                        { SOP( OP_TYPEOF ); }
                    |   expr ',' { SOP( OP_POPVAL ); } expr %prec PREC_LOWER_THAN_ELSE
                    |   TOKEN_EVAL '(' expr ')'
                        { SOP( EVAL ); }
                    |   TOKEN_ISNAN '(' expr ')'
                        { SOP( ISNAN ); }
                    |   TOKEN_ISFINITE '(' expr ')'
                        { SOP( ISFINITE ); }
                    |   TOKEN_PARSEINT '(' expr ')'
                        { int fv = 0; SOPD( PUSH_INT, &fv, sizeof( fv ) ); SOP( PARSEINT ); }
                    |   TOKEN_PARSEINT '(' expr ',' expr ')'
                        { SOP( PARSEINT ); }
                    |   TOKEN_PARSEFLOAT '(' expr ')'
                        { SOP( PARSEFLOAT ); }
                    |   TOKEN_STRINGFUNC '(' expr ')'
                        { SOP( MAKESTRING ); }
                    |   TOKEN_NUMBER '(' expr ')'
                        { SOP( MAKENUMBER ); }
                    |   TOKEN_ESCAPE '(' expr ')'
                        { SOP( ESCAPE ); }
                    |   TOKEN_UNESCAPE '(' expr ')'
                        { SOP( UNESCAPE ); }
                    |   TOKEN_NEW newsym '(' { SOP( FUNC_BEGINPARMS ); } exprlist ')' { SOP( NEW ); SOP( NEW_END ); }
                    |   TOKEN_NEW newsym '(' { SOP( FUNC_BEGINPARMS ); } ')' { SOP( NEW ); SOP( NEW_END ); }
                    |   TOKEN_NEW newsym { SOP( FUNC_BEGINPARMS ); SOP( NEW ); SOP( NEW_END ); }
                    |   lvalue '=' expr
                        { SOP( ASSIGN ); }
                    |   expr '.' TOKEN_SYMBOL  '=' { SOPS( PUSH_REFOP, yylval.text ); } expr { SOP( ASSIGN ); }
                    |   TOKEN_PLUSPLUS lvalue
                        {
                          SOP( EVAL_LVALUE_AND_KEEP );
                          SOP( OP_ADDONE );
                          SOP( ASSIGN );
                        }
                    |   TOKEN_MINUSMINUS lvalue
                        {
                          SOP( EVAL_LVALUE_AND_KEEP );
                          SOP( OP_SUBONE );
                          SOP( ASSIGN );
                        }
                    |   lvalue TOKEN_PLUSPLUS
                        {
                          SOP( EVAL_LVALUE_AND_KEEP ); // later used
                          SOP( MAKENUMBER );
                          SOP( EVAL_LVALUE_AND_KEEP );
                          SOP( OP_ADDONE );
                          SOP( ASSIGN );
                          SOP( OP_POPVAL );
                        }
                    |   lvalue TOKEN_MINUSMINUS
                        {
                          SOP( EVAL_LVALUE_AND_KEEP ); // later used
                          SOP( MAKENUMBER );
                          SOP( EVAL_LVALUE_AND_KEEP );
                          SOP( OP_SUBONE );
                          SOP( ASSIGN );
                          SOP( OP_POPVAL );
                        }
                    |   lvalue TOKEN_PLUSEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_ADD ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_MINUSEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_SUB ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_TIMESEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_MUL ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_DIVEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_DIV ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_MODEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_MOD ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_GGEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_GG ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_LLEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_LL ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_GGGEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_GGG ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_ANDEQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_BINAND ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_OREQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_BINOR ); SOP( ASSIGN ); }
                    |   lvalue TOKEN_XOREQ { SOP( EVAL_LVALUE_AND_KEEP ); } expr
                        { SOP( OP_BINEOR ); SOP( ASSIGN ); }
                    |   expr '.' refop
                    |   expr '[' expr ']' %prec PREC_HIGHEST
                        { SOP( PUSH_ARRAYOP ); SOP( EVAL_LVALUE ); }
                    |   '{' { SOP( CREATEOBJECT ); } objectinitlist '}' { SOP( POP_OCONTEXT ); }
                    |   '[' { SOP( CREATEARRAY ); } arrayinitlist ']' { SOP( POP_OCONTEXT ); }
                    ;

refop:                  TOKEN_SYMBOL %prec PREC_HIGHEST
                        { SOPS( PUSH_REFOP, yylval.text ); SOP( EVAL_LVALUE ); }
                    ;

objectinitlist:         objectinit %prec PREC_LESS_THAN_STATEMENT
                    |   objectinitlist ',' objectinit %prec PREC_STATEMENT
                    ;

objectinit:             TOKEN_SYMBOL { SOPS( PUSH_VARNAME, yylval.text ); } ':' expr
                        { SOP( ASSIGNPROP ); }
                    |   TOKEN_INTEGER { { char buffer[ 32 ]; sprintf( buffer, "%d", yylval.integer ); SOPS( PUSH_VARNAME, buffer ); } } ':' expr
                        { SOP( ASSIGNPROP ); }
                    ; 

arrayinitlist:          arrayinitlist ',' arrayinit %prec PREC_STATEMENT
                    |   arrayinit %prec PREC_LESS_THAN_STATEMENT
                    ;

arrayinit:              expr %prec PREC_LESS_THAN_STATEMENT
                        { SOP( ASSIGNARRAY ); }
                    |   /* empty */ %prec PREC_STATEMENT
                        { SOPS( PUSH_STR, "" ); SOP( ASSIGNARRAY ); }
                    ;

functioncall:           expr '(' { SOP( FUNC_BEGINPARMS ); } funccallend
                    ;

funccallend:            ')'
                        { SOP( FUNC_CALL ); }
                    |   exprlist ')'
                        { SOP( FUNC_CALL ); }
                    ;

lvalue:                 symbol
                    |   lvalue '.' TOKEN_SYMBOL
                        { SOP( EVAL_LVALUE_CHECK ); SOPS( PUSH_REFOP, yylval.text ); }
                    |   lvalue '[' { SOP( EVAL_LVALUE_CHECK ); } expr ']' { SOP( PUSH_ARRAYOP ); }
                    ;

symbol:                 TOKEN_SYMBOL
                        { SOPS( PUSH_VARNAME, yylval.text ); }
                    |   TOKEN_THIS
                        { SOPS( PUSH_VARNAME, "this" ); }
                    |   TOKEN_STRINGFUNC
                        { SOPS( PUSH_VARNAME, "String" ); }
                    ;

exprlist:               exprlist ',' expr %prec PREC_STATEMENT
                    |   expr %prec PREC_LESS_THAN_STATEMENT
                    ;

newsym:                 TOKEN_SYMBOL
                        { SOPS( PUSH_VARNAME, yylval.text ); }
                    |   TOKEN_STRINGFUNC
                        { SOPS( PUSH_VARNAME, "String" ); }
                    |   TOKEN_NUMBER
                        { SOPS( PUSH_VARNAME, "Array" ); }
                    ;

%%
