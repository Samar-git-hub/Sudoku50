#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class variables {
private:
    int value; 
    vector<int> domain;
    int row;
    int column;

public:
    // Default constructor to assign initial domain values
    variables() : value(0), row(0), column(0) {
        for (int i = 1; i <= 9; i++) {
            domain.push_back(i);
        }
    }

    variables(int value, int row, int column) {
        this->row = row;
        this->column = column;
        setValue(value);
    }

    // Accessor methods for private members
    int getValue() const {
        return value;
    }

    vector<int> getDomain() const {
        return domain;
    }

    int getRow() const {
        return row;
    }

    int getColumn() const {
        return column;
    }

    // Set value and update domain
    void setValue(int value) {
        this->value = value;
        domain.clear();  
        if (value == 0) {
            for (int i = 1; i <= 9; i++) {
                domain.push_back(i);
            }
        } else {
            domain.push_back(value);
        }
    }
}; 

class constraints {
private:
    // Check unary constraint (value between 0 and 9)
    bool satisfyunary(int value) {
        return (value >= 0 && value <= 9);
    }

    // Check grid constraints (row, column, and 3x3 block)
    bool satisfygrid(int value, int row, int col, int grid[9][9]) {
        // Row check: value must not appear elsewhere in the row
        for (int i = 0; i < 9; i++) {
            if (grid[row][i] == value && i != col) {
                return false;
            }
        }

        // Column check: value must not appear elsewhere in the column
        for (int j = 0; j < 9; j++) {
            if (grid[j][col] == value && j != row) {
                return false;
            }
        }

        // 3x3 block check: value must not appear elsewhere in the subgrid
        int startr = row - row % 3;
        int startc = col - col % 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (grid[startr + i][startc + j] == value && 
                    (startr + i != row || startc + j != col)) {
                    return false;
                }
            }
        }
        return true;
    }

public:
    // Public validation method used by sudokusolver
    bool isValid(int value, int row, int col, int grid[9][9]) {
        return satisfyunary(value) && satisfygrid(value, row, col, grid);
    }
}; 

class sudokusolver {
private:
    variables sudokuGrid[9][9]; 

public:
    // Initialize solver grid
    sudokusolver(int grid[9][9]) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                sudokuGrid[i][j] = variables(grid[i][j], i, j);
            }
        }
    }

    // Check if grid has no empty cells remaining
    bool isfinalstate(int grid[9][9]) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0) {
                    return false;
                }
            }
        }
        return true;
    }

    // Backtracking search algorithm
    bool backtrack(int grid[9][9]) {
        if (isfinalstate(grid)) {
            return true;
        }
        
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (grid[i][j] == 0) { // Check for empty cell
                    constraints c;
                    for (int val = 1; val <= 9; val++) { // Try possible values 1 to 9
                        if (c.isValid(val, i, j, grid)) { // Check if value satisfies constraints
                            grid[i][j] = val;
                            if (backtrack(grid)) { // Recursively check remaining cells
                                return true;
                            }
                            grid[i][j] = 0; // Reset if value leads to dead end
                        }
                    }
                    return false; // Backtrack if no value works
                }
            }
        }
        return true;
    }

    // Validate that initial non-empty numbers do not conflict
    bool isInitialValid() {
        constraints c;
        int tempGrid[9][9];
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                tempGrid[i][j] = sudokuGrid[i][j].getValue();
            }
        }
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                int val = tempGrid[i][j];
                if (val < 0 || val > 9) return false;
                if (val != 0 && !c.isValid(val, i, j, tempGrid)) {
                    return false;
                }
            }
        }
        return true;
    }

    // Solve puzzle with initial validation and backtracking
    bool solve() {
        if (!isInitialValid()) {
            return false;
        }
        int tempGrid[9][9];
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                tempGrid[i][j] = sudokuGrid[i][j].getValue();
            }
        }
        if (backtrack(tempGrid)) {
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    sudokuGrid[i][j].setValue(tempGrid[i][j]);
                }
            }
            return true;
        }    
        return false;
    }

    int getCell(int r, int c) const {
        return sudokuGrid[r][c].getValue();
    }

    void getGrid(int outGrid[9][9]) const {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                outGrid[i][j] = sudokuGrid[i][j].getValue();
            }
        }
    }

    // Print grid formatted to stdout
    void printGrid() {
        for (int i = 0; i < 9; i++) {
            if (i % 3 == 0) { 
                cout << "+-------+-------+-------+" << endl;
            }
            for (int j = 0; j < 9; j++) {
                if (j % 3 == 0) {
                    cout << "| "; 
                }
                cout << sudokuGrid[i][j].getValue() << " ";
            }
            cout << "|" << endl; 
        }
        cout << "+-------+-------+-------+" << endl;
    }      
}; 

#endif
