#include <fstream>
#include "Puzzle.hpp"

using namespace std;

void readGrids(vector<Puzzle::gridType>&);
void stringToGrid(string&, Puzzle::gridType&);

//----------------------------------------------------------------------------//

int main(){
    vector<Puzzle::gridType>grids;
    
    readGrids(grids);
    
    cout << "Grids ready to solve: " << grids.size() << '\n';
    
    for (auto grid : grids) {
        Puzzle p;
        p.solveSudoku(grid);
    }
    return 0;
}

void stringToGrid(string& s, Puzzle::gridType& g) {
    for (int i(0); i < SIZE; i++) {
        for (int j(0); j < SIZE; j++) {
            g[i][j] = s[i * SIZE + j] & 0x0f;
        }
    }
}

void readGrids(vector<Puzzle::gridType> &grids) {
    const string fn("/Users/prh/sudoku/raw_sudokus.txt");
    char buff[BUFSIZ];
    fstream sIn;
    sIn.open(fn, ios::in);
    assert(sIn);
    while (sIn.getline(buff, BUFSIZ, ',')) {
        sIn.ignore(BUFSIZ, '\n');
        string wrk(buff);
        replace(wrk.begin(), wrk.end(), '.', '0');
        if (wrk.size() == Puzzle::SIZE_SQUARED &&
            all_of(wrk.begin(), wrk.end(), ::isdigit)) {
            Puzzle::gridType g;
            stringToGrid(wrk, g);
            grids.emplace_back(g);
        }
    }
    sIn.close();
}
