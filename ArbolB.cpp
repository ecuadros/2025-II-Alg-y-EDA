//tstbtree.cc  
//Author: Chicana D�az, Johan Pier
//Bachelor in Engineering of System
//#include <iostream.h>
#include <time.h>
#include <stdlib.h>

#include "btree.h"
#include <string>
#include <functional>
#include <iostream>
#include <fstream>

using namespace std;

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyz";

const int BTreeSize = 3;
int main (int argc, char * argv){
       test_default_comparison();
       test_descending_comparison();
       test_integer_comparison();
       test_search();
       test_remove();

       test_move_constructor();
       test_move_assignment();
       test_return_value_optimization();

       test_generic_foreach();
       test_generic_foreach_functor();
       test_generic_firstthat();
       test_generic_foreach_multiparams();
       test_oldstyle_vs_generic();

       test_iterator_basic();
       test_iterator_empty_tree();
       test_iterator_single_element();
       test_iterator_with_stl_algorithms();
       test_iterator_large_tree();
       test_iterator_modification();
       test_iterator_comparison();

       test_reverse_iterator_basic();
       test_reverse_iterator_decrement();
       test_reverse_iterator_empty_tree();
       test_reverse_iterator_single_element();
       test_reverse_iterator_with_stl_algorithms();
       test_bidirectional_navigation();
       test_reverse_iterator_modification();
       test_reverse_iterator_large_tree();

       test_operator_output_basic();
       test_operator_output_empty_tree();
       test_operator_output_large_tree();
       test_operator_output_with_metadata();
       test_operator_output_to_file();
       test_operator_output_chaining();

       test_write_basic();
       test_write_empty_tree();
       test_write_large_tree();
       test_write_single_element();
       test_write_non_unique();
       test_write_file_error();

       test_read_basic();
       test_read_and_verify();
       test_read_large_tree();
       test_read_empty_tree();
       test_read_invalid_file();
       test_read_order_mismatch();
       test_write_read_cycle();

       return 0;

/*
       int result, i;
       BTree <char> bt (BTreeSize);
       for (i = 0; keys1[i]; i++)
       {
               //cout<<"Inserting "<<keys1[i]<<endl;
               result = bt.Insert(keys1[i], i*i);
               //bt.Print(cout);
       }
       bt.Print(cout);
       /*for (i = 0; keys2[i]; i++)
       {
               cout << "Searching " << keys2[i] << " ";
               long ObjID = bt.Search(keys2[i]);
               if( ObjID != -1 )
                       cout << "Achei " << keys2[i] << " ID = " << ObjID << endl;
               else
                       cout <<"Nao achei!" << keys2[i] << endl;
       }*/
       /*cout.flush();

       for (i = 0; keys3[i]; i++)
       {
               cout << "Removing " << keys3[i] << " ";
               if( bt.Remove(keys3[i], -1) )
                       cout << keys3[i] << " removido !" << endl;
               else
                       cout <<"Nao achei!" << keys3[i] << endl;
               bt.Print(cout);
       }
       bt.Print(cout);
       cout.flush();*/
       return 1;
}









/*const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys2="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const int BTreeSize = 3;
main (int argc, char * argv)
{
       //__int64 li;
       BTree <__int64> bt (BTreeSize);
       for (register int i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               bt.Insert(i, i-1);
               //bt.Print(cout);
       }

       for (i = 0; i < 1000; i++)
       {
               __int64 key = 975000+(::rand()%50000);
               //cout << "Searching " << (long)key << " ";
               long ObjID = bt.Search(key);
               if( ObjID != -1 )
                       cout << "Achei " << (long)key << " ID = " << ObjID << endl;
               else
                       cout <<"  Nao achei!" << (long)key << endl;
       }
       cout.flush();

       return 1;
}*/



/*const int BTreeSize = 3;
main (int argc, char * argv)
{
       int result, i;
       BTree <LONGLONG> bt(BTreeSize);
       result = bt.Create ("ernesto3-string-btree-start.dat",ios::in|ios::out);
       if (!result) { cout<<"Please delete testbt.dat"<<endl;return 0; }
       srand( (unsigned)time( NULL ) );
       LARGE_INTEGER key;
       for (i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               char strTmp[50];
               key.LowPart = rand();
               key.HighPart = rand();
               std::string str(strTmp);
               result = bt.Insert(key.QuadPart, i);
               //bt.Print(cout);
               if( i % 100000 == 0 )
               {       cout << i << endl; cout.flush();        }
       }
       //cout << "Searching D " << bt.Search();
       //bt.Search(1,1);
       cout.flush();
       return 1;
}*/
