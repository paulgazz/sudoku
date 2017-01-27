# (Unfortunately Another) Sudoku Solver

I saw a sudoku puzzle in magazine, had heard about for a while, and
decided to see what it was all about. After reading the rules, I
realized that this is something that a computer would be very good at
doing, so I wrote this solver for fun.

## Usage

    make
    cat tests/puz1.sud | ./sudoku

## Input

- Each .sud file is a listing of a sudoku puzzle.
- Each line has one number starting from the top left of the puzzle,
  going to to bottom right.
- When the end of a row is reached, just start again at the left side
  of the next row.
- Zeros indicate blank spaces to be filled.

## Tests

- `tests/puz1.sud` from time magazine Jan. 16, 2006
- `tests/puz2.sud` from sudoku.com Jan. 13, 2006
- `tests/two_sols.sud` has two solutions
- `tests/broken#.sud` are impossible
- `tests/no_sol.sud` is also impossible
- `tests/null.sud` will generate all possible sudoku boards

## Description

There are two phases of the algorithm: the reduction phase and the
solution phase. The second phase, the solution phase, employs a brute
force approach to solving the puzzle by checking every possible number
in each square, within the given sudoku rules, and printing the result
if it can reach all the squares without breaking the sudoku rules.
This step alone can be used to solve a sudoku puzzle. All it needs is
a structure to keep track of which numbers are currently possible at
each square given the current state of the partially solved
puzzle. Unfortunately, this method has trouble with unsolvable puzzles
and it can hang up.

The first stage not only overcomes these limitations but also helps
find the solution faster by greatly limiting the number of
possibilities to check for when performing the brute force method. The
reduction is done by creating a matrix of lists of length 9. The lists
represent the possible numbers that could be placed in that
square. The algorithm checks the number at each square and if there is
a number there, that number is removed from the lists of numbers at
each square in the row, column, and 3 x 3 submatrix. When there is
only one number left in a square's list, that number is the solution
for that square. After this reduction phase, the brute force method is
applied.

##  More

For a more sophisticated solver check out
<del>[this site](http://www.sudokusolver.co.uk/)</del> They have a
sudoku solver that uses no brute force at all, only logic. It is very
cool
