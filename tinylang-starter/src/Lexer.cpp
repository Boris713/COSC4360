#include "Lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

const char *tokenKindName(TokenKind kind)
{
    switch (kind)
    {
    case TokenKind::End:
        return "End";
    case TokenKind::Identifier:
        return "Identifier";
    case TokenKind::Integer:
        return "Integer";
    case TokenKind::KwFn:
        return "fn";
    case TokenKind::KwLet:
        return "let";
    case TokenKind::KwInt:
        return "int";
    case TokenKind::KwBool:
        return "bool";
    case TokenKind::KwIf:
        return "if";
    case TokenKind::KwElse:
        return "else";
    case TokenKind::KwReturn:
        return "return";
    case TokenKind::KwTrue:
        return "true";
    case TokenKind::KwFalse:
        return "false";
    case TokenKind::LParen:
        return "(";
    case TokenKind::RParen:
        return ")";
    case TokenKind::LBrace:
        return "{";
    case TokenKind::RBrace:
        return "}";
    case TokenKind::Colon:
        return ":";
    case TokenKind::Semicolon:
        return ";";
    case TokenKind::Comma:
        return ",";
    case TokenKind::Assign:
        return "=";
    case TokenKind::Plus:
        return "+";
    case TokenKind::Minus:
        return "-";
    case TokenKind::Star:
        return "*";
    case TokenKind::Slash:
        return "/";
    case TokenKind::Less:
        return "<";
    case TokenKind::Greater:
        return ">";
    case TokenKind::EqualEqual:
        return "==";
    case TokenKind::AndAnd:
        return "&&";
    case TokenKind::OrOr:
        return "||";
    }
    return "?";
}

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::peek(size_t offset) const
{
    if (pos_ + offset >= source_.size())
        return '\0';
    return source_[pos_ + offset];
}

bool Lexer::atEnd() const
{
    return pos_ >= source_.size();
}

char Lexer::advance()
{
    char c = source_[pos_++];
    if (c == '\n')
    {
        ++line_;
        column_ = 1;
    }
    else
    {
        ++column_;
    }
    return c;
}

Token Lexer::makeToken(TokenKind kind,
                       const std::string &lexeme,
                       int line,
                       int column) const
{
    return Token{kind, lexeme, line, column};
}

void Lexer::skipWhitespaceAndComments()
{
    while (!atEnd())
    {
        if (std::isspace(static_cast<unsigned char>(peek())))
        {
            advance();
            continue;
        }

        if (peek() == '/' && peek(1) == '/')
        {
            while (!atEnd() && peek() != '\n')
                advance();
            continue;
        }

        break;
    }
}

Token Lexer::lexIdentifierOrKeyword()
{
    // TODO(A1): Read [A-Za-z_][A-Za-z0-9_]*.
    //
    // Then recognize these keywords:
    // fn let int bool if else return true false
    //
    // Otherwise return Identifier.

    int startLine = line_;
    int startColumn = column_;
    std::string text;

    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
    {
        /* dont have to check starts with letter since tokenize() already does */
        text += advance();
    }

    static const std::unordered_map<std::string, TokenKind> keywords = {
        {"fn", TokenKind::KwFn},
        {"let", TokenKind::KwLet},
        {"int", TokenKind::KwInt},
        {"bool", TokenKind::KwBool},
        {"if", TokenKind::KwIf},
        {"else", TokenKind::KwElse},
        {"return", TokenKind::KwReturn},
        {"true", TokenKind::KwTrue},
        {"false", TokenKind::KwFalse},
    };

    auto it = keywords.find(text);
    if (it != keywords.end())
        return makeToken(it->second, text, startLine, startColumn); // its a keyword
    return makeToken(TokenKind::Identifier, text, startLine, startColumn);
}

Token Lexer::lexInteger()
{
    // TODO(A1): Consume a complete sequence of decimal digits.

    int startLine = line_;
    int startColumn = column_;
    std::string text;

    while (std::isdigit(static_cast<unsigned char>(peek())))
    {
        text += advance();
    }

    // letters stuck to a number like 123abc make it a bad number
    if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_')
    {
        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
            text += advance();

        throw std::runtime_error(
            "Lexical error at line " + std::to_string(startLine) +
            ", column " + std::to_string(startColumn) +
            ": invalid number '" + text + "'");
    }

    return makeToken(TokenKind::Integer, text,
                     startLine, startColumn);
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!atEnd())
    {
        skipWhitespaceAndComments();
        if (atEnd())
            break;

        int startLine = line_;
        int startColumn = column_;
        char c = peek();

        auto add = [&](TokenKind kind, const std::string &lexeme)
        {
            tokens.push_back(makeToken(kind, lexeme, startLine, startColumn));
        };

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
        {
            tokens.push_back(lexIdentifierOrKeyword());
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            tokens.push_back(lexInteger());
            continue;
        }

        // TODO(A1): Recognize punctuation and operators:
        // ( ) { } : ; ,
        // = + - * / < >
        // == && ||
        //
        // Remember that ==, && and || are two-character tokens.

        // tokens that are always just one character
        static const std::unordered_map<char, TokenKind> singleChars = {
            {'(', TokenKind::LParen},
            {')', TokenKind::RParen},
            {'{', TokenKind::LBrace},
            {'}', TokenKind::RBrace},
            {':', TokenKind::Colon},
            {';', TokenKind::Semicolon},
            {',', TokenKind::Comma},
            {'+', TokenKind::Plus},
            {'-', TokenKind::Minus},
            {'*', TokenKind::Star},
            {'/', TokenKind::Slash}, // "//" comments were already skipped so this is always division
            {'<', TokenKind::Less},
            {'>', TokenKind::Greater},
        };

        auto single = singleChars.find(c);
        if (single != singleChars.end())
        {
            advance();
            add(single->second, std::string(1, c));
            continue;
        }

        // = && and || need to look at the next character too
        switch (c)
        {
        case '=':
            advance();
            if (peek() == '=')
            {
                advance();
                add(TokenKind::EqualEqual, "==");
            }
            else
            {
                add(TokenKind::Assign, "=");
            }
            continue;

        case '&':
            if (peek(1) == '&')
            {
                advance();
                advance();
                add(TokenKind::AndAnd, "&&");
                continue;
            }
            break; // single & is an error

        case '|':
            if (peek(1) == '|')
            {
                advance();
                advance();
                add(TokenKind::OrOr, "||");
                continue;
            }
            break; // single | is an error
        }
        throw std::runtime_error(
            "Lexical error at line " + std::to_string(startLine) +
            ", column " + std::to_string(startColumn) +
            ": unexpected character '" + std::string(1, c) + "'");
    }

    tokens.push_back(
        makeToken(TokenKind::End, "", line_, column_));
    return tokens;
}
