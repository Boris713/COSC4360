// the x in the block is a new x so the outer x stays 1
fn main(): int {
    let x: int = 1;

    if (true) {
        let x: int = 10;
        x = x + 5;
    }

    return x;
}
