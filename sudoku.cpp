/***********************************************************************
Sudoku Solver 2.0
solves sudoku puzzles.  As a side effect, it can check
whether a puzzle is valid and can generate a very boring list of
sudoku puzzles.
Copyright (C) 2006 Paul Gazzillo

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

You may email me at gazzillo 'at' cs.nyu.edu
***********************************************************************/

#include <iostream>
#include <time.h>

using namespace std;

int numsols = 0, calls = 0;	//number of solutions
clock_t start, endtime;			//time of each
double elapsed;				//solution

void printsudoku(int *matrix) {
	//prints a 9 X 9 sudoku matrix
	int i, j;

	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			cout << matrix[i * 9 + j];
			cout << ((((j+1) % 3) == 0 && j < 8) ? " | " : " ");
		}
		cout << ((((i+1) % 3) == 0 && i < 8) ? "\n------+-------+------\n" : "\n");
	}
}

void solve(int *current, int *usedrow, int *usedcol, int *usedmat, int col, int row, int *usedplaces) {
	int num, rowidx, colidx, i;
	int *newcurrent, *newusedrow, *newusedcol, *newusedmat;

	if (col == 9) {
		//at end of column, go to next row
		col = 0;
		row++;
	}
	if (row == 9) {
		//we have a solution
		endtime = clock();
		numsols++;
		elapsed = ( (double) (endtime - start) ) / CLOCKS_PER_SEC;	//calc time
		cout << "solution " << numsols << endl;					//and print
		printsudoku(current);									//solution
		cout << "solution took " << elapsed;					//data
		if (numsols > 1) cout << " more seconds" << endl;
		else cout << " seconds" << endl;
		cout << endl;
		start = clock();
		return;													//end this call tree
	}

	//optimization opportunity - make a matrix of multiples of 9
	rowidx = row * 9;
	colidx = col * 9;
	//cout << "(" << row << ", " << col << ")" << endl;
	//if this current (row,col) does not already have a number in it
	if ( !usedplaces[rowidx + col] ) {
		//then for each number 1..9
		for (num = 0; num < 9; num++) {
			//that isn't used yet in this
			//row, col, or submatrix
			if ( !(usedrow[rowidx + num] || usedcol[colidx + num] || usedmat[(row / 3 + (col / 3) * 3) * 9 + num]) ) {
				//create a new partially solved matrix
				//and updated matrices that hold info about
				//what numbers have been used already
				//copy the info to new arrays
				newcurrent = new int[81];
				newusedrow = new int[81];
				newusedmat = new int[81];
				newusedcol = new int[81];
				//optimize opportunity - use memcpy
				for (i = 0; i < 81; i++) {
					newcurrent[i] = current[i];
					newusedrow[i] = usedrow[i];
					newusedmat[i] = usedmat[i];
					newusedcol[i] = usedcol[i];
				}
				//make a new matrix for each unused number in the
				//current (row,col) position
				newcurrent[rowidx + col] = num + 1;
				//and set the usage for the number in the current row, col, 
				//and submatrix
				newusedrow[rowidx + num] = 1;
				newusedcol[colidx + num] = 1;
				newusedmat[(row / 3 + (col / 3) * 3) * 9 + num] = 1;
				//then go to the next (row,col) position
				//calls++;
				solve(newcurrent, newusedrow, newusedcol, newusedmat, col+1, row, usedplaces);
				//clean up the matrices
				delete newcurrent;
				delete newusedrow;
				delete newusedmat;
				delete newusedcol;
			}
		}
	}
	else {
		//if (row,col) has a value already, go on to the next place in the matrix
		solve(current, usedrow, usedcol, usedmat, col+1, row, usedplaces);
	}
}

