#include "parser.hpp"
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include <iostream>
#include "ir.hpp"
#include <llvm/Support/raw_ostream.h>
int main(int argc, char *argv[]) {
  using RuleFile = tao::pegtl::must<scheme::File>;
  if(argc < 2) { return 1; }
  tao::pegtl::file_input<> in(argv[1]);
  auto root = tao::pegtl::parse_tree::parse<RuleFile, scheme::ASTSelector>(in);
  tao::pegtl::parse_tree::print_dot(std::cerr, *root);

  std::unique_ptr<llvm::Module> module(
    new llvm::Module("initial", scheme::ir.get_context())
  );
  auto* ir_main_f = scheme::ir.main(module.get());
  llvm::ValueSymbolTable main_sym_table;
  for(auto& sexpr : root->children) {
    std::cerr << sexpr->string() << '\n';
    auto expr_block = llvm::BasicBlock::Create(scheme::ir.get_context(), "", ir_main_f);
    scheme::ir.get_builder().SetInsertPoint(expr_block);
    scheme::sexpr_ir(
      scheme::ir.get_builder(), 
      *sexpr, 
      main_sym_table, 
      *module
    )->setName("ignore");
  }
  auto retb = llvm::BasicBlock::Create(scheme::ir.get_context(), "", ir_main_f);
  scheme::ir.get_builder().SetInsertPoint(retb);
  scheme::ir.get_builder().CreateRet(
    llvm::ConstantInt::get(scheme::ir.get_context(), llvm::APInt(32, 0))
  );
  module->print(llvm::outs(), nullptr);
  return 0;
}