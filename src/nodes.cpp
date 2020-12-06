#include "nodes.hpp"

// Helper functions
static llvm::AllocaInst *CreateEntryBlockAlloca(rooot* rooot, llvm::Function *TheFunction, llvm::StringRef VarName, llvm::Type *VarType);
llvm::Function *getFunction(rooot* rooot, std::string Name);

///////////////////////////////////////// base class: Node ///////////////////////////////////////////

Node::~Node() = default;

bool Node::verify()
{
    rooot *rooot_ = dynamic_cast<rooot *>(this);
    return rooot_->verify(rooot_);
}

bool Node::verify(rooot *rooot)
{
    return rooot->verify(rooot);
}

void Node::print(int indent) {}

void Node::codegen()
{
    printf("Entering Node::codegen\n");
    rooot *rooot_ = dynamic_cast<rooot *>(this);
    this->codegen(rooot_, nullptr);
    printf("Leaving Node::codegen\n");
    return;
}

llvm::Value *Node::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering Node::codegen\n");
    printf("Leaving Node::codegen\n");
    return rooot->codegen(rooot, named_values);
}

///////////////////////////////////////// root ///////////////////////////////////////////

rooot::rooot(std::unique_ptr<function_list> function_list) : Node()
{
    function_list_ = std::move(function_list);
    function_symbol_table_.clear();
    variable_symbol_table_.clear();
    block_count_ = 1;
}

rooot::~rooot() {}

bool rooot::verify(rooot *rooot)
{
    bool result = true;
    if (result && function_list_ != nullptr)
        result = function_list_->verify(rooot);
    // ERROR: main_function
    auto search = function_symbol_table_.find("main");
    if (result && search == function_symbol_table_.end())
    {
        print_error_msg(ERROR_ENUM::ERROR_MAIN_FUNCTION, location.begin);
        return false;
    }
    return result;
}

void rooot::print(int indent)
{
    std::cout << std::setw(indent) << "rooot (" << location.begin << ") {" << std::endl;
    if (function_list_ != nullptr)
        function_list_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *rooot::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering rooot::codegen\n");
    printf("Leaving rooot::codegen\n");
    return function_list_->codegen(rooot, named_values);
}

///////////////////////////////////////// function_list ///////////////////////////////////////////

function_list::function_list(std::unique_ptr<function_list> function_list_prime, std::unique_ptr<function> function) : Node()
{
    function_list_prime_ = std::move(function_list_prime);
    function_ = std::move(function);
}

function_list::~function_list() {}

bool function_list::verify(rooot *rooot)
{
    bool result = true;
    if (result && function_list_prime_ != nullptr)
        result = function_list_prime_->verify(rooot);
    if (result && function_ != nullptr)
        result = function_->verify(rooot);
    return result;
}

void function_list::print(int indent)
{
    std::cout << std::setw(indent) << "function_list (" << location.begin << ") {" << std::endl;
    if (function_list_prime_ != nullptr)
        function_list_prime_->print(indent + 4);
    if (function_ != nullptr)
        function_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Function *function_list::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering function_list::codegen\n");
    function_->codegen(rooot, named_values);
    if (function_list_prime_ != nullptr)
        function_list_prime_->codegen(rooot, named_values);
    printf("Leaving function_list::codegen\n");
    return nullptr;
}

///////////////////////////////////////// function ///////////////////////////////////////////

function::function(std::unique_ptr<function_decl> function_decl) : Node()
{
    function_decl_ = std::move(function_decl);
    function_defn_ = nullptr;
}

function::function(std::unique_ptr<function_defn> function_defn) : Node()
{
    function_decl_ = nullptr;
    function_defn_ = std::move(function_defn);
}

function::~function() {}

bool function::verify(rooot *rooot)
{
    bool result = true;
    if (result && function_decl_ != nullptr)
        result = function_decl_->verify(rooot);
    if (result && function_defn_ != nullptr)
        result = function_defn_->verify(rooot);
    return result;
}

void function::print(int indent)
{
    std::cout << std::setw(indent) << "function (" << location.begin << ") {" << std::endl;
    if (function_decl_ != nullptr)
        function_decl_->print(indent + 4);
    if (function_defn_ != nullptr)
        function_defn_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Function *function::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering function::codegen\n");
    std::string function_name = "";
    if (function_decl_ != nullptr)
        function_name = function_decl_->namee_->identifier_val_;
    else if (function_defn_ != nullptr)
        function_name = function_defn_->function_decl_->namee_->identifier_val_;
    else
        return LogErrorF("Function name does not exist 1.");
    llvm::Function *TheFunction = getFunction(rooot, function_name);

    if (TheFunction == nullptr)
    {
        if (function_decl_ != nullptr){
            TheFunction = function_decl_->codegen(rooot, &named_values_);
        }
        else if (function_defn_ != nullptr){
            TheFunction = function_defn_->function_decl_->codegen(rooot, &named_values_);
        }
        else
            return LogErrorF("Function name does not exist 2.");
    }
    else {/* do nothing */}

    if (!TheFunction->empty())
        return LogErrorF("Function cannot be redefined 3.");

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*(rooot->comp_->context), "entry", TheFunction);
    rooot->comp_->builder->SetInsertPoint(BB);
    named_values_.clear();
    for (auto &Arg : TheFunction->args()) 
    {
        llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(rooot, TheFunction, Arg.getName(), Arg.getType());
        rooot->comp_->builder->CreateStore(&Arg, Alloca);
        named_values_[std::string(Arg.getName())] = Alloca;
    }

    if (function_defn_ != nullptr && function_defn_->block_ != nullptr)
    {
        function_defn_->block_->codegen(rooot, &named_values_);
        llvm::verifyFunction(*TheFunction);
        printf("Leaving function::codegen\n");
        return TheFunction; // return on success
    }
    TheFunction->eraseFromParent(); // remove function
    return LogErrorF("Function cannot be redefined 4.");
}

///////////////////////////////////////// function_decl ///////////////////////////////////////////

function_decl::function_decl(std::string TOK_TYPE, std::unique_ptr<namee> namee, std::unique_ptr<parameter_list> parameter_list) : Node()
{
    type_ = TOK_TYPE;
    namee_ = std::move(namee);
    namee_->is_function_ = true;
    namee_->type_ = TOK_TYPE;
    parameter_list_ = std::move(parameter_list);
    ;
}

function_decl::~function_decl() {}

bool function_decl::verify(rooot *rooot)
{
    // ERROR type_decl: Functions may have return type void, bool, int, float
    std::string void_ = "void";
    std::string bool_ = "bool";
    std::string int_ = "int";
    std::string float_ = "float";
    if (type_.compare(void_) != 0 && type_.compare(bool_) != 0 && type_.compare(int_) != 0 && type_.compare(float_) != 0)
    {
        print_error_msg(ERROR_ENUM::ERROR_TYPE_DECL, location.begin);
        return false;
    }
    // ERROR duplicate_decl
    auto search = rooot->function_symbol_table_.find(namee_->identifier_val_);
    if (search == rooot->function_symbol_table_.end())
    {
        rooot->function_symbol_table_.insert({namee_->identifier_val_, this});
    }
    else
    {
        print_error_msg(ERROR_ENUM::ERROR_DUPLICATE_DECL, location.begin);
        return false;
    }

    bool result = true;
    if (result && namee_ != nullptr)
        result = namee_->verify(rooot);
    if (result && parameter_list_ != nullptr)
        result = parameter_list_->verify(rooot);
    return result;
}

