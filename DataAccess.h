// DataAccess.h
#pragma once

#include "BusinessLogic.h"
#include <string>
#include <fstream>

class DataAccess {
public:
    static bool exportTreeToHtml(WebNode* root, const std::string& filePath, const std::string& analysisUrl, int requestedDepth);
private:
    // <<--- CORRECCI�N: Dejada la versi�n m�s simple y correcta con un solo par�metro ---
    static void writeNodeToHtml(std::ofstream& file, WebNode* node);
};