#include "nodes_backward.hpp"

Node::~Node() = default;

//////////////////////////////// ternary_expression //////////////////////////////////
ternary_expression::ternary_expression( <std::unique_ptr<Node>> expression_1_, TOK_QUESTION_MARK, <std::unique_ptr<Node>> expression_2_, unknow TOK_COLON, <std::unique_ptr<Node>> expression_prime_){
    expression_1 = std::move(expression_1_);
    expression_2 = std::move(expression_2_);
    expression_prime = std::move(expression_prime_);
}
std:bool ternary_expression::verify(){
    return true;
}

//////////////////////////////// cast_expression //////////////////////////////////
cast_expression::cast_expression( unknow TOK_LPAREN, unknow TOK_TYPE, unknow TOK_RPAREN, <std::unique_ptr<Node>> expression_prime_){
    expression_prime = std::move(expression_prime_);
}
std:bool cast_expression::verify(){
    return true;
}

//////////////////////////////// function_call //////////////////////////////////
function_call::function_call( <std::unique_ptr<Node>> name_, unknow TOK_LPAREN,  <std::unique_ptr<Node>> comma_expression_star_quesmark_suite_, unknow TOK_RPAREN){
    name = std::move(name_);
    comma_expression_star_quesmark_suite = std::move(comma_expression_star_quesmark_suite_);
}
std:bool function_call::verify(){
    return true;
}

// comma_expression_star_quesmark_suite and comma_expression_star_suite may can be combined 
//////////////////////////////// comma_expression_star_quesmark_suite //////////////////////////////////
comma_expression_star_quesmark_suite::comma_expression_star_quesmark_suite(unkonw TOK_COMMA, <std::unique_ptr<Node>> expression_, <std::unique_ptr<Node>> comma_expression_star_suite_) {
        expression = std::move(expression_);
        comma_expression_star_suite = std::move(comma_expression_star_suite_);
}
std:bool comma_expression_star_quesmark_suite::verify(){
    return true;
}

//////////////////////////////// comma_expression_star_suite //////////////////////////////////
comma_expression_star_suite::comma_expression_star_suite(unkonw TOK_COMMA, <std::unique_ptr<Node>> expression_, <std::unique_ptr<Node>> comma_expression_star_suite_) {
        expression = std::move(expression_);
        comma_expression_star_suite = std::move(comma_expression_star_suite_);
}
std:bool comma_expression_star_suite::verify(){
    return true;
};
