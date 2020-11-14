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
%token <bool> TOK_TRUE
%token <bool> TOK_FALSE

// may need to assign a type like <int> for all TOK for passing into constructer 
// or assign a value to each token, such as 
// %left  OR 134 "<=" 135  Declares 134 for OR and 135 for "<=".
//lparen, rparen, lbrace, rbrace
%token <int> TOK_LPAREN
%token <int> TOK_RPAREN
%token <int> TOK_LBRACE
%token <int> TOK_RBRACE

//eq, ne, lt, gt, le, ge
%left <int> TOK_EQ
%left <int> TOK_NE
%left <int> TOK_LT
%left <int> TOK_GT
%left <int> TOK_LE
%left <int> TOK_GE

//plus, minus, star, slash
%left <int> TOK_PLUS
%left <int> TOK_MINUS
%left <int> TOK_STAR
%left <int> TOK_SLASH

//log_and, log_or
%left <int> TOK_LOG_AND
%left <int> TOK_LOG_OR

//if, while, for, break, continue, return
%token <int> TOK_IF
%token <int> TOK_WHILE
%token <int> TOK_FOR
%token <int> TOK_BREAK
%token <int> TOK_CONTINUE
%token <int> TOK_RETURN

//comma, semicolon, colon,question_mark
%token <int> TOK_COMMA
%token <int> TOK_SEMICOLON
%token <int> TOK_COLON
%token <int> TOK_QUESTION_MARK

//assign
%right <int> TOK_ASSIGN
%right <int> TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_STAR_ASSIGN, TOK_SLASH_ASSIGN

//type
%token <int> TOK_TYPE

// %type <Node*> root

%type <std::unique_ptr<Node>> root

%type <std::unique_ptr<Node>> function_list // op = function_list, ($1 = function_list, $2 = function) || op = function_list, ($1 = function_list) || op = function_list () 
%type <std::unique_ptr<Node>> function_list_prime 
type <std::unique_ptr<Node>> function // op = function, ($1 = function_decl, $2 = TOK_SEMICOLON) || op = function, ($1 = function_defn)
%type <std::unique_ptr<Node>> function_decl // op = function_decl (TOK_TYPE name TOK_LPAREN parameter_list TOK_RPAREN)
%type <std::unique_ptr<Node>> function_defn // function_decl block
%type <std::unique_ptr<Node>> name  // TOK_IDENTIFIER
%type <std::unique_ptr<Node>> parameter_list // declaration comma_declaration_star_suite // TOK_COMMA declaration comma_declaration_star_suite // 
%type <std::unique_ptr<Node>> comma_declaration_star_suite
%type <std::unique_ptr<Node>> block
%type <std::unique_ptr<Node>> suite
%type <std::unique_ptr<Node>> declaration
%type <std::unique_ptr<Node>> statement
%type <std::unique_ptr<Node>> single_statement
%type <std::unique_ptr<Node>> augmented_assign
%type <std::unique_ptr<Node>> expression
%type <std::unique_ptr<Node>> expression_prime
%type <std::unique_ptr<Node>> compound_statement
%type <std::unique_ptr<Node>> binary_expression
%type <std::unique_ptr<Node>> unary_expression
%type <std::unique_ptr<Node>> relational_expression

%type <std::unique_ptr<Node>> binary_op
%type <std::unique_ptr<Node>> unary_op
%type <std::unique_ptr<Node>> relational_op

%type <std::unique_ptr<Node>> ternary_expression
%type <std::unique_ptr<Node>> cast_expression
%type <std::unique_ptr<Node>> function_call
%type <std::unique_ptr<Node>> comma_expression_star_quesmark_suite
%type <std::unique_ptr<Node>> comma_expression_star_suite



%start root

%%

root
  : function_list  { $$ = make_node<root>(@$, $1); printf("root := function_list\n"); }
  ;

function_list
  /* function+ */
  : function_list_prime function  { $$ = make_node<function_list>(@$, $1, $2);  printf("function_list := function_list_prime function\n"); }
  ;

