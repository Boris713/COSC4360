// an if with no else and an if else where both sides return
fn main(): int {
    let x: int = 5;
    let r: int = 0;

    if (x > 3) {
        r = 40;
    }

    if (r == 40) {
        return r + 1;
    } else {
        return 0;
    }
}
