%{

// this will be added to the top of your lexer.c file

#include "bridge.hpp"

#define GET_COLUMN() yyget_extra(yyscanner)
#define SET_COLUMN(x) yyset_extra((x), yyscanner)
#define INC_COLUMN(x) yyset_extra(GET_COLUMN() + (x), yyscanner)

#define LOC(x) (yy::parser::location_type { nullptr, yylineno, (x) })

#define TOK(t, x, ...) \
	make_symbol(yylval, yy::parser::make_ ##t (__VA_ARGS__ __VA_OPT__(,) LOC(x)))

#define yyterminate() return TOK(YYEOF, GET_COLUMN())

static int make_symbol(YYSTYPE*, YYSTYPE);

%}

/* see https://westes.github.io/flex/manual/Scanner-Options.html */

%option header-file="lexer.h"

%option yylineno
%option noyywrap

%option bison-bridge
%option bison-locations
%option reentrant
%option nounput
%option extra-type="int"

%option debug
%option nodefault
%option verbose
%option warn

WS [ \t]
NL [\r\n]
DIGIT [0-9]+
LETTER [a-zA-Z0-9]
IDENTIFIER [_A-Za-z][_0-9A-Za-z]*
FLOAT [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?

TRUE true

%%

{WS} { INC_COLUMN(1);}
{NL} { SET_COLUMN(1);}

{DIGIT} { int x = GET_COLUMN(); INC_COLUMN(yyleng); return TOK(TOK_INTEGER, x, std::string { yytext, static_cast<size_t>(yyleng) }); }
{FLOAT} { int x = GET_COLUMN(); INC_COLUMN(yyleng); return TOK(TOK_FLOAT, x, std::string { yytext, static_cast<size_t>(yyleng) }); }
"true" { int x = GET_COLUMN(); INC_COLUMN(4); return TOK(TOK_TRUE, x); }
"false" { int x = GET_COLUMN(); INC_COLUMN(5); return TOK(TOK_FALSE, x); }
"(" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_LPAREN, x); }
")" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_RPAREN, x); }
"{" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_LBRACE, x); }
"}" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_RBRACE, x); }
"==" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_EQ, x); }
"!=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_NE, x); }
"<" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_LT, x); }
">" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_GT, x); }
"<=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_LE, x); }
">=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_GE, x); }
"+" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_PLUS, x); }
"-" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_MINUS, x); }
"*" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_STAR, x); }
"/" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_SLASH, x); }
"&" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_LOG_AND, x); }
"||" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_LOG_OR, x); }
"if" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_IF, x); }
"while" { int x = GET_COLUMN(); INC_COLUMN(5); return TOK(TOK_WHILE, x); }
"for" { int x = GET_COLUMN(); INC_COLUMN(3); return TOK(TOK_FOR, x); }
"break" { int x = GET_COLUMN(); INC_COLUMN(5); return TOK(TOK_BREAK, x); }
"continue" { int x = GET_COLUMN(); INC_COLUMN(8); return TOK(TOK_CONTINUE, x); }
"return" { int x = GET_COLUMN(); INC_COLUMN(6); return TOK(TOK_RETURN, x); }
"," { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_COMMA, x); }
";" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_SEMICOLON, x); }
":" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_COLON, x); }
"?" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_QUESTION_MARK, x); }
"=" { int x = GET_COLUMN(); INC_COLUMN(1); return TOK(TOK_ASSIGN, x); }
"+=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_PLUS_ASSIGN, x); }
"-=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_MINUS_ASSIGN, x); }
"*=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_STAR_ASSIGN, x); }
"/=" { int x = GET_COLUMN(); INC_COLUMN(2); return TOK(TOK_SLASH_ASSIGN, x); }
"int" { int x = GET_COLUMN(); INC_COLUMN(3); return TOK(TOK_TYPE, x); }
"float" { int x = GET_COLUMN(); INC_COLUMN(5); return TOK(TOK_TYPE, x); }
"bool" { int x = GET_COLUMN(); INC_COLUMN(4); return TOK(TOK_TYPE, x); }
"void" { int x = GET_COLUMN(); INC_COLUMN(4); return TOK(TOK_TYPE, x); }

{IDENTIFIER} {int x = GET_COLUMN(); INC_COLUMN(yyleng); return TOK(TOK_IDENTIFIER, x, std::string { yytext, static_cast<size_t>(yyleng) }); }



. { printf("[error] invalid token.\n"); return TOK(YYUNDEF, GET_COLUMN()); }

%%

int make_symbol(YYSTYPE* val, YYSTYPE sym) {
	val->move(sym);
	return 1;
}
