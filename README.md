# HashiSolver

HashiSolver is a C++ program for solving a Hashiwokakero (Hashi) puzzle.

## Installation

Copy the files HashiSolver.cpp, dirent.h, hashi_solver.ini and the Puzzles catalogue to a folder on your hard disk.
Compile as a console project using Visual Studio if under Windows. Or compile with g++ if under Linux.
The file dirent.h is only needed if under Windows.

## Description
https://en.wikipedia.org/wiki/Hashiwokakero describes what a Hashi puzzle is.

The program analyzes puzzle files stored in the directory Puzzles.
Either a single file is analyzed or all 122 of them.

The analysis first tries to solve a puzzle by pure logical deduction.
If that does not suffice the program continues with brute force guessing using backtracking.

## License

[MIT](https://choosealicense.com/licenses/mit/)