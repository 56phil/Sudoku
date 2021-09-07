//
//  PuzzleClass.cpp
//  PuzzleClass
//
//  Created by Phil Huffman on 9/6/21.
//

#include "Puzzle.hpp"
void Puzzle::coverColumn(node* col) {
    col->left->right = col->right;
    col->right->left = col->left;
    for (node* node0 = col->down; node0 != col; node0 = node0->down) {
        for (node* node1 = node0->right; node1 != node0; node1 = node1->right) {
            node1->down->up = node1->up;
            node1->up->down = node1->down;
            node1->head->size--;
        }
    }
}

void Puzzle::uncoverColumn(node* col) {
    for (node* node0 = col->up; node0 != col; node0 = node0->up) {
        for (node* node1 = node0->left; node1 != node0; node1 = node1->left) {
            node1->head->size++;
            node1->down->up = node1;
            node1->up->down = node1;
        }
    }
    col->left->right = col;
    col->right->left = col;
}

void Puzzle::search(int k) {
    
    if (headNode->right == headNode) {
        timer2 = clock() - timer;
        gridType grid;
        mapSolutionToGrid(grid);
        solved.push_back(grid);
        if (solved.size() == 1) {
            printGrid(grid);
            cout << "Time Elapsed: "
            << static_cast<double>(timer2) / CLOCKS_PER_SEC
            << " seconds.\n" << endl;
        }
        timer = clock();
        isSolved = true;
        return;
    }
    
    // Choose the column with the smallest size to enhance speed
    node* col = headNode->right;
    for (node* temp = col->right; temp != headNode; temp = temp->right)
        if (temp->size < col->size)
            col = temp;
    
    coverColumn(col);
    
    for (node* temp = col->down; temp != col; temp = temp->down) {
        solution[k] = temp;
        for (node* tNode = temp->right; tNode != temp; tNode = tNode->right) {
            coverColumn(tNode->head);
        }
        
        search(k + 1);
        
        temp = solution[k];
        solution[k] = NULL;
        col = temp->head;
        for (node* tNode = temp->left; tNode != temp; tNode = tNode->left) {
            uncoverColumn(tNode->head);
        }
    }
    uncoverColumn(col);
}


//===========================================================================//
// Functions to turn a Sudoku grid into an Exact Cover problem --------------//
//===========================================================================//

// BUILD THE INITIAL MATRIX CONTAINING ALL POSSIBILITIES --------------------//
void Puzzle::buildSparseMatrix(bool matrix[ROW_NB][COL_NB]) {
    
    // 1: There can only be one value in any given cell
    int j = 0, counter = 0;
    for (int i = 0; i < ROW_NB; i++) { //iterate over all rows
        matrix[i][j] = 1;
        counter++;
        if (counter >= SIZE) {
            j++;
            counter = 0;
        }
    }
    
    // 2: There can only be one instance of a number in any given row
    int x = 0;
    counter = 1;
    for (j = SIZE_SQUARED; j < 2 * SIZE_SQUARED; j++) {
        for (int i = x; i < counter*SIZE_SQUARED; i += SIZE)
            matrix[i][j] = 1;
        
        if ((j + 1) % SIZE == 0) {
            x = counter * SIZE_SQUARED;
            counter++;
        }
        else
            x++;
    }
    
    // 3: There can only be one instance of a number in any given column
    j = 2 * SIZE_SQUARED;
    for (int i = 0; i < ROW_NB; i++)
    {
        matrix[i][j] = 1;
        j++;
        if (j >= 3 * SIZE_SQUARED)
            j = 2 * SIZE_SQUARED;
    }
    
    // 4: There can only be one instance of a number in any given 3x3 region
    x = 0;
    for (j = 3 * SIZE_SQUARED; j < COL_NB; j++) {
        
        for (int l = 0; l < SIZE_SQRT; l++) {
            for (int k = 0; k < SIZE_SQRT; k++)
                matrix[x + l * SIZE + k * SIZE_SQUARED][j] = 1;
        }
        
        int temp = j + 1 - 3 * SIZE_SQUARED;
        
        if (temp % (int)(SIZE_SQRT * SIZE) == 0)
            x += (SIZE_SQRT - 1) * SIZE_SQUARED + (SIZE_SQRT - 1) * SIZE + 1;
        else if (temp % SIZE == 0)
            x += SIZE * (SIZE_SQRT - 1) + 1;
        else
            x++;
    }
}

