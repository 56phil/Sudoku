#include <array>
#include <ctime>
#include <fstream>
#include <iostream>
#include "node.hpp"
#include <string>
#include <vector>

#define MAX_K 1000
#define SIZE 9

using namespace std;

typedef array<array<int, SIZE>, SIZE> puzzleType;

const int SIZE_SQUARED = SIZE*SIZE;
const int SIZE_SQRT = 3;
const int ROW_NB = SIZE*SIZE*SIZE;
const int COL_NB = 4 * SIZE*SIZE;

struct Node Head;
struct Node* HeadNode = &Head;
struct Node* solution[MAX_K];
struct Node* orig_values[MAX_K];

bool matrix[ROW_NB][COL_NB] = { { 0 } };
bool isSolved = false;

void mapSolutionToGrid(puzzleType&);
void printGrid(puzzleType&);
void stringToPuzzleType(string&, puzzleType&);

clock_t timer, timer2;

//===============================================================================================================//
//---------------------------------------------DLX Functions-----------------------------------------------------//
//===============================================================================================================//

void coverColumn(Node* col) {
    col->left->right = col->right;
    col->right->left = col->left;
    for (Node* node = col->down; node != col; node = node->down) {
        for (Node* temp = node->right; temp != node; temp = temp->right) {
            temp->down->up = temp->up;
            temp->up->down = temp->down;
            temp->head->size--;
        }
    }
}

void uncoverColumn(Node* col) {
    for (Node* node = col->up; node != col; node = node->up) {
        for (Node* temp = node->left; temp != node; temp = temp->left) {
            temp->head->size++;
            temp->down->up = temp;
            temp->up->down = temp;
        }
    }
    col->left->right = col;
    col->right->left = col;
}

void search(int k) {

    if (HeadNode->right == HeadNode) {
        timer2 = clock() - timer;
        puzzleType grid;
        mapSolutionToGrid(grid);
        printGrid(grid);
        std::cout << "Time Elapsed: " << (float)timer2 / CLOCKS_PER_SEC << " seconds.\n\n";
        timer = clock();
        isSolved = true;
        return;
    }

    //Choose Column Object Deterministically: Choose the column with the smallest Size
    Node* Col = HeadNode->right;
    for (Node* temp = Col->right; temp != HeadNode; temp = temp->right)
        if (temp->size < Col->size)
            Col = temp;

    coverColumn(Col);

    for (Node* temp = Col->down; temp != Col; temp = temp->down) {
        solution[k] = temp;
        for (Node* node = temp->right; node != temp; node = node->right) {
            coverColumn(node->head);
        }

        search(k + 1);

        temp = solution[k];
        solution[k] = NULL;
        Col = temp->head;
        for (Node* node = temp->left; node != temp; node = node->left) {
            uncoverColumn(node->head);
        }
    }

    uncoverColumn(Col);
}

//===============================================================================================================//
//----------------------Functions to turn a Sudoku grid into an Exact Cover problem -----------------------------//
//===============================================================================================================//

