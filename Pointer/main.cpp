#include <iostream>

#define NumberOf(x) (sizeof(x) / sizeof(x[0]))

using namespace std;

void foo2(void (*func)(string), string s) {
    func(s);
}

void foo(string s) {
    cout << s;
}

void swap(int& a, int& b) {
    int temp;
    temp = a;
    a = b;
    b = temp;
}

void func(int* array, void func(int&, int&)) {
    func(array[0], array[1]);
}

int inner(int a, int b) {
    cout << a << " " << b << endl;
    return a + b;
}

int (*outter())(int, int) {
    return inner;
}

int main() {
    int (*a)(int, int) = outter();
    cout << a(2, 3) << endl;
    return 0;
}



