#include "Parser.h"
#include <sstream>

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

const Token &Parser::peek(size_t offset) const
{
    size_t index = current_ + offset;
    if (index >= tokens_.size())
        return tokens_.back();
    return tokens_[index];
}

const Token &Parser::previous() const
{
    return tokens_[current_ - 1];
}

bool Parser::atEnd() const
{
    return peek().kind == TokenKind::End;
}

bool Parser::check(TokenKind kind) const
{
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind)
{
    if (!check(kind))
        return false;
    ++current_;
    return true;
}

const Token &Parser::consume(TokenKind kind,
                             const std::string &message)
{
    if (check(kind))
        return tokens_[current_++];
    error(peek(), message);
}

[[noreturn]] void Parser::error(const Token &token,
                                const std::string &message) const
{
    std::ostringstream out;
    out << "Syntax error at line "
        << token.line << ", column " << token.column
        << ": " << message;
    throw ParseError(out.str());
}

Program Parser::parseProgram()
{
    // TinyLang deliberately contains exactly one main function.
    Program p{parseMainFunction()};

    consume(TokenKind::End,
            "unexpected tokens after main function");
    return p;
}

Function Parser::parseMainFunction()
{
    // TODO(A1): Parse exactly:
    //
    // fn main() : int { ... }
    //
    // You may require the function name to literally be "main".
    // TinyLang has no parameters and no additional functions.

    consume(TokenKind::KwFn, "expected 'fn' at start of program");

    std::string name = consume(TokenKind::Identifier, "expected 'main' after 'fn'").lexeme;
    if (name != "main")
        error(previous(), "function has to be named 'main' not '" + name + "'");

    consume(TokenKind::LParen, "expected '(' after 'main'");
    consume(TokenKind::RParen, "expected ')' after '('");
    consume(TokenKind::Colon, "expected ':' before return type");
    consume(TokenKind::KwInt, "expected 'int' as the return type"); // main always returns int

    Function function;
    function.name = name;
    function.returnType = Type::Int;
    function.body = parseBlock();
    return function;
}

std::unique_ptr<BlockStmt> Parser::parseBlock()
{
    // TODO(A1):
    // block := "{" statement* "}"
    consume(TokenKind::LBrace, "expected '{' to start block");

    auto block = std::make_unique<BlockStmt>();
    // atEnd stops the loop if the } is missing
    while (!check(TokenKind::RBrace) && !atEnd())
        block->statements.push_back(parseStatement());

    consume(TokenKind::RBrace, "expected '}' to close block");
    return block;
}

StmtPtr Parser::parseStatement()
{
    // TODO(A1): Select among:
    //
    // let declaration
    // if statement
    // return statement
    // assignment (starts with Identifier)

    switch (peek().kind)
    {
    case TokenKind::KwLet:
        return parseVarDecl();
    case TokenKind::KwIf:
        return parseIf();
    case TokenKind::KwReturn:
        return parseReturn();
    case TokenKind::Identifier:
        return parseAssignment();
    default:
        error(peek(), "expected a statement but found '" + peek().lexeme + "'");
    }
}

StmtPtr Parser::parseVarDecl()
{
    // TODO(A1):
    //
    // var_decl :=
    //   "let" Identifier ":" type
    //   ("=" expression)? ";"

    consume(TokenKind::KwLet, "expected 'let'");
    std::string name = consume(TokenKind::Identifier, "expected variable name after 'let'").lexeme;
    consume(TokenKind::Colon, "expected ':' after variable name");
    Type type = parseType();

    ExprPtr initializer; // stays null if there is no = part
    if (match(TokenKind::Assign))
        initializer = parseExpression();

    // point at the last token so the error shows up on the right line
    if (!match(TokenKind::Semicolon))
        error(previous(), "expected ';' after '" + previous().lexeme + "'");
    return std::make_unique<VarDeclStmt>(name, type, std::move(initializer));
}

StmtPtr Parser::parseAssignment()
{
    // TODO(A1):
    //
    // assignment :=
    //   Identifier "=" expression ";"

    std::string name = consume(TokenKind::Identifier, "expected variable name").lexeme;
    consume(TokenKind::Assign, "expected '=' after " + name);
    ExprPtr value = parseExpression();
    if (!match(TokenKind::Semicolon))
        error(previous(), "expected ';' after '" + previous().lexeme + "'");
    return std::make_unique<AssignStmt>(name, std::move(value));
}

