# TAMUCC COSC 4360 - TinyLang Compiler Assignments

**Note**: This instruction is based on Linux. Consider installing a development environment like Visual Studio Code (VS Code) with WSL (a tutorial can be found [here](https://learn.microsoft.com/en-us/windows/wsl/install)) if you plan to write and build code using LLVM on Windows. 
[OnlineGDB](https://www.onlinegdb.com) *might* work without install anything. 

TinyLang is a deliberately small programming language used in **COSC 4360** to study the major stages of a compiler without requiring students to build a production-sized compiler.

Across two coding assignments, you will gradually build a compiler front end and a small LLVM-based back end:

```text
TinyLang source code
        |
        v
+------------------+
|      Lexer       |   Coding Assignment 1
+------------------+
        |
        | tokens
        v
+------------------+
|      Parser      |   Coding Assignment 1
+------------------+
        |
        | AST
        v
+------------------+
| Semantic Analyzer|   Coding Assignment 2
+------------------+
        |
        | semantically valid AST
        v
+------------------+
| LLVM IR CodeGen  |   Coding Assignment 2
+------------------+
        |
        | LLVM IR
        v
     executable
```

The project is intentionally small. TinyLang is **NOT** intended to be a complete programming language. Its purpose is to make the compiler pipeline visible and understandable. You can also found other programming languages or tools called "TinyLang", which however have nothing to do with this course. 

---

## 1. What you will learn

By completing the two assignments, you will practice:

- lexical analysis
- tokens and token streams
- recursive-descent parsing
- operator precedence
- abstract syntax trees (ASTs)
- lexical and syntax errors
- static semantics
- symbol tables
- lexical scope
- declaration and name checking
- type checking
- LLVM intermediate representation (LLVM IR)
- basic code generation
- `alloca`, `load`, `store`, arithmetic instructions, comparisons, and `ret`

The two assignments correspond to two major questions a compiler must answer.

### Coding Assignment 1: Is the program syntactically valid?

For example:

```text
let x: int = 10;
```

has valid TinyLang syntax, while:

```text
let x: int = ;
```

does not.

### Coding Assignment 2: Does the syntactically valid program make sense?

For example:

```text
let x: int = 10;
x = 20;
```

is semantically valid, while:

```text
let x: int = 10;
x = true;
```

is syntactically valid but semantically invalid because an `int` variable is assigned a `bool` value.

---

# 2. TinyLang at a glance

A TinyLang program has file extension `.tl` and contains exactly **one** function (i.e., `main` function):

```text
fn main(): int {
    ...
}
```

A complete example is:

```text
fn main(): int {
    let x: int = 10;
    let y: int = 20;

    if (x < y) {
        x = x + 1;
    } else {
        x = x - 1;
    }

    return x;
}
```

TinyLang intentionally supports only a small subset of typical programming-language features, as shown below:

## Types

```text
int
bool
```

## Variable declarations

```text
let x: int;
let y: int = 10;
let done: bool = false;
```

## Assignment

```text
x = 20;
x = x + 1;
done = true;
```

## Conditional statements

```text
if (x < 10) {
    x = x + 1;
}
```

or:

```text
if (x < 10) {
    x = x + 1;
} else {
    x = x - 1;
}
```

## Return statement

```text
return x;
```

## Expressions

TinyLang supports the following expressions:

```text
10
true
false
x

x + 1
x - 1
x * 2
x / 2

x < y
x > y
x == y

a && b
a || b

(x + 1) * 2
```

TinyLang does **not** include classes, arrays, pointers, loops, strings, floating-point values, arbitrary functions, or function parameters. For the full TinyLang grammar, please read [TinyLang grammar](). 

---

# 3. starter structure

You can download the `tinylang-starter.zip` from this repo to start your assignments. The project uses one shared codebase (i.e., the zip file) for both assignments. You do **not** create a second compiler for Assignment 2. Assignment 2 continues from the AST produced in Assignment 1. 
The zip file structure is shown below: 

```text
tinylang-starter/
|
|-- README.md
|-- CMakeLists.txt
|-- ASSIGNMENT_SPLIT.md
|
|-- include/
|   |-- Token.h
|   |-- Lexer.h
|   |-- AST.h
|   |-- Parser.h
|   |-- Semantic.h
|   `-- CodeGen.h
|
|-- src/
|   |-- main.cpp
|   |-- Lexer.cpp
|   |-- Parser.cpp
|   |-- AST.cpp
|   |-- Semantic.cpp
|   `-- CodeGen.cpp
|
`-- examples/
    |-- valid.tl
    |-- syntax_error.tl
    |-- semantic_error.tl
    `-- codegen.tl
```



# 4. Assignment instructions

Read the detailed assignment document before coding:

- [Coding Assignment 1: TinyLang Lexer and Parser](https://github.com/apace-lab/COSC4360/blob/main/coding-1/CODING1.md)
- [Coding Assignment 2: TinyLang Semantic Analysis and LLVM IR](https://github.com/apace-lab/COSC4360/blob/main/coding-2/CODING2.md )

Each document explains the relevant compiler background, TinyLang rules, required functions, examples, testing strategy, common mistakes, and completion checklist.


# 5. Why LLVM? 
[Why We Use LLVM](https://github.com/apace-lab/COSC4360/blob/main/LLVM.md). 
[My First Language Frontend with LLVM Tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html#my-first-language-frontend-with-llvmtutorial).
