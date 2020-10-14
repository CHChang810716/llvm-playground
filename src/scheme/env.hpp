#pragma once
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include "ir.hpp"
namespace scheme {

constexpr struct Env {
    void main_module_preload(
        llvm::Module& mod,
        llvm::Function& main_f,
        FunctionTable& func_table
    ) const {
        llvm::GlobalVariable* argc = (llvm::GlobalVariable*)mod.getOrInsertGlobal(
            "argc", 
            llvm::Type::getInt32Ty(ir.get_context())
        );
        llvm::GlobalVariable* argv = (llvm::GlobalVariable*)mod.getOrInsertGlobal(
            "argv",
            llvm::PointerType::getUnqual(
                llvm::PointerType::getUnqual(
                    llvm::Type::getInt8Ty(ir.get_context())
                )
            )
        );
        argc->setInitializer(llvm::ConstantInt::get(argc->getType(), 0));
        argv->setInitializer(llvm::ConstantPointerNull::get(argv->getType()));
        ir.get_builder().CreateStore(main_f.arg_begin(), argc);
        ir.get_builder().CreateStore(main_f.arg_begin() + 1, argv);

        func_table["+"] = [](const Operands& operand) -> llvm::Value* {
            return ir.get_builder().CreateAdd(operand[0], operand[1]);
        };
        func_table["-"] = [](const Operands& operand) -> llvm::Value* {
            return ir.get_builder().CreateSub(operand[0], operand[1]);
        };
        func_table["*"] = [](const Operands& operand) -> llvm::Value* {
            return ir.get_builder().CreateMul(operand[0], operand[1]);
        };
        func_table["/"] = [](const Operands& operand) -> llvm::Value* {
            return ir.get_builder().CreateSDiv(operand[0], operand[1]);
        };
        func_table["=="] = [](const Operands& operand) -> llvm::Value* {
            if(operand[0]->getType() != llvm::Type::getInt8PtrTy(ir.get_context())) {
                return ir.get_builder().CreateICmpEQ(operand[0], operand[1]);
            } else if(operand[1]->getType() != llvm::Type::getInt8PtrTy(ir.get_context())) {
                throw std::runtime_error("string must compare to string");
            } else {
                throw std::runtime_error("string compare to string not yet support");
            }
        };
        func_table["atoi"] = [&mod](const Operands& operand) -> llvm::Value* {
            auto* atoi = ir.cfunc<int(const char*)>(
                "atoi", mod
            );
            ir.get_builder().CreateCall(atoi, operand);
        };
        func_table["printf"] = [&mod](const Operands& operand) -> llvm::Value* {
            return ir.get_builder().CreateCall(ir.printf(mod), operand);
        };
        func_table["at"] = [&mod](const Operands& operand) -> llvm::Value* {
            auto addr = ir.get_builder().CreateGEP(operand[0], operand[1]);
            return ir.get_builder().CreateLoad(addr);
        };
        func_table["if"] = [](const Operands& operand) -> llvm::Value* {
            auto* res = ir.get_builder().CreateAlloca(operand[1]->getType());
            auto* cond_merge = llvm::BasicBlock::Create(ir.get_context(), "cond_merge", ir.get_current_function());
            auto* cond_false = llvm::BasicBlock::Create(ir.get_context(), "cond_false", ir.get_current_function(), cond_merge);
            auto* cond_true = llvm::BasicBlock::Create(ir.get_context(), "cond_true", ir.get_current_function(), cond_false);
            ir.get_builder().CreateCondBr(operand[0], cond_true, cond_false);
            ir.get_builder().SetInsertPoint(cond_true);
            ir.get_builder().CreateStore(operand[1], res);
            ir.get_builder().CreateBr(cond_merge);
            ir.get_builder().SetInsertPoint(cond_false);
            ir.get_builder().CreateStore(operand[2], res);
            ir.get_builder().CreateBr(cond_merge);
            ir.get_builder().SetInsertPoint(cond_merge);
            return ir.get_builder().CreateLoad(res);
        };
    }
} env;

}