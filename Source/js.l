%{

/* $Id: js.l,v 1.36 2003/04/25 19:13:54 zapek Exp $ */

extern int js_lineno;
char *js_buffptr;
int js_bufflen;

static int addbuffer(char);
void yyerror(char *);
static char *buf;
static int buflen;
static int maxbuflen;
#define BUFINC  1024

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

%option stack
%option noyy_top_state
%option noyy_scan_buffer
%option noyy_scan_bytes
%option noyy_scan_string
%option noyywrap

%x COMMENT
%x DQUOTED
%x SQUOTED
%x REGEXP

ws [ \xa0\t]+
digit [0-9]
octdigit [0-7]
hexdigit [0-9a-fA-F]
alpha [A-Za-z]
alphanum [0-9A-Za-z]
varfirst [A-Za-z_$]
varmiddle [A-Za-z0-9_$]
%%

<REGEXP>{
"\\\/"                  { if(!addbuffer('/')) return 0; }
"/"                     { yylval->text = buf; yylval->re_flags_g = 0; yylval->re_flags_i = 0; return TOKEN_REGEXP; }
"/g"                    { yylval->text = buf; yylval->re_flags_g = 1; yylval->re_flags_i = 0; return TOKEN_REGEXP; }
"/i"                    { yylval->text = buf; yylval->re_flags_g = 0; yylval->re_flags_i = 1; return TOKEN_REGEXP; }
\/("gi"|"ig")           { yylval->text = buf; yylval->re_flags_g = 1; yylval->re_flags_i = 1; return TOKEN_REGEXP; }
\n                      {
                             yyerror("unterminated regular expression");
                             return 0;
                        }
.                       { if(!addbuffer(yytext[0])) return 0; }
}
<SQUOTED>\'                     |
<DQUOTED>\"                     {
                                    BEGIN INITIAL;
                                    yylval->text=buf;
                                    return TOKEN_STRING;
                                }
<SQUOTED>\\\\                   |
<DQUOTED>\\\\                   {if(!addbuffer('\\')) return 0;}
<SQUOTED>\\\'                   |
<DQUOTED>\\\'                   {if(!addbuffer('\'')) return 0;}
<SQUOTED>\\\"                   |
<DQUOTED>\\\"                   {if(!addbuffer('"')) return 0;}
<SQUOTED>\\t                    |
<DQUOTED>\\t                    {if(!addbuffer('\t')) return 0;}
<SQUOTED>\\r                    |
<DQUOTED>\\r                    {if(!addbuffer('\r')) return 0;}
<SQUOTED>\\b                    |
<DQUOTED>\\b                    {if(!addbuffer('\b')) return 0;}
<SQUOTED>\\f                    |
<DQUOTED>\\f                    {if(!addbuffer('\f')) return 0;}
<SQUOTED>\\n                    |
<DQUOTED>\\n                    {if(!addbuffer('\n')) return 0;}
<SQUOTED>\\.                    |
<DQUOTED>\\.                    {if(!addbuffer(yytext[1])) return 0;}
<SQUOTED>\\\n                   |
<DQUOTED>\\\n                   { /* Skip this, line continuation */ };
<SQUOTED>\n                     |
<DQUOTED>\n                     {
                                    yyerror("unterminated string constant");
                                    return 0;
                                }
