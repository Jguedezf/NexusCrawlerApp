#pragma once

#include "BusinessLogic.h"
#include <string>
#include <fstream>
#include <vector>

// Clase para operaciones de acceso a datos y exportación
class DataAccess {
public:
    // Exporta el árbol de navegación a un archivo HTML
    static bool exportTreeToHtml(WebNode* root, const std::string& filePath, const std::string& analysisUrl, int requestedDepth);
private:
    // Escribe recursivamente los nodos en formato de lista HTML
    static void writeNodeToList(std::ofstream& file, WebNode* node);
    // Escribe los datos de los nodos en formato JS para visualización
    static void writeNodeDataForJs(std::ofstream& file, WebNode* node, int& nodeIdCounter);
};