function_list_prime //
  : function_list { $$ = make_node<function_list>(@$, $1); printf("function_list_prime := function_list\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("function_list_prime := %%empty\n"); }
  ;

function
  /* if pass in construcor by ';', not sure how to check ';' with enum. May check manually by if statement*/
  /* if pass by $2, not sure what type and value does the constructor accpet from $2 */
  : function_decl TOK_SEMICOLON { $$ = make_node<function>(@$, $1, 1); printf("function := function_decl TOK_SEMICOLON\n"); }
  | function_defn { $$ = make_node<function>(@$, $1); printf("function := function_defn\n"); }
  ;

function_decl
  : TOK_TYPE name TOK_LPAREN parameter_list TOK_RPAREN { $$ = make_node<function_decl>(@$, $1, $2, $3, $4, $5); printf("function_decl := TOK_TYPE name TOK_LPAREN parameter_list TOK_RPAREN\n"); }
  ;

function_defn
  : function_decl block { $$ = make_node<function_defn>(@$, $1, $2); printf("function_defn := function_decl block\n"); }
  ;

/*
type
  : TOK_IDENTIFIER { printf("type := TOK_IDENTIFIER\n"); }
  ;
*/

name
  : TOK_IDENTIFIER { $$ = make_node<name>(@$, $1); printf("name := TOK_IDENTIFIER\n"); }
  ;

parameter_list
  /* (declaration (comma declaration)*)? */
  : declaration comma_declaration_star_suite { $$ = make_node<parameter_list>(@$, $1, $2); printf("parameter_list := declaration comma_declaration_star_suite\n"); }
  | /*epsilon*/ { printf("parameter_list := %%empty\n"); }
  ;

// self defined suite
comma_declaration_star_suite
  /* comma declaration)* */
  : TOK_COMMA declaration comma_declaration_star_suite { $$ = make_node<parameter_list>(@$, $1, $2, $3); printf("comma_declaration_star_suite := TOK_COMMA declaration comma_declaration_star_suite\n"); }
  | /*epsilon*/ { $$ = nullprt; printf("comma_declaration_star_suite := %%empty\n"); }
  ;

block
  : TOK_LBRACE suite TOK_RBRACE { $$ = make_node<block>(@$, $1, $2, $3); printf("block := TOK_LBRACE suite TOK_RBRACE\n"); }
  ;

suite
  /* (statement)* */
  : statement suite  { $$ = make_node<suite>(@$, $1, $2); printf("suite := statement suite\n"); }
  | /*epsilon*/ { $$ = null_ptr; printf("suite := %%empty\n"); }
  ;

declaration
  : TOK_TYPE name { $$ = make_node<declaration>(@$, $1, $2); printf("declaration := TOK_TYPE name\n"); }
  ;

statement
  : single_statement TOK_SEMICOLON { $$ = make_node<statement>(@$, $1, $2); printf("statement := single_statement TOK_SEMICOLON\n"); }
  |	compound_statement { $$ = make_node<statement>(@$, $1); printf("statement := compound_statement\n"); }
  ;

single_statement
  : declaration TOK_ASSIGN expression { $$ = make_node<single_statement>(@$, $1, $2, $3); printf("single_statement := declaration TOK_ASSIGN expression\n"); }
  | name TOK_ASSIGN expression { $$ = make_node<single_statement>(@$, $1, $2, $3); printf("single_statement := name TOK_ASSIGN expression\n"); }
  | name augmented_assign expression { $$ = make_node<single_statement>(@$, $1, $2, $3); printf("single_statement := name augmented_assign expression\n"); }
  | TOK_BREAK { $$ = make_node<single_statement>(@$, $1); printf("single_statement := TOK_BREAK\n"); }
  | TOK_CONTINUE { $$ = make_node<single_statement>(@$, $1); printf("single_statement := TOK_CONTINUE\n"); }
  | TOK_RETURN { $$ = make_node<single_statement>(@$, $1); printf("single_statement := TOK_RETURN\n"); }
  | TOK_RETURN expression { $$ = make_node<single_statement>(@$, $1, $2); printf("single_statement := TOK_RETURN expression\n"); }
  | expression { $$ = make_node<single_statement>(@$, $1); printf("single_statement := expression\n"); }
  ;

augmented_assign
  : TOK_PLUS_ASSIGN { $$ = make_node<augmented_assign>(@$, $1); printf("augmented_assign := TOK_PLUS_ASSIGN\n"); }
  | TOK_MINUS_ASSIGN { $$ = make_node<augmented_assign>(@$, $1); printf("augmented_assign := TOK_MINUS_ASSIGN\n"); }
  | TOK_STAR_ASSIGN { $$ = make_node<augmented_assign>(@$, $1); printf("augmented_assign := TOK_STAR_ASSIGN\n"); }
  | TOK_SLASH_ASSIGN { $$ = make_node<augmented_assign>(@$, $1); printf("augmented_assign := TOK_SLASH_ASSIGN\n"); }
  ;

expression
  : name { $$ = make_node<expression>(@$, $1); printf("expression := name\n"); }
  | expression_prime { $$ = make_node<expression>(@$, $1); printf("expression := expression_prime\n"); }
  | binary_expression { $$ = make_node<expression>(@$, $1); printf("expression := binary_expression\n"); }
  | unary_expression { $$ = make_node<expression>(@$, $1); printf("expression := unary_expression\n"); }
  | relational_expression { $$ = make_node<expression>(@$, $1); printf("expression := relational_expression\n"); }
  | ternary_expression { $$ = make_node<expression>(@$, $1); printf("expression := ternary_expression\n"); }
  | cast_expression { $$ = make_node<expression>(@$, $1); printf("expression := cast_expression\n"); }
  | function_call { $$ = make_node<expression>(@$, $1); printf("expression := function_call\n"); }
  ;

expression_prime
  : TOK_TRUE { $$ = make_node<expression_prime>(@$, $1); printf("expression_prime := TOK_TRUE\n"); }
  | TOK_FALSE { $$ = make_node<expression_prime>(@$, $1); printf("expression_prime := TOK_FALSE\n"); }
  | TOK_INTEGER { $$ = make_node<expression_prime>(@$, $1); printf("expression_prime := TOK_INTEGER\n"); }
  | TOK_FLOAT { $$ = make_node<expression_prime>(@$, $1); printf("expression_prime := TOK_FLOAT\n"); }
  | TOK_LPAREN expression TOK_RPAREN { $$ = make_node<expression_prime>(@$, $1, $2, $3); printf("expression_prime := TOK_LPAREN expression TOK_RPAREN\n"); }
  ;

compound_statement
  : TOK_IF TOK_LPAREN expression TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5); printf("expressicompound_statementon_prime := TOK_IF TOK_LPAREN expression TOK_RPAREN block\n"); }
  /* for lparen single_statement? semicolon expression? semicolon single_statement? rparen block */
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6 , $7); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6 , $7); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON expression TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6, $7, $8); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6 , $7); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6 , $7); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6 , $7, $8); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5, $6 , $7, $8, $9); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_WHILE TOK_LPAREN expression TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, $1, $2, $3, $4, $5); printf("compound_statement := TOK_WHILE TOK_LPAREN expression TOK_RPAREN block\n"); }
  ;

