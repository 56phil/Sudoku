//
//  node.hpp
//  node
//
//  Created by Phil Huffman on 9/6/21.
//

#ifndef node_h
#define node_h

struct Node {

    Node *left;
    Node *right;
    Node *up;
    Node *down;
    Node *head;

    int size;        //used for Column header
    int rowID[3];    //used to identify row in order to map solutions to a sudoku grid
                     //ID Format: (Candidate, Row, Column)
};


#endif /* node_h */
