#ifndef ECE467_NODE_HPP_INCLUDED
#define ECE467_NODE_HPP_INCLUDED

#include "location.hh"
#include <memory>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <iomanip>
#include <iostream>

// Token Type Enum
enum TOK_ENUM {
	TOK_UNDEFINE = 0,
	// TOK_IDENTIFIER = 1,
	// TOK_INTEGER = 2,
	// TOK_FLOAT = 3,
	// TOK_TYPE = 4
	// TOK_TRUE = 5,
	// TOK_FALSE = 6,
	TOK_LPAREN = 7,
	TOK_RPAREN = 8,
	TOK_LBRACE = 9,
	TOK_RBRACE = 10,
	TOK_EQ = 11,
	TOK_NE = 12,
	TOK_LT = 13,
	TOK_GT = 14,
	TOK_LE = 15,
	TOK_GE = 16,
	TOK_PLUS = 17,
	TOK_MINUS = 18,
	TOK_STAR = 19,
	TOK_SLASH = 20,
	TOK_LOG_AND = 21,
	TOK_LOG_OR = 22,
	TOK_IF = 23,
	TOK_WHILE = 24,
	TOK_FOR = 25,
	TOK_BREAK = 26,
	TOK_CONTINUE = 27,
	TOK_RETURN = 28,
	TOK_COMMA = 29,
	TOK_SEMICOLON = 30,
	TOK_COLON = 31,
	TOK_QUESTION_MARK = 32,
	TOK_ASSIGN = 33,
	TOK_PLUS_ASSIGN = 34,
	TOK_MINUS_ASSIGN = 35,
	TOK_STAR_ASSIGN = 36,
	TOK_SLASH_ASSIGN = 37
};

// Error Types and Messages
enum ERROR_ENUM {
	ERROR_UNDEFINE = 0,
	ERROR_TYPE_DECL = 1,
	ERROR_TYPE_MISMATCH = 2, 
	ERROR_TYPE_ARG = 3, 
	ERROR_TYPE_BOOL = 4,
	ERROR_TYPE_RETURN = 5, 
	ERROR_MAIN_FUNCTION = 6, 
	ERROR_RETURN_STMT = 7,
	ERROR_DUPLICATE_DECL = 8,
	ERROR_NOT_DECL = 9
};

void print_error_msg (ERROR_ENUM ERROR_NUM, yy::position location);

// Class Declarations and Definitions
class Node;
class rooot;
class function_list;
class function;
class function_decl;
class function_defn;
class namee;
class parameter_list;
class comma_declaration_star_suite;
class block;
class suite;
class declaration;
class statement;
class single_statement;
class expression;
class expression_prime;
class compound_statement;
class binary_expression;
class unary_expression;
class relational_expression;
class ternary_expression;
class cast_expression;
class function_call;
class comma_expression_star_quesmark_suite;
class comma_expression_star_suite;

///////////////////////////////////////// base class: Node ///////////////////////////////////////////

class Node {
public:
	yy::location location;

	virtual ~Node();

	virtual bool verify();
	virtual bool verify(rooot* rooot);
	virtual void print(int indent);
};

///////////////////////////////////////// root ///////////////////////////////////////////

class rooot : public Node {
public:
	std::unique_ptr<function_list> function_list_;
	std::unordered_map<std::string, function_decl*> function_symbol_table_; // namee, function_decl
	std::unordered_map<std::string, declaration*> variable_symbol_table_; // namee, declaration
	int block_count_; 

	rooot(std::unique_ptr<function_list> function_list);
	~rooot();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// function_list ///////////////////////////////////////////

class function_list : public Node {
public:
	std::unique_ptr<function_list> function_list_prime_;
	std::unique_ptr<function> function_;

	// function_list_prime function
	function_list(std::unique_ptr<function_list> function_list_prime, std::unique_ptr<function> function);
	~function_list();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// function ///////////////////////////////////////////

class function : public Node {
public:
	std::unique_ptr<function_decl> function_decl_;
	std::unique_ptr<function_defn> function_defn_;

	// TOK_TYPE namee TOK_LPAREN parameter_list TOK_RPAREN
	function(std::unique_ptr<function_decl> function_decl);
	function(std::unique_ptr<function_defn> function_defn);
	~function();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// function_decl ///////////////////////////////////////////

class function_decl : public Node {
public:
	std::string type_;
	std::unique_ptr<namee> namee_;
	std::unique_ptr<parameter_list> parameter_list_;

	// TOK_TYPE namee TOK_LPAREN parameter_list TOK_RPAREN
	function_decl(std::string TOK_TYPE, std::unique_ptr<namee>namee, std::unique_ptr<parameter_list> parameter_list);
	~function_decl();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// function_defn ///////////////////////////////////////////

class function_defn : public Node {
public:
	std::unique_ptr<function_decl> function_decl_;
	std::unique_ptr<block> block_;

	// function_decl block
	function_defn(std::unique_ptr<function_decl> function_decl, std::unique_ptr<block> block);
	~function_defn();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// namee ///////////////////////////////////////////

class namee : public Node {
public:
	std::string identifier_val_;
	std::string type_;
	bool is_function_; 
	bool is_variable_;