void function_decl::print(int indent)
{
    std::cout << std::setw(indent) << "function_decl (" << location.begin << ") {" << std::endl;
    if (namee_ != nullptr)
        namee_->print(indent + 4);
    if (parameter_list_ != nullptr)
        parameter_list_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Function *function_decl::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering function_decl::codegen\n");
    // build parameter_names_ and parameter_types_
    parameter_names_.clear();
    parameter_types_.clear();
    if (parameter_list_ != nullptr)
    {
        declaration * decl = parameter_list_->declaration_.get();
        comma_declaration_star_suite * next_decl = parameter_list_->comma_declaration_star_suite_.get();
        while (true)
        {
            if (decl == nullptr)
                break;
            std::string p_name = decl->namee_->identifier_val_;
            parameter_names_.push_back(p_name);
            llvm::Type * p_type;
            if (decl->namee_->type_ == "bool")
                p_type = llvm::Type::getFloatTy(*(rooot->comp_->context));
            else if (decl->namee_->type_ == "float")
                p_type = llvm::Type::getFloatTy(*(rooot->comp_->context));
            else if (decl->namee_->type_ == "int")
                p_type = llvm::Type::getInt32Ty(*(rooot->comp_->context));
            else
                return LogErrorF("Unsupported type");
            parameter_types_.push_back(p_type);
            // go to next parameter
            if (next_decl == nullptr)
                break;
            decl = next_decl->declaration_.get();
            next_decl = next_decl->comma_declaration_star_suite_.get();
        }
    }

    llvm::FunctionType* function_type = nullptr;
    if (type_ == "int")
        function_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*(rooot->comp_->context)), parameter_types_, /* isVarArg */ false);
    else if (type_ == "float" || type_ == "bool")
        function_type = llvm::FunctionType::get(llvm::Type::getFloatTy(*(rooot->comp_->context)), parameter_types_, /* isVarArg */ false);
    else if (type_ == "void")
        function_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*(rooot->comp_->context)), parameter_types_, /* isVarArg */ false);
    else
        return LogErrorF("Unsupported function return type");
    llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, namee_->identifier_val_, rooot->comp_->module.get());
    unsigned Idx = 0;
    for (auto &Arg : function->args())
        Arg.setName(parameter_names_[Idx++]);
    
    printf("Leaving function_decl::codegen\n");
    return function;
}

///////////////////////////////////////// function_defn ///////////////////////////////////////////

function_defn::function_defn(std::unique_ptr<function_decl> function_decl, std::unique_ptr<block> block) : Node()
{
    function_decl_ = std::move(function_decl);
    block_ = std::move(block);
}

function_defn::~function_defn() {}

bool function_defn::verify(rooot *rooot)
{
    bool result = true;
    if (result && function_decl_ != nullptr)
        result = result && function_decl_->verify(rooot);
    if (result && block_ != nullptr)
        result = result && block_->verify(rooot);
    if (result && block_->find_return_)
    {
        if (block_->return_type_.compare("") == 0){}
        else if (block_->return_type_.compare(function_decl_->type_) != 0)
        {
            if (block_->return_type_.compare("void") == 0 || function_decl_->type_.compare("void") != 0)
                print_error_msg(ERROR_ENUM::ERROR_RETURN_STMT, location.begin);
            else
                print_error_msg(ERROR_ENUM::ERROR_TYPE_RETURN, location.begin);
            return false;
        }
    }
    return result;
}

void function_defn::print(int indent)
{
    std::cout << std::setw(indent) << "function_defn (" << location.begin << ") {" << std::endl;
    if (function_decl_ != nullptr)
        function_decl_->print(indent + 4);
    if (block_ != nullptr)
        block_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Function *function_defn::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering function_defn::codegen\n");
    llvm::Function *function_F = function_decl_->codegen(rooot, named_values);
    if (function_F == nullptr)
        LogErrorF("Empty function_decl_ within function_defn");
    printf("Leaving function_defn::codegen\n");
    return function_F;
}

///////////////////////////////////////// namee ///////////////////////////////////////////

namee::namee(std::string identifier_val) : Node()
{
    identifier_val_ = identifier_val;
    type_ = "";
    is_function_ = false;
    is_variable_ = false;
}

namee::~namee() {}

bool namee::verify(rooot *rooot)
{
    return true; // leaf node
}

void namee::print(int indent)
{
    std::cout << std::setw(indent) << "namee (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent + 4) << "identifier_val_: " << identifier_val_ << std::endl;
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *namee::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering namee::codegen\n");
    llvm::Value *V = (*named_values)[identifier_val_];
    if (V == nullptr)
        LogErrorV("Emptyy value within namee name");
    printf("Leaving namee::codegen\n");
    return V;
}

///////////////////////////////////////// parameter_list ///////////////////////////////////////////

parameter_list::parameter_list(std::unique_ptr<declaration> declaration, std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite) : Node()
{
    declaration_ = std::move(declaration);
    comma_declaration_star_suite_ = std::move(comma_declaration_star_suite);
}

parameter_list::~parameter_list() {}

bool parameter_list::verify(rooot *rooot)
{
    bool result = true;
    if (result && declaration_ != nullptr)
        result = declaration_->verify(rooot);
    if (result && comma_declaration_star_suite_ != nullptr)
        result = comma_declaration_star_suite_->verify(rooot);
    return result;
}

void parameter_list::print(int indent)
{
    std::cout << std::setw(indent) << "parameter_list (" << location.begin << ") {" << std::endl;
    if (declaration_ != nullptr)
        declaration_->print(indent + 4);
    if (comma_declaration_star_suite_ != nullptr)
        comma_declaration_star_suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *parameter_list::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    // do not use this function
    printf("Entering parameter_list::codegen\n");
    printf("Leaving parameter_list::codegen\n");
    return LogErrorV("Non-callable function");
}

// llvm::ArrayRef<llvm::Type *> parameter_list::codegen_list(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
// {
//     // do not use this function
//     return LogErrorV("Non-callable function");
// }

///////////////////////////////////////// comma_declaration_star_suite ///////////////////////////////////////////

comma_declaration_star_suite::comma_declaration_star_suite(std::unique_ptr<declaration> declaration, std::unique_ptr<comma_declaration_star_suite> comma_declaration_star_suite) : Node()
{
    declaration_ = std::move(declaration);
    comma_declaration_star_suite_ = std::move(comma_declaration_star_suite);
}

comma_declaration_star_suite::~comma_declaration_star_suite() {}

bool comma_declaration_star_suite::verify(rooot *rooot)
{
    bool result = true;
    if (result && declaration_ != nullptr)
        result = declaration_->verify(rooot);
    if (result && comma_declaration_star_suite_ != nullptr)
        result = comma_declaration_star_suite_->verify(rooot);
    return result;
}

void comma_declaration_star_suite::print(int indent)
{
    std::cout << std::setw(indent) << "comma_declaration_star_suite (" << location.begin << ") {" << std::endl;
    if (declaration_ != nullptr)
        declaration_->print(indent + 4);
    if (comma_declaration_star_suite_ != nullptr)
        comma_declaration_star_suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *comma_declaration_star_suite::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    // do not use this function
    printf("Entering comma_declaration_star_suite::codegen\n");
    printf("Leaving comma_declaration_star_suite::codegen\n");
    return LogErrorV("Non-callable function");
}

///////////////////////////////////////// block ///////////////////////////////////////////

block::block(std::unique_ptr<suite> suite) : Node()
{
    suite_ = std::move(suite);
    variable_symbol_table_.clear();
    block_num_ = 0;
    return_type_ = "";
    find_return_ = false;
}

block::~block() {}

bool block::verify(rooot *rooot)
{
    rooot->block_count_++;
    block_num_ = rooot->block_count_;
    suite *suite_itr = suite_.get();
    statement *statement_itr = nullptr;
    single_statement *single_statement_itr = nullptr;
    while (suite_itr != nullptr)
    {
        statement_itr = suite_itr->statement_.get();
        if (statement_itr != nullptr)
        {
            single_statement_itr = statement_itr->single_statement_.get();
        }
        if (single_statement_itr != nullptr)
        {
            // ERRPR_RETURN_TYPE
            if (single_statement_itr->with_return_)
            {
                if (!find_return_)
                {
                    return_type_ = single_statement_itr->return_type_;
                    find_return_ = true;
                }
                else
                {
                    if (return_type_ != single_statement_itr->return_type_)
                    {
                        print_error_msg(ERROR_ENUM::ERROR_TYPE_RETURN, location.begin);
                        return false;
                    }
                }
            }
            if (single_statement_itr->declaration_ != nullptr)
            {
                // ERROR_DUPLICATE_DECL
                auto search = variable_symbol_table_.find(single_statement_itr->declaration_->namee_->identifier_val_);
                if (search == variable_symbol_table_.end())
                {
                    variable_symbol_table_.insert({single_statement_itr->declaration_->namee_->identifier_val_, single_statement_itr->declaration_.get()});
                }
                else
                {
                    print_error_msg(ERROR_ENUM::ERROR_DUPLICATE_DECL, location.begin);
                    return false;
                }
            }
        }
        suite_itr = suite_itr->suite_.get();
    }
    bool result = true;
    if (result && suite_ != nullptr)
    {
        result = suite_->verify(rooot);
    }
    return result;
}

