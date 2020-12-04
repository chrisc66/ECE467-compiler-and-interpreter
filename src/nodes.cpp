#include "nodes.hpp"

///////////////////////////////////////// base class: Node ///////////////////////////////////////////

Node::~Node() = default;

bool Node::verify() {
    rooot* rooot_ = dynamic_cast<rooot*> (this);
    return rooot_->verify(rooot_);
}

bool Node::verify(rooot* rooot) {
    return rooot->verify(rooot);
}

void Node::print(int indent) {}

///////////////////////////////////////// root ///////////////////////////////////////////

rooot::rooot(std::unique_ptr<function_list> function_list) : Node() {
    function_list_ = std::move(function_list);
    function_symbol_table_.clear();
    variable_symbol_table_.clear();
    block_count_ = 1;
}

rooot::~rooot() {}

bool rooot::verify(rooot* rooot) {
    bool result = true;
    if (result && function_list_ != nullptr){
        result = function_list_->verify(rooot);
    }
    // ERROR: main_function
    auto search = function_symbol_table_.find("main");
    if (result && search == function_symbol_table_.end()){
        print_error_msg(ERROR_ENUM::ERROR_MAIN_FUNCTION, location.begin);
        return false;
    }
    return result;
}

void rooot::print(int indent) {
    std::cout << std::setw(indent) << "rooot (" << location.begin << ") {" << std::endl;
    if (function_list_ != nullptr)
        function_list_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// function_list ///////////////////////////////////////////

function_list::function_list(std::unique_ptr<function_list> function_list_prime, std::unique_ptr<function> function) : Node() {
    function_list_prime_ = std::move(function_list_prime);
    function_ = std::move(function);
}

function_list::~function_list() {}

bool function_list::verify(rooot* rooot) {
    bool result = true;
    if (result && function_list_prime_ != nullptr){
        result = function_list_prime_->verify(rooot);
    }
    if (result && function_ != nullptr){
        result = function_->verify(rooot);
    }
    return result;
}

void function_list::print(int indent) {
    std::cout << std::setw(indent) << "function_list (" << location.begin << ") {" << std::endl;
    if (function_list_prime_ != nullptr)
        function_list_prime_->print(indent+4);
    if (function_ != nullptr)
        function_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// function ///////////////////////////////////////////

function::function(std::unique_ptr<function_decl> function_decl) : Node() {
    function_decl_ = std::move(function_decl);
    function_defn_ = nullptr;
}

function::function(std::unique_ptr<function_defn> function_defn) : Node() {
    function_decl_ = nullptr;
    function_defn_ = std::move(function_defn);
}

function::~function() {}

bool function::verify(rooot* rooot) {
    bool result = true;
    if (result && function_decl_ != nullptr)
        result = function_decl_->verify(rooot);
    if (result && function_defn_ != nullptr)
        result = function_defn_->verify(rooot);
    return result;
}

void function::print(int indent) {
    std::cout << std::setw(indent) << "function (" << location.begin << ") {" << std::endl;
    if (function_decl_ != nullptr)
        function_decl_->print(indent+4);
    if (function_defn_ != nullptr)
        function_defn_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// function_decl ///////////////////////////////////////////

function_decl::function_decl(std::string TOK_TYPE, std::unique_ptr<namee>namee, std::unique_ptr<parameter_list> parameter_list) : Node() {
    type_ = TOK_TYPE;
    namee_ = std::move(namee);
    namee_->is_function_ = true;
    namee_->type_ = TOK_TYPE;
    parameter_list_ = std::move(parameter_list);;
}

function_decl::~function_decl() {}

bool function_decl::verify(rooot* rooot) {
    // ERROR type_decl: Functions may have return type void, bool, int, float
    std::string void_ = "void";
    std::string bool_ = "bool";
    std::string int_ = "int";
    std::string float_ = "float";
    if (type_.compare(void_) != 0 && type_.compare(bool_) != 0 && type_.compare(int_) != 0 && type_.compare(float_) != 0){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_DECL, location.begin);
        return false;
    }
    // ERROR duplicate_decl
    auto search = rooot->function_symbol_table_.find(namee_->identifier_val_);
    if (search == rooot->function_symbol_table_.end()){
        rooot->function_symbol_table_.insert({namee_->identifier_val_, this});
    }
    else {
        print_error_msg(ERROR_ENUM::ERROR_DUPLICATE_DECL, location.begin);
        return false;
    }
    
    bool result = true;
    if (result && namee_ != nullptr){
        result = namee_->verify(rooot);
    }
    if (result && parameter_list_ != nullptr){
        result = parameter_list_->verify(rooot);
    }
    return result;
}

void function_decl::print(int indent) {
    std::cout << std::setw(indent) << "function_decl (" << location.begin << ") {" << std::endl;
    if (namee_ != nullptr)
        namee_->print(indent+4);
    if (parameter_list_ != nullptr)
        parameter_list_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// function_defn ///////////////////////////////////////////

function_defn::function_defn(std::unique_ptr<function_decl> function_decl, std::unique_ptr<block> block) : Node() {
    function_decl_ = std::move(function_decl);
    block_ = std::move(block);
}

function_defn::~function_defn() {}

bool function_defn::verify(rooot* rooot) {
    bool result = true;
    if (result && function_decl_ != nullptr){
        result = result && function_decl_->verify(rooot);
    }
    if (result && block_ != nullptr){
        result = result && block_->verify(rooot);
    }
    if (result && block_->find_return_) {
        if (block_->return_type_.compare("") == 0){}
        else if (block_->return_type_.compare(function_decl_->type_) != 0) {
            if ( block_->return_type_.compare("void") == 0 || function_decl_->type_.compare("void") != 0){
                print_error_msg(ERROR_ENUM::ERROR_RETURN_STMT, location.begin);
            } else {
                print_error_msg(ERROR_ENUM::ERROR_TYPE_RETURN, location.begin);
            }
            return false;
        }
    }    
    return result;
}

void function_defn::print(int indent) {
    std::cout << std::setw(indent) << "function_defn (" << location.begin << ") {" << std::endl;
    if (function_decl_ != nullptr)
        function_decl_->print(indent+4);
    if (block_ != nullptr)
        block_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// namee ///////////////////////////////////////////

namee::namee(std::string identifier_val) : Node() {
    identifier_val_ = identifier_val;
    type_ = "";
    is_function_ = false;
    is_variable_ = false;
}

namee::~namee() {}

bool namee::verify(rooot* rooot) {
    return true; // leaf node
}

void namee::print(int indent) {
    std::cout << std::setw(indent) << "namee (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent+4) << "identifier_val_: " << identifier_val_ << std::endl;
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// parameter_list ///////////////////////////////////////////

parameter_list::parameter_list(std::unique_ptr<declaration> declaration, std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite) : Node()  {
    declaration_ = std::move(declaration);
    comma_declaration_star_suite_ = std::move(comma_declaration_star_suite);
}

parameter_list::~parameter_list() {}

bool parameter_list::verify(rooot* rooot) {
    bool result = true;
    if (result && declaration_ != nullptr){
        result = declaration_->verify(rooot);
    }
    if (result && comma_declaration_star_suite_ != nullptr){
        result = comma_declaration_star_suite_->verify(rooot);
    }
    return result;
}

void parameter_list::print(int indent) {
    std::cout << std::setw(indent) << "parameter_list (" << location.begin << ") {" << std::endl;
    if (declaration_ != nullptr)
        declaration_->print(indent+4);
    if (comma_declaration_star_suite_ != nullptr)
        comma_declaration_star_suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// comma_declaration_star_suite ///////////////////////////////////////////

comma_declaration_star_suite::comma_declaration_star_suite(std::unique_ptr<declaration> declaration, std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite) : Node() {
    declaration_ = std::move(declaration);
    comma_declaration_star_suite_ = std::move(comma_declaration_star_suite);
}

comma_declaration_star_suite::~comma_declaration_star_suite() {}

bool comma_declaration_star_suite::verify(rooot* rooot) {
    bool result = true;
    if (result && declaration_ != nullptr){
        result = declaration_->verify(rooot);
    }
    if (result && comma_declaration_star_suite_ != nullptr){
        result = comma_declaration_star_suite_->verify(rooot);
    }
    return result;
}

void comma_declaration_star_suite::print(int indent) {
    std::cout << std::setw(indent) << "comma_declaration_star_suite (" << location.begin << ") {" << std::endl;
    if (declaration_ != nullptr)
        declaration_->print(indent+4);
    if (comma_declaration_star_suite_ != nullptr)
        comma_declaration_star_suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// block ///////////////////////////////////////////

block::block(std::unique_ptr<suite> suite) : Node() {
    suite_ = std::move(suite);
    variable_symbol_table_.clear();
    block_num_ = 0;
    return_type_ = "";
    find_return_ = false;
}

block::~block() {}

bool block::verify(rooot* rooot) {
    rooot->block_count_ ++;
    block_num_  = rooot->block_count_;
    suite* suite_itr = suite_.get(); 
    statement* statement_itr = nullptr;
    single_statement* single_statement_itr = nullptr;
    while (suite_itr != nullptr) {
        statement_itr = suite_itr->statement_.get();
        if (statement_itr != nullptr) {
            single_statement_itr = statement_itr->single_statement_.get();
        } 
        if (single_statement_itr != nullptr) {
            // ERRPR_RETURN_TYPE
            if (single_statement_itr->with_return_) {
                if (!find_return_) {
                    return_type_ = single_statement_itr->return_type_;
                    find_return_ = true;
                } else {
                    if (return_type_ != single_statement_itr->return_type_) {
                        print_error_msg(ERROR_ENUM::ERROR_TYPE_RETURN, location.begin);
                        return false;
                    }
                }
            }
            if (single_statement_itr->declaration_ != nullptr) { 
                // ERROR_DUPLICATE_DECL
                auto search = variable_symbol_table_.find(single_statement_itr->declaration_->namee_->identifier_val_);
                if (search == variable_symbol_table_.end()){
                    variable_symbol_table_.insert({single_statement_itr->declaration_->namee_->identifier_val_, single_statement_itr->declaration_.get()});
                }
                else {
                    print_error_msg(ERROR_ENUM::ERROR_DUPLICATE_DECL, location.begin);
                    return false;
                }
            }
        }
        suite_itr = suite_itr->suite_.get();
    }
    bool result = true;
    if (result && suite_ != nullptr){
        result = suite_->verify(rooot);
    }
    return result;
}

void block::print(int indent) {
    std::cout << std::setw(indent) << "block (" << location.begin << ") {" << std::endl;
    if (suite_ != nullptr)
        suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// suite ///////////////////////////////////////////

suite::suite(std::unique_ptr<statement> statement, std::unique_ptr<suite> suite) : Node() {
    statement_ = std::move(statement);
    suite_ = std::move(suite);
}

suite::~suite() {}

bool suite::verify(rooot* rooot) {
    bool result = true;
    if (result && statement_ != nullptr){
        result = statement_->verify(rooot);
    }
    if (result && suite_ != nullptr){
        result = suite_->verify(rooot);
    }
    return result;
}

void suite::print(int indent) {
    std::cout << std::setw(indent) << "suite (" << location.begin << ") {" << std::endl;
    if (statement_ != nullptr)
        statement_->print(indent+4);
    if (suite_ != nullptr)
        suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// declaration ///////////////////////////////////////////

declaration::declaration(std::string TOK_TYPE, std::unique_ptr<namee> namee) : Node() {
    type_ = TOK_TYPE;
    namee_ = std::move(namee);
    namee_->is_variable_ = true;
    namee_->type_ = TOK_TYPE;
}

declaration::~declaration() {}

bool declaration::verify(rooot* rooot) {
    // ERROR type_decl: variable types must be one of bool, int, float
    std::string bool_ = "bool";
    std::string int_ = "int";
    std::string float_ = "float";
    if (type_.compare(bool_) != 0 && type_.compare(int_) != 0 && type_.compare(float_) != 0){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_DECL, location.begin);
        return false;
    }
    bool result = true;
    if (result && namee_ != nullptr){
        result = namee_->verify(rooot);
    }
    return result;
}

void declaration::print(int indent) {
    std::cout << std::setw(indent) << "declaration (" << location.begin << ") {" << std::endl;
    if (namee_ != nullptr)
        namee_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// statement ///////////////////////////////////////////

statement::statement(std::unique_ptr<single_statement> single_statement) : Node() {
    single_statement_ = std::move(single_statement);
    compound_statement_ = nullptr;
}

statement::statement(std::unique_ptr<compound_statement> compound_statement) : Node() {
    single_statement_ = nullptr;
    compound_statement_ = std::move(compound_statement);
}

statement::~statement() {}

bool statement::verify(rooot* rooot) {
    bool result = true;
    if (result && single_statement_ != nullptr){
        result = single_statement_->verify(rooot);
    }
    if (result && compound_statement_ != nullptr){
        result = compound_statement_->verify(rooot);
    }
    return result;
}

void statement::print(int indent) {
    std::cout << std::setw(indent) << "statement (" << location.begin << ") {" << std::endl;
    if (single_statement_ != nullptr)
        single_statement_->print(indent+4);
    if (compound_statement_ != nullptr)
        compound_statement_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// single_statement ///////////////////////////////////////////

single_statement::single_statement(std::unique_ptr<declaration> declaration, TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector) : Node() {
    selector_ = selector;
    if (selector == 1) {
        declaration_ = std::move(declaration);
        token_val_ = TOK_VAL;
        expression_ = std::move(expression);
    }
    else {
        declaration_ = nullptr;
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
    }
    return_type_ = "";
    with_return_ = false;
    namee_ = nullptr;
}

single_statement::single_statement(std::unique_ptr<namee> namee, TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector) : Node() {
    selector_ = selector;
    if (selector >= 2 && selector <= 6) {
        namee_ = std::move(namee);
        token_val_ = TOK_VAL;
        expression_ = std::move(expression);
    }
    else {
        declaration_ = nullptr;
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
    }
    return_type_ = "";
    with_return_ = false;
    declaration_ = nullptr;
}

single_statement::single_statement(TOK_ENUM TOK_VAL, int selector) : Node() {
    selector_ = selector;
    if (selector >= 7 && selector <= 9){
        token_val_ = TOK_VAL;
    }
    else {
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
    }
    if (selector == 9) {
        return_type_ = "void"; 
        with_return_ = true;
    } else {
        return_type_ = "";
        with_return_ = false;
    }
    declaration_ = nullptr;
    expression_ = nullptr;
    namee_ = nullptr;
}

single_statement::single_statement(TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector) : Node() {
    selector_ = selector;
    if (selector_ == 10) {
        token_val_ = TOK_VAL;
        expression_ = std::move(expression);
        return_type_ = expression_->type_;
        with_return_ = true;
    }
    else {
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
    }
    declaration_ = nullptr;
    namee_ = nullptr;
}

single_statement::single_statement(std::unique_ptr<expression> expression, int selector) : Node () {
    selector_ = selector;
    if (selector == 11){
        expression_ = std::move(expression);
    }
    else {
        expression_ = nullptr;
    }
    token_val_ = TOK_ENUM::TOK_UNDEFINE;
    declaration_ = nullptr;
    namee_ = nullptr;
    return_type_ = "";
    with_return_ = false;
}

single_statement::~single_statement() {}

bool single_statement::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    if (result && declaration_ != nullptr){
        result = declaration_->verify(rooot);
    }
    if (result && namee_ != nullptr){
        result = namee_->verify(rooot);
    }
    return result;
}

void single_statement::print(int indent) {
    std::cout << std::setw(indent) << "single_statement (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent+4) << "Token Enum: " << token_val_ << std::endl;
    if (declaration_ != nullptr)
        declaration_->print(indent+4);
    if (expression_ != nullptr)
        expression_->print(indent+4);
    if (namee_ != nullptr)
        namee_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// expression ///////////////////////////////////////////

expression::expression(std::unique_ptr<expression_prime> expression_prime) : Node() {
    expression_prime_ = std::move(expression_prime);
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = expression_prime_->type_;
}

expression::expression(std::unique_ptr<binary_expression> binary_expression) : Node() {
    expression_prime_ = nullptr;
    binary_expression_ = std::move(binary_expression);
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = binary_expression_->type_;
}

expression::expression(std::unique_ptr<unary_expression> unary_expression) : Node() {
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = std::move(unary_expression);
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = unary_expression_->type_;
}

expression::expression(std::unique_ptr<relational_expression> relational_expression) : Node() {
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = std::move(relational_expression);
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = "bool";
}

expression::expression(std::unique_ptr<ternary_expression> ternary_expression) : Node() {
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = std::move(ternary_expression);
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = ternary_expression_->type_;
}

expression::expression(std::unique_ptr<cast_expression> cast_expression) : Node() {
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = std::move(cast_expression);
    function_call_ = nullptr;
    type_ = cast_expression_->type_;
}

expression::expression(std::unique_ptr<function_call> function_call) : Node() {
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = std::move(function_call);
    type_ = function_call_->type_;
}

expression::~expression() {}

bool expression::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_prime_ != nullptr){
        result = expression_prime_->verify(rooot);
    }
    if (result && binary_expression_ != nullptr){
        result = binary_expression_->verify(rooot);
    }
    if (result && relational_expression_ != nullptr){
        result = relational_expression_->verify(rooot);
    }
    if (result && ternary_expression_ != nullptr){
        result = ternary_expression_->verify(rooot);
    }
    if (result && cast_expression_ != nullptr){
        result = cast_expression_->verify(rooot);
    }
    if (result && function_call_ != nullptr){
        result = function_call_->verify(rooot);
    }
    return result;
}

void expression::print(int indent) {
    std::cout << std::setw(indent) << "expression (" << location.begin << ") {" << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent+4);
    if (binary_expression_ != nullptr)
        binary_expression_->print(indent+4);
    if (unary_expression_ != nullptr)
        unary_expression_->print(indent+4);
    if (relational_expression_ != nullptr)
        relational_expression_->print(indent+4);
    if (ternary_expression_ != nullptr)
        ternary_expression_->print(indent+4);
    if (cast_expression_ != nullptr)
        cast_expression_->print(indent+4);
    if (function_call_ != nullptr)
        function_call_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// expression_prime ///////////////////////////////////////////

expression_prime::expression_prime(std::unique_ptr<namee> namee, int selector) : Node() {
    selector_ = selector;
    if (selector == 1){
        namee_ = std::move(namee);
        type_ = namee_->type_;
    }
    expression_ = nullptr;
}

expression_prime::expression_prime(bool TOK_BOOL, int selector) : Node() {
    selector_ = selector;
    if (selector == 2 || selector == 3){
        bool_val_ = TOK_BOOL;
        type_ = "bool";
    }
    namee_ = nullptr;
    expression_ = nullptr;
}

expression_prime::expression_prime(int integer_val, int selector) : Node() {
    selector_ = selector;
    if (selector == 4){
        integer_val_ = integer_val;
        type_ = "int";
    }
    namee_ = nullptr;
    expression_ = nullptr;
}

expression_prime::expression_prime(float float_val, int selector) : Node() {
    selector_ = selector;
    if (selector == 5){
        float_val_ = float_val;
        type_ = "float";
    }
    namee_ = nullptr;
    expression_ = nullptr;
}

expression_prime::expression_prime(std::unique_ptr<expression> expression, int selector) : Node() {
    selector_ = selector;
    if (selector == 6){
        expression_ = std::move(expression);
        type_ = expression_->type_;
    }
    else {
        expression_ = nullptr;
    }
    namee_ = nullptr;
}

expression_prime::~expression_prime() {}

bool expression_prime::verify(rooot* rooot) {
    bool result = true;
    if (result && namee_ != nullptr){
        result = namee_->verify(rooot);
    }
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    return result;
}

void expression_prime::print(int indent) {
    std::cout << std::setw(indent) << "expression_prime (" << location.begin << ") {" << std::endl;
    if (selector_ == 1 && namee_ != nullptr)
        namee_->print(indent+4);
    else if (selector_ == 2 || selector_ == 3)
        if (bool_val_)
            std::cout << std::setw(indent+4) << "bool: true" << std::endl;
        else
            std::cout << std::setw(indent+4) << "bool: false" << std::endl;
    else if (selector_ == 4)
        std::cout << std::setw(indent+4) << "int: " << integer_val_ << std::endl;
    else if (selector_ == 5)
        std::cout << std::setw(indent+4) << "float: " << float_val_ << std::endl;
    else if (selector_ == 6 && expression_ != nullptr)
        expression_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// compound_statement ///////////////////////////////////////////

compound_statement::compound_statement(TOK_ENUM token, std::unique_ptr<expression> expression, std::unique_ptr<block> block, int selector) : Node() {
    selector_ = selector;
    if (selector == 1 || selector_== 10) {
        token_ = token;
        expression_ = std::move(expression);
        block_ = std::move(block);
    }
    else {
        token_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
        block_ = nullptr;
    }
    single_statement_1_ = nullptr;
    single_statement_2_ = nullptr;
}

compound_statement::compound_statement(TOK_ENUM token, std::unique_ptr<single_statement> single_statement_1, std::unique_ptr<expression> expression, std::unique_ptr<single_statement> single_statement_2, std::unique_ptr<block> block, int selector) : Node() {
    selector_ = selector;
    if (selector >= 2 && selector_ <= 9){
        token_ = token_;
        single_statement_1_ = std::move(single_statement_1);
        expression_ = std::move(expression);
        single_statement_2_ = std::move(single_statement_2);
        block_ = std::move(block);
    }
    else {
        token_ = TOK_ENUM::TOK_UNDEFINE;
        single_statement_1_ = nullptr;
        expression_ = nullptr;
        single_statement_2_ = nullptr;
        block_ = nullptr;
    }
}

compound_statement::~compound_statement() {}

bool compound_statement::verify(rooot* rooot) {
    // ERROR type_bool: type of an if-statement, for-statement, while-statement predicate must be bool
    if (selector_ == 1 || selector_ == 10){ // if-statement, while-statement
        std::string stmt_type = expression_->type_;
        if (stmt_type.compare("bool") != 0){
            print_error_msg(ERROR_ENUM::ERROR_TYPE_BOOL, location.begin);
            return false;
        }
        // if (selector_ == 1 && expression_->expression_prime_!= nullptr) {
        //     std::cout<<"DEAD CODE ELIMINATION: IF WITH CONSTANT EXPRESSSION"<<std::endl;
        //     exit(1);
        // }

        // if (selector_ == 10 && expression_->expression_prime_!= nullptr && 
        //     (expression_->expression_prime_->bool_val_ == false )) {
        //     std::cout<<"DEAD CODE ELIMINATION: WHILE WITH false EXPRESSSION"<<std::endl;
        //     exit(1);
        // }
    }
    if (selector_ >= 2 && selector_ <= 9){ // while-statement
        std::string stmt_type = "empty";
        if (expression_ != nullptr)
            stmt_type = expression_->type_;
        if (stmt_type.compare("bool") != 0 && stmt_type.compare("empty") != 0){
            print_error_msg(ERROR_ENUM::ERROR_TYPE_BOOL, location.begin);
            return false;
        }
    }
    bool result = true;
    if (result && single_statement_1_ != nullptr){
        result = single_statement_1_->verify(rooot);
    }
    if (result && single_statement_2_ != nullptr){
        result = single_statement_2_->verify(rooot);
    }
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    if (result && block_ != nullptr){
        result = block_->verify(rooot);
    }
    return result;
}

void compound_statement::print(int indent) {
    std::cout << std::setw(indent) << "compound_statement (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent+4) << "Token Enum: " << token_ << std::endl;
    if (single_statement_1_ != nullptr)
        single_statement_1_->print(indent+4);
    if (single_statement_2_ != nullptr)
        single_statement_2_->print(indent+4);
    if (expression_ != nullptr)
        expression_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// binary_expression ///////////////////////////////////////////

binary_expression::binary_expression(std::unique_ptr<expression> expression, TOK_ENUM binary_op, std::unique_ptr<expression_prime> expression_prime) : Node() {
    expression_ = std::move(expression);
    binary_op_ = binary_op;
    expression_prime_ = std::move(expression_prime);
    type_ = expression_prime_->type_;
}

binary_expression::~binary_expression() {}

bool binary_expression::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    if (result && expression_prime_ != nullptr){
        result = expression_prime_->verify(rooot);
    }
    // ERROR type_mismatch: operands of a binary or relational operator have the same type
    std::string expr_type_1 = expression_->type_;
    std::string expr_type_2 = expression_prime_->type_;
    if (result && expr_type_1.compare(expr_type_2) != 0 && expr_type_1.compare("") != 0 && expr_type_2.compare("") != 0){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_MISMATCH, location.begin);
        return false;
    }
    return result;
}

void binary_expression::print(int indent) {
    std::cout << std::setw(indent) << "binary_expression (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent+4);
    std::cout << std::setw(indent+4) << "Token Enum: " << binary_op_ << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// unary_expression ///////////////////////////////////////////

unary_expression::unary_expression(TOK_ENUM unary_op, std::unique_ptr<expression_prime> expression_prime) : Node() {
    unary_op_ = unary_op;
    expression_prime_ = std::move(expression_prime);
    type_ = expression_prime_->type_;
}

unary_expression::~unary_expression() {}

bool unary_expression::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_prime_ != nullptr){
        result = expression_prime_->verify(rooot);
    }
    return result;
}

void unary_expression::print(int indent) {
    std::cout << std::setw(indent) << "unary_expression (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent+4) << "Token Enum: " << unary_op_ << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// relational_expression ///////////////////////////////////////////

relational_expression::relational_expression(std::unique_ptr<expression> expression, TOK_ENUM relational_op, std::unique_ptr<expression_prime> expression_prime) : Node() {
    expression_ = std::move(expression);
    relational_op_ = relational_op;
    expression_prime_ = std::move(expression_prime);
    type_ = "bool";
}

relational_expression::~relational_expression() {}

bool relational_expression::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    if (result && expression_prime_ != nullptr){
        result = expression_prime_->verify(rooot);
    }
    // ERROR type_mismatch: operands of a binary or relational operator have the same type
    std::string expr_type_1 = expression_->type_;
    std::string expr_type_2 = expression_prime_->type_;
    if (result && expr_type_1.compare(expr_type_2) != 0 && expr_type_1.compare("") != 0 && expr_type_2.compare("") != 0){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_MISMATCH, location.begin);
        return false;
    }
    return result;
}

void relational_expression::print(int indent) {
    std::cout << std::setw(indent) << "relational_expression (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent+4);
    std::cout << std::setw(indent+4) << "Token Enum: " << relational_op_ << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// ternary_expression ///////////////////////////////////////////

ternary_expression::ternary_expression(std::unique_ptr<expression> expression_1, std::unique_ptr<expression> expression_2, std::unique_ptr<expression_prime> expression_prime) : Node() {
    expression_1_ = std::move(expression_1);
    expression_2_ =std::move(expression_2);
    expression_prime_ = std::move(expression_prime);
    type_ = expression_prime_->type_;
}

ternary_expression::~ternary_expression() {}

bool ternary_expression::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_1_ != nullptr){
        result = expression_1_->verify(rooot);
    }
    if (result && expression_2_ != nullptr){
        result = expression_2_->verify(rooot);
    }
    if (result && expression_prime_ != nullptr){
        result = expression_prime_->verify(rooot);
    }
    return result;
}

void ternary_expression::print(int indent) {
    std::cout << std::setw(indent) << "ternary_expression (" << location.begin << ") {" << std::endl;
    if (expression_1_ != nullptr)
        expression_1_->print(indent+4);
    if (expression_2_ != nullptr)
        expression_2_->print(indent+4);
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// cast_expression ///////////////////////////////////////////

cast_expression::cast_expression(std::string TOK_TYPE, std::unique_ptr<expression_prime> expression_prime) : Node() {
    type_ = TOK_TYPE;
    expression_prime_ = std::move(expression_prime);
}

cast_expression::~cast_expression() {}

bool cast_expression::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_prime_ != nullptr){
        result = expression_prime_->verify(rooot);
    }
    return result;
}

void cast_expression::print(int indent) {
    std::cout << std::setw(indent) << "cast_expression (" << location.begin << ") {" << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// function_call ///////////////////////////////////////////

function_call::function_call(std::unique_ptr<namee> namee, std::unique_ptr<comma_expression_star_quesmark_suite> comma_expression_star_quesmark_suite) : Node() {
    namee_ = std::move(namee);
    comma_expression_star_quesmark_suite_ = std::move(comma_expression_star_quesmark_suite);
    type_ = namee_->type_;
}

function_call::~function_call() {}

bool function_call::verify(rooot* rooot) {
    // ERROR : not declared 
    auto search = rooot->function_symbol_table_.find(namee_->identifier_val_);
    function_decl* search_function_decl = std::get<1>(*search);
    if (search == rooot->function_symbol_table_.end()){
        print_error_msg(ERROR_ENUM::ERROR_NOT_DECL, location.begin);
        return false;
    }
    // ERROR type_arg: type of a function call argument does not match function declaration
    // NAME_1: function_call parameters: namee_, comma_expression_star_quesmark_suite_
    // NAME_2: symbol table parameter_list: search_function_decl->parameter_list_
    bool match = true;
    std::string arg_type_1 = "";
    std::string arg_type_2 = "";
    // first iteration
    comma_expression_star_quesmark_suite* it1 = comma_expression_star_quesmark_suite_.get();
    if ( (comma_expression_star_quesmark_suite_ == nullptr && search_function_decl->parameter_list_ != nullptr) ||
        (comma_expression_star_quesmark_suite_ != nullptr && search_function_decl->parameter_list_ == nullptr) ){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
        return false;
    }
    else{
        arg_type_1 = comma_expression_star_quesmark_suite_->expression_->expression_prime_->type_;
        arg_type_2 = search_function_decl->parameter_list_->declaration_->namee_->type_;
        if (arg_type_1.compare(arg_type_2) != 0){
            print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
            return false;
        }
    }
    // second iteration
    comma_expression_star_suite* it3 = it1->comma_expression_star_suite_.get();
    comma_declaration_star_suite* it2 = search_function_decl->parameter_list_->comma_declaration_star_suite_.get();
    if ( (it3 != nullptr && it2 == nullptr) || (it3 == nullptr && it2 != nullptr)){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
        return false;
    }
    else if (it3 == nullptr && it2 == nullptr){}
    else{
        arg_type_1 = it1->expression_->expression_prime_->type_;
        arg_type_2 = it2->declaration_->type_;
        if (arg_type_1.compare(arg_type_2) != 0){
            print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
            return false;
        }
    }
    // third iteration
    while (match){
        if (it3 == nullptr && it2 == nullptr)
            break;
        it3 = it3->comma_expression_star_suite_.get();
        it2 = it2->comma_declaration_star_suite_.get();
        if ( (it2 == nullptr && it3 != nullptr) || (it2 != nullptr && it3 == nullptr) )
            match = false;
        arg_type_1 = it3->expression_->expression_prime_->type_;
        arg_type_2 = it2->declaration_->type_;
        if (arg_type_1.compare(arg_type_2) != 0)
            match = false;
    }

    if (!match){
        print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
        return false;
    }
    bool result = true;
    if (result && namee_ != nullptr){
        result = namee_->verify(rooot);
    }
    if (result && comma_expression_star_quesmark_suite_ != nullptr){
        result = comma_expression_star_quesmark_suite_->verify(rooot);
    }
    return result;
}

void function_call::print(int indent) {
    std::cout << std::setw(indent) << "function_call (" << location.begin << ") {" << std::endl;
    if (namee_ != nullptr)
        namee_->print(indent+4);
    if (comma_expression_star_quesmark_suite_ != nullptr)
        comma_expression_star_quesmark_suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// comma_expression_star_quesmark_suite ///////////////////////////////////////////

comma_expression_star_quesmark_suite::comma_expression_star_quesmark_suite(std::unique_ptr<expression> expression, std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite) : Node() {
    expression_ = std::move(expression);
    comma_expression_star_suite_ = std::move(comma_expression_star_suite);
}

comma_expression_star_quesmark_suite::~comma_expression_star_quesmark_suite() {}

bool comma_expression_star_quesmark_suite::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    if (result && comma_expression_star_suite_ != nullptr){
        result = comma_expression_star_suite_->verify(rooot);
    }
    return result;
}

void comma_expression_star_quesmark_suite::print(int indent) {
    std::cout << std::setw(indent) << "comma_expression_star_quesmark_suite (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent+4);
    if (comma_expression_star_suite_ != nullptr)
        comma_expression_star_suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

///////////////////////////////////////// comma_expression_star_suite ///////////////////////////////////////////

comma_expression_star_suite::comma_expression_star_suite(std::unique_ptr<expression> expression, std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite) : Node() {
    expression_ = std::move(expression);
    comma_expression_star_suite_ = std::move(comma_expression_star_suite);
}

comma_expression_star_suite::~comma_expression_star_suite() {}

bool comma_expression_star_suite::verify(rooot* rooot) {
    bool result = true;
    if (result && expression_ != nullptr){
        result = expression_->verify(rooot);
    }
    if (result && comma_expression_star_suite_ != nullptr){
        result = comma_expression_star_suite_->verify(rooot);
    }
    return result;
}

void comma_expression_star_suite::print(int indent) {
    std::cout << std::setw(indent) << "comma_expression_star_suite (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent+4);
    if (comma_expression_star_suite_ != nullptr)
        comma_expression_star_suite_->print(indent+4);
    std::cout << std::setw(indent) << "}" << std::endl;
}


///////////////////////////////////////// helper functions ///////////////////////////////////////////

void print_error_msg (ERROR_ENUM ERROR_NUM, yy::position location){
	std::string error_msg = "";
    std::string error_type = "";
	switch (ERROR_NUM){
		case (ERROR_ENUM::ERROR_TYPE_DECL):
			error_type = "type_decl";
            error_msg = "Types must be one of bool, int, float. Functions may have return type void. ";
			break;
		case (ERROR_ENUM::ERROR_TYPE_MISMATCH):
			error_type = "type_mismatch";
			error_msg = "Operands of a binary or relational operator have the same type. ";
			break;
		case (ERROR_ENUM::ERROR_TYPE_ARG):
			error_type = "type_arg";
			error_msg = "Type of a function call argument/parameter does not match function declaration. ";
			break;
		case (ERROR_ENUM::ERROR_TYPE_BOOL):
			error_type = "type_bool";
			error_msg = "Type of an if-statement, for-statement, while-statement, or ternary expression predicate must be bool. ";
			break;
		case (ERROR_ENUM::ERROR_TYPE_RETURN):
			error_type = "type_return";
			error_msg = "Type of return expression does not match function declaration. ";
			break;
		case (ERROR_ENUM::ERROR_MAIN_FUNCTION):
			error_type = "main_function";
			error_msg = "A main function exists, and has return type int. ";
			break;
		case (ERROR_ENUM::ERROR_RETURN_STMT):
			error_type = "return_statement";
			error_msg = "Void functions do not return expressions. Non-void functions have a return statement on control flow paths. ";
			break;
		case (ERROR_ENUM::ERROR_DUPLICATE_DECL):
			error_type = "duplicate_decl";
			error_msg = "Duplicate function or variable declaration in the same scope. ";
			break;
        case (ERROR_ENUM::ERROR_NOT_DECL):
			error_type = "not_decl";
			error_msg = "Use of undeclared function or variable. ";
			break;
        default:
			error_type = "";
			error_msg = "Unknown error: ";
	}
	std::cout << "[output] Error " << error_type << ": line " << location << ". " << error_msg << std::endl;
}