<SQUOTED>.                      |
<DQUOTED>.                      {if(!addbuffer(yytext[0])) return 0;}
<COMMENT>[^*\n]*           		;
<COMMENT>"*"+[^\/\n]*            ;
<COMMENT>\n                     {js_lineno++;}
<COMMENT>\*\/                   {BEGIN INITIAL; }
\<!--.*--\>						;
\-\-\-\>						;
\n                              |
\<!--.*\n                      	|
\/\/.*\n                        |
\/\/.*                          {js_lineno++;}
\/\*                            {BEGIN COMMENT; }
break            {return TOKEN_BREAK;}
continue         {return TOKEN_CONTINUE;}
else             {return TOKEN_ELSE;}
false            {return TOKEN_FALSE;}
for              {return TOKEN_FOR;}
debug            {return TOKEN_DEBUG;}
function         {return TOKEN_FUNCTION;}
if               {return TOKEN_IF;}
in               {return TOKEN_IN;}
new              {return TOKEN_NEW;}
delete           {return TOKEN_DELETE;}
null             {return TOKEN_NULL;}
return           {return TOKEN_RETURN;}
do               {return TOKEN_DO;}
this             {return TOKEN_THIS;}
void             {return TOKEN_VOID;}
debugestack      {return TOKEN_DEBUGESTACK;}
typeof           {return TOKEN_TYPEOF;}
true             {return TOKEN_TRUE;}
undefined        {return TOKEN_UNDEFINED;}
NaN              {return TOKEN_NAN;}
Infinity         {return TOKEN_INF;}
var              {return TOKEN_VAR;}
while            {return TOKEN_WHILE;}
with             {return TOKEN_WITH;}
switch			 {return TOKEN_SWITCH;}
case			 {return TOKEN_CASE;}
default          {return TOKEN_DEFAULT;}
eval             {return TOKEN_EVAL;}
isNaN            {return TOKEN_ISNAN;}
isFinite		 {return TOKEN_ISFINITE;}
parseInt         {return TOKEN_PARSEINT;}
parseFloat       {return TOKEN_PARSEFLOAT;}
String       	{return TOKEN_STRINGFUNC;}
Number       	{return TOKEN_NUMBER;}
escape       	{return TOKEN_ESCAPE;}
unescape       	{return TOKEN_UNESCAPE;}
\<\=                            {return TOKEN_LEQ;}
\>\=                            {return TOKEN_GEQ;}
\=\=                            {return TOKEN_EQEQ;}
\=\=\=                          {return TOKEN_STRONGEQ;}
\+\=                            {return TOKEN_PLUSEQ;}
\-\=                            {return TOKEN_MINUSEQ;}
\*\=                            {return TOKEN_TIMESEQ;}
\/\=                            {return TOKEN_DIVEQ;}
\!\=                            {return TOKEN_NOTEQ;}
\!\=\=                          {return TOKEN_STRONGNOTEQ;}
\%\=                            {return TOKEN_MODEQ;}
\&\=                            {return TOKEN_ANDEQ;}
\|\=                            {return TOKEN_OREQ;}
\^\=                            {return TOKEN_XOREQ;}
\>\>\>\=                        {return TOKEN_GGGEQ;}
\>\>\=                          {return TOKEN_GGEQ;}
\<\<\=                          {return TOKEN_LLEQ;}
\>\>\>                          {return TOKEN_GGG;}
\>\>                            {return TOKEN_GG;}
\<\<                            {return TOKEN_LL;}
\+\+                            {return TOKEN_PLUSPLUS;}
\-\-                            {return TOKEN_MINUSMINUS;}
\&\&                            {return TOKEN_ANDAND;}
\|\|                            {return TOKEN_OROR;}
\/								{maxbuflen=buflen=0;return '/';}
[\(\)\{\}\[\]\;\.\,\:\=\+\*\-]  |
[\?\<\>\!\%\&\|\^\~]          {return yytext[0];}
[\'\"]                          {
                                    buf=alloca(BUFINC);
                                    if(!buf) {
                                        yyerror("out of memory");
                                        return 0;
                                    }
                                    maxbuflen=BUFINC;
                                    *buf=0;
                                    buflen=0;
                                    if(yytext[0]=='\'')
                                        BEGIN SQUOTED;
                                    else
                                        BEGIN DQUOTED;
                                }
