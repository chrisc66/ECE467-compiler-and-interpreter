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
%left TOK_EQ
%left TOK_NE
%left TOK_LT
%left TOK_GT
%left TOK_LE
%left TOK_GE

//plus, minus, star, slash
%left TOK_PLUS
%left TOK_MINUS
%left TOK_STAR
%left TOK_SLASH

//log_and, log_or
%left TOK_LOG_AND
%left TOK_LOG_OR

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
%right TOK_ASSIGN
%right TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_STAR_ASSIGN, TOK_SLASH_ASSIGN

//type
%token TOK_TYPE

%type <Node*> root

%start root

%%

root
  : function_list                               { printf("root := function_list\n"); }
  ;

function_list
  /* function+ */
  : function_list_ function  { printf("function_list := function_list_ function\n"); }
  ;

function_list_
  : function_list { printf("function_list_ := function_list\n"); }
  | /*epsilon*/ { printf("function_list_ := %%empty\n"); }
  ;

function
  : function_decl TOK_SEMICOLON { printf("function := function_decl TOK_SEMICOLON\n"); }
  | function_defn { printf("function := function_defn\n"); }
  ;

function_decl
  : TOK_TYPE name TOK_LPAREN parameter_list TOK_RPAREN { printf("function_decl := TOK_TYPE name TOK_LPAREN parameter_list TOK_RPAREN\n"); }
  ;

function_defn
  : function_decl block { printf("function_defn := function_decl block\n"); }
  ;

/*
type
  : TOK_IDENTIFIER { printf("type := TOK_IDENTIFIER\n"); }
  ;
*/

name
  : TOK_IDENTIFIER { printf("name := TOK_IDENTIFIER\n"); }
  ;

parameter_list
  /* (declaration (comma declaration)*)? */
  : declaration comma_declaration_star_suite { printf("parameter_list := declaration comma_declaration_star_suite\n"); }
  | /*epsilon*/ { printf("parameter_list := %%empty\n"); }
  ;

// self defined suite
comma_declaration_star_suite
  /* comma declaration)* */
  : TOK_COMMA declaration comma_declaration_star_suite { printf("comma_declaration_star_suite := TOK_COMMA declaration comma_declaration_star_suite\n"); }
  | /*epsilon*/ { printf("comma_declaration_star_suite := %%empty\n"); }
  ;

block
  : TOK_LBRACE suite TOK_RBRACE { printf("block := TOK_LBRACE suite TOK_RBRACE\n"); }
  ;

suite
  /* (statement)* */
  : statement suite  { printf("suite := statement suite\n"); }
  | /*epsilon*/ { printf("suite := %%empty\n"); }
  ;

declaration
  : TOK_TYPE name { printf("declaration := TOK_TYPE name\n"); }
  ;

statement
  : single_statement TOK_SEMICOLON { printf("statement := single_statement TOK_SEMICOLON\n"); }
  |	compound_statement { printf("statement := compound_statement\n"); }
  ;

single_statement
  : declaration TOK_ASSIGN expression { printf("single_statement := declaration TOK_ASSIGN expression\n"); }
  | name TOK_ASSIGN expression { printf("single_statement := name TOK_ASSIGN expression\n"); }
  | name augmented_assign expression { printf("single_statement := name augmented_assign expression\n"); }
  | TOK_BREAK { printf("single_statement := TOK_BREAK\n"); }
  | TOK_CONTINUE { printf("single_statement := TOK_CONTINUE\n"); }
  | TOK_RETURN { printf("single_statement := TOK_RETURN\n"); }
  | TOK_RETURN expression { printf("single_statement := TOK_RETURN expression\n"); }
  | expression { printf("single_statement := expression\n"); }
  ;

augmented_assign
  : TOK_PLUS_ASSIGN { printf("augmented_assign := TOK_PLUS_ASSIGN\n"); }
  | TOK_MINUS_ASSIGN { printf("augmented_assign := TOK_MINUS_ASSIGN\n"); }
  | TOK_STAR_ASSIGN { printf("augmented_assign := TOK_STAR_ASSIGN\n"); }
  | TOK_SLASH_ASSIGN { printf("augmented_assign := TOK_SLASH_ASSIGN\n"); }
  ;

