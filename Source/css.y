%{

/*
** $Id: css.y,v 1.9 2001/07/02 19:43:43 owagner Exp $
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
int cssparse (void);
void csserror(char *);

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
#endif /* !__MORPHOS__ */
#include "css_scanner.h"
%}

%token CSSTOKEN_IDENT
%token CSSTOKEN_STRING CSSTOKEN_ATKEYWORD
%token CSSTOKEN_HASH
%token CSSTOKEN_NUMBER
%token CSSTOKEN_PERCENTAGE
%token CSSTOKEN_DIMENSION
%token CSSTOKEN_URI
%token CSSTOKEN_UNICODE_RANGE
%token CSSTOKEN_S
%token CSSTOKEN_FUNCTION
%token CSSTOKEN_INCLUDES
%token CSSTOKEN_DASHMATCH
%token CSSTOKEN_DELIM

%%

stylespec: sheet
        | decllist
        ;

sheet: { }
            | statementlist
            ;

statementlist:      statement
                |   statement statementlist
                ;

statement: selector declblock


selector: CSSTOKEN_IDENT
        | selector '.' CSSTOKEN_IDENT
        ;

decl: CSSTOKEN_IDENT


declstatement:          decl ';'
                    |   decl
                    |   ';'
                    ;

decllist:           declstatement
                |   declstatement decllist
                ;

declblock:        '{' '}'
            | '{' decllist '}'
            ;

%%
