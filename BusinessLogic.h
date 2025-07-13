#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <gumbo.h>
#include <queue>
#include <map>

// Adelanto de clase para evitar dependencias circulares
class WebNode;

// --- Estructuras de Datos ---

// Resultado del análisis del árbol: nodos totales, enlaces internos/externos y profundidad máxima
struct AnalysisResult {
    int totalNodes = 0;
    int internalLinks = 0;
    int externalLinks = 0;
    int maxDepth = 0;
};

// Resultado de búsqueda de ruta: indica si se encontró y la ruta de URLs
struct PathResult {
    bool found = false;
    std::vector<std::string> path;
};

// Información para dibujar un nodo
struct DrawableNodeInfo {
    WebNode* nodePtr;
    float x;
    float y;
};

// --- Enumeraciones ---

// Tipo de enlace
enum class LinkType {
    Internal, // Enlace dentro del dominio base
    External  // Enlace fuera del dominio base
};

// Estado del enlace
enum class LinkStatus {
    OK,        // Enlace válido
    Broken,    // Enlace roto
    NotChecked // Estado no verificado
};

// --- Clases ---

// Nodo del árbol de navegación web
class WebNode {
public:
    std::string url;
    LinkType type;
    LinkStatus status;
    WebNode* parent;
    int depth;
    int js_id;
    std::vector<WebNode*> children;

    // Propiedades para el algoritmo de dibujo
    float x_pos = 0;
    float y_pos = 0;
    float modifier = 0;
    WebNode* thread = nullptr;
    WebNode* ancestor = nullptr;
    
    WebNode(const std::string& url, LinkType type, int depth, WebNode* parentNode = nullptr);
    ~WebNode();
    void addChild(WebNode* child); // Agrega hijo al nodo
};

// Árbol de navegación principal
class NavigationTree {
private:
    WebNode* root;
    std::unordered_set<std::string> visitedUrls; // URLs ya visitadas
    std::string baseDomain;
    std::string scheme;
    bool bIncludeSubdomains;
    std::vector<DrawableNodeInfo> drawableTreeCache;
    bool positionsCalculated = false;

    // Métodos internos de crawling y análisis
    void crawl(WebNode* currentNode, int maxDepth); // Recorrido recursivo
    std::string downloadHtml(const std::string& url, long* http_code); // Descarga HTML
    void extractLinks(WebNode* node); // Extrae enlaces de un nodo
    std::string getBaseDomain(const std::string& url); // Obtiene dominio base
    void search_for_links(GumboNode* node, WebNode* parentNode); // Busca enlaces en HTML
    void countNodesRecursive(WebNode* node, AnalysisResult& result); // Cuenta nodos y enlaces
    void collectNodesToCheck(WebNode* node, std::vector<WebNode*>& nodesToCheck); // Recolecta nodos para verificación

    // Métodos para calcular posiciones de nodos (dibujo)
    void firstWalk(WebNode* node); // Primer recorrido para layout
    void secondWalk(WebNode* node, float modifier); // Segundo recorrido para layout
    void calculateNodePositions(); // Calcula posiciones de todos los nodos
    void populateDrawableTree(WebNode* node, std::vector<DrawableNodeInfo>& tree); // Llena estructura para dibujar

public:
    NavigationTree();
    ~NavigationTree();

    // Inicia el crawling desde una URL base
    void startCrawling(const std::string& startUrl, int maxDepth, bool includeSubdomains = true);

    WebNode* getRoot() const;
    AnalysisResult getAnalysisResult();
    std::vector<std::string> checkAllLinksStatus();
    PathResult findShortestPathToKeyword(const std::string& keyword);
    const std::vector<DrawableNodeInfo>& getDrawableTree() const;
};