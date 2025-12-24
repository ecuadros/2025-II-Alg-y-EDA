#include "heap.h"
#include <iostream>
#include <string>

int main () {
    cout << "Demo HEAP " << endl;
    CHeap<MaxHeapTraits<int>> minHeap;

    minHeap.Push(2);
    minHeap.Push(9);
    minHeap.Push(7);
    minHeap.Push(4);
    minHeap.Push(8);
    minHeap.Push(1);
    minHeap.Push(5);
    minHeap.Push(6);
    minHeap.Push(3);

    
    cout << "Top: " << minHeap.Top() << endl; 
    
    cout << "Elimino con Pop() " << minHeap.Pop() << endl;
    cout << "Nuevo Top: " << minHeap.Top() << endl; 
    cout << minHeap << endl;
    CHeap<MaxHeapTraits<int>> nuevoHeap(minHeap);
    cout << "Constructor Copia" << endl;
    cout << nuevoHeap << endl;
    cout << "\n Read and Write" << endl;
    ofstream outFile("DemoHeap.dat");
    nuevoHeap.Write(outFile);
}