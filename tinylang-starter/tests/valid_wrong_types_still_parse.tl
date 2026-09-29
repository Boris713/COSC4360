// this has type mistakes but the syntax is fine so the parser should accept it
// catching these is for assignment 2
fn main(): int {
    let x: int = true;       // bool going into an int
    y = 10;                  // y was never declared
    let z: bool = 5 + true;  // adding a number and a bool
    return z;
}
