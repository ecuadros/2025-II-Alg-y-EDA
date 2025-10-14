#include "BinaryTreeDemo.h"
#include "binarytree.h"
#include <vector>

void BinaryTreeDemo() {

	std::vector< std::pair<T1, Ref> > v1 = {
        {4, 8}, {2, 5}, {7, 3}, {1, 9}, {5, 14}
    };

	CBinaryTree<BinaryTreeAscTraits<int>> tree;
	for (auto &par : v1){
		tree.insert(par.first, par.second);
	}

	CBinaryTree<BinaryTreeAscTraits<int>> tree2(tree);

	for (auto _ = tree.begin(); _ != tree.end(); ++_){
		std::cout << *_ << std::endl;
	}

}