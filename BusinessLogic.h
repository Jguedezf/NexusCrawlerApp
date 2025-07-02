#pragma once

// Este archivo, BusinessLogic.h, define las estructuras de datos y la lógica principal de la aplicación.
// Aquí residen las clases para el árbol de navegación y el crawler.

// Usaremos tipos de datos estándar de C++ y STL (Standard Template Library).
#include <string>
#include <vector>
#include <unordered_set> // Muy eficiente para guardar las URLs visitadas y evitar duplicados.

// --- Definiciones de Tipos y Enumeraciones ---

// Define el tipo de enlace para saber si debemos seguir explorándolo o no.
enum class LinkType {
    Internal, // Enlace dentro del mismo dominio.
    External  // Enlace a un dominio externo.
};

// Define el estado de una URL para saber si está funcional.
enum class LinkStatus {
    OK,          // El enlace funciona correctamente.
    Broken,      // El enlace está roto (ej. error 404).
    NotChecked   // Aún no se ha verificado (para enlaces externos).
};


// --- Clase WebNode: Representa un nodo en nuestro árbol de navegación ---

class WebNode {
public:
    // --- Atributos del Nodo ---
    std::string url;                      // La URL completa de esta página.
    LinkType type;                        // Tipo de enlace (interno o externo).
    LinkStatus status;                    // Estado del enlace (OK, Roto, etc.).
    int depth;                            // Profundidad del nodo en el árbol (raíz es 0).
    std::vector<WebNode*> children;       // Vector de punteros a los nodos hijos (los enlaces de esta página).

    // --- Constructor y Destructor ---

    // Constructor: inicializa un nodo con su URL, tipo y profundidad.
    WebNode(const std::string& url, LinkType type, int depth)
        : url(url), type(type), depth(depth), status(LinkStatus::NotChecked) {
    }

    // Destructor: es crucial para liberar la memoria de los nodos hijos recursivamente.
    // Esto previene fugas de memoria (memory leaks).
    ~WebNode() {
        for (WebNode* child : children) {
            delete child; // Esto llamará al destructor de cada hijo.
        }
    }

    // --- Métodos del Nodo ---

    // Añade un nuevo nodo hijo a este nodo.
    void addChild(WebNode* child) {
        children.push_back(child);
    }
};


// --- Clase NavigationTree: Gestiona el árbol completo y el proceso de crawling ---

class NavigationTree {
private:
    // --- Atributos del Árbol ---
    WebNode* root;                               // Puntero al nodo raíz del árbol.
    std::unordered_set<std::string> visitedUrls; // Conjunto para almacenar URLs ya visitadas y evitar ciclos.
    std::string baseDomain;                      // El dominio base para diferenciar enlaces internos/externos.

public:
    // --- Constructor y Destructor ---

    // Constructor: inicializa un árbol vacío.
    NavigationTree() : root(nullptr) {}

    // Destructor: libera la memoria del nodo raíz (y por ende, de todo el árbol).
    ~NavigationTree() {
        delete root;
    }

    // --- Métodos Principales (Lógica del Crawler) ---

    // El método principal que inicia el proceso de crawling.
    // Recibe la URL inicial y la profundidad máxima.
    // Por ahora, solo declaramos las funciones. Las implementaremos en el archivo .cpp.
    void startCrawling(const std::string& startUrl, int maxDepth);

private:
    // --- Métodos Auxiliares (serán privados) ---

    // Función recursiva que explora una URL específica.
    void crawl(WebNode* currentNode, int maxDepth);

    // Función para descargar el contenido HTML de una URL (usará cURL).
    std::string downloadHtml(const std::string& url);

    // Función para extraer todos los enlaces de un bloque de HTML.
    std::vector<std::string> extractLinks(const std::string& htmlContent, const std::string& baseUrl);

    // Función para obtener el dominio base de una URL.
    std::string getBaseDomain(const std::string& url);
};