	// TOK_IDENTIFIER
	namee(std::string identifier_val);
	~namee();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// parameter_list ///////////////////////////////////////////

class parameter_list : public Node {
public:
	std::unique_ptr<declaration> declaration_;
	std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite_;

	// declaration comma_declaration_star_suite
	parameter_list(std::unique_ptr<declaration> declaration, std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite);
	~parameter_list();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// comma_declaration_star_suite ///////////////////////////////////////////

class comma_declaration_star_suite : public Node {
public:
	std::unique_ptr<declaration> declaration_;
	std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite_;

	// TOK_COMMA declaration comma_declaration_star_suite
	comma_declaration_star_suite(std::unique_ptr<declaration> declaration, std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite);
	~comma_declaration_star_suite();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// block ///////////////////////////////////////////

class block : public Node {
	public:
		std::unique_ptr<suite> suite_;
		std::unordered_map<std::string, declaration*> variable_symbol_table_; // namee, declaration
		int block_num_;
		std::string return_type_;
		bool find_return_;

		// TOK_LBRACE suite TOK_RBRACE
		block(std::unique_ptr<suite> suite);
		~block();

		bool verify(rooot* rooot) override;
		void print(int indent) override;
};

///////////////////////////////////////// suite ///////////////////////////////////////////

class suite : public Node {
public:
	std::unique_ptr<statement> statement_;
	std::unique_ptr<suite> suite_;

	// statement suite
	suite(std::unique_ptr<statement> statement, std::unique_ptr<suite> suite);
	~suite();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// declaration ///////////////////////////////////////////

class declaration : public Node {
public:
	std::string type_;
	std::unique_ptr<namee> namee_;

	// TOK_TYPE namee
	declaration(std::string TOK_TYPE, std::unique_ptr<namee> namee);
	~declaration();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// statement ///////////////////////////////////////////

class statement : public Node {
public:
	std::unique_ptr<single_statement> single_statement_;
	std::unique_ptr<compound_statement> compound_statement_;

	// single_statement TOK_SEMICOLON
	statement(std::unique_ptr<single_statement> single_statement);
	// compound_statement
	statement(std::unique_ptr<compound_statement> compound_statement);
	~statement();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// single_statement ///////////////////////////////////////////

class single_statement : public Node {
public:
	int selector_;
	TOK_ENUM token_val_;
	std::unique_ptr<declaration> declaration_;
	std::unique_ptr<expression> expression_;
	std::unique_ptr<namee> namee_;
	std::string return_type_; 
	bool with_return_;

	// selector = 1, declaration TOK_ASSIGN expression
	single_statement(std::unique_ptr<declaration> declaration, TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector);
	// selector = 2, namee TOK_ASSIGN expression
	// selector = 3, namee TOK_PLUS_ASSIGN expression
	// selector = 4, namee TOK_MINUS_ASSIGN expression
	// selector = 5, namee TOK_STAR_ASSIGN expression
	// selector = 6, namee TOK_SLASH_ASSIGN expression
	single_statement(std::unique_ptr<namee> namee, TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector);
	// selector = 7, TOK_BREAK
	// selector = 8, TOK_CONTINUE
	// selector = 9, TOK_RETURN
	single_statement(TOK_ENUM TOK_VAL, int selector);
	// selector = 10, TOK_RETURN expression
	single_statement(TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector);
	// selector = 11, expression
	single_statement(std::unique_ptr<expression> expression, int selector);
	~single_statement();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// expression ///////////////////////////////////////////

class expression : public Node {
public:
	std::unique_ptr<expression_prime> expression_prime_;
	std::unique_ptr<binary_expression> binary_expression_;
	std::unique_ptr<unary_expression> unary_expression_;
	std::unique_ptr<relational_expression> relational_expression_;
	std::unique_ptr<ternary_expression> ternary_expression_;
	std::unique_ptr<cast_expression> cast_expression_;
	std::unique_ptr<function_call> function_call_;
	std::string type_;

	expression(std::unique_ptr<expression_prime> expression_prime);
	expression(std::unique_ptr<binary_expression> binary_expression);
	expression(std::unique_ptr<unary_expression> unary_expression);
	expression(std::unique_ptr<relational_expression> relational_expression);
	expression(std::unique_ptr<ternary_expression> ternary_expression);
	expression(std::unique_ptr<cast_expression> cast_expression);
	expression(std::unique_ptr<function_call> function_call);
	~expression();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// expression_prime ///////////////////////////////////////////

class expression_prime : public Node {
public:
	std::unique_ptr<namee> namee_;
	std::string type_;
	int selector_;
	int integer_val_;
	float float_val_;
	bool bool_val_;
	std::unique_ptr<expression> expression_;

