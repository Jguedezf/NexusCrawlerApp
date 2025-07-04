// DataAccess.cpp
#include "DataAccess.h"

bool DataAccess::exportTreeToFile(WebNode* root, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    writeNodeToFile(file, root, 0);
    return true;
}

void DataAccess::writeNodeToFile(std::ofstream& file, WebNode* node, int indentLevel) {
    if (!node) {
        return;
    }
    file << std::string(indentLevel * 4, ' ') << "- " << node->url << "\n";
    for (WebNode* child : node->children) {
        writeNodeToFile(file, child, indentLevel + 1);
    }
}