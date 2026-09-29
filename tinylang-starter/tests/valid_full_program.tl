// a bigger program that uses pretty much everything
fn main(): int {
    let x: int;              // no starting value
    let big: bool = false;
    x = 5;

    if (x > 3 && big == false) {
        // nested if with an else
        if (x < 10 || big) {
            x = ((x + 1)) * 2;   // extra parens should be fine
        } else {
            x = x / 2;
        }
    } else {
        big = true;
    }

    // empty blocks are allowed
    if (big) {}

    return x - 1;
} // no newline after this comment on purpose