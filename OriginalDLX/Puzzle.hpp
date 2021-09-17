//  Puzzle.hpp
//  Puzzle
//
//  Created by Phil Huffman on 9/6/21.
//

#ifndef Puzzle_hpp
#define Puzzle_hpp
#include <array>
#include <ctime>
#include <iostream>
#include "node.hpp"
#include <vector>

#define MAX_K 512
#define SIZE 9

using namespace std;

class Puzzle {
public:
    typedef array<array<int, SIZE>, SIZE> gridType;
    static const int SIZE_SQRT = 3;
    static const int SIZE_SQUARED = SIZE * SIZE;
    static const int ROW_NB = SIZE_SQUARED * SIZE;
    static const int COL_NB = SIZE_SQUARED << 2;

    void solveSudoku(gridType);
    Puzzle();

 private:
    node head;
    node* headNode = &head;
    array<node*, MAX_K>solution;
    array<node*, MAX_K>orig_values;

    bool matrix[ROW_NB][COL_NB];
    bool isSolved = false;
    clock_t timer, timer2;
    vector<gridType>solved;

// prototypes
    void coverColumn(node*);
    void uncoverColumn(node*);
    void search(int);
    void buildSparseMatrix(bool matrix[ROW_NB][COL_NB]);
    void buildLinkedList(bool matrix[ROW_NB][COL_NB]);
    void transformListToCurrentGrid(gridType& puzzle);
    void mapSolutionToGrid(gridType&);
    void printGrid(gridType&);
    void initMatrix();
};
#endif /* PuzzleClass_hpp */