binary_expression
  : expression binary_op expression_prime { $$ = make_node<binary_expression>(@$, $1, $2, $3); printf("binary_expression := expression binary_op expression_prime\n"); }
  ;

unary_expression
  : unary_op expression_prime { $$ = make_node<unary_expression>(@$, $1, $2); printf("unary_expression := unary_op expression_prime\n"); }
  ;

relational_expression
  : expression relational_op expression_prime { $$ = make_node<relational_expression>(@$, $1, $2, $3); printf("relational_expression := expression relational_op expression_prime\n"); }
  ;

binary_op
  : TOK_PLUS { $$ = make_node<binary_op>(@$, $1); printf("binary_op := TOK_PLUS\n"); }
  | TOK_MINUS  { $$ = make_node<binary_op>(@$, $1); printf("binary_op := TOK_MINUS\n"); }
  | TOK_STAR { $$ = make_node<binary_op>(@$, $1); printf("binary_op := TOK_STAR\n"); }
  | TOK_SLASH { $$ = make_node<binary_op>(@$, $1); printf("binary_op := TOK_SLASH\n"); }
  | TOK_LOG_AND { $$ = make_node<binary_op>(@$, $1); printf("binary_op := TOK_LOG_AND\n"); }
  | TOK_LOG_OR { $$ = make_node<binary_op>(@$, $1); printf("binary_op := TOK_LOG_OR\n"); }
  ;

unary_op
  : TOK_MINUS { $$ = make_node<unary_op>(@$, $1); printf("unary_op := TOK_MINUS\n"); }
  ;

relational_op
  : TOK_EQ { $$ = make_node<relational_op>(@$, $1); printf("relational_op := TOK_EQ\n"); }
  | TOK_NE { $$ = make_node<relational_op>(@$, $1); printf("relational_op := TOK_NE\n"); }
  | TOK_LT { $$ = make_node<relational_op>(@$, $1); printf("relational_op := TOK_LT\n"); }
  | TOK_GT { $$ = make_node<relational_op>(@$, $1); printf("relational_op := TOK_GT\n"); }
  | TOK_LE { $$ = make_node<relational_op>(@$, $1); printf("relational_op := TOK_LE\n"); }
  | TOK_GE { $$ = make_node<relational_op>(@$, $1); printf("relational_op := TOK_GE\n"); }
  ;

ternary_expression
  : expression TOK_QUESTION_MARK expression TOK_COLON expression_prime { $$ = make_node<ternary_expression>(@$, $1, $2, $3, $4, $5); printf("ternary_expression := expression TOK_QUESTION_MARK expression TOK_COLON expression_prime\n"); }
  ;

cast_expression
  : TOK_LPAREN TOK_TYPE TOK_RPAREN expression_prime { $$ = make_node<cast_expression>(@$, $1, $2, $3, $4); printf("cast_expression := TOK_LPAREN TOK_TYPE TOK_RPAREN expression_prime\n"); }
  ;

function_call
  /* name lparen (expression (TOK_COMMA expression)*)? rparen */
  : name TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN { $$ = make_node<function_call>(@$, $1, $2, $3, $4); printf("function_call := name TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN\n"); }
  ;

/* (TOK_COMMA expression)*)? */
comma_expression_star_quesmark_suite
  : expression comma_expression_star_suite { $$ = make_node<comma_expression_star_quesmark_suite>(@$, $1, $2); printf("comma_expression_star_quesmark_suite := expression comma_expression_star_suite\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("comma_expression_star_quesmark_suite := %%empty\n"); }
  ;

/* (TOK_COMMA expression)* */
comma_expression_star_suite
  : TOK_COMMA expression comma_expression_star_suite { $$ = make_node<comma_expression_star_quesmark_suite>(@$, $1, $2, $3); printf("comma_expression_star_suite := TOK_COMMA expression comma_expression_star_suite\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("comma_expression_star_suite := %%empty\n"); }
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
