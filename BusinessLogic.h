#pragma once

// Este archivo, BusinessLogic.h, define las estructuras de datos y la l�gica principal de la aplicaci�n.
// Aqu� residen las clases para el �rbol de navegaci�n y el crawler.

// Usaremos tipos de datos est�ndar de C++ y STL (Standard Template Library).
#include <string>
#include <vector>
#include <unordered_set> // Muy eficiente para guardar las URLs visitadas y evitar duplicados.

// --- Definiciones de Tipos y Enumeraciones ---

// Define el tipo de enlace para saber si debemos seguir explor�ndolo o no.
enum class LinkType {
    Internal, // Enlace dentro del mismo dominio.
    External  // Enlace a un dominio externo.
};

// Define el estado de una URL para saber si est� funcional.
enum class LinkStatus {
    OK,          // El enlace funciona correctamente.
    Broken,      // El enlace est� roto (ej. error 404).
    NotChecked   // A�n no se ha verificado (para enlaces externos).
};


// --- Clase WebNode: Representa un nodo en nuestro �rbol de navegaci�n ---

class WebNode {
public:
    // --- Atributos del Nodo ---
    std::string url;                      // La URL completa de esta p�gina.
    LinkType type;                        // Tipo de enlace (interno o externo).
    LinkStatus status;                    // Estado del enlace (OK, Roto, etc.).
    int depth;                            // Profundidad del nodo en el �rbol (ra�z es 0).
    std::vector<WebNode*> children;       // Vector de punteros a los nodos hijos (los enlaces de esta p�gina).

    // --- Constructor y Destructor ---

    // Constructor: inicializa un nodo con su URL, tipo y profundidad.
    WebNode(const std::string& url, LinkType type, int depth)
        : url(url), type(type), depth(depth), status(LinkStatus::NotChecked) {
    }

    // Destructor: es crucial para liberar la memoria de los nodos hijos recursivamente.
    // Esto previene fugas de memoria (memory leaks).
    ~WebNode() {
        for (WebNode* child : children) {
            delete child; // Esto llamar� al destructor de cada hijo.
        }
    }

    // --- M�todos del Nodo ---

    // A�ade un nuevo nodo hijo a este nodo.
    void addChild(WebNode* child) {
        children.push_back(child);
    }
};


// --- Clase NavigationTree: Gestiona el �rbol completo y el proceso de crawling ---

class NavigationTree {
private:
    // --- Atributos del �rbol ---
    WebNode* root;                               // Puntero al nodo ra�z del �rbol.
    std::unordered_set<std::string> visitedUrls; // Conjunto para almacenar URLs ya visitadas y evitar ciclos.
    std::string baseDomain;                      // El dominio base para diferenciar enlaces internos/externos.

public:
    // --- Constructor y Destructor ---

    // Constructor: inicializa un �rbol vac�o.
    NavigationTree() : root(nullptr) {}

    // Destructor: libera la memoria del nodo ra�z (y por ende, de todo el �rbol).
    ~NavigationTree() {
        delete root;
    }

    // --- M�todos Principales (L�gica del Crawler) ---

    // El m�todo principal que inicia el proceso de crawling.
    // Recibe la URL inicial y la profundidad m�xima.
    // Por ahora, solo declaramos las funciones. Las implementaremos en el archivo .cpp.
    void startCrawling(const std::string& startUrl, int maxDepth);

private:
    // --- M�todos Auxiliares (ser�n privados) ---

    // Funci�n recursiva que explora una URL espec�fica.
    void crawl(WebNode* currentNode, int maxDepth);

    // Funci�n para descargar el contenido HTML de una URL (usar� cURL).
    std::string downloadHtml(const std::string& url);

    // Funci�n para extraer todos los enlaces de un bloque de HTML.
    std::vector<std::string> extractLinks(const std::string& htmlContent, const std::string& baseUrl);

    // Funci�n para obtener el dominio base de una URL.
    std::string getBaseDomain(const std::string& url);
};