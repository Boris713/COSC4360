// bool variables with and or and equals
fn main(): int {
    let yes: bool = 3 > 2;
    let no: bool = false;
    let r: int = 0;

    if (yes && no) { r = r + 1; }
    if (yes || no) { r = r + 10; }
    if (no == false) { r = r + 100; }

    return r;
}
