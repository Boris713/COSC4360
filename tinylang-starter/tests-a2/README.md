# Tests

these are the tests I used for my semantic analyzer and code generator. Like last time I kept it to fewer tests that each check something that matters

## How to run

From the `tinylang-starter` folder:

```
./build/tinylang tests/codegen_math.tl --check
```

Use `--check` for all of them. For the codegen ones you can also run the program and see what main returns:

```
./build/tinylang tests/codegen_math.tl --emit-llvm > out.ll
clang out.ll -o out
./out; echo $?
```

## Should work

- `codegen_math.tl`: `+ - * /` with variables. Math goes left to right and `-7 / 2` is `-3`. Should return 6.
- `codegen_if_else.tl`: an if with no else, then an if/else where both sides return. Should return 41.
- `codegen_bools.tl`: bool variables with `<`, `&&`, `||` and `==`. Should return 110.
- `codegen_scopes.tl`: a new `x` inside a block doesn't change the outer `x`. Should return 1.
- `codegen_return_early.tl`: the line after a return gets skipped. Should return 40.
- `codegen_no_return.tl`: main gets to the end with no return so it returns 0 on its own. Should return 0.
- `codegen_no_start_value.tl`: an int with no starting value is 0 and a bool is false. Should return 5.

## Should fail

- `sem_error_out_of_scope.tl`: semantic error. Uses `y` after its block is over. Should only show 1 error.
- `sem_error_assign_undeclared.tl`: semantic error. Assigns to `y` but it was never declared.
- `sem_error_duplicate.tl`: semantic error. `x` is declared twice in the same block.
- `sem_error_init_type.tl`: semantic error. An int starts as a bool.
- `sem_error_assign_type.tl`: semantic error. Puts a bool into an int variable.
- `sem_error_operator_types.tl`: semantic error. `10 == true` compares an int to a bool.
- `sem_error_if_condition.tl`: semantic error. Uses an int as an if condition.
- `sem_error_return_type.tl`: semantic error. Returns a bool from main.
- `sem_error_self_init.tl`: semantic error. Uses `x` to set up `x` before it exists. Should only show 1 error.
