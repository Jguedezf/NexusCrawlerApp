// DataAccess.h
#pragma once

#include "BusinessLogic.h"
#include <string>
#include <fstream>
#include <vector>

class DataAccess {
public:
    static bool exportTreeToHtml(WebNode* root, const std::string& filePath, const std::string& analysisUrl, int requestedDepth);
private:
    // <<--- CORRECCIÓN: Dejada la versión más simple y correcta con un solo parámetro ---
    static void writeNodeToList(std::ofstream& file, WebNode* node);
    static void writeNodeDataForJs(std::ofstream& file, WebNode* node, int& nodeIdCounter);
};