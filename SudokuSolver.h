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
    variables() : value(0), row(0), column(0) { // default constructor to assign default values to variables object
        for (int i = 1; i <= 9; i++) {
            domain.push_back(i);
        }
    }
    variables(int value, int row, int column){
        this->row = row;
        this->column = column;
        setValue(value);
    }
    // the below methods for controlled access outside the class,
    // as the variables themselves are private above
    int getValue(){
        return value;
    }
    vector<int> getDomain(){
        return domain;
    }
    int getRow(){
        return row;
    }
    int getColumn(){
        return column;
    }

    // set value
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

    // if the value lies between 0 and 9 (it is valid, hence returns true)
    bool satisfyunary(int value){
       return (value >= 0 && value <= 9);
    }

    bool satisfygrid(int value, int row, int col, int grid[9][9]){
        
        // checking row: 
        for(int i = 0; i < 9; i++){ // basically doesnt satusfy if the row 
        // has the same value in another position that is not the column
            if (grid[row][i] == value && i != col){
                return false;
            }
        }

        // checking columns
        for(int j = 0; j < 9; j++){ // basically doesnt satusfy if the column
        // has the same value in another position that is not the row
            if (grid[j][col] == value && j != row){
                return false;
            }
        }

        // checking 3x3 grid
        int startr = row - row % 3;
        int startc = col - col % 3;
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                if (grid[startr + i][startc + j] == value && 
                    (startr + i != row || startc + j != col)){
                    return false;
                }
            }
        }
        return true;
    }

public: // as the above methods are private, this public method
        // is usable by sudokusolver
    bool isValid(int value, int row, int col, int grid[9][9]){
        return satisfyunary(value) && satisfygrid(value, row, col, grid);
    }
}; 

class sudokusolver {
private:
    variables sudokuGrid[9][9]; 

public:
    // initializing in the constructor (with our private sudokuGrid)
    sudokusolver(int grid[9][9]) {
        for (int i = 0; i < 9; i++){
            for (int j = 0; j < 9; j++){
                sudokuGrid[i][j] = variables(grid[i][j], i, j);
            }
        }
    }

    bool isfinalstate(int grid[9][9]){
        for (int i = 0; i < 9; i++){
            for (int j = 0; j < 9; j++){
                if (grid[i][j] == 0) 
                return false;
            }
        }
        return true;
    }

    bool backtrack(int grid[9][9]){
        if (isfinalstate(grid)){
            return true;
        }
        
        for(int i = 0; i < 9; i++){
            for(int j = 0; j < 9; j++){
                if (grid[i][j] == 0){ // checking for empty values
                    constraints c; // creating an instance for constraints
                    for (int val = 1; val <= 9; val++){ // iterate through all possible values for the variable (1 to 9)
                        if (c.isValid(val, i, j, grid)){ // check if value is valid
                            grid[i][j] = val;
                            if (backtrack(grid)){ // if yes, recrusively check for other values
                                return true;
                            }
                            else {
                                grid[i][j] = 0; // reset if the variable cannot have that value
                            }
                        }
                    }
                    return false; // if the variable cannot have any value from domain, backtrack
                }
            }
        }
        return true; // redudant but basically indicates final state
    }

    bool solve() {
        int tempGrid[9][9];
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                tempGrid[i][j] = sudokuGrid[i][j].getValue();
            }
        }
        if (backtrack(tempGrid)){
            for (int i = 0; i < 9; i++){
                for (int j = 0; j < 9; j++){
                    sudokuGrid[i][j].setValue(tempGrid[i][j]);
                }
            }
            return true;
        }    
        return false;
    }


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