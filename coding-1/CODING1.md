# Coding Assignment 1: TinyLang Lexer and Parser

## 1. Overview

In Coding Assignment 1, you will implement the first two stages of the TinyLang compiler:

```text
TinyLang source code
        |
        v
+------------------+
|      Lexer       |
+------------------+
        |
        | Token stream
        v
+------------------+
|      Parser      |
+------------------+
        |
        | Abstract Syntax Tree (AST)
        v
       AST
```

Your job is to take a TinyLang source file such as:

```text
fn main(): int {
    let x: int = 10;
    let y: int = 20;

    if (x < y) {
        x = x + 1;
    }

    return x;
}
```

and convert it first into **tokens**, then into an **abstract syntax tree (AST)**.
The AST classes are already provided. You are responsible for recognizing the source-language structure and constructing the correct AST nodes.

This assignment focuses on:
- lexical analysis
- tokens
- syntax analysis
- recursive-descent parsing
- operator precedence
- AST construction
- lexical and syntax errors

You are **not** responsible for type checking in Assignment 1.

For example:

```text
let x: int = true;
```

is syntactically valid TinyLang and should parse successfully in Assignment 1. It will later be rejected by the semantic analyzer in Assignment 2.

---

# 2. Files for Assignment 1

Read these files:

```text
include/Token.h
include/Lexer.h
include/AST.h
include/Parser.h
src/AST.cpp
src/main.cpp
```

Primarily edit:

```text
src/Lexer.cpp
src/Parser.cpp
```

Search for:

```cpp
TODO(A1)
```
And you need to fill in those missing TODOs. 
The supplied headers define the interfaces your implementation must follow.

---

# 3. Background: characters, tokens, and syntax

Consider:

```text
let x: int = 10 + 20;
```

Initially, the compiler sees a sequence of characters:

```text
l e t   x :   i n t   =   1 0   +   2 0 ;
```

The lexer groups characters into meaningful units called **tokens**:

```text
"let" -> KwLet (Keyword Let)
"x"   -> Identifier
":"   -> Colon
"int" -> KwInt (Keyword Int)
"="   -> Assign
"10"  -> Integer
"+"   -> Plus
"20"  -> Integer
";"   -> Semicolon
```

The parser then consumes the token stream according to the TinyLang grammar.

The lexer answers:

```text
What token is this character sequence?
```

The parser answers:

```text
Do these tokens form a legal TinyLang program?
```

---

# 4. Part 1: Lexical analysis

The lexer is represented by:
`class Lexer`
Its implementation is in:
`src/Lexer.cpp`
The goal of lexical analysis is to translate 
characters to tokens.

---

## 4.1 Token categories

`include/Token.h` already defines all token kinds required by the assignment, including: 
general tokens, keywords, punctuation, and operators.

A keyword is not returned as an ordinary identifier.
For example:
`let`
must become:
`TokenKind::KwLet`,
while:
`letter`
becomes:
`TokenKind::Identifier`.

---

## 4.2 Identifier rules

Use the simplified identifier rule (regex):

```text
[A-Za-z_][A-Za-z0-9_]*
```

Valid examples are:

```text
x
count
my_value
_x
x2
value123
```

Your starter contains function `Token Lexer::lexIdentifierOrKeyword()`.
Complete this function so it:

1. remembers the starting line and column
2. consumes the entire identifier
3. compares the final text against the keyword list
4. returns the correct keyword token or `Identifier`

Do not return one token per character.

---

## 4.3 Integer literals

TinyLang supports decimal integer literals:

```text
0
1
10
12345
```

Complete function `Token Lexer::lexInteger()`.
For number `12345`,
produce one token with lexeme:
`12345`,
not five tokens.

Negative values do not need to be a special lexical token. `-10` may be tokenized as:

```text
Minus
Integer("10")
```

---

## 4.4 Whitespace and comments

Whitespace separates tokens but does not become a token.
The starter also supports single-line comments:

```text
// this is a comment
```

Everything after `//` through the newline should be ignored.
For example:

```text
let x: int = 10; // initial value
return x;
```

should tokenize as if the comment were absent.

---

## 4.5 Source locations

Each token records:
```cpp
int line;
int column;
```

These locations support useful error messages such as:

```text
Syntax error at line 4, column 18: expected expression
```

The starter's `advance()` function already updates line and column state. Preserve this behavior carefully.

---

## 4.6 Completing `tokenize()`

Complete the missing punctuation/operator logic in:
`std::vector<Token> Lexer::tokenize()`. 
Be careful with multi-character operators.
For example:
`=` vs. `==`
must be distinguished.
Likewise: `&&` and `||` must each be recognized as one token.
A single `&` or `|` is not a valid TinyLang operator.

At the end of the file, the lexer appends:
```cpp
TokenKind::End
```
The parser relies on this marker.

---

## 4.7 Testing the lexer

Use the command:

```bash
./tinylang program.tl --tokens
```

A good first input is:

```text
fn main(): int {
    let x: int = 10 + 20;
    return x;
}
```