	// selector = 1, namee
	expression_prime(std::unique_ptr<namee> namee, int selector);
	// selector = 2, TOK_TRUE
	// selector = 3, TOK_FALSE
	expression_prime(bool TOK_BOOL_VAL, int selector);
	// selector = 4, TOK_INTEGER
	expression_prime(int integer_val, int selector);
	// selector = 5, TOK_FLOAT
	expression_prime(float float_val, int selector);
	// selector = 6, TOK_LPAREN expression TOK_RPAREN
	expression_prime(std::unique_ptr<expression> expression, int selector);
	~expression_prime();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// compound_statement ///////////////////////////////////////////

class compound_statement : public Node {
public:
	int selector_;
	TOK_ENUM token_;
	std::unique_ptr<single_statement> single_statement_1_;
	std::unique_ptr<single_statement> single_statement_2_;
	std::unique_ptr<expression> expression_;
	std::unique_ptr<block> block_;

	// TOK_IF TOK_LPAREN expression TOK_RPAREN block
	// TOK_WHILE TOK_LPAREN expression TOK_RPAREN block
	compound_statement(TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, std::unique_ptr<block> block, int selector);
	// TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
	// TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON                  TOK_RPAREN block
	// TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
	// TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON                  TOK_RPAREN block
	// TOK_FOR TOK_LPAREN                  TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
	// TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON            TOK_SEMICOLON single_statement TOK_RPAREN block
	// TOK_FOR TOK_LPAREN                  TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
	// TOK_FOR TOK_LPAREN single_statement TOK_SEMICOLON expression TOK_SEMICOLON single_statement TOK_RPAREN block
	compound_statement(TOK_ENUM TOK_VAL, std::unique_ptr<single_statement> single_statement_1, std::unique_ptr<expression> expression, std::unique_ptr<single_statement> single_statement_2, std::unique_ptr<block> block, int selector);
	~compound_statement();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// binary_expression ///////////////////////////////////////////

class binary_expression : public Node {
public:
	std::string type_;
	std::unique_ptr<expression> expression_;
	TOK_ENUM binary_op_;
	std::unique_ptr<expression_prime> expression_prime_;

	// expression binary_op expression_prime
	binary_expression(std::unique_ptr<expression> expression, TOK_ENUM binary_op, std::unique_ptr<expression_prime> expression_prime);
	~binary_expression();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// unary_expression ///////////////////////////////////////////

class unary_expression : public Node {
public:
	std::string type_;
	TOK_ENUM unary_op_;
	std::unique_ptr<expression_prime> expression_prime_;

	// unary_op expression_prime
	unary_expression(TOK_ENUM unary_op, std::unique_ptr<expression_prime> expression_prime);
	~unary_expression();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// relational_expression ///////////////////////////////////////////

class relational_expression : public Node {
public:
	std::string type_;
	std::unique_ptr<expression> expression_;
	TOK_ENUM relational_op_;
	std::unique_ptr<expression_prime> expression_prime_;

	// expression relational_op expression_prime
	relational_expression(std::unique_ptr<expression> expression, TOK_ENUM relational_op, std::unique_ptr<expression_prime> expression_prime);
	~relational_expression();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// ternary_expression ///////////////////////////////////////////

class ternary_expression : public Node {
public:
	std::string type_;
	std::unique_ptr<expression> expression_1_;
	std::unique_ptr<expression> expression_2_;
	std::unique_ptr<expression_prime> expression_prime_;

	// expression TOK_QUESTION_MARK expression TOK_COLON expression_prime
	ternary_expression(std::unique_ptr<expression> expression_1, std::unique_ptr<expression> expression_2, std::unique_ptr<expression_prime> expression_prime);
	~ternary_expression();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// cast_expression ///////////////////////////////////////////

class cast_expression : public Node {
public:

	std::string type_;
	std::unique_ptr<expression_prime> expression_prime_;

	// TOK_LPAREN TOK_TYPE TOK_RPAREN expression_prime
	cast_expression(std::string TOK_TYPE, std::unique_ptr<expression_prime> expression_prime);
	~cast_expression();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// function_call ///////////////////////////////////////////

class function_call : public Node {
public:
	std::string type_;
	std::unique_ptr<namee> namee_;
	std::unique_ptr<comma_expression_star_quesmark_suite> comma_expression_star_quesmark_suite_;

	// namee TOK_LPAREN comma_expression_star_quesmark_suite TOK_RPAREN
	function_call(std::unique_ptr<namee> namee, std::unique_ptr<comma_expression_star_quesmark_suite> comma_expression_star_quesmark_suite);
	~function_call();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// comma_expression_star_quesmark_suite ///////////////////////////////////////////

class comma_expression_star_quesmark_suite : public Node {
public:
	std::unique_ptr<expression> expression_;
	std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite_;

	// expression comma_expression_star_suite
	comma_expression_star_quesmark_suite(std::unique_ptr<expression> expression, std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite);
	~comma_expression_star_quesmark_suite();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

///////////////////////////////////////// comma_expression_star_suite ///////////////////////////////////////////

class comma_expression_star_suite : public Node {
public:
	std::unique_ptr<expression> expression_;
	std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite_;

	// TOK_COMMA expression comma_expression_star_suite
	comma_expression_star_suite(std::unique_ptr<expression> expression, std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite);
	~comma_expression_star_suite();

	bool verify(rooot* rooot) override;
	void print(int indent) override;
};

#endif // ECE467_NODE_HPP_INCLUDED