\$[0-9]							|
\$\*							|
\$_								|
\$\+							|
\$\&							|
\$\`							|
\$\'							|
{varfirst}{varmiddle}*          {
                                    buf=alloca(yyleng+1);
                                    if(!buf) {
                                        yyerror("not enough memory");
                                        return 0;
                                    }
                                    strcpy(buf,yytext);
                                    yylval->text=buf;
                                    return TOKEN_SYMBOL;
                                }
{digit}*\.{digit}+([eE][\+\-]?{digit}+)?    |
{digit}+\.{digit}*([eE][\+\-]?{digit}+)?    |
{digit}+[eE][\+\-]?{digit}+     {
                                    double val=0,mul;
                                    char c,*cp=yytext;
                                    int sign=1,eval;
                                    while((c=*cp++)&&(c>='0')&&(c<='9'))
                                        val=val*10+(c-'0');
                                    if(c=='.') {
                                        mul=0.1;
                                        while((c=*cp++)&&(c>='0')&&(c<='9')) {
                                            val+=(c-'0')*mul;
                                            mul/=10;
                                        }
                                    }
                                    if((c=='e')||(c=='E')) {
                                        c=*cp++;
                                        if(c=='-')
                                            sign=-1;
                                        else if(c!='+')
                                            cp--;
                                        eval=0;
                                        while((c=*cp++)&&(c>='0')&&(c<='9'))
                                            eval=eval*10+(c-'0');
                                        val*=exp(log(10.0)*eval*sign);
                                    }
                                    yylval->real=val;
                                    return TOKEN_REAL;
                                }
0[xX]{hexdigit}+                {
                                    double val=0;
                                    char *cp=yytext+2,c;
                                    while(c=*cp++) 
									{
                                        val*=16.0;
                                        if((c>='0')&&(c<='9'))
                                            val+=c-'0';
                                        else if((c>='A')&&(c<='F'))
                                            val+=c-'A'+10;
                                        else
                                            val+=c-'a'+10;
                                    }
                                    yylval->real=val;
                                    return TOKEN_REAL;
                                }
{digit}+			{
					if( yytext[ 0 ] == '0' )
					{
						// Check for a valid octal sequence, and convert
						char *cp = yytext + 1;
						double val = 0;
						char c;
						while( c = *cp++ )
						{
							if( c >= '8' )
								break; // Invalid octal sequence, abort, and handle it as decimal
							val = val * 8 + ( c - '0' );
						}
						if( !c )
						{
							yylval->real = val;
							return TOKEN_REAL;
						}
					}

					if( strlen( yytext ) < 8 )
					{
						yylval->integer=atoi( yytext );
						return TOKEN_INTEGER;
					}
					else
					{
						yylval->real = atof( yytext );
						return TOKEN_REAL;
					}
                                }
{ws}                            ;
\\{ws}\n                        {js_lineno++;}
.                               {
                                    char buffer[ 128 ];

                                    sprintf( buffer, "invalid lexical token: '%.32s' (%c)", yytext, yytext[ 0 ] );
                                    yyerror( buffer );
                                    return 0;
                                }
%%
static int addbuffer(char c) {
    char *buf2;
    if(buflen>maxbuflen-5) {
        buf2=alloca(maxbuflen+BUFINC);
        if(!buf2) {
            yyerror("not enough memory");
            return 0;
        }
        memcpy(buf2,buf,maxbuflen);
        buf=buf2;
        maxbuflen=maxbuflen+BUFINC;
    }
    buf[buflen++]=c;
    buf[buflen]=0;
    return 1;
}

void lex_initialize( char *ptr, int len, int lineno ) 
{
    yy_init=1;
    js_buffptr = ptr;
    js_bufflen = len;
    js_lineno = lineno;
    yyrestart( 0 );
    BEGIN( INITIAL );
}

static int my_yyinput( char *buf, int max ) 
{
    int cnt = 0;

    while( max && ( js_bufflen-- > 0 ) )
    {
        if( *js_buffptr == '\r' )
        {
            js_buffptr++;
            if( *js_buffptr == '\n' )
                continue;
            else
                *buf++ = '\n';
        }
        else
            *buf++ = *js_buffptr++;
        cnt++;
        max--;
    }
    return( cnt );  
}
