#ifndef __UTIL_H__
#define __UTIL_H__
#include <iostream> // cout
using namespace std;

void DemoUtil();

template <typename T>
void intercambio(T &a, T &b){
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
void Print(T &n){   cout << n << " "; }

#endif