void block::print(int indent)
{
    std::cout << std::setw(indent) << "block (" << location.begin << ") {" << std::endl;
    if (suite_ != nullptr)
        suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *block::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering block::codegen\n");
    llvm::Value *suite_V = suite_->codegen(rooot, named_values);
    if (suite_V == nullptr)
        LogErrorV("Empty suite_ within block");
    printf("Leaving block::codegen\n");
    return suite_V;
}

///////////////////////////////////////// suite ///////////////////////////////////////////

suite::suite(std::unique_ptr<statement> statement, std::unique_ptr<suite> suite) : Node()
{
    statement_ = std::move(statement);
    suite_ = std::move(suite);
}

suite::~suite() {}

bool suite::verify(rooot *rooot)
{
    bool result = true;
    if (result && statement_ != nullptr)
        result = statement_->verify(rooot);
    if (result && suite_ != nullptr)
        result = suite_->verify(rooot);
    return result;
}

void suite::print(int indent)
{
    std::cout << std::setw(indent) << "suite (" << location.begin << ") {" << std::endl;
    if (statement_ != nullptr)
        statement_->print(indent + 4);
    if (suite_ != nullptr)
        suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *suite::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering suite::codegen\n");
    if (statement_ != nullptr)
        statement_->codegen(rooot, named_values);
    if (suite_ != nullptr)
        suite_->codegen(rooot, named_values);
    printf("Leaving suite::codegen\n");
    return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*(rooot->comp_->context)));
}

///////////////////////////////////////// declaration ///////////////////////////////////////////

declaration::declaration(std::string TOK_TYPE, std::unique_ptr<namee> namee) : Node()
{
    type_ = TOK_TYPE;
    namee_ = std::move(namee);
    namee_->is_variable_ = true;
    namee_->type_ = TOK_TYPE;
}

declaration::~declaration() {}

bool declaration::verify(rooot *rooot)
{
    // ERROR type_decl: variable types must be one of bool, int, float
    std::string bool_ = "bool";
    std::string int_ = "int";
    std::string float_ = "float";
    if (type_.compare(bool_) != 0 && type_.compare(int_) != 0 && type_.compare(float_) != 0)
    {
        print_error_msg(ERROR_ENUM::ERROR_TYPE_DECL, location.begin);
        return false;
    }
    bool result = true;
    if (result && namee_ != nullptr)
        result = namee_->verify(rooot);
    return result;
}

void declaration::print(int indent)
{
    std::cout << std::setw(indent) << "declaration (" << location.begin << ") {" << std::endl;
    if (namee_ != nullptr)
        namee_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *declaration::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering declaration::codegen\n");
    llvm::Value *namee_V = namee_->codegen(rooot, named_values);
    if (namee_V == nullptr)
        LogErrorV("Empty namee_ within declaration");
    printf("Leaving declaration::codegen\n");
    return namee_V;
}

///////////////////////////////////////// statement ///////////////////////////////////////////

statement::statement(std::unique_ptr<single_statement> single_statement) : Node()
{
    single_statement_ = std::move(single_statement);
    compound_statement_ = nullptr;
}

statement::statement(std::unique_ptr<compound_statement> compound_statement) : Node()
{
    single_statement_ = nullptr;
    compound_statement_ = std::move(compound_statement);
}

statement::~statement() {}

bool statement::verify(rooot *rooot)
{
    bool result = true;
    if (result && single_statement_ != nullptr)
        result = single_statement_->verify(rooot);
    if (result && compound_statement_ != nullptr)
        result = compound_statement_->verify(rooot);
    return result;
}

void statement::print(int indent)
{
    std::cout << std::setw(indent) << "statement (" << location.begin << ") {" << std::endl;
    if (single_statement_ != nullptr)
        single_statement_->print(indent + 4);
    if (compound_statement_ != nullptr)
        compound_statement_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *statement::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering statement::codegen\n");
    llvm::Value * ret = nullptr;
    if (single_statement_ != nullptr)
        ret = single_statement_->codegen(rooot, named_values);
    else if (compound_statement_ != nullptr)
        ret = compound_statement_->codegen(rooot, named_values);
    else 
        return LogErrorV("Empty single_statement_V, compound_statement_V within statement");
    printf("Leaving statement::codegen\n");
    return ret;
}

///////////////////////////////////////// single_statement ///////////////////////////////////////////

single_statement::single_statement(std::unique_ptr<declaration> declaration, TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector) : Node()
{
    selector_ = selector;
    if (selector == 1)
    {
        declaration_ = std::move(declaration);
        token_val_ = TOK_VAL;
        expression_ = std::move(expression);
    }
    else
    {
        declaration_ = nullptr;
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
    }
    return_type_ = "";
    with_return_ = false;
    namee_ = nullptr;
}

single_statement::single_statement(std::unique_ptr<namee> namee, TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector) : Node()
{
    selector_ = selector;
    if (selector >= 2 && selector <= 6)
    {
        namee_ = std::move(namee);
        token_val_ = TOK_VAL;
        expression_ = std::move(expression);
    }
    else
    {
        declaration_ = nullptr;
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
    }
    return_type_ = "";
    with_return_ = false;
    declaration_ = nullptr;
}

single_statement::single_statement(TOK_ENUM TOK_VAL, int selector) : Node()
{
    selector_ = selector;
    if (selector >= 7 && selector <= 9)
        token_val_ = TOK_VAL;
    else
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
    if (selector == 9)
    {
        return_type_ = "void";
        with_return_ = true;
    }
    else
    {
        return_type_ = "";
        with_return_ = false;
    }
    declaration_ = nullptr;
    expression_ = nullptr;
    namee_ = nullptr;
}

single_statement::single_statement(TOK_ENUM TOK_VAL, std::unique_ptr<expression> expression, int selector) : Node()
{
    selector_ = selector;
    if (selector_ == 10)
    {
        token_val_ = TOK_VAL;
        expression_ = std::move(expression);
        return_type_ = expression_->type_;
        with_return_ = true;
    }
    else
    {
        token_val_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
    }
    declaration_ = nullptr;
    namee_ = nullptr;
}

single_statement::single_statement(std::unique_ptr<expression> expression, int selector) : Node()
{
    selector_ = selector;
    if (selector == 11)
        expression_ = std::move(expression);
    else
        expression_ = nullptr;
    token_val_ = TOK_ENUM::TOK_UNDEFINE;
    declaration_ = nullptr;
    namee_ = nullptr;
    return_type_ = "";
    with_return_ = false;
}

single_statement::~single_statement() {}

bool single_statement::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    if (result && declaration_ != nullptr)
        result = declaration_->verify(rooot);
    if (result && namee_ != nullptr)
        result = namee_->verify(rooot);
    return result;
}