int main(int argc, char** argv) {
	int mat[81];	// mat - 9 X 9
	int elm[729];
	int cnt[81];	//keeps count of how many numbers left in each row, col, submat, in the elm matrix
	int i, j, k, m, n;
	int *usedrow, *usedcol, *usedmat, *usedplaces;
	int curnum;
	int starti, startj;
	int exists;
	int eliminating;
	clock_t redstart, redend, elimstart, elimend, solstart, solend;
	double redelapsed, elimelapsed, solelapsed;
	int tmpi, tmpj;
	int idx;
	
	printf("Sudoku solver 2.0, Copyright (C) 2006 Paul Gazzillo\n");
	printf("\nDIRECTIONS\nEnter the puzzle from left to right and top to bottom, going across each row,\nthen proceeding to the next lower row, entering 0 for empty squares, one line\nper number.\nThere is no input checking, so try to enter only numbers from 0 to 9.\n");

	//input sudoku matrix
	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
			cin >> mat[i*9 + j];

	//print sudoku matrix
	cout << "input puzzle" << endl;
	printsudoku(mat);
	cout << endl;

	redstart = clock();

	//build reduction matrix
	//optimization opportunity: represent elimination matrix in a 9 x 9 x 9 bit matrix
	//via 81 integers (using only 9 bits of each integer)
	for (i = 0; i < 729; i++) {
		elm[i] = 1;
	}
	for (i = 0; i < 81; i++) {
		cnt[i] = 9;
	}

	//strategy:
	//reduction - for each of the 81 spaces, allocate an array of 9, with each
	//element representing 1..9.  then, as each number in the original puzzle
	//is encountered, remove the number from the reduction matrix for each
	//space in the row, column, and submatrix.  for example, if a 1 is encountered
	//in the first position, remove 1 from each of the spaces in the row, column, and
	//submatrix.  for the current space, remove all other numbers except 1.
	//when a space only has one number available, then that space is solved.
	//continue this process until no more numbers can be eliminated from any space
	//permutation - when reduction is complete, permute all possible solutions from
	//remaining numbers, keeping track of the numbers used in each row, col, etc...
	
	eliminating = 1;
	while (eliminating) {
		eliminating = 0;
		for (i = 0; i < 9; i++) {
			for (j = 0; j < 9; j++) {
				curnum = mat[i*9 + j];
				if (curnum != 0) {
					starti = (i / 3) * 3;
					startj = (j / 3) * 3;
					for (k = 0; k < 9; k++) {
						if (k != curnum - 1) {
							idx = 81*i + 9*j + k;
							if (elm[idx]) {
								eliminating = 1;
								elm[idx] = 0;
								cnt[9*i + j]--;
							}
						}
						if (k != j) {
							idx = 81*i + 9*k + curnum - 1;
							if (elm[idx]) {
								eliminating = 1;
								elm[idx] = 0;
								cnt[9*i + k]--;
							}
						}
						if (k != i) {
							idx = 81*k + 9*j + curnum - 1;
							if (elm[idx]) {
								eliminating = 1;
								elm[idx] = 0;
								cnt[9*k + j]--;
							}
						}
						tmpi = (starti + (k/3));
						tmpj = (startj + (k%3));
						if (tmpi != i && tmpj != j) {
							idx = 81*tmpi + 9*tmpj + curnum - 1;
							if (elm[idx]) {
								eliminating = 1;
								elm[idx] = 0;
								cnt[tmpi*9 + tmpj]--;
							}
						}
					}
				}
			}
		}

		for (i = 0; i < 9; i++) {
			for (j = 0; j < 9; j++) {
				/*cout << "{";
				for (k = 0; k < 9; k++) {
					if (elm[81*i + 9*j + k]) {
						cout << (k+1) << ", ";
					}
				}*/
				if (cnt[9*i + j] > 1) {
					curnum = 0;
				}
				else if (cnt[9*i + j] == 1) {
					for (k = 0; k < 9; k++) {
						if (elm[81*i + 9*j + k]) {
							curnum = k + 1;
							break;
						}
					}
				}
				else {
					cout << endl << "no solutions!" << endl;
					return 1;
				}
				mat[i*9 + j] = curnum;
				//cout << "} ";
			}
			//cout << endl;
		}
	
		//printsudoku(mat);
	}

	redend = clock();
	redelapsed = ( (double) (redend - redstart) ) / CLOCKS_PER_SEC;

	cout << "reduced puzzle" << endl;
	printsudoku(mat);
	cout << "reduction time " << redelapsed << " seconds" << endl;
	cout << endl;

	elimstart = clock();

	//build elimination matrices
	//optimization opportunity: build the elim matrices from elm
	usedrow = new int[81];
	usedcol = new int[81];
	usedmat = new int[81];
	usedplaces = new int[81];

	for (i = 0; i < 81; i++) {
		usedrow[i] = 0;
		usedcol[i] = 0;
		usedmat[i] = 0;
		usedplaces[i] = 0;
	}
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			curnum = mat[i*9 + j];
			if (curnum != 0) {
				usedrow[i*9 + curnum - 1] = 1;
				usedcol[j*9 + curnum - 1] = 1;
				usedmat[(i / 3 + (j / 3) * 3) * 9 + curnum - 1] = 1;
				usedplaces[i*9 + j] = 1;
			}
		}
	}

	elimend = clock();
	elimelapsed = ( (double) (elimend - elimstart) ) / CLOCKS_PER_SEC;
	cout << "time to build elimination matrices: " << elimelapsed << " seconds" << endl;
	cout << endl;

	/*
	printsudoku(usedrow);
	cout << endl;
	printsudoku(usedcol);
	cout << endl;
	printsudoku(usedmat);
	cout << endl;
	printsudoku(usedplaces);
	cout << endl;
	*/

	solstart = start = clock();

	solve(mat, usedrow, usedcol, usedmat, 0, 0, usedplaces);

	solend = clock();
	solelapsed = ( (double) (solend - solstart) ) / CLOCKS_PER_SEC;

	if ( numsols == 0 ) cout << "no solutions were found for this puzzle" << endl << endl;
	cout << "time for function call: " << solelapsed << " seconds" << endl;
	cout << "reduction + elimination matrix build + permutation:" << (( (double) ((solend - solstart) + (elimend - elimstart) + (redend - redstart)) ) / CLOCKS_PER_SEC) << endl;
	//cout << calls;
}
