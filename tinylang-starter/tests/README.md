# Tests

these are the tests I used for my lexer and parser. I tried to keep it to fewer tests that each check something that matters instead of a ton of tiny ones

## How to run

From the `tinylang-starter` folder:

```
./build/tinylang tests/valid_order_of_ops.tl --ast
```

Use `--ast` for all of them except `tokens_all.tl`, which uses `--tokens` since it's not a real program.

## Should work

- `valid_order_of_ops.tl`: `*` and `/` go before `+` and `-`, parens go first, math goes before `<`, and `&&` goes before `||`. These are the 4 examples from the assignment.
- `valid_left_to_right.tl`: things like `10 - 3 - 2` should group from the left, so `(10 - 3) - 2`.
- `valid_full_program.tl`: a bigger program with nested if/else, an empty block, a variable with no starting value, extra parens, and comments all over. The file also ends on a comment with no newline after it.
- `valid_wrong_types_still_parse.tl`: has type mistakes and a variable that was never declared, but the syntax is fine, so the parser should still accept it. Catching that stuff is for assignment 2.

## Lexer check

- `tokens_all.tl`: every keyword, symbol and operator. Words like `letter` and `iffy` should come out as identifiers, not keywords. `===` should come out as `==` then `=`.

## Should fail

- `lex_error_single_amp.tl`: lexical error. A single `&` isn't an operator, only `&&` is.
- `lex_error_bad_number.tl`: lexical error. `123abc` isn't a number and names can't start with a digit.
- `syntax_error_missing_semicolon.tl`: syntax error. No `;` after return.
- `syntax_error_missing_paren.tl`: syntax error. The if is missing its `)`.
- `syntax_error_unclosed_block.tl`: syntax error. A `{` never gets closed, so the parser hits the end of the file.
- `syntax_error_else_if.tl`: syntax error. TinyLang doesn't have else if.
- `syntax_error_negative_number.tl`: syntax error. The grammar has no minus sign in front of a number, so `-5` isn't allowed (`0 - 5` works though).
- `syntax_error_number_too_big.tl`: error. The number is too big to fit in an int.
- `syntax_error_two_functions.tl`: syntax error. Only one main function is allowed.
