%code requires {

// this will be added to your parser.hpp file

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include <memory>

#include "nodes.hpp"

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
// %define api.value.type {bool, int} /* https://www.gnu.org/software/bison/manual/html_node/Value-Type.html */
%define api.token.constructor
%define parse.trace
%define parse.assert

%token <std::string> HI
%token BYE

//identifier, integer, float, type
%token <std::string> TOK_IDENTIFIER
%token <std::string> TOK_INTEGER
%token <std::string> TOK_FLOAT
%token <std::string> TOK_TYPE

//true, false
%token <bool> TOK_TRUE 
%token <bool> TOK_FALSE 

// may need to assign a type like <int> for all TOK for passing into constructer 
// or assign a value to each token, such as 
// %left  OR 134 "<=" 135  Declares 134 for OR and 135 for "<=".
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
%left TOK_COMMA
%token TOK_SEMICOLON
%right TOK_COLON
%right TOK_QUESTION_MARK

//assign
%right TOK_ASSIGN
%right TOK_PLUS_ASSIGN
%right TOK_MINUS_ASSIGN
%right TOK_STAR_ASSIGN
%right TOK_SLASH_ASSIGN


%type <std::unique_ptr<rooot>> root
%type <std::unique_ptr<function_list>> function_list // op = function_list, ($1 = function_list, $2 = function) || op = function_list, ($1 = function_list) || op = function_list () 
%type <std::unique_ptr<function_list>> function_list_prime 
%type <std::unique_ptr<function>> function // op = function, ($1 = function_decl, $2 = TOK_SEMICOLON) || op = function, ($1 = function_defn)
%type <std::unique_ptr<function_decl>> function_decl // op = function_decl (TOK_TYPE namee TOK_LPAREN parameter_list TOK_RPAREN)
%type <std::unique_ptr<function_defn>> function_defn // function_decl block
%type <std::unique_ptr<namee>> namee  // TOK_IDENTIFIER
%type <std::unique_ptr<parameter_list>> parameter_list // declaration comma_declaration_star_suite // TOK_COMMA declaration comma_declaration_star_suite // 
%type <std::unique_ptr<comma_declaration_star_suite>> comma_declaration_star_suite
%type <std::unique_ptr<block>> block
%type <std::unique_ptr<suite>> suite
%type <std::unique_ptr<declaration>> declaration
%type <std::unique_ptr<statement>> statement
%type <std::unique_ptr<single_statement>> single_statement
%type <std::unique_ptr<expression>> expression
%type <std::unique_ptr<expression_prime>> expression_prime
%type <std::unique_ptr<compound_statement>> compound_statement
%type <std::unique_ptr<binary_expression>> binary_expression
%type <std::unique_ptr<unary_expression>> unary_expression
%type <std::unique_ptr<relational_expression>> relational_expression
%type <std::unique_ptr<ternary_expression>> ternary_expression
%type <std::unique_ptr<cast_expression>> cast_expression
%type <std::unique_ptr<function_call>> function_call
%type <std::unique_ptr<comma_expression_star_quesmark_suite>> comma_expression_star_quesmark_suite
%type <std::unique_ptr<comma_expression_star_suite>> comma_expression_star_suite


%start root

%%

root
  : function_list  { root = make_node<rooot>(@$, std::move($1)); printf("root := function_list\n"); }
  ;

function_list
  /* function+ */
  : function_list_prime function  { $$ = make_node<function_list>(@$, std::move($1), std::move($2));  printf("function_list := function_list_prime function\n"); }
  ;

function_list_prime
  : function_list { $$ = std::move($1); printf("function_list_prime := function_list\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("function_list_prime := %%empty\n"); }
  ;

function
  : function_decl TOK_SEMICOLON { $$ = make_node<function>(@$, std::move($1)); printf("function := function_decl TOK_SEMICOLON\n"); }
  | function_defn { $$ = make_node<function>(@$, std::move($1)); printf("function := function_defn\n"); }
  ;

function_decl
  : TOK_TYPE namee TOK_LPAREN parameter_list TOK_RPAREN { 
      // if (($1.compare("void") != 0) && ($1.compare("int") != 0) && ($1.compare("float") != 0) && ($1.compare("bool") != 0)) {
      //     // raise type_decl flag
      //     // function_decl should add a type_decl flag
      //     printf(" function_decl type_decl error !!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      //     exit(1); 
      // }
      $$ =  make_node<function_decl>(@$, $1, std::move($2), std::move($4)); printf("function_decl := TOK_TYPE namee TOK_LPAREN parameter_list TOK_RPAREN\n"); 
    }
  ;

function_defn
  : function_decl block { $$ = make_node<function_defn>(@$, std::move($1), std::move($2)); printf("function_defn := function_decl block\n"); }
  ;

namee
  : TOK_IDENTIFIER { $$ = make_node<namee>(@$, $1); printf("namee := TOK_IDENTIFIER\n"); }
  ;

parameter_list
  /* (declaration (comma declaration)*)? */
  : declaration comma_declaration_star_suite { $$ = make_node<parameter_list>(@$, std::move($1), std::move($2)); printf("parameter_list := declaration comma_declaration_star_suite\n");  }
  | /*epsilon*/ { $$ = nullptr; printf("parameter_list := %%empty\n"); }
  ;

// self defined suite
comma_declaration_star_suite
  /* comma declaration)* */
  : TOK_COMMA declaration comma_declaration_star_suite { $$ = make_node<comma_declaration_star_suite>(@$, std::move($2), std::move($3)); printf("comma_declaration_star_suite := TOK_COMMA declaration comma_declaration_star_suite\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("comma_declaration_star_suite := %%empty\n"); }
  ;

block
  : TOK_LBRACE suite TOK_RBRACE { $$ = make_node<block>(@$, std::move($2)); printf("block := TOK_LBRACE suite TOK_RBRACE\n"); }
  ; 

suite
  /* (statement)* */
  : statement suite  { $$ = make_node<suite>(@$, std::move($1), std::move($2)); printf("suite := statement suite\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("suite := %%empty\n"); }
  ;

declaration
  : TOK_TYPE namee { 
      // if (($1.compare("int") != 0) && ($1.compare("float") != 0) && ($1.compare("bool") != 0)) {
      //     // raise type_decl flag
      //     // declaration should add a type_decl flag
      //     printf(" declaration type_decl error !!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      //     exit(1);
      // } 
      $$ = make_node<declaration>(@$, $1, std::move($2)); printf("declaration := TOK_TYPE namee\n"); 
    }
  ;

statement
  : single_statement TOK_SEMICOLON { $$ = make_node<statement>(@$, std::move($1)); printf("statement := single_statement TOK_SEMICOLON\n"); }
  |	compound_statement { $$ = make_node<statement>(@$, std::move($1)); printf("statement := compound_statement\n"); }
  ;

single_statement
  : declaration TOK_ASSIGN expression { $$ = make_node<single_statement>(@$, std::move($1), TOK_ENUM::TOK_ASSIGN, std::move($3), 1); printf("single_statement := declaration TOK_ASSIGN expression\n"); }
  | namee TOK_ASSIGN expression { $$ = make_node<single_statement>(@$, std::move($1), TOK_ENUM::TOK_ASSIGN, std::move($3), 2); printf("single_statement := namee TOK_ASSIGN expression\n"); }
  | namee TOK_PLUS_ASSIGN expression { $$ = make_node<single_statement>(@$, std::move($1), TOK_ENUM::TOK_PLUS_ASSIGN, std::move($3), 3); printf("single_statement := namee TOK_PLUS_ASSIGN expression\n"); }
  | namee TOK_MINUS_ASSIGN expression { $$ = make_node<single_statement>(@$, std::move($1), TOK_ENUM::TOK_MINUS_ASSIGN, std::move($3), 4); printf("single_statement := namee TOK_MINUS_ASSIGN expression\n"); }
  | namee TOK_STAR_ASSIGN expression { $$ = make_node<single_statement>(@$, std::move($1), TOK_ENUM::TOK_STAR_ASSIGN, std::move($3), 5); printf("single_statement := namee TOK_STAR_ASSIGN expression\n"); }
  | namee TOK_SLASH_ASSIGN expression { $$ =  make_node<single_statement>(@$, std::move($1), TOK_ENUM::TOK_SLASH_ASSIGN, std::move($3), 6); printf("single_statement := namee TOK_SLASH_ASSIGN expression\n"); }
  | TOK_BREAK { $$ = make_node<single_statement>(@$, TOK_ENUM::TOK_BREAK, 7); printf("single_statement := TOK_BREAK\n"); }
  | TOK_CONTINUE { $$ = make_node<single_statement>(@$, TOK_ENUM::TOK_CONTINUE, 8); printf("single_statement := TOK_CONTINUE\n"); }
  | TOK_RETURN { $$ = make_node<single_statement>(@$, TOK_ENUM::TOK_RETURN, 9); printf("single_statement := TOK_RETURN\n"); }
  | TOK_RETURN expression { $$ = make_node<single_statement>(@$, TOK_ENUM::TOK_RETURN, std::move($2), 10); printf("single_statement := TOK_RETURN expression\n"); }
  | expression { $$ = make_node<single_statement>(@$, std::move($1), 11); printf("single_statement := expression\n"); }
  ;

expression
  : expression_prime { $$ = make_node<expression>(@$, std::move($1)); printf("expression := expression_prime\n"); }
  | binary_expression { $$ = make_node<expression>(@$, std::move($1)); printf("expression := binary_expression\n"); }
  | unary_expression { $$ = make_node<expression>(@$, std::move($1)); printf("expression := unary_expression\n"); }
  | relational_expression { $$ = make_node<expression>(@$, std::move($1)); printf("expression := relational_expression\n"); }
  | ternary_expression { $$ = make_node<expression>(@$, std::move($1)); printf("expression := ternary_expression\n"); }
  | cast_expression { $$ = make_node<expression>(@$, std::move($1)); printf("expression := cast_expression\n"); }
  | function_call { $$ = make_node<expression>(@$, std::move($1)); printf("expression := function_call\n"); }
  ;

expression_prime
  : namee { $$ = make_node<expression_prime>(@$, std::move($1), 1); printf("expression_prime := namee\n"); }
  | TOK_TRUE { $$ = make_node<expression_prime>(@$, $1, 2); printf("expression_prime := TOK_TRUE\n"); }
  | TOK_FALSE { $$ = make_node<expression_prime>(@$, $1, 3); printf("expression_prime := TOK_FALSE\n"); }
  | TOK_INTEGER { $$ = make_node<expression_prime>(@$, std::stoi($1), 4); printf("expression_prime := TOK_INTEGER\n"); }
  | TOK_FLOAT { $$ = make_node<expression_prime>(@$, std::stof($1), 5); printf("expression_prime := TOK_FLOAT\n"); }
  | TOK_LPAREN expression TOK_RPAREN { $$ = make_node<expression_prime>(@$, std::move($2), 6); printf("expression_prime := TOK_LPAREN expression TOK_RPAREN\n"); }
  ;

compound_statement
  : TOK_IF TOK_LPAREN expression TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_IF, std::move($3), std::move($5), 1); printf("expressicompound_statementon_prime := TOK_IF TOK_LPAREN expression TOK_RPAREN block\n"); }
  /* for lparen single_statement? semicolon expression? semicolon single_statement? rparen block */
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, nullptr, nullptr, nullptr, std::move($6), 2); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, std::move($3), nullptr, nullptr, std::move($7), 3); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, nullptr, std::move($4), nullptr, std::move($7), 4); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON expression TOK_SEMICOLON TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, std::move($3), std::move($5), nullptr, std::move($8), 5); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, nullptr, nullptr, std::move($5), std::move($7), 6); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, std::move($3), nullptr, std::move($6), std::move($8), 7); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, nullptr, std::move($4), std::move($6), std::move($8), 8); printf("compound_statement := TOK_FOR TOK_LPAREN TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_FOR, std::move($3), std::move($5), std::move($7), std::move($9), 9); printf("compound_statement := TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block\n"); }
  | TOK_WHILE TOK_LPAREN expression TOK_RPAREN block
   { $$ = make_node<compound_statement>(@$, TOK_ENUM::TOK_WHILE, std::move($3), std::move($5), 10); printf("compound_statement := TOK_WHILE TOK_LPAREN expression TOK_RPAREN block\n"); }
  ;

binary_expression
  : expression TOK_PLUS expression { $$ = make_node<binary_expression>(@$, std::move($1), TOK_ENUM::TOK_PLUS, std::move($3)); printf("binary_expression := expression TOK_PLUS expression\n"); }
  | expression TOK_MINUS expression { $$ = make_node<binary_expression>(@$, std::move($1), TOK_ENUM::TOK_MINUS, std::move($3)); printf("binary_expression := expression TOK_MINUS expression\n"); }
  | expression TOK_STAR expression { $$ = make_node<binary_expression>(@$, std::move($1), TOK_ENUM::TOK_STAR, std::move($3)); printf("binary_expression := expression TOK_STAR expression\n"); }
  | expression TOK_SLASH expression { $$ = make_node<binary_expression>(@$, std::move($1), TOK_ENUM::TOK_SLASH, std::move($3)); printf("binary_expression := expression TOK_SLASH expression\n"); }
  | expression TOK_LOG_AND expression { $$ = make_node<binary_expression>(@$, std::move($1), TOK_ENUM::TOK_LOG_AND, std::move($3)); printf("binary_expression := expression TOK_LOG_AND expression\n"); }
  | expression TOK_LOG_OR expression { $$ = make_node<binary_expression>(@$, std::move($1), TOK_ENUM::TOK_LOG_OR, std::move($3)); printf("binary_expression := expression TOK_LOG_OR expression\n"); }
  ;

unary_expression
  : TOK_MINUS expression { $$ = make_node<unary_expression>(@$, TOK_ENUM::TOK_MINUS, std::move($2)); printf("unary_expression := TOK_MINUS expression\n"); }
  ;

relational_expression
  : expression TOK_EQ expression { $$ = make_node<relational_expression>(@$, std::move($1), TOK_ENUM::TOK_EQ, std::move($3)); printf("relational_expression := expression TOK_EQ expression\n"); }
  | expression TOK_NE expression { $$ = make_node<relational_expression>(@$, std::move($1), TOK_ENUM::TOK_NE, std::move($3)); printf("relational_expression := expression TOK_NE expression\n"); }
  | expression TOK_LT expression { $$ = make_node<relational_expression>(@$, std::move($1), TOK_ENUM::TOK_LT, std::move($3)); printf("relational_expression := expression TOK_LT expression\n"); }
  | expression TOK_GT expression { $$ = make_node<relational_expression>(@$, std::move($1), TOK_ENUM::TOK_GT, std::move($3)); printf("relational_expression := expression TOK_GT expression\n"); }
  | expression TOK_LE expression { $$ = make_node<relational_expression>(@$, std::move($1), TOK_ENUM::TOK_LE, std::move($3)); printf("relational_expression := expression TOK_LE expression\n"); }
  | expression TOK_GE expression { $$ = make_node<relational_expression>(@$, std::move($1), TOK_ENUM::TOK_GE, std::move($3)); printf("relational_expression := expression TOK_GE expression\n"); }
  ;

ternary_expression
  : expression TOK_QUESTION_MARK expression TOK_COLON expression { $$ =  make_node<ternary_expression>(@$, std::move($1), std::move($3), std::move($5)); printf("ternary_expression := expression TOK_QUESTION_MARK expression TOK_COLON expression\n"); }
  ;

cast_expression
  : TOK_LPAREN TOK_TYPE TOK_RPAREN expression { 
      // if (($2.compare("int") != 0) && ($2.compare("float") != 0) && ($2.compare("bool") != 0)) {
      //     // raise type_decl flag
      //     // declaration should add a type_decl flag
      //     printf(" cast_expression type_decl error !!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      //     exit(1);
      // }
      $$ = make_node<cast_expression>(@$, $2, std::move($4)); printf("cast_expression := TOK_LPAREN TOK_TYPE TOK_RPAREN expression\n"); 
    }
  ;

function_call
  /* namee lparen (expression (TOK_COMMA expression)*)? rparen */
  : namee TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN { $$ = make_node<function_call>(@$, std::move($1), std::move($3)); printf("function_call := namee TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN\n"); }
  ;

/* (TOK_COMMA expression)*)? */
comma_expression_star_quesmark_suite
  : expression comma_expression_star_suite { $$ = make_node<comma_expression_star_quesmark_suite>(@$, std::move($1), std::move($2)); printf("comma_expression_star_quesmark_suite := expression comma_expression_star_suite\n"); }
  | /*epsilon*/ { $$ = nullptr; printf("comma_expression_star_quesmark_suite := %%empty\n"); }
  ;

/* (TOK_COMMA expression)* */
comma_expression_star_suite
  : TOK_COMMA expression comma_expression_star_suite { $$ = make_node<comma_expression_star_suite>(@$, std::move($2), std::move($3)); printf("comma_expression_star_suite := TOK_COMMA expression comma_expression_star_suite\n"); }
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
