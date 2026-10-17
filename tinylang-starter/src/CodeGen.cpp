#include "CodeGen.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

CodeGenerator::CodeGenerator()
    : context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(
          "tinylang", *context_)),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)) {}

CodeGenerator::~CodeGenerator() = default;

bool CodeGenerator::generate(const Program &program)
{
    variables_.clear();

    // TinyLang Assignment 2 requires main(): int only.
    auto *functionType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*context_),
            false);

    auto *function =
        llvm::Function::Create(
            functionType,
            llvm::Function::ExternalLinkage,
            "main",
            module_.get());

    auto *entry =
        llvm::BasicBlock::Create(
            *context_, "entry", function);

    builder_->SetInsertPoint(entry);

    return emitBlock(*program.mainFunction.body);
}

bool CodeGenerator::emitBlock(const BlockStmt &block)
{
    for (const auto &stmt : block.statements)
    {
        if (!emitStmt(*stmt))
            return false;
    }
    return true;
}

bool CodeGenerator::emitStmt(const Stmt &stmt)
{
    // TODO(A2):
    //
    // Required:
    //   VarDeclStmt for int
    //   AssignStmt for int
    //   ReturnStmt for int
    //
    // Suggested VarDecl implementation:
    //
    //   auto* slot = builder_->CreateAlloca(
    //       llvm::Type::getInt32Ty(*context_),
    //       nullptr,
    //       variableName);
    //
    // Save slot in variables_.
    //
    // IfStmt and bool LLVM generation may be extra credit.

    // code after a return in the same block can never run so skip it
    if (builder_->GetInsertBlock()->getTerminator())
        return true;

    if (auto *decl = dynamic_cast<const VarDeclStmt *>(&stmt))
    {
        llvm::Type *type = llvm::Type::getInt32Ty(*context_);
        if (decl->type == Type::Bool)
            type = llvm::Type::getInt1Ty(*context_);

        // get the starting value before making the new variable
        // so let x: int = x + 1; still uses the outer x
        llvm::Value *value;
        if (decl->initializer)
        {
            value = emitExpr(*decl->initializer);
            if (!value)
                return false;
        }
        else
        {
            value = llvm::ConstantInt::get(type, 0); // no starting value means 0 or false
        }

        auto *slot = builder_->CreateAlloca(type, nullptr, decl->name);
        builder_->CreateStore(value, slot);
        variables_[decl->name] = slot;
        return true;
    }

    if (auto *assign = dynamic_cast<const AssignStmt *>(&stmt))
    {
        auto it = variables_.find(assign->name);
        if (it == variables_.end())
        {
            llvm::errs() << "unknown variable '" << assign->name << "'\n";
            return false;
        }

        llvm::Value *value = emitExpr(*assign->value);
        if (!value)
            return false;

        builder_->CreateStore(value, it->second);
        return true;
    }

    if (auto *ret = dynamic_cast<const ReturnStmt *>(&stmt))
    {
        llvm::Value *value = emitExpr(*ret->value);
        if (!value)
            return false;

        builder_->CreateRet(value);
        return true;
    }

    if (auto *ifStmt = dynamic_cast<const IfStmt *>(&stmt))
    {
        llvm::Value *cond = emitExpr(*ifStmt->condition);
        if (!cond)
            return false;

        llvm::Function *function = builder_->GetInsertBlock()->getParent();
        auto *thenBlock = llvm::BasicBlock::Create(*context_, "then", function);
        auto *elseBlock = llvm::BasicBlock::Create(*context_, "else", function);
        auto *endBlock = llvm::BasicBlock::Create(*context_, "endif", function);

        builder_->CreateCondBr(cond, thenBlock, elseBlock);

        // variables made inside a block should go away after it
        auto saved = variables_;

        builder_->SetInsertPoint(thenBlock);
        if (!emitBlock(*ifStmt->thenBlock))
            return false;
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(endBlock);
        variables_ = saved;

        builder_->SetInsertPoint(elseBlock);
        if (ifStmt->elseBlock && !emitBlock(*ifStmt->elseBlock))
            return false;
        if (!builder_->GetInsertBlock()->getTerminator())
            builder_->CreateBr(endBlock);
        variables_ = saved;

        builder_->SetInsertPoint(endBlock);

        // if both sides returned nothing can get here
        if (endBlock->hasNPredecessors(0))
            builder_->CreateUnreachable();
        return true;
    }

    llvm::errs() << "unknown statement\n";
    return false;
}

llvm::Value *CodeGenerator::emitExpr(const Expr &expr)
{
    // TODO(A2):
    //
    // Required:
    //   IntegerExpr
    //   VariableExpr
    //   BinaryExpr: + - * / < > ==
    //
    // Integer constant:
    //
    //   llvm::ConstantInt::get(
    //       llvm::Type::getInt32Ty(*context_),
    //       value,
    //       true);
    //
    // Variable load:
    //
    //   builder_->CreateLoad(
    //       llvm::Type::getInt32Ty(*context_),
    //       slot,
    //       name);
    //
    // Useful IRBuilder operations:
    //
    //   CreateAdd
    //   CreateSub
    //   CreateMul
    //   CreateSDiv
    //   CreateICmpSLT
    //   CreateICmpSGT
    //   CreateICmpEQ

    if (auto *num = dynamic_cast<const IntegerExpr *>(&expr))
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), num->value, true);

    if (auto *boolean = dynamic_cast<const BoolExpr *>(&expr))
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), boolean->value ? 1 : 0);

    if (auto *var = dynamic_cast<const VariableExpr *>(&expr))
    {
        auto it = variables_.find(var->name);
        if (it == variables_.end())
        {
            llvm::errs() << "unknown variable '" << var->name << "'\n";
            return nullptr;
        }

        // the slot knows if it holds an int (i32) or a bool (i1)
        llvm::AllocaInst *slot = it->second;
        return builder_->CreateLoad(slot->getAllocatedType(), slot, var->name);
    }

    if (auto *bin = dynamic_cast<const BinaryExpr *>(&expr))
    {
        llvm::Value *left = emitExpr(*bin->lhs);
        llvm::Value *right = emitExpr(*bin->rhs);
        if (!left || !right)
            return nullptr;

        if (bin->op == "+")
            return builder_->CreateAdd(left, right, "add");
        if (bin->op == "-")
            return builder_->CreateSub(left, right, "sub");
        if (bin->op == "*")
            return builder_->CreateMul(left, right, "mul");
        if (bin->op == "/")
            return builder_->CreateSDiv(left, right, "div");
        if (bin->op == "<")
            return builder_->CreateICmpSLT(left, right, "lt");
        if (bin->op == ">")
            return builder_->CreateICmpSGT(left, right, "gt");
        if (bin->op == "==")
            return builder_->CreateICmpEQ(left, right, "eq");

        // both sides always get worked out here, there is no short circuit
        if (bin->op == "&&")
            return builder_->CreateAnd(left, right, "and");
        if (bin->op == "||")
            return builder_->CreateOr(left, right, "or");

        llvm::errs() << "unknown operator '" << bin->op << "'\n";
        return nullptr;
    }

    llvm::errs() << "unknown expression\n";
    return nullptr;
}

void CodeGenerator::printIR() const
{
    module_->print(llvm::outs(), nullptr);
}