//--------------------------BUILD THE INITIAL MATRIX CONTAINING ALL POSSIBILITIES--------------------------------//
void BuildSparseMatrix(bool matrix[ROW_NB][COL_NB]) {

    //Constraint 1: There can only be one value in any given cell
    int j = 0, counter = 0;
    for (int i = 0; i < ROW_NB; i++) { //iterate over all rows
        matrix[i][j] = 1;
        counter++;
        if (counter >= SIZE) {
            j++;
            counter = 0;
        }
    }

    //Constraint 2: There can only be one instance of a number in any given row
    int x = 0;
    counter = 1;
    for (j = SIZE_SQUARED; j < 2 * SIZE_SQUARED; j++) {
        for (int i = x; i < counter*SIZE_SQUARED; i += SIZE)
            matrix[i][j] = 1;

        if ((j + 1) % SIZE == 0) {
            x = counter*SIZE_SQUARED;
            counter++;
        }
        else
            x++;
    }

    //Constraint 3: There can only be one instance of a number in any given column
    j = 2 * SIZE_SQUARED;
    for (int i = 0; i < ROW_NB; i++)
    {
        matrix[i][j] = 1;
        j++;
        if (j >= 3 * SIZE_SQUARED)
            j = 2 * SIZE_SQUARED;
    }

    //Constraint 4: There can only be one instance of a number in any given region
    x = 0;
    for (j = 3 * SIZE_SQUARED; j < COL_NB; j++) {

        for (int l = 0; l < SIZE_SQRT; l++) {
            for (int k = 0; k<SIZE_SQRT; k++)
                matrix[x + l*SIZE + k*SIZE_SQUARED][j] = 1;
        }

        int temp = j + 1 - 3 * SIZE_SQUARED;

        if (temp % (int)(SIZE_SQRT * SIZE) == 0)
            x += (SIZE_SQRT - 1)*SIZE_SQUARED + (SIZE_SQRT - 1)*SIZE + 1;
        else if (temp % SIZE == 0)
            x += SIZE*(SIZE_SQRT - 1) + 1;
        else
            x++;
    }
}

//-------------------BUILD A TOROIDAL DOUBLY LINKED LIST OUT OF THE SPARSE MATRIX-------------------------//
void BuildLinkedList(bool matrix[ROW_NB][COL_NB]) {

    Node* header = new Node;
    header->left = header;
    header->right = header;
    header->down = header;
    header->up = header;
    header->size = -1;
    header->head = header;
    Node* temp = header;

    //Create all Column Nodes
    for (int i = 0; i < COL_NB; i++) {
        Node* newNode = new Node;
        newNode->size = 0;
        newNode->up = newNode;
        newNode->down = newNode;
        newNode->head = newNode;
        newNode->right = header;
        newNode->left = temp;
        temp->right = newNode;
        temp = newNode;
    }

    int ID[3] = { 0,1,1 };
    //Add a Node for each 1 present in the sparse matrix and update Column Nodes accordingly
    for (int i = 0; i < ROW_NB; i++) {
        Node* top = header->right;
        Node* prev = NULL;

        if (i != 0 && i%SIZE_SQUARED == 0) {
            ID[0] -= SIZE - 1;
            ID[1]++;
            ID[2] -= SIZE - 1;
        }
        else if (i!= 0 && i%SIZE == 0) {
            ID[0] -= SIZE - 1;
            ID[2]++;
        }
        else {
            ID[0]++;
        }

        for (int j = 0; j < COL_NB; j++, top = top->right) {
            if (matrix[i][j]) {
                Node* newNode = new Node;
                newNode->rowID[0] = ID[0];
                newNode->rowID[1] = ID[1];
                newNode->rowID[2] = ID[2];
                if (prev == NULL) {
                    prev = newNode;
                    prev->right = newNode;
                }
                newNode->left = prev;
                newNode->right = prev->right;
                newNode->right->left = newNode;
                prev->right = newNode;
                newNode->head = top;
                newNode->down = top;
                newNode->up = top->up;
                top->up->down = newNode;
                top->size++;
                top->up = newNode;
                if (top->down == top)
                    top->down = newNode;
                prev = newNode;
            }
        }
    }

    HeadNode = header;
}

// COVERS VALUES THAT ARE ALREADY PRESENT IN THE GRID -----------------------------------------------------//
void TransformListToCurrentGrid(puzzleType& puzzle) {
    int index = 0;
    for(int i = 0 ; i<SIZE; i++ )
        for(int j = 0 ; j<SIZE; j++)
            if (puzzle[i][j] > 0) {
                Node* Col = NULL;
                Node* temp = NULL;
                for (Col = HeadNode->right; Col != HeadNode; Col = Col->right) {
                    for (temp = Col->down; temp != Col; temp = temp->down)
                        if (temp->rowID[0] == puzzle[i][j] && (temp->rowID[1] - 1) == i && (temp->rowID[2] - 1) == j)
                            goto ExitLoops;
                }
            ExitLoops:
                coverColumn(Col);
                orig_values[index] = temp;
                index++;
                for (Node* node = temp->right; node != temp; node = node->right) {
                    coverColumn(node->head);
                }

            }

}

