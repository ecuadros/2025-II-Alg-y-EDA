//tstbtree.cc
//Author: Chicana Díaz, Johan Pier
//Bachelor in Engineering of System
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

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
	int i;

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

	std::cout << "\n=== BTree con comparador por defecto (std::less) ===" << std::endl;
	BTree<CharTrait> bt (BTreeSize);

	for (i = 0; i < 10 && keys1[i]; i++)
	{
		std::cout << "Insertando '" << keys1[i] << "'" << std::endl;
		bt.Insert(keys1[i], i*i);
	}

	std::cout << "\n=== Usando operator<< ===" << std::endl;
	std::cout << bt;

	std::cout << "\n=== Forward Iterator ===" << std::endl;
	for(auto it = bt.begin(); it != bt.end(); ++it) {
		std::cout << it->key << "->" << it->ObjID << " ";
	}
	std::cout << std::endl;

	std::cout << "\n=== Backward Iterator ===" << std::endl;
	for(auto it = bt.rbegin(); it != bt.rend(); ++it) {
		std::cout << it->key << "->" << it->ObjID << " ";
	}
	std::cout << std::endl;

	std::cout << "\n=== ForEach: Contando elementos mayores a 'J' ===" << std::endl;
	int count = 0;
	bt.ForEach([](auto& obj, size_t level, int* counter) {
		if(obj.key > 'J') {
			(*counter)++;
		}
	}, &count);
	std::cout << "Elementos mayores a 'J': " << count << std::endl;

	std::cout << "\n=== ForEach: Imprimiendo con nivel de profundidad ===" << std::endl;
	bt.ForEach([](auto& obj, size_t level) {
		for(size_t i = 0; i < level; i++) std::cout << "  ";
		std::cout << obj.key << "->" << obj.ObjID << " (nivel " << level << ")" << std::endl;
	});

	std::cout << "\n=== FirstThat: Buscando primer elemento > 'K' ===" << std::endl;
	auto* found = bt.FirstThat([](auto& obj, size_t level) {
		return obj.key > 'K';
	});
	if(found) {
		std::cout << "Encontrado: " << found->key << "->" << found->ObjID << std::endl;
	} else {
		std::cout << "No encontrado" << std::endl;
	}

	std::cout << "\n=== FirstThat: Buscando elemento con ObjID == 16 ===" << std::endl;
	found = bt.FirstThat([](auto& obj, size_t level, long targetID) {
		return obj.ObjID == targetID;
	}, 16L);
	if(found) {
		std::cout << "Encontrado: " << found->key << "->" << found->ObjID << std::endl;
	} else {
		std::cout << "No encontrado" << std::endl;
	}

	std::cout << "\n=== Write: Guardando árbol en archivo ===" << std::endl;
	std::ofstream outFile("btree_data.txt");
	if(outFile) {
		bt.Write(outFile);
		outFile.close();
		std::cout << "Árbol guardado en btree_data.txt" << std::endl;
	}

	std::cout << "\n=== Read: Cargando árbol desde archivo ===" << std::endl;
	BTree<CharTrait> btLoaded(BTreeSize);
	std::ifstream inFile("btree_data.txt");
	if(inFile) {
		btLoaded.Read(inFile);
		inFile.close();
		std::cout << "Árbol cargado desde archivo:" << std::endl;
		std::cout << btLoaded;
	}

	std::cout << "\n=== Move Constructor ===" << std::endl;
	std::cout << "BTree original - size: " << bt.size() << ", height: " << bt.height() << std::endl;

	BTree<CharTrait> bt2(std::move(bt));

	std::cout << "BTree movido - size: " << bt2.size() << ", height: " << bt2.height() << std::endl;

	std::cout << "BTree original despues del move - size: " << bt.size() << ", height: " << bt.height() << std::endl;

	std::cout << "\nContenido del BTree movido (usando operator<<):" << std::endl;
	std::cout << bt2;

	std::cout << "\n=== Concurrencia: Inserciones y búsquedas paralelas ===" << std::endl;
	BTree<CharTrait> btConcurrent(BTreeSize);

	auto insertWorker = [&](int start, int count) {
		for(int i = 0; i < count; i++) {
			int idx = start + i;
			if(keys1[idx]) {
				btConcurrent.Insert(keys1[idx], idx * idx);
			}
		}
	};

	auto searchWorker = [&](int count) {
		for(int i = 0; i < count; i++) {
			if(keys1[i]) {
				btConcurrent.Search(keys1[i]);
			}
		}
	};

	std::vector<std::thread> threads;
	threads.push_back(std::thread(insertWorker, 0, 10));
	threads.push_back(std::thread(insertWorker, 10, 10));
	threads.push_back(std::thread(insertWorker, 20, 10));
	threads.push_back(std::thread(searchWorker, 30));

	for(auto& t : threads) {
		t.join();
	}

	std::cout << "BTree concurrente - size: " << btConcurrent.size() << std::endl;
	std::cout << "Contenido (usando operator<<):" << std::endl;
	std::cout << btConcurrent;

	return 0;
}