Verify that:
- keywords are recognized correctly
- `main` and `x` are identifiers
- integer literals are complete
- punctuation is correct
- operators are correct
- line/column positions are reasonable
- an `End` token appears exactly once at the end

Do not begin debugging the parser until basic tokenization works.

---

# 5. Part 2: Syntax analysis

The parser is implemented in:
`src/Parser.cpp`.
The goal is to translate tokens to AST.
The parser is a **recursive-descent parser**. Each major grammar rule corresponds to a parsing function.
For example:
`block`
corresponds to 
`parseBlock()`, 
and
`if_stmt`
corresponds to
`parseIf()`.

---

## 5.1 TinyLang grammar

Use the following grammar:

```text
program
    -> main_function EOF

main_function
    -> "fn" "main" "(" ")" ":" "int" block

block
    -> "{" statement* "}"

statement
    -> var_decl
     | assignment
     | if_stmt
     | return_stmt

var_decl
    -> "let" Identifier ":" type ("=" expression)? ";"

assignment
    -> Identifier "=" expression ";"

if_stmt
    -> "if" "(" expression ")" block ("else" block)?

return_stmt
    -> "return" expression ";"

type
    -> "int"
     | "bool"

expression
    -> or

or
    -> and ("||" and)*

and
    -> equality ("&&" equality)*

equality
    -> comparison ("==" comparison)*

comparison
    -> term (("<" | ">") term)*

term
    -> factor (("+" | "-") factor)*

factor
    -> primary (("*" | "/") primary)*

primary
    -> Integer
     | "true"
     | "false"
     | Identifier
     | "(" expression ")"
```

`EOF` (end-of-file) corresponds to `TokenKind::End`.

---

## 5.2 Why expression parsing has multiple levels

Consider:

```text
1 + 2 * 3
```

The correct interpretation is:

```text
1 + (2 * 3)
```

not:

```text
(1 + 2) * 3
```

The grammar encodes operator precedence by using multiple **recursive-descent** levels.

From lowest to highest precedence:

| Parser level | Operators |
|---|---|
| `parseOr()` | `\|\|` |
| `parseAnd()` | `&&` |
| `parseEquality()` | `==` |
| `parseComparison()` | `<`, `>` |
| `parseTerm()` | `+`, `-` |
| `parseFactor()` | `*`, `/` |
| `parsePrimary()` | literals, identifiers, parentheses |

This organization is intentional.

---

## 5.3 AST background

An AST represents program structure without preserving every punctuation token.
For example:

```text
x + y * 2
```

the AST should conceptually be:

```text
Binary(+)
  Variable(x)
  Binary(*)
    Variable(y)
    Integer(2)
```

The AST classes are already defined in:
`include/AST.h`.
Important expression nodes are:
```cpp
IntegerExpr
BoolExpr
VariableExpr
BinaryExpr
```

Important statement nodes are:
```cpp
VarDeclStmt
AssignStmt
ReturnStmt
IfStmt
BlockStmt
```

Top-level structures are:
```cpp
Function
Program
```

Read `include/AST.h` before implementing parser functions.

---

## 5.4 Smart pointers and ownership

You will need them when implementing your compiler.
Expressions use:
`using ExprPtr = std::unique_ptr<Expr>;`. 
Statements use:
`using StmtPtr = std::unique_ptr<Stmt>;`.
Therefore you will frequently use:
`std::make_unique<...>()` 
and
`std::move(...)`. 
For example:

```cpp
left = std::make_unique<BinaryExpr>(
    "+",
    std::move(left),
    std::move(right)
);
```

`std::unique_ptr` cannot normally be copied because each AST node has one owner.

---

## 5.5 Parser helper functions

The starter provides helpers that simplify recursive-descent parsing.

- `peek()`: Inspect the current token without consuming it: `peek()`. 
Look ahead: `peek(1)`.

- `check(kind)`: Check the current token kind: `check(TokenKind::KwLet)`

- `match(kind)`: If the current token matches: 1. consume it, 2. return `true`. 
Otherwise return `false`.
Useful for optional grammar parts:

```cpp
if (match(TokenKind::KwElse)) {
    ...
}
```

- `consume(kind, message)`: Require a particular token. For example:
```cpp
consume(TokenKind::Semicolon,
        "expected ';' after declaration");
```
If the token is missing, the parser reports a syntax error.

---

## 5.6 Functions you must implement

- `parseMainFunction()`: Parse exactly the main function (shown below). 
TinyLang contains one function, named `main`, with no parameters and return type `int`.
Do not implement arbitrary function declarations.

```text
fn main(): int {
    ...
}
```

- `parseBlock()`: Parse code blocks below. Continue until `}` is reached.

```text
{
    statement
    statement
    ...
}
```

- `parseStatement()`: Dispatch based on the current token (as shown below). 
Anything else in statement position is a syntax error.

```text
let         -> variable declaration
if          -> if statement
return      -> return statement
Identifier  -> assignment
```

- `parseVarDecl()`: Parse `let x: int;` or `let x: int = 10;`.
The initializer is optional.

- `parseAssignment()`: Parse `x = expression;`. 

- `parseIf()`: Parse if block:

```text
if (expression) {
    ...
}
```

with optional else block:

```text
else {
    ...
}
```

Do not type-check the condition here.

- `parseReturn()`: Parse `return expression;`.
Do not check the return type here.

- `parseType()`: Recognize:

```text
int
bool
```

and return:

```cpp
Type::Int
Type::Bool
```

---

## 5.7 Expression parser functions

Implement:

```cpp
parseOr()
parseAnd()
parseEquality()
parseComparison()
parseTerm()
parseFactor()
parsePrimary()
```

A common pattern for a binary-precedence level is:

```text
left = parse next-higher-precedence level

while current token is an operator at this level:
    consume operator
    right = parse next-higher-precedence level
    left = BinaryExpr(operator, left, right)

return left
```

For example, `parseTerm()` handles `+` and `-` but obtains each operand by calling `parseFactor()`.

This gives `*` and `/` higher precedence.

---

## 5.8 Implementing `parsePrimary()`

`parsePrimary()` handles the smallest expression units: 
- Integer `123` creates `IntegerExpr`
- Boolean `true` and `false` create `BoolExpr`
- Variable `x` creates `VariableExpr`

### Parenthesized expression

For example, the procedure to parse the expression `(x + 1)` is:

1. consume `(`
2. call `parseExpression()`
3. require `)`
4. return the contained expression

The parentheses affect parsing but do not need their own AST node.

---

## 5.9 Syntax-error examples

Your parser should reject malformed input such as:

`let x: int = ;`

`let x int = 10;`

```text
if (x < 10 {
    x = 1;
}
```

```text
return x
```

Use specific messages when calling `consume()` to provide more detailed information.
For example, use:

```text
expected ';' after return expression
```

rather than:

```text
bad syntax
```

---

# 6. What Assignment 1 must NOT check

Do not implement semantic rules in the parser.
This is syntactically valid, even though `x` is undeclared:

```text
fn main(): int {
    x = 10;
    return 0;
}
```

This is also syntactically valid, even though the declared and initializer types disagree:

```text
fn main(): int {
    let x: int = true;
    return x;
}
```

Assignment 2 is responsible for those errors.
A useful distinction is:

```text
Parser:
    Does the program have the correct grammatical shape?

Semantic analyzer:
    Does the grammatically valid program make sense?
```

---

# 7. Important precedence tests

You can try your code with the following test cases: 

## Test 1

```text
return 1 + 2 * 3;
```

Expected AST shape:

```text
+
  1
  *
    2
    3
```

## Test 2

```text
return (1 + 2) * 3;
```

Expected:

```text
*
  +
    1
    2
  3
```

## Test 3

```text
if (x + 1 < y * 2) {
    return x;
}
```

Expected condition:

```text
<
  +
    x
    1
  *
    y
    2
```

## Test 4

```text
a == b || c == d && true
```

should group as:

```text
(a == b) || ((c == d) && true)
```

---

# 8. Suggested implementation order

1. Implement `lexIdentifierOrKeyword()`.
2. Implement `lexInteger()`.
3. Complete punctuation/operator handling in `tokenize()`.
4. Test thoroughly with `--tokens`.
5. Implement `parseMainFunction()`.
6. Implement `parseBlock()`.
7. Implement simple statement parsing.
8. Implement `parseType()`.
9. Implement `parsePrimary()`.
10. Implement `parseFactor()` and `parseTerm()`.
11. Add comparison/equality parsing.
12. Add `&&` and `||`.
13. Implement `parseIf()`.
14. Test thoroughly with `--ast`.
15. Test malformed inputs and error messages.

Do not try to implement everything before compiling.

---

# 9. Files you normally should not modify

The intended Assignment 1 implementation files are:
```text
src/Lexer.cpp
src/Parser.cpp
```

Supporting files include:

```text
include/Token.h
include/Lexer.h
include/Parser.h
include/AST.h
src/AST.cpp
src/main.cpp
```

Read them, but avoid changing their public interfaces unless explicitly instructed.
You do not need to complete:

```text
src/Semantic.cpp
src/CodeGen.cpp
```

for Assignment 1.

---

# 10. Completion checklist

Before finishing Assignment 1, verify that your compiler can:
- [ ] recognize all TinyLang keywords
- [ ] recognize identifiers
- [ ] recognize multi-digit integers
- [ ] recognize punctuation
- [ ] recognize single-character operators
- [ ] distinguish `=` from `==`
- [ ] recognize `&&` and `||`
- [ ] skip whitespace
- [ ] skip `//` comments
- [ ] report invalid characters
- [ ] parse exactly one `main` function
- [ ] parse blocks (i.e., { ... })
- [ ] parse variable declarations
- [ ] parse assignments
- [ ] parse `if/else`
- [ ] parse returns
- [ ] parse `int` and `bool`
- [ ] parse literals
- [ ] parse variable expressions
- [ ] parse parentheses
- [ ] preserve operator precedence
- [ ] construct the expected AST
- [ ] reject malformed syntax with useful messages

Most importantly, be able to explain:

```text
characters -> tokens -> AST
```

and the role of the lexer and parser in that transformation.
