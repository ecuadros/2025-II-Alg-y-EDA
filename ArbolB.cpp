//tstbtree.cc
//Author: Chicana Díaz, Johan Pier
//Bachelor in Engineering of System
#include <time.h>
#include <stdlib.h>
#include <iostream>

#include "btree.h"
#include <string>

const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";

const int BTreeSize = 3;

template<typename T>
bool compararClaves(const T& a, const T& b) {
	typedef BTreeTrait<T, long> MyTrait;
	return MyTrait::isEqual(a, b);
}

int main (int argc, char * argv[]){
	int result, i;

	typedef BTreeTrait<char, long> CharTrait;

	char key1 = 'D';
	char key2 = 'D';
	char key3 = 'X';

	if (CharTrait::isEqual(key1, key2)) {
		std::cout << "'" << key1 << "' es igual a '" << key2 << "' usando CharTrait::isEqual()" << std::endl;
	}

	if (!CharTrait::isEqual(key1, key3)) {
		std::cout << "'" << key1 << "' NO es igual a '" << key3 << "' usando CharTrait::isEqual()" << std::endl;
	}

	if (compararClaves('A', 'A')) {
		std::cout << "'A' es igual a 'A' usando compararClaves()" << std::endl;
	}

	if (!compararClaves('A', 'B')) {
		std::cout << "'A' NO es igual a 'B' usando compararClaves()" << std::endl;
	}

	typedef BTreeTrait<int, long> IntTrait;

	if (IntTrait::isEqual(5, 5)) {
		std::cout << "5 es igual a 5 usando IntTrait::isEqual()" << std::endl;
	}

	if (!IntTrait::isEqual(5, 10)) {
		std::cout << "5 NO es igual a 10 usando IntTrait::isEqual()" << std::endl;
	}

	BTree<CharTrait> bt (BTreeSize);

	for (i = 0; i < 10 && keys1[i]; i++)  // Solo primeros 10 para prueba
	{
		std::cout << "Insertando '" << keys1[i] << "'" << std::endl;
		result = bt.Insert(keys1[i], i*i);
	}

	bt.Print(std::cout);

	return 0;
}
