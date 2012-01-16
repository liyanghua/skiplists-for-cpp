#include <iostream>

#include "skiplists.hpp"

using namespace std;


int main(int argc, char* argv[])
{
    SkipLists<int, int> skip_list;
    
    cout << "insert (1,2)" << endl;
    bool r = skip_list.insert(1,2);
    if (!r) {
        cout << "Insertion Failure!" << endl;
    }

    cout << "insert (3,4)" << endl;
    r = skip_list.insert(3, 4);
    if (!r) {
        cout << "Insertion Failure!" << endl;
    }

    cout << "insert (5,6)" << endl;
    r = skip_list.insert(5, 6);
    if (!r) {
        cout << "Insertion Failure!" << endl;
    }
    skip_list.print();

    int v = -1;
    r = skip_list.find(1, v);
    assert(v == 2);

    r = skip_list.find(3, v);
    assert(v == 4);

    r = skip_list.find(5, v);

    assert(v == 6);


    cout << "delete key:1" << endl; 
    r = skip_list.remove(1);
    if (!r) {
        cout << "Deletion failure.." << endl;
    }

    skip_list.print();


    cout << "delete key:3" << endl; 
    r = skip_list.remove(3);
    if (!r) {
        cout << "Deletion failure.." << endl;
    }

    skip_list.print();


    cout << "delete key:5" << endl; 
    r = skip_list.remove(5);
    if (!r) {
        cout << "Deletion failure.." << endl;
    }
    skip_list.print();
    
    return 0;
}