expression
  : name { printf("expression := name\n"); }
  | expression_prime { printf("expression := expression_prime\n"); }
  | binary_expression { printf("expression := binary_expression\n"); }
  | unary_expression { printf("expression := unary_expression\n"); }
  | relational_expression { printf("expression := relational_expression\n"); }
  | ternary_expression { printf("expression := ternary_expression\n"); }
  | cast_expression { printf("expression := cast_expression\n"); }
  | function_call { printf("expression := function_call\n"); }
  ;

expression_prime
  : TOK_TRUE { printf("expression_prime := TOK_TRUE\n"); }
  | TOK_FALSE { printf("expression_prime := TOK_FALSE\n"); }
  | TOK_INTEGER { printf("expression_prime := TOK_INTEGER\n"); }
  | TOK_FLOAT { printf("expression_prime := TOK_FLOAT\n"); }
  | TOK_LPAREN expression TOK_RPAREN { printf("expression_prime := TOK_LPAREN expression TOK_RPAREN\n"); }
  ;

compound_statement
  : TOK_IF TOK_LPAREN expression TOK_RPAREN block
   { printf("expressicompound_statementon_prime := TOK_IF TOK_LPAREN expression TOK_RPAREN block\n"); }
  /* for lparen single_statement? semicolon expression? semicolon single_statement? rparen block */
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON expression TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
   { printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_WHILE TOK_LPAREN expression TOK_RPAREN block
   { printf("compound_statement := TOK_WHILE TOK_LPAREN expression TOK_RPAREN block\n"); }
  ;

binary_expression
  : expression binary_op expression_prime { printf("binary_expression := expression binary_op expression_prime\n"); }
  ;

unary_expression
  : unary_op expression_prime { printf("unary_expression := unary_op expression_prime\n"); }
  ;

relational_expression
  : expression relational_op expression_prime { printf("relational_expression := expression relational_op expression_prime\n"); }
  ;

binary_op
  : TOK_PLUS { printf("binary_op := TOK_PLUS\n"); }
  | TOK_MINUS  { printf("binary_op := TOK_MINUS\n"); }
  | TOK_STAR { printf("binary_op := TOK_STAR\n"); }
  | TOK_SLASH { printf("binary_op := TOK_SLASH\n"); }
  | TOK_LOG_AND { printf("binary_op := TOK_LOG_AND\n"); }
  | TOK_LOG_OR { printf("binary_op := TOK_LOG_OR\n"); }
  ;

unary_op
  : TOK_MINUS { printf("unary_op := TOK_MINUS\n"); }
  ;

relational_op
  : TOK_EQ { printf("relational_op := TOK_EQ\n"); }
  | TOK_NE { printf("relational_op := TOK_NE\n"); }
  | TOK_LT { printf("relational_op := TOK_LT\n"); }
  | TOK_GT { printf("relational_op := TOK_GT\n"); }
  | TOK_LE { printf("relational_op := TOK_LE\n"); }
  | TOK_GE { printf("relational_op := TOK_GE\n"); }
  ;

ternary_expression
  : expression TOK_QUESTION_MARK expression TOK_COLON expression_prime { printf("ternary_expression := expression TOK_QUESTION_MARK expression TOK_COLON expression_prime\n"); }
  ;

cast_expression
  : TOK_LPAREN TOK_TYPE TOK_RPAREN expression_prime { printf("cast_expression := TOK_LPAREN TOK_TYPE TOK_RPAREN expression_prime\n"); }
  ;

function_call
  /* name lparen (expression (TOK_COMMA expression)*)? rparen */
  : name TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN { printf("function_call := name TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN\n"); }
  ;

/* (TOK_COMMA expression)*)? */
comma_expression_star_quesmark_suite
  : expression comma_expression_star_suite { printf("comma_expression_star_quesmark_suite := expression comma_expression_star_suite\n"); }
  | /*epsilon*/ { printf("comma_expression_star_quesmark_suite := %%empty\n"); }
  ;

/* (TOK_COMMA expression)* */
comma_expression_star_suite
  : TOK_COMMA expression comma_expression_star_suite { printf("comma_expression_star_suite := TOK_COMMA expression comma_expression_star_suite\n"); }
  | /*epsilon*/ { printf("comma_expression_star_suite := %%empty\n"); }
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