void single_statement::print(int indent)
{
    std::cout << std::setw(indent) << "single_statement (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent + 4) << "Token Enum: " << token_val_ << std::endl;
    if (declaration_ != nullptr)
        declaration_->print(indent + 4);
    if (expression_ != nullptr)
        expression_->print(indent + 4);
    if (namee_ != nullptr)
        namee_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *single_statement::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering single_statement::codegen\n");
    if (selector_ == 1) 
    { // declaration TOK_ASSIGN expression
        if (declaration_ == nullptr && declaration_->namee_ == nullptr)
            return LogErrorV("Invalid single statement assignment 1");
        std::string VarName = declaration_->namee_->identifier_val_;
        llvm::Function *TheFunction = rooot->comp_->builder->GetInsertBlock()->getParent();
        llvm::Value * ExprVal = expression_->codegen(rooot, named_values);
        if (ExprVal == nullptr)
            return LogErrorV("Invalid single statement assignment 2");
        llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(rooot, TheFunction, VarName, ExprVal->getType());
        rooot->comp_->builder->CreateStore(ExprVal, Alloca);
        (*named_values)[VarName] = Alloca;
        printf("Leaving single_statement::codegen\n");
        return ExprVal;
    }
    else if (selector_ == 2)
    { // namee TOK_ASSIGN expression
        std::string VarName = namee_->identifier_val_;
        llvm::Value * ExprVal = expression_->codegen(rooot, named_values);
        if (ExprVal == nullptr)
            return LogErrorV("Invalid single statement assignment 2");
        llvm::AllocaInst *Alloca = (*named_values)[VarName];
        rooot->comp_->builder->CreateStore(ExprVal, Alloca);
        printf("Leaving single_statement::codegen\n");
        return ExprVal;
    }
    else if (selector_ == 3)
    { // namee TOK_PLUS_ASSIGN expression
        std::string VarName = namee_->identifier_val_;
        llvm::AllocaInst *Alloca = (*named_values)[VarName];
        llvm::Value *CurVar = rooot->comp_->builder->CreateLoad(Alloca, VarName.c_str());
        llvm::Value * AddVar = expression_->codegen(rooot, named_values);
        llvm::Value *SumVar = rooot->comp_->builder->CreateFAdd(CurVar, AddVar, "PLUS_ASSIGN");
        rooot->comp_->builder->CreateStore(SumVar, Alloca);
        printf("Leaving single_statement::codegen\n");
        return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*(rooot->comp_->context)));
    }
    else if (selector_ == 4)
    { // namee TOK_MINUS_ASSIGN expression
        std::string VarName = namee_->identifier_val_;
        llvm::AllocaInst *Alloca = (*named_values)[VarName];
        llvm::Value *CurVar = rooot->comp_->builder->CreateLoad(Alloca, VarName.c_str());
        llvm::Value * MinusVar = expression_->codegen(rooot, named_values);
        llvm::Value *SumVar = rooot->comp_->builder->CreateFSub(CurVar, MinusVar, "MINUS_ASSIGN");
        rooot->comp_->builder->CreateStore(SumVar, Alloca);
        printf("Leaving single_statement::codegen\n");
        return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*(rooot->comp_->context)));
    }
    else if (selector_ == 5)
    { // namee TOK_STAR_ASSIGN expression
        std::string VarName = namee_->identifier_val_;
        llvm::AllocaInst *Alloca = (*named_values)[VarName];
        llvm::Value *CurVar = rooot->comp_->builder->CreateLoad(Alloca, VarName.c_str());
        llvm::Value * StarVar = expression_->codegen(rooot, named_values);
        llvm::Value *SumVar = rooot->comp_->builder->CreateFMul(CurVar, StarVar, "STAR_ASSIGN");
        rooot->comp_->builder->CreateStore(SumVar, Alloca);
        printf("Leaving single_statement::codegen\n");
        return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*(rooot->comp_->context)));
    }
    else if (selector_ == 6)
    { // namee TOK_SLASH_ASSIGN expression
        std::string VarName = namee_->identifier_val_;
        llvm::AllocaInst *Alloca = (*named_values)[VarName];
        llvm::Value *CurVar = rooot->comp_->builder->CreateLoad(Alloca, VarName.c_str());
        llvm::Value * SlashVar = expression_->codegen(rooot, named_values);
        llvm::Value *SumVar = rooot->comp_->builder->CreateFDiv(CurVar, SlashVar, "SLASH_ASSIGN");
        rooot->comp_->builder->CreateStore(SumVar, Alloca);
        printf("Leaving single_statement::codegen\n");
        return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*(rooot->comp_->context)));
    }
    else if (selector_ == 7)
    { // TOK_BREAK
        // Basic Block List: condition, true, false, merge
        llvm::Function *TheFunction = rooot->comp_->builder->GetInsertBlock()->getParent();
        if (TheFunction->getBasicBlockList().size() <= 0)
            return LogErrorV("Invalid basic block list");
        auto it = TheFunction->getBasicBlockList().end();
        llvm::BasicBlock *temp_BB = &(*(it));
        printf("Leaving single_statement::codegen\n");
        return rooot->comp_->builder->CreateBr(temp_BB);
    }
    else if (selector_ == 8)
    { // TOK_CONTINUE
        // Basic Block List: condition, true, false, merge
        llvm::Function *TheFunction = rooot->comp_->builder->GetInsertBlock()->getParent();
        if (TheFunction->getBasicBlockList().size() <= 0)
            return LogErrorV("Invalid basic block list");
        auto it = TheFunction->getBasicBlockList().end();
        it = std::prev(it, 3);
        llvm::BasicBlock *temp_BB = &(*(it));
        printf("Leaving single_statement::codegen\n");
        return rooot->comp_->builder->CreateBr(temp_BB);
    }
    else if (selector_ == 9)
    { //TOK_RETURN
        printf("Leaving single_statement::codegen\n");
        return rooot->comp_->builder->CreateRetVoid();
    }
    else if (selector_ == 10)
    { // TOK_RETURN expression
        llvm::Value * RetVal = expression_->codegen(rooot, named_values);
        printf("Leaving single_statement::codegen\n");
        return rooot->comp_->builder->CreateRet(RetVal);
    }
    else if (selector_ == 11)
    { // expression
        printf("Leaving single_statement::codegen\n");
        return expression_->codegen(rooot, named_values);
    }
    else
    { // error
        return LogErrorV("Invalid single statement");
    }
}

///////////////////////////////////////// expression ///////////////////////////////////////////

expression::expression(std::unique_ptr<expression_prime> expression_prime) : Node()
{
    expression_prime_ = std::move(expression_prime);
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = expression_prime_->type_;
}

expression::expression(std::unique_ptr<binary_expression> binary_expression) : Node()
{
    expression_prime_ = nullptr;
    binary_expression_ = std::move(binary_expression);
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = binary_expression_->type_;
}

expression::expression(std::unique_ptr<unary_expression> unary_expression) : Node()
{
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = std::move(unary_expression);
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = unary_expression_->type_;
}

expression::expression(std::unique_ptr<relational_expression> relational_expression) : Node()
{
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = std::move(relational_expression);
    ternary_expression_ = nullptr;
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = "bool";
}

expression::expression(std::unique_ptr<ternary_expression> ternary_expression) : Node()
{
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = std::move(ternary_expression);
    cast_expression_ = nullptr;
    function_call_ = nullptr;
    type_ = ternary_expression_->type_;
}

expression::expression(std::unique_ptr<cast_expression> cast_expression) : Node()
{
    expression_prime_ = nullptr;
    binary_expression_ = nullptr;
    unary_expression_ = nullptr;
    relational_expression_ = nullptr;
    ternary_expression_ = nullptr;
    cast_expression_ = std::move(cast_expression);
    function_call_ = nullptr;
    type_ = cast_expression_->type_;
}

expression::expression(std::unique_ptr<function_call> function_call) : Node()
{
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

bool expression::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_prime_ != nullptr)
        result = expression_prime_->verify(rooot);
    if (result && binary_expression_ != nullptr)
        result = binary_expression_->verify(rooot);
    if (result && relational_expression_ != nullptr)
        result = relational_expression_->verify(rooot);
    if (result && ternary_expression_ != nullptr)
        result = ternary_expression_->verify(rooot);
    if (result && cast_expression_ != nullptr)
        result = cast_expression_->verify(rooot);
    if (result && function_call_ != nullptr)
        result = function_call_->verify(rooot);
    return result;
}

void expression::print(int indent)
{
    std::cout << std::setw(indent) << "expression (" << location.begin << ") {" << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent + 4);
    if (binary_expression_ != nullptr)
        binary_expression_->print(indent + 4);
    if (unary_expression_ != nullptr)
        unary_expression_->print(indent + 4);
    if (relational_expression_ != nullptr)
        relational_expression_->print(indent + 4);
    if (ternary_expression_ != nullptr)
        ternary_expression_->print(indent + 4);
    if (cast_expression_ != nullptr)
        cast_expression_->print(indent + 4);
    if (function_call_ != nullptr)
        function_call_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *expression::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering expression::codegen\n");
    if (expression_prime_ != nullptr){
        llvm::Value * ret = expression_prime_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, expression_prime_\n");
        return ret;
    }
    if (binary_expression_ != nullptr){
        llvm::Value * ret = binary_expression_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, binary_expression_\n");
        return ret;
    }
    if (unary_expression_ != nullptr){
        llvm::Value * ret = unary_expression_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, unary_expression_\n");
        return ret;
    }
    if (relational_expression_ != nullptr){
        llvm::Value * ret = relational_expression_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, relational_expression_\n");
        return ret;
    }
    if (ternary_expression_ != nullptr){
        llvm::Value * ret = ternary_expression_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, ternary_expression_\n");
        return ret;
    }
    if (cast_expression_ != nullptr){
        llvm::Value * ret = cast_expression_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, cast_expression_\n");
        return ret;
    }
    if (function_call_ != nullptr){
        llvm::Value * ret = function_call_->codegen(rooot, named_values);
        printf("Leaving expression::codegen, function_call_\n");
        return ret;
    }
    else
        return LogErrorV("Empty expression_ within expression");
}

