#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <gumbo.h>
#include <queue>
#include <map>

// Adelantamos la declaración de la clase para evitar dependencias circulares
class WebNode;

// --- Estructuras de Datos ---
struct AnalysisResult {
    int totalNodes = 0;
    int internalLinks = 0;
    int externalLinks = 0;
    int maxDepth = 0;
};

struct PathResult {
    bool found = false;
    std::vector<std::string> path;
};

struct DrawableNodeInfo {
    WebNode* nodePtr;
    float x;
    float y;
};

// --- Enumeraciones ---
enum class LinkType {
    Internal,
    External
};

enum class LinkStatus {
    OK,
    Broken,
    NotChecked
};

// --- Clases ---
class WebNode {
public:
    std::string url;
    LinkType type;
    LinkStatus status;
    int depth;
    std::vector<WebNode*> children;

    // Propiedades para el algoritmo de dibujo de árbol
    float x_pos = 0;
    float y_pos = 0;
    float modifier = 0;
    WebNode* thread = nullptr;
    WebNode* ancestor = nullptr;

    WebNode(const std::string& url, LinkType type, int depth);
    ~WebNode();
    void addChild(WebNode* child);
};

class NavigationTree {
private:
    WebNode* root;
    std::unordered_set<std::string> visitedUrls;
    std::string baseDomain;
    std::string scheme;
    bool bIncludeSubdomains;
    std::vector<DrawableNodeInfo> drawableTreeCache;
    bool positionsCalculated = false;

    // Métodos privados de crawling
    void crawl(WebNode* currentNode, int maxDepth);
    std::string downloadHtml(const std::string& url, long* http_code);
    void extractLinks(WebNode* node);
    std::string getBaseDomain(const std::string& url);
    void search_for_links(GumboNode* node, WebNode* parentNode);
    void countNodesRecursive(WebNode* node, AnalysisResult& result);
    void collectNodesToCheck(WebNode* node, std::vector<WebNode*>& nodesToCheck);

    // Métodos privados para calcular la posición de los nodos para el dibujo
    void firstWalk(WebNode* node);
    void secondWalk(WebNode* node, float modifier);
    void calculateNodePositions();
    void populateDrawableTree(WebNode* node, std::vector<DrawableNodeInfo>& tree);

public:
    NavigationTree();
    ~NavigationTree();

    void startCrawling(const std::string& startUrl, int maxDepth, bool includeSubdomains = true);
    WebNode* getRoot() const;
    AnalysisResult getAnalysisResult();
    std::vector<std::string> checkAllLinksStatus();
    PathResult findShortestPathToKeyword(const std::string& keyword);
    const std::vector<DrawableNodeInfo>& getDrawableTree() const;
};