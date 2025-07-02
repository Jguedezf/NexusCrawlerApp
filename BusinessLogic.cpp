#include "BusinessLogic.h"

// Este archivo, BusinessLogic.cpp, contiene la implementaci�n de los m�todos
// declarados en BusinessLogic.h.

// --- Implementaci�n de los M�todos de NavigationTree ---

void NavigationTree::startCrawling(const std::string& startUrl, int maxDepth) {
    // L�gica para iniciar el crawling.
    // 1. Limpiar cualquier �rbol anterior.
    // 2. Establecer el nodo ra�z.
    // 3. Llamar a la funci�n recursiva crawl().
    // (Lo implementaremos en el siguiente paso)
}

void NavigationTree::crawl(WebNode* currentNode, int maxDepth) {
    // L�gica recursiva para explorar una p�gina.
    // 1. Verificar si ya visitamos la URL o si excedimos la profundidad.
    // 2. Descargar HTML.
    // 3. Extraer enlaces.
    // 4. Crear nodos hijos y llamarse a s� misma para cada enlace interno.
    // (Lo implementaremos en el siguiente paso)
}

std::string NavigationTree::downloadHtml(const std::string& url) {
    // Aqu� ir� el c�digo que utiliza la librer�a cURL para descargar una p�gina web.
    // Devolver� el HTML como un string.
    // (Lo implementaremos despu�s de instalar cURL)
    return "<html><body><h1>P�gina de prueba</h1><a href='/pagina2.html'>Enlace</a></body></html>"; // Valor temporal
}

std::vector<std::string> NavigationTree::extractLinks(const std::string& htmlContent, const std::string& baseUrl) {
    // Aqu� ir� el c�digo para parsear el string de HTML y encontrar todas las etiquetas <a href="...">.
    // Devolver� un vector de strings con las URLs encontradas.
    // (Lo implementaremos en el siguiente paso)
    std::vector<std::string> links;
    return links; // Valor temporal
}

std::string NavigationTree::getBaseDomain(const std::string& url) {
    // L�gica para extraer el dominio de una URL. Ej: "https://www.uneg.edu.ve/postgrado" -> "uneg.edu.ve"
    // (Lo implementaremos en el siguiente paso)
    return "uneg.edu.ve"; // Valor temporal
}