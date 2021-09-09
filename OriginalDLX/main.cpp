#include <fstream>
#include "node.hpp"
#include "Puzzle.hpp"
#include <vector>

using namespace std;

void loadPuzzles(vector<Puzzle::gridType>&);
void stringToPuzzleType(string&, Puzzle::gridType&);

//----------------------------------------------------------------------------//

int main(){
    vector<Puzzle::gridType>puzzles;
    
    loadPuzzles(puzzles);
    
    cout << "Grids in vector: " << puzzles.size() << '\n';
    
    for (auto puzzle : puzzles) {
        Puzzle p;
        p.solveSudoku(puzzle);
    }
    return 0;
}

void stringToPuzzleType(string& s, Puzzle::gridType& m) {
    for (int i(0); i < SIZE; i++) {
        for (int j(0); j < SIZE; j++) {
            m[i][j] = s[i * SIZE + j] & 0x0f;
        }
    }
}

void loadPuzzles(vector<Puzzle::gridType> &puzzles) {
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
            Puzzle::gridType m;
            stringToPuzzleType(wrk, m);
            puzzles.emplace_back(m);
        }
    }
    sIn.close();
}
