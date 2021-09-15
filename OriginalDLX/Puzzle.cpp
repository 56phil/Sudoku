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
    for (node* n0 = col->down; n0 != col; n0 = n0->down) {
        for (node* n1 = n0->right; n1 != n0; n1 = n1->right) {
            n1->down->up = n1->up;
            n1->up->down = n1->down;
            n1->head->size--;
        }
    }
}

void Puzzle::uncoverColumn(node* col) {
    for (node* n0 = col->up; n0 != col; n0 = n0->up) {
        for (node* n1 = n0->left; n1 != n0; n1 = n1->left) {
            n1->head->size++;
            n1->down->up = n1;
            n1->up->down = n1;
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
    for (node* n0 = col->right; n0 != headNode; n0 = n0->right)
        if (n0->size < col->size)
            col = n0;

    coverColumn(col);

    for (node* n0 = col->down; n0 != col; n0 = n0->down) {
        solution[k] = n0;
        for (node* n1 = n0->right; n1 != n0; n1 = n1->right) {
            coverColumn(n1->head);
        }

        search(k + 1);

        n0 = solution[k];
        solution[k] = nullptr;
        col = n0->head;
        for (node* n1 = n0->left; n1 != n0; n1 = n1->left) {
            uncoverColumn(n1->head);
        }
    }
    uncoverColumn(col);
}


//===========================================================================//
// Functions to turn a Sudoku grid into an Exact Cover problem --------------//
//===========================================================================//

static void c1(int &counter, int &j, bool (*matrix)[324]) {
    j = 0;
    counter = 0;
    for (int i = 0; i < Puzzle::ROW_NB; i++) { //iterate over all rows
        matrix[i][j] = true;
        counter++;
        if (counter >= SIZE) {
            j++;
            counter = 0;
        }
    }
}

static int c2(int &counter, int &j, bool (*matrix)[324]) {
    int x = 0;
    counter = 1;
    for (j = Puzzle::SIZE_SQUARED; j < 2 * Puzzle::SIZE_SQUARED; j++) {
        for (int i = x; i < counter * Puzzle::SIZE_SQUARED; i += SIZE)
            matrix[i][j] = true;

        if ((j + 1) % SIZE == 0) {
            x = counter * Puzzle::SIZE_SQUARED;
            counter++;
        } else {
            x++;
        }
    }
    return x;
}

static void c3(int &j, bool (*matrix)[324]) {
    for (int i = 0; i < Puzzle::ROW_NB; i++)
    {
        matrix[i][j] = true;
        j++;
        if (j >= 3 * Puzzle::SIZE_SQUARED)
            j = 2 * Puzzle::SIZE_SQUARED;
    }
}

static void c4(int &j, bool (*matrix)[324], int &x) {
    x = 0;
    for (j = 3 * Puzzle::SIZE_SQUARED; j < Puzzle::COL_NB; j++) {

        for (int l = 0; l < Puzzle::SIZE_SQRT; l++) {
            for (int k = 0; k < Puzzle::SIZE_SQRT; k++)
                matrix[x + l * SIZE + k * Puzzle::SIZE_SQUARED][j] = true;
        }

        int temp = j + 1 - 3 * Puzzle::SIZE_SQUARED;

        if (temp % (int)(Puzzle::SIZE_SQRT * SIZE) == 0)
            x += (Puzzle::SIZE_SQRT - 1) * Puzzle::SIZE_SQUARED + (Puzzle::SIZE_SQRT - 1) * SIZE + 1;
        else if (temp % SIZE == 0)
            x += SIZE * (Puzzle::SIZE_SQRT - 1) + 1;
        else
            x++;
    }
}

// BUILD THE INITIAL MATRIX CONTAINING ALL POSSIBILITIES --------------------//
void Puzzle::buildSparseMatrix(bool matrix[ROW_NB][COL_NB]) {

    // 1: There can only be one value in any given cell
    int j;
    int counter;
    c1(counter, j, matrix);

    // 2: There can only be one instance of a number in any given row
    int x = c2
    (counter, j, matrix);

    // 3: There can only be one instance of a number in any given column
    j = 2 * SIZE_SQUARED;
    c3(j, matrix);

    // 4: There can only be one instance of a number in any given 3x3 region
    c4(j, matrix, x);
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
        node* prev = nullptr;

        if (i != 0 && i % SIZE_SQUARED == 0) {
            ID[0] -= SIZE - 1;
            ID[1]++;
            ID[2] -= SIZE - 1;
        } else if (i!= 0 && i%SIZE == 0) {
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
                if (prev == nullptr) {
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
                node* col = nullptr;
                node* row = nullptr;
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
    if (!isSolved) {
        cout << "No Solution for:\n";
        printGrid(sudoku);
    }
    isSolved = false;
    if (solved.size() > 1) {
        cerr << "Solutions: " << solved.size() << '\n';
        unique(solved.begin(), solved.end());
        cerr << "Solutions: " << solved.size() << '\n';
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
    for (int i = 0; solution[i] != nullptr; i++) {
        sudoku[solution[i]->rowID[1] - 1][solution[i]->rowID[2] - 1] =
        solution[i]->rowID[0];
    }
    for (int i = 0; orig_values[i] != nullptr; i++) {
        sudoku[orig_values[i]->rowID[1] - 1]
        [orig_values[i]->rowID[2] - 1] = orig_values[i]->rowID[0];
    }
}

// PRINTS A SUDOKU GRID OF ANY SIZE -----------------------------------------//
void Puzzle::printGrid(gridType& grid){
    string int_border = "|";
    int additional(SIZE > 9 ? SIZE : 0);
    int counter(1);
    for (int i = 0; i < ((SIZE + SIZE_SQRT - 1) * 2 + additional + 1); i++) {
        if (i > 0 &&
            i % ((SIZE_SQRT * 2 + SIZE_SQRT * (SIZE > 9) + 1) * counter + counter - 1) == 0) {
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
            cout << ext_border;
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

void init1dArrays(node* arg [], int n) {
    for (int i(0); i < n; i++) {
        arg[i] = nullptr;
    }
}

Puzzle::Puzzle() {
    head.size = 0;
    head.rowID[0] = head.rowID[1] = head.rowID[2] = 0;
    head.left = head.right = head.up = head.down = nullptr;
    headNode = &head;
    isSolved = false;
    timer = timer2 = clock();
    initMatrix();
    init1dArrays(solution, MAX_K);
    init1dArrays(orig_values, MAX_K);
    solved.clear();
}
