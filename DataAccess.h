// DataAccess.h
#pragma once

#include "BusinessLogic.h"
#include <string>
#include <fstream>

class DataAccess {
public:
    static bool exportTreeToFile(WebNode* root, const std::string& filePath);
private:
    static void writeNodeToFile(std::ofstream& file, WebNode* node, int indentLevel);
};
