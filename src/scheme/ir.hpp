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
  llvm::Function* printf(llvm::Module* mod) const {
    llvm::FunctionType* printf_type = 
      llvm::TypeBuilder<int(char*, ...), false>::get(get_context());
    llvm::Function *func = llvm::cast<llvm::Function>(mod->getOrInsertFunction(
      "printf", printf_type,
      llvm::AttributeList().addAttribute(mod->getContext(), 1U, llvm::Attribute::NoAlias)));
    return func;
  }
  llvm::Function* main(llvm::Module* mod) const {
    llvm::FunctionType *foo_type =
      llvm::TypeBuilder<int(int, char **), false>::get(get_context());
    llvm::Function *func = llvm::cast<llvm::Function>(mod->getOrInsertFunction("main", foo_type));
    return func;
  }
} ir;

constexpr struct SExprIR {
  llvm::Value* operator()(
    llvm::IRBuilder<>& irb, 
    tao::pegtl::parse_tree::node& sexpr, 
    llvm::ValueSymbolTable& sym_table,
    llvm::Module& mod
  ) const {
    auto& opr = *(sexpr.children[0]);
    std::vector<llvm::Value*> operand;
    for(std::size_t i = sexpr.children.size() - 1; i > 0; i --) {
      auto& elm = *(sexpr.children[i]);
      if(elm.name() == "scheme::SExpr") {
        operand.push_back(this->operator()(irb, elm, sym_table, mod));
      } else if(elm.name() == "scheme::Symbol" ) {
        operand.push_back(sym_table.lookup(elm.string()));
      } else if(elm.name() == "scheme::Number") {
        operand.push_back(llvm::ConstantInt::get(
          ir.get_context(),
          llvm::APInt(64, std::stoi(elm.string()))
        ));
      }
    }
    if(opr.string() == "+") {
      return irb.CreateAdd(operand[1], operand[0]);
    } else if(opr.string() == "-") {
      return irb.CreateSub(operand[1], operand[0]);
    } else if(opr.string() == "*") {
      return irb.CreateMul(operand[1], operand[0]);
    } else if(opr.string() == "/") {
      return irb.CreateSDiv(operand[1], operand[0]);
    } else if(opr.string() == "println") {
      std::vector<llvm::Value*> args;
      args.push_back(irb.CreateGlobalStringPtr("%d\n"));
      args.insert(args.end(), operand.begin(), operand.end());
      return irb.CreateCall(ir.printf(&mod), args);
    }
  }
} sexpr_ir;

}