void SolveSudoku(puzzleType& sudoku) {
    timer = clock();
    BuildSparseMatrix(matrix);
    BuildLinkedList(matrix);
    TransformListToCurrentGrid(sudoku);
    search(0);
    if (!isSolved)
        std::cout << "No Solution!" << std::endl;
    isSolved = false;
}

//=====================================================================================================//
//------------------------------------- Print Functions -----------------------------------------------//
//=====================================================================================================//

void mapSolutionToGrid(puzzleType& sudoku) {

    for (int i = 0; solution[i] != NULL; i++) {
        sudoku[solution[i]->rowID[1]-1][solution[i]->rowID[2]-1] = solution[i]->rowID[0];
    }
    for (int i = 0; orig_values[i] != NULL; i++) {
        sudoku[orig_values[i]->rowID[1] - 1][orig_values[i]->rowID[2] - 1] = orig_values[i]->rowID[0];
    }
}

//---------------------------------PRINTS A SUDOKU GRID OF ANY SIZE---------------------------------------------//
void printGrid(puzzleType& grid){
    std::string ext_border = "+", int_border = "|";
    int counter = 1;
    int additional = 0;
    if (SIZE > 9)
        additional = SIZE;
    for (int i = 0; i < ((SIZE +SIZE_SQRT - 1) * 2 +additional+ 1); i++) {
        ext_border += '-';

        if (i > 0 && i % ((SIZE_SQRT*2+SIZE_SQRT*(SIZE>9)+1)*counter + counter-1) == 0) {
            int_border += '+';
            counter++;
        }
        else
            int_border += '-';
    }
    ext_border += '+';
    int_border += "|";

    std::cout << ext_border << std::endl;
    for (int i = 0; i<SIZE; i++){
        std::cout << "| ";
        for (int j = 0; j<SIZE; j++){
            if (grid[i][j] == 0)
                std::cout << ". ";
            else
                std::cout << grid[i][j] << " ";
            if (additional > 0 && grid[i][j]<10)
                std::cout << " ";
            if ((j+1)%SIZE_SQRT == 0)
                std::cout << "| ";
        }
        std::cout << std::endl;
        if ((i + 1) % SIZE_SQRT == 0 && (i+1)<SIZE)
            std::cout << int_border << std::endl;
    }
    std::cout << ext_border << std::endl << std::endl;
}

void stringToPuzzleType(string& s, puzzleType& m) {
    for (int i(0); i < SIZE; i++) {
        for (int j(0); j < SIZE; j++) {
            m[i][j] = s[i * SIZE + j] & 0x0f;
        }
    }
}

static void loadPuzzles(vector<puzzleType> &puzzles) {
    const string fn("/Users/prh/Keepers/code/cpp/SudokuStuff/raw_sudokus.txt");
    char buff[BUFSIZ];
    fstream sIn;
    sIn.open(fn, ios::in);
    assert(sIn);
    while (sIn.getline(buff, BUFSIZ, ',')) {
        sIn.ignore(BUFSIZ, '\n');
        string wrk(buff);
        replace(wrk.begin(), wrk.end(), '.', '0');
        if (wrk.size() == SIZE_SQUARED && all_of(wrk.begin(), wrk.end(), ::isdigit)) {
            puzzleType m;
            stringToPuzzleType(wrk, m);
            puzzles.emplace_back(m);
        }
    }
    sIn.close();
}

//--------------------------------------------------------------------------------//

int main(){
    vector<puzzleType>puzzles;

    loadPuzzles(puzzles);

    for (auto puzzle : puzzles) {
        SolveSudoku(puzzle);
    }
    return 0;
}
