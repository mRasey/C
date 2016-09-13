#include <cstdlib>
#include "iostream"

using namespace std;

int test() {
    int i = 0;
    srand(0);
    while(i < 50) {
        int a = abs(rand() % 1000);
        int b = abs(rand() % 1000);
        if(a <= b && (b - a) < 100) {
            cout << a << " " << b << endl;
            i++;
        }
    }
}

