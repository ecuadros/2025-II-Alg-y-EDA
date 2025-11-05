#ifndef __BTREE_FORMAT_H__
#define __BTREE_FORMAT_H__

#include <iostream>
#include <ios>
namespace btree_format {

// Format styles for BTree output
enum class FormatStyle {
    LINEAR,   
    COMPACT,    
    DETAILED,   
    TREE,  
    JSON_LIKE,   
    VERTICAL
};

// Internal index for storing format state in stream
inline int getFormatIndex() {
    static int index = std::ios_base::xalloc();
    return index;
}

// Base manipulator class
class FormatManipulator {
public:
    FormatStyle style;
    explicit FormatManipulator(FormatStyle s) : style(s) {}
};

// Manipulator application
inline std::ostream& operator<<(std::ostream& os, const FormatManipulator& manip) {
    os.iword(getFormatIndex()) = static_cast<long>(manip.style);
    return os;
}

// Helper function to get current format style
inline FormatStyle getFormatStyle(std::ostream& os) {
    long value = os.iword(getFormatIndex());
    return static_cast<FormatStyle>(value);
}

// Linear format
inline FormatManipulator linear() {
    return FormatManipulator(FormatStyle::LINEAR);
}

// Compact format
inline FormatManipulator compact() {
    return FormatManipulator(FormatStyle::COMPACT);
}

// Detailed format
inline FormatManipulator detailed() {
    return FormatManipulator(FormatStyle::DETAILED);
}

// Tree format
inline FormatManipulator tree() {
    return FormatManipulator(FormatStyle::TREE);
}

// JSON-like format
inline FormatManipulator json_like() {
    return FormatManipulator(FormatStyle::JSON_LIKE);
}

// Vertical format
inline FormatManipulator vertical() {
    return FormatManipulator(FormatStyle::VERTICAL);
}

}

#endif
