#include "Semantic.h"

void SemanticAnalyzer::enterScope()
{
    scopes_.push_back({});
}

void SemanticAnalyzer::leaveScope()
{
    scopes_.pop_back();
}

void SemanticAnalyzer::report(const std::string &message)
{
    errors_.push_back(message);
}

bool SemanticAnalyzer::declare(const std::string &name, Type type)
{
    // TODO(A2):
    // Reject duplicate declarations in the current scope only.
    //
    // Return true on success, false on duplicate.
    auto &current = scopes_.back();
    if (current.count(name) > 0)
        return false;

    current[name] = type;
    return true;
}

Type SemanticAnalyzer::lookup(const std::string &name) const
{
    // TODO(A2):
    // Search scopes from innermost to outermost.
    // Return Type::Error if not found.
    for (int i = (int)scopes_.size() - 1; i >= 0; i--)
    {
        auto it = scopes_[i].find(name);
        if (it != scopes_[i].end())
            return it->second;
    }
    return Type::Error;
}

bool SemanticAnalyzer::analyze(const Program &program)
{
    errors_.clear();
    scopes_.clear();

    currentReturnType_ = program.mainFunction.returnType;

    enterScope();
    checkBlock(*program.mainFunction.body);
    leaveScope();

    return errors_.empty();
}

void SemanticAnalyzer::checkBlock(const BlockStmt &block)
{
    // Each block receives its own lexical scope.
    enterScope();

    for (const auto &stmt : block.statements)
        checkStmt(*stmt);

    leaveScope();
}

void SemanticAnalyzer::checkStmt(const Stmt &stmt)
{
    // TODO(A2):
    //
    // VarDecl:
    //   - reject duplicate declaration
    //   - if initializer exists, require same type
    //
    // Assign:
    //   - variable must exist
    //   - assigned expression must have same type
    //
    // Return:
    //   - expression type must match currentReturnType_
    //
    // If:
    //   - condition must be bool
    //   - analyze then/else blocks

    if (auto *decl = dynamic_cast<const VarDeclStmt *>(&stmt))
    {
        // check the initializer first so let x: int = x; can't see the new x
        if (decl->initializer)
        {
            Type initType = checkExpr(*decl->initializer);
            if (initType != decl->type)
                report("cannot initialize variable '" + decl->name + "' of type " +
                       typeName(decl->type) + " with " + typeName(initType));
        }

        if (!declare(decl->name, decl->type))
            report("duplicate declaration of variable '" + decl->name + "'");
        return;
    }

    if (auto *assign = dynamic_cast<const AssignStmt *>(&stmt))
    {
        Type varType = lookup(assign->name);
        Type valueType = checkExpr(*assign->value);

        if (varType == Type::Error)
        {
            report("use of undeclared variable '" + assign->name + "'");
            return;
        }

        if (valueType != varType)
            report("cannot assign " + typeName(valueType) + " to variable '" +
                   assign->name + "' of type " + typeName(varType));
        return;
    }

    if (auto *ret = dynamic_cast<const ReturnStmt *>(&stmt))
    {
        Type type = checkExpr(*ret->value);
        if (type != currentReturnType_)
            report("return expression has type " + typeName(type) +
                   "; expected " + typeName(currentReturnType_));
        return;
    }

    if (auto *ifStmt = dynamic_cast<const IfStmt *>(&stmt))
    {
        Type condType = checkExpr(*ifStmt->condition);
        if (condType != Type::Bool)
            report("if condition must have type bool");

        // checkBlock gives each block its own scope
        checkBlock(*ifStmt->thenBlock);
        if (ifStmt->elseBlock)
            checkBlock(*ifStmt->elseBlock);
        return;
    }

    report("unknown statement");
}

Type SemanticAnalyzer::checkExpr(const Expr &expr)
{
    // TODO(A2):
    //
    // Literals:
    //   IntegerExpr -> int
    //   BoolExpr    -> bool
    //
    // VariableExpr:
    //   look up variable; report undeclared name
    //
    // BinaryExpr:
    //
    //   + - * /
    //      int x int -> int
    //
    //   < >
    //      int x int -> bool
    //
    //   ==
    //      operands must have same type -> bool
    //
    //   && ||
    //      bool x bool -> bool

    if (dynamic_cast<const IntegerExpr *>(&expr))
        return Type::Int;

    if (dynamic_cast<const BoolExpr *>(&expr))
        return Type::Bool;

    if (auto *var = dynamic_cast<const VariableExpr *>(&expr))
    {
        Type type = lookup(var->name);
        if (type == Type::Error)
            report("use of undeclared variable '" + var->name + "'");
        return type;
    }

    if (auto *bin = dynamic_cast<const BinaryExpr *>(&expr))
    {
        Type left = checkExpr(*bin->lhs);
        Type right = checkExpr(*bin->rhs);
        std::string op = bin->op;

        if (op == "+")
        {
            if (left != Type::Int || right != Type::Int)
            {
                report("operator '+' requires int operands");
                return Type::Error;
            }
            return Type::Int;
        }

        if (op == "-")
        {
            if (left != Type::Int || right != Type::Int)
            {
                report("operator '-' requires int operands");
                return Type::Error;
            }
            return Type::Int;
        }

        if (op == "*")
        {
            if (left != Type::Int || right != Type::Int)
            {
                report("operator '*' requires int operands");
                return Type::Error;
            }
            return Type::Int;
        }

        if (op == "/")
        {
            if (left != Type::Int || right != Type::Int)
            {
                report("operator '/' requires int operands");
                return Type::Error;
            }
            return Type::Int;
        }

        if (op == "<")
        {
            if (left != Type::Int || right != Type::Int)
            {
                report("operator '<' requires int operands");
                return Type::Error;
            }
            return Type::Bool;
        }

        if (op == ">")
        {
            if (left != Type::Int || right != Type::Int)
            {
                report("operator '>' requires int operands");
                return Type::Error;
            }
            return Type::Bool;
        }

        if (op == "==")
        {
            if (left != right)
            {
                report("operator '==' requires both sides to have the same type");
                return Type::Error;
            }
            return Type::Bool;
        }

        if (op == "&&" || op == "||")
        {
            if (left != Type::Bool || right != Type::Bool)
            {
                report("operator '" + op + "' requires bool operands");
                return Type::Error;
            }
            return Type::Bool;
        }

        report("unknown operator '" + op + "'");
        return Type::Error;
    }

    report("unknown expression");
    return Type::Error;
}
