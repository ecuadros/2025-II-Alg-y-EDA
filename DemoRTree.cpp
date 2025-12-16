#include <iostream>
#include <string>
#include <fstream>
#include "rtree.h"


struct MyTraits {
	using CoordinateType = int;
	using DataType = std::string;
	static const int D = 2; 
	static const int M = 4; 
	static const int m = 2; 
};

using MyTree = CRTree<MyTraits>;
using MyPoint = Point<int, 2>;
using MyRect = Rect<int, 2>;

void DemoRTree() {
	std::cout << "R-Tree" << std::endl << std::endl; 
	
	MyTree tree;

	std::cout << "Insert" << std::endl; 
	
	tree.insert(MyRect(MyPoint({0,0}), MyPoint({10,10})), "R1");
	std::cout << "R1" << std::endl;
	
	tree.insert(MyRect(MyPoint({20,20}), MyPoint({30,30})), "R2");
	std::cout << "R2" << std::endl;
	
	tree.insert(MyRect(MyPoint({5,5}), MyPoint({15,15})), "R3");
	std::cout << "R3" << std::endl;
	
	tree.insert(MyRect(MyPoint({100,100}), MyPoint({110,110})), "R4");
	std::cout << "R4" << std::endl;
	
	tree.insert(MyRect(MyPoint({2,2}), MyPoint({3,3})), "R5");
	std::cout << "R5" << std::endl;

	std::cout << "Insert R6 to split" << std::endl; 
	tree.insert(MyRect(MyPoint({50,50}), MyPoint({60,60})), "R6");
	std::cout << "R6" << std::endl;
	
	std::cout << "Insert R7" << std::endl; 
	tree.insert(MyRect(MyPoint({0,50}), MyPoint({10,60})), "R7");
	std::cout << "R7" << std::endl << std::endl;

	std::cout << "Range Search" << std::endl; 
	MyRect query(MyPoint({0,0}), MyPoint({12,12}));
	std::cout << "Query: " << query << std::endl; 
	
	auto results = tree.search(query);
	std::cout << "Found " << results.size() << " items: "; 
	for(const auto& id : results) std::cout << id << " "; 
	std::cout << std::endl << std::endl; 

	std::cout << "Saving to file" << std::endl; 
	std::ofstream ofs("rtree2_save.txt"); 
	tree.Write(ofs);
	ofs.close();
	std::cout << std::endl;

	std::cout << "Loading from file" << std::endl; 
	MyTree tree2;
	std::ifstream ifs("rtree2_save.txt"); 
	tree2.Read(ifs);
	ifs.close();
	
	std::cout << "Querying loaded tree" << std::endl; 
	results = tree2.search(query);
	std::cout << "Found " << results.size() << " items: "; 
	for(const auto& id : results) std::cout << id << " "; 
	std::cout << std::endl << std::endl; 
	
	MyRect query2(MyPoint({90,90}), MyPoint({120,120}));
	std::cout << "Query2: " << query2 << std::endl; 
	results = tree2.search(query2);
	std::cout << "Found: "; 
	for(const auto& id : results) std::cout << id << " "; 
	std::cout << std::endl << std::endl; 

	std::cout << "Delete R1" << std::endl; 
	bool removed = tree2.remove(MyRect(MyPoint({0,0}), MyPoint({10,10})), "R1");
	std::cout << "Remove status: " << (removed ? "Success" : "Failed") << std::endl; 
	
	std::cout << "Query after delete" << std::endl;
	results = tree2.search(query);
	std::cout << "Found after delete (Query 1): " << results.size() << " items: "; 
	for(const auto& id : results) std::cout << id << " "; 
	std::cout << std::endl << std::endl; 
}
