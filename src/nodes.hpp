#ifndef ECE467_NODE_HPP_INCLUDED
#define ECE467_NODE_HPP_INCLUDED

#include "location.hh"
#include <memory>

class Node {
public:
	yy::location location;

	virtual ~Node() = 0;
};

#endif // ECE467_NODE_HPP_INCLUDED

///////////////////////////////////////// root /////////////////////////////////////////// 
class root : public Node {
public:
	<std::unique_ptr<Node>> function_list = nullptr;
	root(<std::unique_ptr<Node>> function_list_) {
		function_list = function_list_;
	}

	std:bool verify() {
		std:bool result = true;
		if (function_list != nullptr) {
			result = function_list.verify();
			if (result == false) {
				return false;
			}
		}
	}

	virtual ~root() = 0;
};

///////////////////////////////////////// function_list /////////////////////////////////////////// 
class function_list : public Node {
public:
	<std::unique_ptr<Node>> function_list_prime = nullptr; 
	<std::unique_ptr<Node>> function = nullptr; 
	<std::unique_ptr<Node>> function_list = nullptr; 
	function_list(<std::unique_ptr<Node>> function_list_prime_, <std::unique_ptr<Node>> function_) {
		function_list_prime = function_list_prime_;
		function = function_;
	}

	function_list(<std::unique_ptr<Node>> function_list_) {
		function_list = function_list_;
	}

	std:bool verify() {
		std:bool result = true;
		if (function_list_prime != nullptr) {
			result = function_list_prime.verify();
			if (result == false) {
				return false;
			}
		}

		if (function != nullptr) {
			result = function.verify();
			if (result == false) {
				return false;
			}
		}

		if (function_list != nullptr) {
			result = function_list.verify();
			if (result == false) {
				return false;
			}
		}
		return result;
	}

	virtual ~function_list() = 0;
};

static enum Semicolon { TOK_SEMICOLON = 1};
///////////////////////////////////////// function /////////////////////////////////////////// 
class function : public Node {
public:
	<std::unique_ptr<Node>> function_decl;
	<std::unique_ptr<Node>> function_defn;
	std::bool with_semicolon = false; 
	
	//not sure what type and value does $1 (TOK_SEMICOLON) pass into the the constructer
	//this is a draft code that needs correct later
	function(<std::unique_ptr<Node>> function_decl_, int tok_) {
		if (Semicolon tok = static_cast<Semicolon>(tok_)){
			with_semicolon = true;
			function_decl = function_decl_;
		}
	}

	function(<std::unique_ptr<Node>> function_defn_) {
		function_defn = function_defn_; 
	}

	std:bool verify() {
		if (with_semicolon && function_decl_ ) {
			return function_decl.verify();
		} else if (function_defn) {
			return function_defn.verify();
		}
	}

	virtual ~function() = 0;
};

static enum Type { TOK_BOOL = 1, TOK_INT, TOK_FLOAT};
static enum Paren { TOK_LPAREN = 1, TOK_RPAREN};
///////////////////////////////////////// function_decl /////////////////////////////////////////// 
class function_decl : public Node {
public:
	<std::unique_ptr<Node>> name;
	<std::unique_ptr<Node>> parameter_list;

	// use unknow as key word for token, change later 
	function_decl(unknow TOK_TYPE, <std::unique_ptr<Node>>name_, unknow TOK_LPAREN, <std::unique_ptr<Node>> parameter_list_, unknow TOK_RPAREN) {
		name = name_;
		parameter_list = parameter_list_;
	}
	
	std:bool verify() {
	}

	virtual ~function_decl() = 0;
};




///////////////////////////////////////// temp /////////////////////////////////////////// 
class temp : public Node {
public:
	<std::unique_ptr<Node>> ;
	
	temp() {}

	std:bool verify() {}

	virtual ~temp() = 0;
};