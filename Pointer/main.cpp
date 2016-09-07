#include <iostream>

using namespace std;

void foo2(void (*func)) {
    func;
}

void foo() {
    cout << "foo";
}

int main() {
    foo2((void *) foo);
    return 0;
}



