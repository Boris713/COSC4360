// code after a return never runs
fn main(): int {
    let x: int = 4;

    if (x == 4) {
        return 40;
        x = 5;
    }

    return 50;
}
