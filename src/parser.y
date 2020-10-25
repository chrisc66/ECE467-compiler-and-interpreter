%code requires {

// this will be added to your parser.hpp file

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include <memory>

class Node;

}

%code {

// this will be added to your parser.cpp file

#include "lexer.hpp"
#include "nodes.hpp"

static yy::parser::symbol_type yylex(yyscan_t);

template <typename T, typename... Args> static std::unique_ptr<T> make_node(yy::parser::location_type const&, Args&&...);

}

/* see https://www.gnu.org/software/bison/manual/html_node/Declarations.html */

%require "3.6"
%language "c++"
%locations
%param { yyscan_t lexer }
%parse-param { std::unique_ptr<Node>& root }
%verbose
%define api.value.type variant
%define api.token.constructor
%define parse.trace
%define parse.assert

%token <std::string> HI
%token BYE

//identifier, integer, float
%token <std::string> TOK_IDENTIFIER
%token <std::string> TOK_INTEGER 
%token <std::string> TOK_FLOAT

//true, false
%token TOK_TRUE
%token TOK_FALSE

//lparen, rparen, lbrace, rbrace
%token TOK_LPAREN 
%token TOK_RPAREN
%token TOK_LBRACE
%token TOK_RBRACE

//eq, ne, lt, gt, le, ge
%token TOK_EQ
%token TOK_NE
%token TOK_LT
%token TOK_GT
%token TOK_LE
%token TOK_GE

//plus, minus, star, slash
%token TOK_PLUS
%token TOK_MINUS
%token TOK_STAR
%token TOK_SLASH

//log_and, log_or
%token TOK_LOG_AND
%token TOK_LOG_OR

//if, while, for, break, continue, return
%token TOK_IF
%token TOK_WHILE
%token TOK_FOR
%token TOK_BREAK
%token TOK_CONTINUE
%token TOK_RETURN

//comma, semicolon, colon,question_mark
%token TOK_COMMA
%token TOK_SEMICOLON
%token TOK_COLON
%token TOK_QUESTION_MARK

//assign
%token TOK_ASSIGN




%type <Node*> root

%start root

%%

root
	: declarations { $$ = nullptr; }
	;

declarations
	: declarations decl
	| %empty
	;

decl
	: HI { printf("decl: %s\n", $1.c_str()); }
	;

%%

yy::parser::symbol_type yylex(yyscan_t lexer) {
	yy::parser::symbol_type s;
	int x = yylex(&s, nullptr, lexer);
	assert(x == 1);
	return s;
}

void yy::parser::error(location_type const& loc, std::string const& msg) {
	std::cout << "[error] parser error at " << loc << ": " << msg << ".\n";
}

template <typename T, typename... Args> static std::unique_ptr<T> make_node(yy::parser::location_type const& loc, Args&&... args) {
	std::unique_ptr<T> n = std::make_unique<T>(std::forward<Args>(args)...);
	n->location = loc;
	return n;
}