// BUILD A TOROIDAL DOUBLY LINKED LIST OUT OF THE SPARSE MATRIX -------------//
void Puzzle::buildLinkedList(bool matrix[ROW_NB][COL_NB]) {
    
    node* header = new node;
    header->left = header;
    header->right = header;
    header->down = header;
    header->up = header;
    header->size = -1;
    header->head = header;
    node* temp = header;
    
    // Create all Column Nodes
    for (int i = 0; i < COL_NB; i++) {
        node* newNode = new node;
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
    // Add a Node for each 1 present in the sparse matrix and
    // update Column Nodes accordingly
    for (int i = 0; i < ROW_NB; i++) {
        node* top = header->right;
        node* prev = NULL;
        
        if (i != 0 && i % SIZE_SQUARED == 0) {
            ID[0] -= SIZE - 1;
            ID[1]++;
            ID[2] -= SIZE - 1;
        }
        else if (i!= 0 && i%SIZE == 0) {
            ID[0] -= SIZE - 1;
            ID[2]++;
        } else {
            ID[0]++;
        }
        
        for (int j = 0; j < COL_NB; j++, top = top->right) {
            if (matrix[i][j]) {
                node* newNode = new node;
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
    headNode = header;
}

// COVERS VALUES THAT ARE ALREADY PRESENT IN THE GRID -----------------------//
void Puzzle::transformListToCurrentGrid(gridType& puzzle) {
    int index = 0;
    for(int i = 0; i < SIZE; i++ )
        for(int j = 0; j < SIZE; j++)
            if (puzzle[i][j] > 0) {
                node* col = NULL;
                node* row = NULL;
                for (col = headNode->right; col != headNode; col = col->right) {
                    for (row = col->down; row != col; row = row->down)
                        if (row->rowID[0] == puzzle[i][j] &&
                            row->rowID[1] - 1 == i &&
                            row->rowID[2] - 1 == j)
                            goto ExitLoops;
                }
            ExitLoops:
                coverColumn(col);
                orig_values[index] = row;
                index++;
                for (node* node0 = row->right; node0 != row; node0 = node0->right) {
                    coverColumn(node0->head);
                }
            }
}

void Puzzle::solveSudoku(gridType sudoku) {
    timer = clock();
    buildSparseMatrix(matrix);
    buildLinkedList(matrix);
    transformListToCurrentGrid(sudoku);
    search(0);
    if (!isSolved)
        cout << "No Solution for:\n";
    isSolved = false;
    if (solved.size() > 1) {
        unique(solved.begin(), solved.end());
        int cntr(0);
        for (auto g = solved.begin() + 1; g < solved.end(); g++) {
            printGrid(*g);
            if ((++cntr) > SIZE_SQUARED) {
                break;
            }
        }
        cout << "Total solutions: " << solved.size() << "\n\n" << endl;
    }
}

//===========================================================================//
// Print Functions ----------------------------------------------------------//
//===========================================================================//

void Puzzle::mapSolutionToGrid(gridType& sudoku) {
    for (int i = 0; solution[i] != NULL; i++) {
        sudoku[solution[i]->rowID[1]-1][solution[i]->rowID[2]-1] = solution[i]->rowID[0];
    }
    for (int i = 0; orig_values[i] != NULL; i++) {
        sudoku[orig_values[i]->rowID[1] - 1][orig_values[i]->rowID[2] - 1] = orig_values[i]->rowID[0];
    }
}

// PRINTS A SUDOKU GRID OF ANY SIZE -----------------------------------------//
void Puzzle::printGrid(gridType& grid){
    string int_border = "|";
    int counter = 1;
    int additional = 0;
    if (SIZE > 9)
        additional = SIZE;
    for (int i = 0; i < ((SIZE + SIZE_SQRT - 1) * 2 + additional + 1); i++) {
        
        if (i > 0 && i % ((SIZE_SQRT * 2 + SIZE_SQRT * (SIZE > 9) + 1) * counter + counter - 1) == 0) {
            int_border += '+';
            counter++;
        } else {
            int_border += '-';
        }
    }
    int_border += "|\n";
    string ext_border(int_border);
    replace(ext_border.begin(), ext_border.end(), '|', '+');
    cout << ext_border;
    for (int i = 0; i < SIZE; i++){
        cout << "| ";
        for (int j = 0; j < SIZE; j++){
            if (grid[i][j] == 0)
                cout << ". ";
            else
                cout << grid[i][j] << " ";
            if (additional > 0 && grid[i][j] < 10)
                cout << " ";
            if ((j+1)%SIZE_SQRT == 0)
                cout << "| ";
        }
        cout << endl;
        if ((i + 1) % SIZE_SQRT == 0 && (i + 1) < SIZE)
            cout << int_border;
    }
    cout << ext_border << "\n";
}

void Puzzle::initMatrix() {
    for (int i(0); i < ROW_NB; i++) {
        for (int j(0); j < COL_NB; j++) {
            matrix[i][j] = false;
        }
    }
}

Puzzle::Puzzle() {
    initMatrix();
    headNode = &head;
    solved.clear();
};
