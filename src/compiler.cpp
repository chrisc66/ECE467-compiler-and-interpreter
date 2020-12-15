#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include <cstdio>
#include <cassert>
#include <cstring>

int lex(char const* path) {
	FILE* in = fopen(path, "r");
	if (in == nullptr) {
		printf("[error] unable to open file: %s\n", std::strerror(errno));
		return 1;
	}

	yyscan_t lexer;
	yylex_init_extra(1, &lexer);

	yyset_in(in, lexer);

	while (true) {
		yy::parser::symbol_type s;
		int x = yylex(&s, nullptr, lexer);
		assert(x == 1);
		printf("[output] lexer got symbol: %s (%d, %d).\n", s.name(), s.location.begin.line, s.location.begin.column);
		if (s.kind() == yy::parser::symbol_kind_type::S_YYEOF) {
			break;
		}
	}

	yylex_destroy(lexer);
	fclose(in);

	return 0;
}

int parse(char const* path, std::unique_ptr<Node>& root) {
	FILE* in = fopen(path, "r");
	if (in == nullptr) {
		printf("[error] unable to open file: %s\n", std::strerror(errno));
		return 1;
	}

	yyscan_t lexer;
	yylex_init_extra(1, &lexer);

	yyset_in(in, lexer);

	yy::parser p(lexer, root);
	int x = p.parse();

	yylex_destroy(lexer);
	fclose(in);

	return x;
}

bool verify_ast(Node* root) {
	// TODO: lab 3
	bool result = true; 
	if (root != nullptr){
		printf("verify_ast(), root != NULL\n");
		result = root->verify();
	}
	return result;
}

std::unique_ptr<Node> optimize(std::unique_ptr<Node> root) {
	// TODO: lab 3; return a new tree
	return root;
}

void print_ast(Node* root) {
	// TODO: lab 3
	int indent = 0;
	root->print(indent);
	return;
}

std::unique_ptr<CompilationUnit> compile(Node* root) {
	std::unique_ptr<CompilationUnit> unit = std::make_unique<CompilationUnit>();
	root->comp_ = unit.get();
	if (!unit->process(root)) {
		return nullptr;
	}
	return unit;
}

llvm::Value * LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}

llvm::Function * LogErrorF(const char *Str) {
  LogError(Str);
  return nullptr;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<Node> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

void CompilationUnit::initialize() {
	llvm::InitializeNativeTarget();
}

CompilationUnit::CompilationUnit() : context(std::make_unique<llvm::LLVMContext>()), builder(std::make_unique<llvm::IRBuilder<>>(*context)) {
	module = std::make_unique<llvm::Module>("ece467", *context);
}

bool CompilationUnit::process(Node* root) {
	(void) root;
	// TODO: lab 4
	root->codegen();
	llvm::verifyModule(*module, &llvm::errs());
	return true;
}

std::error_code CompilationUnit::dump(std::string path) {
	std::error_code ec;
	llvm::raw_fd_ostream out(path, ec, llvm::sys::fs::OpenFlags::F_None);
	//llvm::WriteBitcodeToFile(*this->module, out);
	this->module->print(out, nullptr);
	return ec;
}
