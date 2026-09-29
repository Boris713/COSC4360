// tinylang has no else if so else has to be followed by a block
fn main(): int {
    let x: int = 1;
    if (x < 5) {
        x = 2;
    } else if (x < 10) {
        x = 3;
    }
    return x;
}
