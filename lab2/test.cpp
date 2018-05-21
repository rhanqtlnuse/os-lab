#include <iostream>
#include <fstream>
using namespace std;

int main() {
    ifstream fin;
    fin.open("a.img", ios::binary);
    fin.seekg(0x2601, ios::beg);
    
    int a;
    fin >> a;
    cout << a;
    return 0;
}