///////////////////////////////////////// expression_prime ///////////////////////////////////////////

expression_prime::expression_prime(std::unique_ptr<namee> namee, int selector) : Node()
{
    selector_ = selector;
    if (selector == 1)
    {
        namee_ = std::move(namee);
        type_ = namee_->type_;
    }
    expression_ = nullptr;
}

expression_prime::expression_prime(bool TOK_BOOL, int selector) : Node()
{
    selector_ = selector;
    if (selector == 2 || selector == 3)
    {
        bool_val_ = TOK_BOOL;
        type_ = "bool";
    }
    namee_ = nullptr;
    expression_ = nullptr;
}

expression_prime::expression_prime(int integer_val, int selector) : Node()
{
    selector_ = selector;
    if (selector == 4)
    {
        integer_val_ = integer_val;
        type_ = "int";
    }
    namee_ = nullptr;
    expression_ = nullptr;
}

expression_prime::expression_prime(float float_val, int selector) : Node()
{
    selector_ = selector;
    if (selector == 5)
    {
        float_val_ = float_val;
        type_ = "float";
    }
    namee_ = nullptr;
    expression_ = nullptr;
}

expression_prime::expression_prime(std::unique_ptr<expression> expression, int selector) : Node()
{
    selector_ = selector;
    if (selector == 6)
    {
        expression_ = std::move(expression);
        type_ = expression_->type_;
    }
    else
        expression_ = nullptr;
    namee_ = nullptr;
}

expression_prime::~expression_prime() {}

bool expression_prime::verify(rooot *rooot)
{
    bool result = true;
    if (result && namee_ != nullptr)
        result = namee_->verify(rooot);
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    return result;
}

void expression_prime::print(int indent)
{
    std::cout << std::setw(indent) << "expression_prime (" << location.begin << ") {" << std::endl;
    if (selector_ == 1 && namee_ != nullptr)
        namee_->print(indent + 4);
    else if (selector_ == 2 || selector_ == 3)
        if (bool_val_)
            std::cout << std::setw(indent + 4) << "bool: true" << std::endl;
        else
            std::cout << std::setw(indent + 4) << "bool: false" << std::endl;
    else if (selector_ == 4)
        std::cout << std::setw(indent + 4) << "int: " << integer_val_ << std::endl;
    else if (selector_ == 5)
        std::cout << std::setw(indent + 4) << "float: " << float_val_ << std::endl;
    else if (selector_ == 6 && expression_ != nullptr)
        expression_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *expression_prime::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering expression_prime::codegen\n"); 
    // ??????????
    // return according to selector_
    if (selector_ == 1)
    { // namee
        printf("Leaving expression_prime::codegen selector 1\n"); 
        return namee_->codegen(rooot, named_values);
    }
    if (selector_ == 2 || selector_ == 3)
    { // true || false
        printf("Leaving expression_prime::codegen selector 2/3\n"); 
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(*(rooot->comp_->context)), bool_val_); // ??? dangerous type conversion bool -> double
    }
    if (selector_ == 4)
    { // int
         printf("Leaving expression_prime::codegen selector 4\n"); 
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(rooot->comp_->context)), integer_val_ /* uint64_t */, true /* isSigned */);
    }
    if (selector_ == 5)
    { // float
         printf("Leaving expression_prime::codegen selector 5\n"); 
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(*(rooot->comp_->context)), float_val_);
    }
    if (selector_ == 6)
    { // expression
        printf("Leaving expression_prime::codegen selector 6\n"); 
        return expression_->codegen(rooot, named_values);
    }
    else
    {
        return LogErrorV("Undefined expression_prime");
    }
    
}

///////////////////////////////////////// compound_statement ///////////////////////////////////////////

compound_statement::compound_statement(TOK_ENUM token, std::unique_ptr<expression> expression, std::unique_ptr<block> block, int selector) : Node()
{
    selector_ = selector;
    if (selector == 1 || selector_ == 10)
    {
        token_ = token;
        expression_ = std::move(expression);
        block_ = std::move(block);
    }
    else
    {
        token_ = TOK_ENUM::TOK_UNDEFINE;
        expression_ = nullptr;
        block_ = nullptr;
    }
    single_statement_1_ = nullptr;
    single_statement_2_ = nullptr;
}

compound_statement::compound_statement(TOK_ENUM token, std::unique_ptr<single_statement> single_statement_1, std::unique_ptr<expression> expression, std::unique_ptr<single_statement> single_statement_2, std::unique_ptr<block> block, int selector) : Node()
{
    selector_ = selector;
    if (selector >= 2 && selector_ <= 9)
    {
        token_ = token_;
        single_statement_1_ = std::move(single_statement_1);
        expression_ = std::move(expression);
        single_statement_2_ = std::move(single_statement_2);
        block_ = std::move(block);
    }
    else
    {
        token_ = TOK_ENUM::TOK_UNDEFINE;
        single_statement_1_ = nullptr;
        expression_ = nullptr;
        single_statement_2_ = nullptr;
        block_ = nullptr;
    }
}

compound_statement::~compound_statement() {}

