// checks that operators on the same level group from the left
fn main(): int {
    // should do 10 minus 3 first then minus 2 which gives 5 not 9
    let a: int = 10 - 3 - 2;

    // should do 8 divided by 4 first then divided by 2 which gives 1 not 4
    let b: int = 8 / 4 / 2;

    // the less than happens first then the double equals
    let c: bool = 1 < 2 == true;

    return a + b;
}
