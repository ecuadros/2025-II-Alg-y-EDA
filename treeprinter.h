#ifndef TREE_UTILS_H
#define TREE_UTILS_H

#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <sstream>

// https://github.com/williamfiset/Algorithms/blob/master/src/main/java/com/williamfiset/algorithms/datastructures/utils/TreePrinter.java

template <typename Node>
std::string getTreeDisplay(Node* root) {
    if (root == nullptr) return "";

    // to build the string
    std::ostringstream sb;
    std::vector<std::vector<std::optional<std::string>>> lines;
    std::vector<Node*> level;
    std::vector<Node*> next;

    level.push_back(root);
    int nn = 1;
    size_t widest = 0; // track the widest node value

    while (nn != 0) {
        nn = 0;
        // store current level
        std::vector<std::optional<std::string>> line;
        for (Node* n : level) {
            if (n == nullptr) {
                line.push_back(std::nullopt);
                next.push_back(nullptr);
                next.push_back(nullptr);
            } else {
                std::ostringstream oss;
                oss << n->getData(); // data = node key
                if (n->getRef() != 0) { // ref = node value
                    oss << "(" << n->getRef() << ")";
                }
                std::string aa = oss.str();
                line.push_back(aa);
                if (aa.size() > widest) widest = aa.size();

                next.push_back(n->getpChildren()[0]);
                next.push_back(n->getpChildren()[1]);

                if (n->getpChildren()[0] != nullptr) nn++;
                if (n->getpChildren()[1] != nullptr) nn++;
            }
        }

        if (widest % 2 == 1) widest++;

        lines.push_back(std::move(line));
        level.swap(next);
        next.clear();
    }

    if (lines.empty()) return "";

    int perpiece = lines.back().size() * (widest + 4);
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        int hpw = static_cast<int>(std::floor(perpiece / 2.0)) - 1;
        if (i > 0) {
            for (size_t j = 0; j < line.size(); ++j) {

                // split node
                char c = ' ';
                if ((j & 1) == 1) {
                    if (line[j - 1].has_value()) {
                        c = (line[j].has_value()) ? '+' : '\\';
                    } else {
                        if (j < line.size() && line[j].has_value()) c = '/';
                    }
                }
                sb << c;

                if (!line[j].has_value()) {
                    for (int k = 0; k < perpiece - 1; ++k) sb << ' ';
                } else {
                    for (int k = 0; k < hpw; ++k) sb << ((j % 2 == 0) ? ' ' : '-');
                    sb << ((j % 2 == 0) ? '/' : '\\');
                    for (int k = 0; k < hpw; ++k) sb << ((j % 2 == 0) ? '-' : ' ');
                }
            }
            sb << '\n';
        }

        for (size_t j = 0; j < line.size(); ++j) {
            std::string f = line[j].has_value() ? *line[j] : std::string();
            int gap1 = std::ceil(perpiece / 2.0 - f.size() / 2.0);
            int gap2 = std::floor(perpiece / 2.0 - f.size() / 2.0);

            for (int k = 0; k < gap1; ++k) sb << ' ';
            sb << f;
            for (int k = 0; k < gap2; ++k) sb << ' ';
        }
        sb << '\n';

        perpiece /= 2;
    }

    return sb.str();
}

#endif // TREE_UTILS_H