bool compound_statement::verify(rooot *rooot)
{
    // ERROR type_bool: type of an if-statement, for-statement, while-statement predicate must be bool
    if (selector_ == 1 || selector_ == 10)
    { // if-statement, while-statement
        std::string stmt_type = expression_->type_;
        if (stmt_type.compare("bool") != 0)
        {
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
    else if (selector_ >= 2 && selector_ <= 9)
    { // while-statement
        std::string stmt_type = "empty";
        if (expression_ != nullptr)
            stmt_type = expression_->type_;
        if (stmt_type.compare("bool") != 0 && stmt_type.compare("empty") != 0)
        {
            print_error_msg(ERROR_ENUM::ERROR_TYPE_BOOL, location.begin);
            return false;
        }
    }
    bool result = true;
    if (result && single_statement_1_ != nullptr)
        result = single_statement_1_->verify(rooot);
    if (result && single_statement_2_ != nullptr)
        result = single_statement_2_->verify(rooot);
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    if (result && block_ != nullptr)
        result = block_->verify(rooot);
    return result;
}

void compound_statement::print(int indent)
{
    std::cout << std::setw(indent) << "compound_statement (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent + 4) << "Token Enum: " << token_ << std::endl;
    if (single_statement_1_ != nullptr)
        single_statement_1_->print(indent + 4);
    if (single_statement_2_ != nullptr)
        single_statement_2_->print(indent + 4);
    if (expression_ != nullptr)
        expression_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *compound_statement::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{   
    printf("Entering compound_statement::codegen\n");  
    llvm::Value * expression_V = nullptr;
    llvm::Function *TheFunction = rooot->comp_->builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *ConditionBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "ConditionBB", TheFunction);
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "TrueBB", TheFunction);
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "FalseBB", TheFunction); // not needed in this comparison
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "MergeBB", TheFunction);
    TheFunction->getBasicBlockList().push_back(ConditionBB);
    TheFunction->getBasicBlockList().push_back(TrueBB);
    TheFunction->getBasicBlockList().push_back(FalseBB);
    TheFunction->getBasicBlockList().push_back(MergeBB);
    
    if (selector_ == 1)
    { // if statement
        if (expression_ != nullptr)
            expression_V = expression_->codegen(rooot, named_values);
        else
            LogErrorV("No condition in if-statement"); 
        expression_V = rooot->comp_->builder->CreateFCmpONE(expression_V, llvm::ConstantFP::get(*(rooot->comp_->context), llvm::APFloat(0.0)), "if cond");
        rooot->comp_->builder->CreateCondBr(expression_V, TrueBB, MergeBB);
        rooot->comp_->builder->SetInsertPoint(TrueBB);
        block_->codegen(rooot, named_values);
        rooot->comp_->builder->SetInsertPoint(MergeBB);
    }
    else if (selector_ >= 2 && selector_ <= 9)
    { // for loop
        if (single_statement_1_ != nullptr)
            single_statement_1_->codegen(rooot, named_values);
        rooot->comp_->builder->SetInsertPoint(ConditionBB);
        if (expression_ != nullptr)
            expression_V = expression_->codegen(rooot, named_values);
        else
            expression_V = llvm::ConstantFP::get(*(rooot->comp_->context), llvm::APFloat(0.0));
        expression_V = rooot->comp_->builder->CreateFCmpONE(expression_V, llvm::ConstantFP::get(*(rooot->comp_->context), llvm::APFloat(0.0)), "for cond");
        rooot->comp_->builder->CreateCondBr(expression_V, TrueBB, MergeBB);
        if (single_statement_2_ != nullptr)
            single_statement_2_->codegen(rooot, named_values);
        rooot->comp_->builder->SetInsertPoint(TrueBB);
        block_->codegen(rooot, named_values);
        rooot->comp_->builder->CreateBr(ConditionBB);
        rooot->comp_->builder->SetInsertPoint(MergeBB);
    }
    else if (selector_ == 10)
    { // while loop
        rooot->comp_->builder->SetInsertPoint(ConditionBB);
        if (expression_ != nullptr)
            expression_V = expression_->codegen(rooot, named_values);
        else
            expression_V = llvm::ConstantFP::get(*(rooot->comp_->context), llvm::APFloat(0.0));
        expression_V = rooot->comp_->builder->CreateFCmpONE(expression_V, llvm::ConstantFP::get(*(rooot->comp_->context), llvm::APFloat(0.0)), "while cond");
        rooot->comp_->builder->CreateCondBr(expression_V, TrueBB, MergeBB);
        rooot->comp_->builder->SetInsertPoint(TrueBB);
        block_->codegen(rooot, named_values);
        rooot->comp_->builder->CreateBr(ConditionBB);
        rooot->comp_->builder->SetInsertPoint(MergeBB);
    }
    else
        LogErrorV("Undeclared compound_statment type"); 
    
    TheFunction->getBasicBlockList().pop_back();
    TheFunction->getBasicBlockList().pop_back();
    TheFunction->getBasicBlockList().pop_back();
    TheFunction->getBasicBlockList().pop_back();
    printf("Leaving compound_statement::codegen\n");  
    return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*(rooot->comp_->context)));
}

///////////////////////////////////////// binary_expression ///////////////////////////////////////////

binary_expression::binary_expression(std::unique_ptr<expression> expression, TOK_ENUM binary_op, std::unique_ptr<expression_prime> expression_prime) : Node()
{
    expression_ = std::move(expression);
    binary_op_ = binary_op;
    expression_prime_ = std::move(expression_prime);
    type_ = expression_prime_->type_;
}

binary_expression::~binary_expression() {}

bool binary_expression::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    if (result && expression_prime_ != nullptr)
        result = expression_prime_->verify(rooot);
    // ERROR type_mismatch: operands of a binary or relational operator have the same type
    std::string expr_type_1 = expression_->type_;
    std::string expr_type_2 = expression_prime_->type_;
    if (result && expr_type_1.compare(expr_type_2) != 0 && expr_type_1.compare("") != 0 && expr_type_2.compare("") != 0)
    {
        print_error_msg(ERROR_ENUM::ERROR_TYPE_MISMATCH, location.begin);
        return false;
    }
    return result;
}

void binary_expression::print(int indent)
{
    std::cout << std::setw(indent) << "binary_expression (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent + 4);
    std::cout << std::setw(indent + 4) << "Token Enum: " << binary_op_ << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *binary_expression::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering binary_expression::codegen\n");  
    llvm::Value *expression_V = expression_->codegen(rooot, named_values);
    llvm::Value *expression_prime_V = expression_prime_->codegen(rooot, named_values);
    if (expression_V == nullptr || expression_prime_V == nullptr)
        return LogErrorV("Empty expression_, expression_prime_ within binary_expression");
    switch (binary_op_)
    {
        case TOK_ENUM::TOK_PLUS:
            printf("Leaving binary_expression::codegen--Plus\n");  
            return rooot->comp_->builder->CreateFAdd(expression_V, expression_prime_V, "addtmp");
        case TOK_ENUM::TOK_MINUS:
            printf("Leaving binary_expression::codegen--Minus\n");  
            return rooot->comp_->builder->CreateFSub(expression_V, expression_prime_V, "subtmp");
        case TOK_ENUM::TOK_STAR:
            printf("Leaving binary_expression::codegen--Star\n");  
            return rooot->comp_->builder->CreateFMul(expression_V, expression_prime_V, "multmp");
        case TOK_ENUM::TOK_SLASH:
            printf("Leaving binary_expression::codegen--Slash\n");  
            return rooot->comp_->builder->CreateFDiv(expression_V, expression_prime_V, "divtmp");
        case TOK_ENUM::TOK_LOG_AND:
            printf("Leaving binary_expression::codegen--LogAnd\n");  
            return rooot->comp_->builder->CreateAnd(expression_V, expression_prime_V, "andtmp");
        case TOK_ENUM::TOK_LOG_OR:
            printf("Leaving binary_expression::codegen--LogOR\n");  
            return rooot->comp_->builder->CreateOr(expression_V, expression_prime_V, "ortmp");
        default:
            return LogErrorV("undefined binary operator");
    }
}

///////////////////////////////////////// unary_expression ///////////////////////////////////////////

unary_expression::unary_expression(TOK_ENUM unary_op, std::unique_ptr<expression_prime> expression_prime) : Node()
{
    unary_op_ = unary_op;
    expression_prime_ = std::move(expression_prime);
    type_ = expression_prime_->type_;
}

unary_expression::~unary_expression() {}

bool unary_expression::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_prime_ != nullptr)
        result = expression_prime_->verify(rooot);
    return result;
}

void unary_expression::print(int indent)
{
    std::cout << std::setw(indent) << "unary_expression (" << location.begin << ") {" << std::endl;
    std::cout << std::setw(indent + 4) << "Token Enum: " << unary_op_ << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *unary_expression::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering unary_expression::codegen\n");  
    llvm::Value *expression_prime_V = expression_prime_->codegen(rooot, named_values);
    if (expression_prime_V == nullptr)
        return LogErrorV("Empty expression_, expression_prime_ within unary_expression");
    switch (unary_op_)
    {
        case TOK_ENUM::TOK_MINUS:
            printf("Leaving unary_expression::codegen\n");  
            return rooot->comp_->builder->CreateFSub(nullptr, expression_prime_V, "unatmp");
        default:
            return LogErrorV("undefined unary operator");
    }
}

///////////////////////////////////////// relational_expression ///////////////////////////////////////////

relational_expression::relational_expression(std::unique_ptr<expression> expression, TOK_ENUM relational_op, std::unique_ptr<expression_prime> expression_prime) : Node()
{
    expression_ = std::move(expression);
    relational_op_ = relational_op;
    expression_prime_ = std::move(expression_prime);
    type_ = "bool";
}

relational_expression::~relational_expression() {}

bool relational_expression::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    if (result && expression_prime_ != nullptr)
        result = expression_prime_->verify(rooot);
    // ERROR type_mismatch: operands of a binary or relational operator have the same type
    std::string expr_type_1 = expression_->type_;
    std::string expr_type_2 = expression_prime_->type_;
    if (result && expr_type_1.compare(expr_type_2) != 0 && expr_type_1.compare("") != 0 && expr_type_2.compare("") != 0)
    {
        print_error_msg(ERROR_ENUM::ERROR_TYPE_MISMATCH, location.begin);
        return false;
    }
    return result;
}