StmtPtr Parser::parseIf()
{
    // TODO(A1):
    //
    // if_stmt :=
    //   "if" "(" expression ")" block
    //   ("else" block)?

    consume(TokenKind::KwIf, "expected 'if'");
    consume(TokenKind::LParen, "expected '(' after 'if'");
    ExprPtr condition = parseExpression();
    consume(TokenKind::RParen, "expected ')' after if condition");

    std::unique_ptr<BlockStmt> thenBlock = parseBlock();

    std::unique_ptr<BlockStmt> elseBlock; // stays null if there is no else
    if (match(TokenKind::KwElse))
        elseBlock = parseBlock();

    return std::make_unique<IfStmt>(std::move(condition),
                                    std::move(thenBlock),
                                    std::move(elseBlock));
}

StmtPtr Parser::parseReturn()
{
    // TODO(A1):
    //
    // return_stmt :=
    //   "return" expression ";"

    consume(TokenKind::KwReturn, "expected 'return'");
    ExprPtr value = parseExpression();
    if (!match(TokenKind::Semicolon))
        error(previous(), "expected ';' after '" + previous().lexeme + "'");
    return std::make_unique<ReturnStmt>(std::move(value));
}

Type Parser::parseType()
{
    // TODO(A1): Recognize int and bool.
    if (match(TokenKind::KwInt))
        return Type::Int;
    if (match(TokenKind::KwBool))
        return Type::Bool;
    if (atEnd())
        error(peek(), "expected type 'int' or 'bool' but found end of file");
    error(peek(), "expected type 'int' or 'bool' but found '" + peek().lexeme + "'");
}

ExprPtr Parser::parseExpression()
{
    return parseOr();
}

ExprPtr Parser::parseOr()
{
    // TODO(A1):
    // or := and ("||" and)*
    ExprPtr left = parseAnd();
    while (match(TokenKind::OrOr))
    {
        std::string op = previous().lexeme;
        ExprPtr right = parseAnd();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parseAnd()
{
    // TODO(A1):
    // and := equality ("&&" equality)*
    ExprPtr left = parseEquality();
    while (match(TokenKind::AndAnd))
    {
        std::string op = previous().lexeme;
        ExprPtr right = parseEquality();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parseEquality()
{
    // TODO(A1):
    // equality := comparison ("==" comparison)*
    ExprPtr left = parseComparison();
    while (match(TokenKind::EqualEqual))
    {
        std::string op = previous().lexeme;
        ExprPtr right = parseComparison();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parseComparison()
{
    // TODO(A1):
    // comparison := term (("<" | ">") term)*
    ExprPtr left = parseTerm();
    while (match(TokenKind::Less) || match(TokenKind::Greater))
    {
        std::string op = previous().lexeme;
        ExprPtr right = parseTerm();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parseTerm()
{
    // TODO(A1):
    // term := factor (("+" | "-") factor)*
    // the loop makes 10 - 3 - 2 group from the left
    ExprPtr left = parseFactor();
    while (match(TokenKind::Plus) || match(TokenKind::Minus))
    {
        std::string op = previous().lexeme;
        ExprPtr right = parseFactor();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parseFactor()
{
    // TODO(A1):
    // factor := primary (("*" | "/") primary)*
    ExprPtr left = parsePrimary();
    while (match(TokenKind::Star) || match(TokenKind::Slash))
    {
        std::string op = previous().lexeme;
        ExprPtr right = parsePrimary();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::parsePrimary()
{
    // TODO(A1): Handle:
    //
    // Integer
    // true
    // false
    // Identifier
    // "(" expression ")"

    if (match(TokenKind::Integer))
    {
        try
        {
            return std::make_unique<IntegerExpr>(std::stoi(previous().lexeme));
        }
        catch (const std::out_of_range &)
        {
            error(previous(), "number is too big for an int");
        }
    }

    if (match(TokenKind::KwTrue))
        return std::make_unique<BoolExpr>(true);
    if (match(TokenKind::KwFalse))
        return std::make_unique<BoolExpr>(false);

    if (match(TokenKind::Identifier))
        return std::make_unique<VariableExpr>(previous().lexeme);

    if (match(TokenKind::LParen))
    {
        ExprPtr inner = parseExpression();
        consume(TokenKind::RParen, "expected ')' after expression");
        return inner;
    }

    if (atEnd())
        error(peek(), "expected expression but found end of file");
    error(peek(), "expected expression but found '" + peek().lexeme + "'");
}
