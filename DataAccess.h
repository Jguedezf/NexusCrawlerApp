#pragma once

#include "BusinessLogic.h"
#include <string>
#include <fstream>
#include <vector>

// Clase para operaciones de acceso a datos y exportaci�n
class DataAccess {
public:
    // Exporta el �rbol de navegaci�n a un archivo HTML
    static bool exportTreeToHtml(WebNode* root, const std::string& filePath, const std::string& analysisUrl, int requestedDepth);
private:
    // Escribe recursivamente los nodos en formato de lista HTML
    static void writeNodeToList(std::ofstream& file, WebNode* node);
    // Escribe los datos de los nodos en formato JS para visualizaci�n
    static void writeNodeDataForJs(std::ofstream& file, WebNode* node, int& nodeIdCounter);
};