void relational_expression::print(int indent)
{
    std::cout << std::setw(indent) << "relational_expression (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent + 4);
    std::cout << std::setw(indent + 4) << "Token Enum: " << relational_op_ << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *relational_expression::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering relational_expression::codegen\n");
    llvm::Value *expression_V = expression_->codegen(rooot, named_values);
    llvm::Value *expression_prime_V = expression_prime_->codegen(rooot, named_values);
    if (expression_V == nullptr || expression_prime_V == nullptr)
        return LogErrorV("Empty expression_, expression_prime_ within relational_expression");
    switch (relational_op_)
    {
        // Convert bool 0/1 to double 0.0 or 1.0
        case TOK_ENUM::TOK_EQ:
            expression_V = rooot->comp_->builder->CreateICmpEQ(expression_V, expression_prime_V, "cmptmp");
            printf("Leaving relational_expression::codegen-- EQ\n");
            return rooot->comp_->builder->CreateUIToFP(expression_V, llvm::Type::getFloatTy(*rooot->comp_->context), "booltmp"); 
        case TOK_ENUM::TOK_NE:
            expression_V = rooot->comp_->builder->CreateICmpNE(expression_V, expression_prime_V, "cmptmp");
            printf("Leaving relational_expression::codegen--NE\n");  
            return rooot->comp_->builder->CreateUIToFP(expression_V, llvm::Type::getFloatTy(*rooot->comp_->context), "booltmp"); 
        case TOK_ENUM::TOK_LT:
            expression_V = rooot->comp_->builder->CreateFCmpULT(expression_V, expression_prime_V, "cmptmp");
            printf("Leaving relational_expression::codegen--LT\n");  
            return rooot->comp_->builder->CreateUIToFP(expression_V, llvm::Type::getFloatTy(*rooot->comp_->context), "booltmp"); 
        case TOK_ENUM::TOK_GT:
            expression_V = rooot->comp_->builder->CreateFCmpUGT(expression_V, expression_prime_V, "cmptmp");
            printf("Leaving relational_expression::codegen--GT\n");  
            return rooot->comp_->builder->CreateUIToFP(expression_V, llvm::Type::getFloatTy(*rooot->comp_->context), "booltmp"); 
        case TOK_ENUM::TOK_LE:
            expression_V = rooot->comp_->builder->CreateFCmpULE(expression_V, expression_prime_V, "cmptmp");
            printf("Leaving relational_expression::codegen--LE\n");  
            return rooot->comp_->builder->CreateUIToFP(expression_V, llvm::Type::getFloatTy(*rooot->comp_->context), "booltmp"); 
        case TOK_ENUM::TOK_GE:
            expression_V = rooot->comp_->builder->CreateFCmpUGE(expression_V, expression_prime_V, "cmptmp");
            printf("Leaving relational_expression::codegen--GE\n");  
            return rooot->comp_->builder->CreateUIToFP(expression_V, llvm::Type::getFloatTy(*rooot->comp_->context), "booltmp"); 
        default:
            return LogErrorV("undefined relational operator");
    }
}

///////////////////////////////////////// ternary_expression ///////////////////////////////////////////

ternary_expression::ternary_expression(std::unique_ptr<expression> expression_1, std::unique_ptr<expression> expression_2, std::unique_ptr<expression_prime> expression_prime) : Node()
{
    expression_1_ = std::move(expression_1);
    expression_2_ = std::move(expression_2);
    expression_prime_ = std::move(expression_prime);
    type_ = expression_prime_->type_;
}

ternary_expression::~ternary_expression() {}

bool ternary_expression::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_1_ != nullptr)
        result = expression_1_->verify(rooot);
    if (result && expression_2_ != nullptr)
        result = expression_2_->verify(rooot);
    if (result && expression_prime_ != nullptr)
        result = expression_prime_->verify(rooot);
    return result;
}

void ternary_expression::print(int indent)
{
    std::cout << std::setw(indent) << "ternary_expression (" << location.begin << ") {" << std::endl;
    if (expression_1_ != nullptr)
        expression_1_->print(indent + 4);
    if (expression_2_ != nullptr)
        expression_2_->print(indent + 4);
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *ternary_expression::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering ternary_expression::codegen\n");
    if (expression_1_ == nullptr || expression_2_ == nullptr || expression_prime_ == nullptr)
        return LogErrorV("Empty expression_1_, expression_2_, expression_prime_ within ternary_expression");
    
    // expression_1_ ? expression_2_ : expression_prime_
    // Creating Basic Blocks / IR labels
    llvm::Function *TheFunction = rooot->comp_->builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *ConditionBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "TrueBB", TheFunction); // not needed here
    llvm::BasicBlock *TrueBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "TrueBB", TheFunction);
    llvm::BasicBlock *FalseBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "FalseBB", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*(rooot->comp_->context), "MergeBB", TheFunction);
    TheFunction->getBasicBlockList().push_back(ConditionBB);
    TheFunction->getBasicBlockList().push_back(TrueBB);
    TheFunction->getBasicBlockList().push_back(FalseBB);
    TheFunction->getBasicBlockList().push_back(MergeBB);
    
    // Evaluate condition 
    llvm::Value * expression_1_V = expression_1_->codegen(rooot, named_values);
    expression_1_V = rooot->comp_->builder->CreateFCmpONE(expression_1_V, llvm::ConstantFP::get(*(rooot->comp_->context), llvm::APFloat(0.0)), "if cond");
    rooot->comp_->builder->CreateCondBr(expression_1_V, TrueBB, FalseBB);
    // Evaluate to true
    rooot->comp_->builder->SetInsertPoint(TrueBB);
    llvm::Value * expression_2_V = expression_2_->codegen(rooot, named_values);
    rooot->comp_->builder->CreateBr(MergeBB);
    // Evaluate to false
    rooot->comp_->builder->SetInsertPoint(FalseBB);
    llvm::Value * expression_prime_V = expression_prime_->codegen(rooot, named_values);
    rooot->comp_->builder->CreateBr(MergeBB);
    // Merge and clean up
    rooot->comp_->builder->SetInsertPoint(MergeBB);
    TheFunction->getBasicBlockList().pop_back();
    TheFunction->getBasicBlockList().pop_back();
    TheFunction->getBasicBlockList().pop_back();
    TheFunction->getBasicBlockList().pop_back();

    // return PHI node
    llvm::PHINode *PN = rooot->comp_->builder->CreatePHI(llvm::Type::getFloatTy(*(rooot->comp_->context)), 2, "ternary stmt");
    PN->addIncoming(expression_2_V, TrueBB);
    PN->addIncoming(expression_prime_V, FalseBB);
    printf("Leaving ternary_expression::codegen\n");
    return PN;
}

///////////////////////////////////////// cast_expression ///////////////////////////////////////////

cast_expression::cast_expression(std::string TOK_TYPE, std::unique_ptr<expression_prime> expression_prime) : Node()
{
    type_ = TOK_TYPE;
    expression_prime_ = std::move(expression_prime);
}

cast_expression::~cast_expression() {}

bool cast_expression::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_prime_ != nullptr)
        result = expression_prime_->verify(rooot);
    return result;
}

void cast_expression::print(int indent)
{
    std::cout << std::setw(indent) << "cast_expression (" << location.begin << ") {" << std::endl;
    if (expression_prime_ != nullptr)
        expression_prime_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *cast_expression::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering cast_expression::codegen\n");
    // change type of expression
    // return expression value
    llvm::Value *expression_prime_V = expression_prime_->codegen(rooot, named_values);
    if (expression_prime_V == nullptr)
        LogErrorV("Empty expression_prime_ within cast_expression");
    if (type_ == "float"){ // try createsitofp ??????????
        printf("Leave cast_expression::codegen float\n");
        return rooot->comp_->builder->CreateFPCast(expression_prime_V, llvm::Type::getFloatTy(*rooot->comp_->context), "castftmp");
    }
    else if (type_ == "int"){ // try createfptosi ??????????
        printf("Leave cast_expression::codegen int\n");
        return rooot->comp_->builder->CreateIntCast(expression_prime_V, llvm::Type::getInt32Ty(*rooot->comp_->context), true /* isSigned */, "castitmp");
    }
    else
        return LogErrorV("undefined cast operation");
}

///////////////////////////////////////// function_call ///////////////////////////////////////////

function_call::function_call(std::unique_ptr<namee> namee, std::unique_ptr<comma_expression_star_quesmark_suite> comma_expression_star_quesmark_suite) : Node()
{
    namee_ = std::move(namee);
    comma_expression_star_quesmark_suite_ = std::move(comma_expression_star_quesmark_suite);
    type_ = namee_->type_;
}

