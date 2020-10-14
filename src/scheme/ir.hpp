#pragma once
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <tao/pegtl/contrib/parse_tree.hpp>
namespace scheme {

constexpr struct IR {
  llvm::LLVMContext& get_context() const {
    static thread_local llvm::LLVMContext context;
    return context;
  }
  llvm::IRBuilder<>& get_builder() const {
    static thread_local llvm::IRBuilder<> irbuilder(
      get_context()
    );
    return irbuilder;
  }
  llvm::Function* printf(llvm::Module& mod) const {
    llvm::FunctionType* printf_type = 
      llvm::TypeBuilder<int(char*, ...), false>::get(get_context());
    llvm::Function *func = llvm::cast<llvm::Function>(mod.getOrInsertFunction(
      "printf", printf_type,
      llvm::AttributeList().addAttribute(mod.getContext(), 1U, llvm::Attribute::NoAlias)));
    return func;
  }
  template<class FSig>
  llvm::Function* cfunc(const std::string& name, llvm::Module& mod) const {
    llvm::FunctionType* ftype = llvm::TypeBuilder<FSig, false>::get(get_context());
    llvm::Function *func = llvm::cast<llvm::Function>(mod.getOrInsertFunction(
      name, ftype,
      llvm::AttributeList().addAttribute(mod.getContext(), 1U, llvm::Attribute::NoAlias)));
    return func;
  }
  llvm::Function* main(llvm::Module& mod) const {
    llvm::FunctionType *foo_type =
      llvm::TypeBuilder<int(int, char **), false>::get(get_context());
    llvm::Function *func = llvm::cast<llvm::Function>(mod.getOrInsertFunction("main", foo_type));
    return func;
  }
} ir;
using Operands = std::vector<llvm::Value*>;
using FunctionTable = std::map<
  std::string, 
  std::function<llvm::Value*(const Operands&)>
>;
constexpr auto transform_escape_char = [](std::string& str) {
  if(str.size() < 2) return;
  std::size_t len = str.size();
  for(std::size_t i = str.size() - 1; i > 0; i --) {
    if(str[i - 1] != '\\') continue;
    len --;
    auto& c = str[i - 1];
    auto& nc = str[i];
    switch(nc) {
      case 'n':
        c = '\n';
        break;
      case 'r':
        c = '\r';
        break;
      case 't':
        c = '\t';
        break;
      default:
        c = '\n';
        break;
    }
    std::strcpy(&nc, &nc + 1);
  }
  str.resize(len);
};
constexpr struct SExprIR {
  llvm::Value* operator()(
    llvm::IRBuilder<>& irb, 
    tao::pegtl::parse_tree::node& sexpr, 
    llvm::ValueSymbolTable& sym_table,
    FunctionTable& func_table,
    llvm::Module& mod
  ) const {
    auto& opr = *(sexpr.children[0]);
    std::vector<llvm::Value*> operand;
    for(std::size_t i = 1; i < sexpr.children.size(); i ++) {
      auto& elm = *(sexpr.children[i]);
      if(elm.name() == "scheme::SExpr") {
        operand.push_back(this->operator()(irb, elm, sym_table, func_table, mod));
      } else if(elm.name() == "scheme::Symbol" ) {
        auto local_ptr = sym_table.lookup(elm.string());
        if(local_ptr != nullptr) {
          operand.push_back(local_ptr);
        } else {
          auto global_ptr = mod.getGlobalVariable(elm.string());
          if(global_ptr == nullptr) {
            throw std::runtime_error("identifier not found");
          }
          operand.push_back(irb.CreateLoad(global_ptr));
        }
      } else if(elm.name() == "scheme::Number") {
        operand.push_back(llvm::ConstantInt::get(
          ir.get_context(),
          llvm::APInt(32, std::stoi(elm.string()))
        ));
      } else if(elm.name() == "scheme::String") {
        auto str = elm.string();
        transform_escape_char(str);
        operand.push_back(irb.CreateGlobalStringPtr(str.substr(1, str.size() - 2)));
      }
    }
    return func_table.at(opr.string())(operand);
  }
} sexpr_ir;

}