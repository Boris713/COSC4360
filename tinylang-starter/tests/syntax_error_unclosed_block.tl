// the if block never gets closed so the parser runs into the end of the file
fn main(): int {
    let x: int = 1;
    if (x < 10) {
        x = 2;
    return x;
}