function_call::~function_call() {}

bool function_call::verify(rooot *rooot)
{
    // // ERROR : not declared
    // auto search = rooot->function_symbol_table_.find(namee_->identifier_val_);
    // function_decl *search_function_decl = std::get<1>(*search);
    // if (search == rooot->function_symbol_table_.end())
    // {
    //     print_error_msg(ERROR_ENUM::ERROR_NOT_DECL, location.begin);
    //     return false;
    // }
    // // ERROR type_arg: type of a function call argument does not match function declaration
    // // NAME_1: function_call parameters: namee_, comma_expression_star_quesmark_suite_
    // // NAME_2: symbol table parameter_list: search_function_decl->parameter_list_
    // bool match = true;
    // std::string arg_type_1 = "";
    // std::string arg_type_2 = "";
    // // first iteration
    // comma_expression_star_quesmark_suite *it1 = comma_expression_star_quesmark_suite_.get();
    // if ((comma_expression_star_quesmark_suite_ == nullptr && search_function_decl->parameter_list_ != nullptr) ||
    //     (comma_expression_star_quesmark_suite_ != nullptr && search_function_decl->parameter_list_ == nullptr))
    // {
    //     print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
    //     return false;
    // }
    // else
    // {
    //     arg_type_1 = comma_expression_star_quesmark_suite_->expression_->expression_prime_->type_;
    //     arg_type_2 = search_function_decl->parameter_list_->declaration_->namee_->type_;
    //     if (arg_type_1.compare(arg_type_2) != 0)
    //     {
    //         print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
    //         return false;
    //     }
    // }
    // second iteration
    // comma_expression_star_suite *it3 = it1->comma_expression_star_suite_.get();
    // comma_declaration_star_suite *it2 = search_function_decl->parameter_list_->comma_declaration_star_suite_.get();
    // if ((it3 != nullptr && it2 == nullptr) || (it3 == nullptr && it2 != nullptr))
    // {
    //     print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
    //     return false;
    // }
    // else if (it3 == nullptr && it2 == nullptr) {}
    // else
    // {
    //     arg_type_1 = it1->expression_->expression_prime_->type_;
    //     arg_type_2 = it2->declaration_->type_;
    //     if (arg_type_1.compare(arg_type_2) != 0)
    //     {
    //         print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
    //         return false;
    //     }
    // }
    // // third iteration
    // while (match)
    // {
    //     if (it3 == nullptr && it2 == nullptr)
    //         break;
    //     it3 = it3->comma_expression_star_suite_.get();
    //     it2 = it2->comma_declaration_star_suite_.get();
    //     if ((it2 == nullptr && it3 != nullptr) || (it2 != nullptr && it3 == nullptr))
    //         match = false;
    //     arg_type_1 = it3->expression_->expression_prime_->type_;
    //     arg_type_2 = it2->declaration_->type_;
    //     if (arg_type_1.compare(arg_type_2) != 0)
    //         match = false;
    // }

    // if (!match)
    // {
    //     print_error_msg(ERROR_ENUM::ERROR_TYPE_ARG, location.begin);
    //     return false;
    // }
    bool result = true;
    if (result && namee_ != nullptr)
        result = namee_->verify(rooot);
    if (result && comma_expression_star_quesmark_suite_ != nullptr)
        result = comma_expression_star_quesmark_suite_->verify(rooot);
    return result;
}

void function_call::print(int indent)
{
    std::cout << std::setw(indent) << "function_call (" << location.begin << ") {" << std::endl;
    if (namee_ != nullptr)
        namee_->print(indent + 4);
    if (comma_expression_star_quesmark_suite_ != nullptr)
        comma_expression_star_quesmark_suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *function_call::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    printf("Entering function_call::codegen\n");
    // Look up the name in the global module table.
    std::string Callee = namee_->identifier_val_;
    llvm::Function *CalleeF = getFunction(rooot, Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced");
    
    // traverse through the tree of arguments, and create arguments_ vector
    arguments_.clear();
    if (comma_expression_star_quesmark_suite_ != nullptr && comma_expression_star_quesmark_suite_->expression_ != nullptr)
    {
        auto tmp = comma_expression_star_quesmark_suite_.get();
        arguments_.push_back(tmp->expression_->codegen(rooot, named_values));
        auto head = tmp->comma_expression_star_suite_.get();
        while (head != nullptr && head->expression_ != nullptr)
        {
            arguments_.push_back(head->expression_->codegen(rooot, named_values));
            head = head->comma_expression_star_suite_.get();
        }
    }

    // If argument mismatch error.
    if (CalleeF->arg_size() != arguments_.size())
        return LogErrorV("Incorrect # arguments passed");

    printf("Leaving function_call::codegen\n");

    return rooot->comp_->builder->CreateCall(CalleeF, arguments_, "calltmp");
}

///////////////////////////////////////// comma_expression_star_quesmark_suite ///////////////////////////////////////////

comma_expression_star_quesmark_suite::comma_expression_star_quesmark_suite(std::unique_ptr<expression> expression, std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite) : Node()
{
    expression_ = std::move(expression);
    comma_expression_star_suite_ = std::move(comma_expression_star_suite);
}

comma_expression_star_quesmark_suite::~comma_expression_star_quesmark_suite() {}

bool comma_expression_star_quesmark_suite::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    if (result && comma_expression_star_suite_ != nullptr)
        result = comma_expression_star_suite_->verify(rooot);
    return result;
}

void comma_expression_star_quesmark_suite::print(int indent)
{
    std::cout << std::setw(indent) << "comma_expression_star_quesmark_suite (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent + 4);
    if (comma_expression_star_suite_ != nullptr)
        comma_expression_star_suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *comma_expression_star_quesmark_suite::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    // do not use this function
    return LogErrorV("Non-callable function");
}

///////////////////////////////////////// comma_expression_star_suite ///////////////////////////////////////////

comma_expression_star_suite::comma_expression_star_suite(std::unique_ptr<expression> expression, std::unique_ptr<comma_expression_star_suite> comma_expression_star_suite) : Node()
{
    expression_ = std::move(expression);
    comma_expression_star_suite_ = std::move(comma_expression_star_suite);
}

comma_expression_star_suite::~comma_expression_star_suite() {}

bool comma_expression_star_suite::verify(rooot *rooot)
{
    bool result = true;
    if (result && expression_ != nullptr)
        result = expression_->verify(rooot);
    if (result && comma_expression_star_suite_ != nullptr)
        result = comma_expression_star_suite_->verify(rooot);
    return result;
}

void comma_expression_star_suite::print(int indent)
{
    std::cout << std::setw(indent) << "comma_expression_star_suite (" << location.begin << ") {" << std::endl;
    if (expression_ != nullptr)
        expression_->print(indent + 4);
    if (comma_expression_star_suite_ != nullptr)
        comma_expression_star_suite_->print(indent + 4);
    std::cout << std::setw(indent) << "}" << std::endl;
}

llvm::Value *comma_expression_star_suite::codegen(rooot *rooot, std::map<std::string, llvm::AllocaInst *> * named_values)
{
    // do not use this function
    printf("Entering comma_expression_star_suite::codegen\n");
    return LogErrorV("Non-callable function");
}

///////////////////////////////////////// helper functions ///////////////////////////////////////////

void print_error_msg(ERROR_ENUM ERROR_NUM, yy::position location)
{
    std::string error_msg = "";
    std::string error_type = "";
    switch (ERROR_NUM)
    {
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

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static llvm::AllocaInst *CreateEntryBlockAlloca(rooot* rooot, llvm::Function *TheFunction, llvm::StringRef VarName, llvm::Type *VarType) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(VarType, nullptr, VarName);
}

// Search and return for function in root function_list
llvm::Function *getFunction(rooot* rooot, std::string Name) {
  // First, see if the function has already been added to the current module.
  if (llvm::Function *F = rooot->comp_->module->getFunction(Name))
    return F;

//   // If not, check whether we can codegen the declaration from some existing
//   // prototype.
//   llvm::Function FI = FunctionProtos.find(Name);
//   if (FI != FunctionProtos.end())
//     return FI->second->codegen();

  // If no existing prototype exists, return null.
  return nullptr;
}
