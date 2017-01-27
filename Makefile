.PHONY: all

all: sudoku

sudoku: sudoku.cpp
	$(CXX) -o sudoku sudoku.cpp

clean:
	$(RM) sudoku
