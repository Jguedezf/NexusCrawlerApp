#include "BusinessLogic.h"

// Este archivo, BusinessLogic.cpp, contiene la implementación de los métodos
// declarados en BusinessLogic.h.

// --- Implementación de los Métodos de NavigationTree ---

void NavigationTree::startCrawling(const std::string& startUrl, int maxDepth) {
    // Lógica para iniciar el crawling.
    // 1. Limpiar cualquier árbol anterior.
    // 2. Establecer el nodo raíz.
    // 3. Llamar a la función recursiva crawl().
    // (Lo implementaremos en el siguiente paso)
}

void NavigationTree::crawl(WebNode* currentNode, int maxDepth) {
    // Lógica recursiva para explorar una página.
    // 1. Verificar si ya visitamos la URL o si excedimos la profundidad.
    // 2. Descargar HTML.
    // 3. Extraer enlaces.
    // 4. Crear nodos hijos y llamarse a sí misma para cada enlace interno.
    // (Lo implementaremos en el siguiente paso)
}

std::string NavigationTree::downloadHtml(const std::string& url) {
    // Aquí irá el código que utiliza la librería cURL para descargar una página web.
    // Devolverá el HTML como un string.
    // (Lo implementaremos después de instalar cURL)
    return "<html><body><h1>Página de prueba</h1><a href='/pagina2.html'>Enlace</a></body></html>"; // Valor temporal
}

std::vector<std::string> NavigationTree::extractLinks(const std::string& htmlContent, const std::string& baseUrl) {
    // Aquí irá el código para parsear el string de HTML y encontrar todas las etiquetas <a href="...">.
    // Devolverá un vector de strings con las URLs encontradas.
    // (Lo implementaremos en el siguiente paso)
    std::vector<std::string> links;
    return links; // Valor temporal
}

std::string NavigationTree::getBaseDomain(const std::string& url) {
    // Lógica para extraer el dominio de una URL. Ej: "https://www.uneg.edu.ve/postgrado" -> "uneg.edu.ve"
    // (Lo implementaremos en el siguiente paso)
    return "uneg.edu.ve"; // Valor temporal
}