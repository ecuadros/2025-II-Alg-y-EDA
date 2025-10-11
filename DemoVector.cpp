#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

template <typename _T>
struct GeneralTraits{
    using value_type = _T;
    
};

void DemoVector(){
    CVector< GeneralTraits<int> > vector(10);
    for(int i = 0; i < 5; i++) {
        int val = i * 10;
        vector.insert(val);
    }
    vector[3] = 8;
    cout << vector << endl;
}