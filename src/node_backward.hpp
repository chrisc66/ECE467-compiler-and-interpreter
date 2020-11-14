class Node {
public:
	yy::location location;

	virtual ~Node() = 0;
};

//////////////////////////////// ternary_expression //////////////////////////////////
class ternary_expression : public Node{
public: 
    <std::unique_ptr<Node>> expression_1;
    <std::unique_ptr<Node>> expression_2;
    <std::unique_ptr<Node>> expression_prime;
    // TOK_COMMA may need to set

    ternary_expression( <std::unique_ptr<Node>> expression_1_，unknow TOK_QUESTION_MARK, <std::unique_ptr<Node>> expression_2_, unknow TOK_COLON, <std::unique_ptr<Node>> expression_prime_);
    std:bool verify();
}

//////////////////////////////// cast_expression //////////////////////////////////
class cast_expression : public Node{
public: 
    <std::unique_ptr<Node>> expression_prime;
    // TOK_COMMA may need to set

    cast_expression( unknow TOK_LPAREN, unknow TOK_TYPE, unknow TOK_RPAREN, <std::unique_ptr<Node>> expression_prime_);
    std:bool verify();
}

//////////////////////////////// function_call //////////////////////////////////
class function_call : public Node{
public: 
    <std::unique_ptr<Node>> name;
    <std::unique_ptr<Node>> comma_expression_star_quesmark_suite;
    // TOK_COMMA may need to set

    function_call( <std::unique_ptr<Node>> name_, unknow TOK_LPAREN,  <std::unique_ptr<Node>> comma_expression_star_quesmark_suite_, unknow TOK_RPAREN);
    std:bool verify();
}

// comma_expression_star_quesmark_suite and comma_expression_star_suite may can be combined 
//////////////////////////////// comma_expression_star_quesmark_suite //////////////////////////////////
class comma_expression_star_quesmark_suite : public Node{
public: 
    <std::unique_ptr<Node>> expression;
    <std::unique_ptr<Node>> comma_expression_star_suite;
    // TOK_COMMA may need to set

    comma_expression_star_quesmark_suite( <std::unique_ptr<Node>> expression_, <std::unique_ptr<Node>> comma_expression_star_suite_);
    std:bool verify();
}

//////////////////////////////// comma_expression_star_suite //////////////////////////////////
class comma_expression_star_suite : public Node{
public: 
    <std::unique_ptr<Node>> expression;
    <std::unique_ptr<Node>> comma_expression_star_suite;
    // TOK_COMMA may need to set

    comma_expression_star_suite(unkonw TOK_COMMA, <std::unique_ptr<Node>> expression_, <std::unique_ptr<Node>> comma_expression_star_suite_);
    std:bool verify();
}