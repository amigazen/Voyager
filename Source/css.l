%{

/* $Id: css.l,v 1.7 2001/07/02 19:43:43 owagner Exp $ */

#define YY_INPUT(b,r,m) r=my_yyinput(b,m)
static int my_yyinput(char *,int);

#define ECHO
#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1

#define YY_FATAL_ERROR(x) yyerror(x)

static void dummy( void ) { }
static int input( void );

#define fprintf(a,b,c) dummy()

%}

%option noinput
%option nounput
%option noyy_push_state
%option noyy_pop_state
%option noyy_top_state
%option noyy_scan_buffer
%option noyy_scan_bytes
%option noyy_scan_string
%option noyywrap

%x COMMENT
%x DQUOTED
%x SQUOTED

ident 		{nmstart}{nmchar}*
name 		{nmchar}+
nmstart 	[a-zA-Z]|{nonascii}|{escape}
nonascii	[^\0-\177]
unicode		\\[0-9a-f]{1,6}[ \n\r\t\f]?
escape		{unicode}
nmchar		[a-z0-9-]|{nonascii}|{escape}
num 		[0-9]+|[0-9]*\.[0-9]+
string1 	\"([\t !#$%&(-~]|\\{nl}|\'|{nonascii}|{escape})*\"
string2 	\'([\t !#$%&(-~]|\\{nl}|\"|{nonascii}|{escape})*\' 
string		{string1}|{string2}
nl 			\n|\r\n|\r|\f
w			[ \t\r\n\f]*
%%

\<!--.*\n                      	|
\<!--                       	|
--\>			{/* HTML comment, skip*/}

{ident}         { return CSSTOKEN_IDENT; }
@{ident}		{ return CSSTOKEN_ATKEYWORD; }
{string}        { return CSSTOKEN_STRING; }
#{name}			{ return CSSTOKEN_HASH; }
{num}			{ return CSSTOKEN_NUMBER; }
{num}%			{ return CSSTOKEN_PERCENTAGE; }
{num}{ident}	{ return CSSTOKEN_DIMENSION; }
url\({w}{string}{w}\)|url\({w}([!#$%&*-~]|{nonascii}|{escape})*{w}\)		{ return CSSTOKEN_URI; } 
U\+[0-9A-F?]{1,6}(-[0-9A-F]{1,6})?		{ return CSSTOKEN_UNICODE_RANGE; } 
;				{ return ';'; } 
\{				{ return '{'; } 
\}				{ return '}'; }
\(				{ return '('; } 
\)				{ return ')'; } 
\[				{ return '['; } 
\]				{ return ']'; } 
[ \t\r\n\f]+	{ return CSSTOKEN_S; } 
\/\*[^*]*\*+([^/][^*]*\*+)*\/		{ /* Eat comment */ } 
{ident}\(		{ return CSSTOKEN_FUNCTION; } 
~=				{ return CSSTOKEN_INCLUDES; } 
\|=				{ return CSSTOKEN_DASHMATCH; } 
.				{ return CSSTOKEN_DELIM; } 
%%

static int my_yyinput( char *buf, int max ) 
{
    return( 0 );  
}

void csserror(char*msg)
{

}
