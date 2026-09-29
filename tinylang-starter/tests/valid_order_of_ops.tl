// checks order of operations using the 4 examples from the assignment
fn main(): int {
    // times goes before plus
    let a: int = 1 + 2 * 3;

    // parens go first
    let b: int = (1 + 2) * 3;

    // math happens before less than
    let c: bool = a + 1 < b * 2;

    // and goes before or
    let d: bool = a == b || c == false && true;

    return a;
}
