#include <iostream>
#include <thread>
#include <vector>
#include "hilos.h"
#include "vector.h"
#include <mutex>
using namespace std;

mutex cout_mutex;

// Function to be run by the thread
void func(int thread_id, CVector<int> &v)
{
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Hello from the thread!" << endl;
    }
    for (int i = 0; i < 5; ++i)
    {
        // Bloqueamos el mutex para que la salida sea atómica
        // lock_guard<mutex> lock(cout_mutex);
        lock_guard<mutex> lock(cout_mutex);
        cout << "Thread ID: " << this_thread::get_id()
             << " | Thread número: " << thread_id
             << " | Iteración: " << i << endl;
    }

    for (int i = 0; i < 5; ++i)
    {
        v.insert(i);
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Thread " << thread_id << " insertó valor: " << i << endl;
        }
    }
}

void DemoThreads()
{
    const int num_threads = 3;
    vector<thread> threads;
    CVector<int> vect(10);
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Creando " << num_threads << " threads..." << endl;
    }

    // Crear y lanzar los threads
    for (int i = 0; i < num_threads; ++i)
    {
        // Crear el objeto thread
        thread t(func, i, ref(vect));

        // Mover el thread al vector (no se puede copiar, solo mover)
        threads.push_back(move(t));
    }

    // Esperar a que todos los threads terminen
    for (auto &t : threads)
    {
        t.join();
    }
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Todos los threads han terminado." << endl;
        cout << "Vector final: " << vect << endl;
        cout << "Tamaño final del vector: " << vect.size() << endl;
    }
}