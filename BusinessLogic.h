#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <gumbo.h>
#include <queue>
#include <map>

class WebNode;

// --- Structs de Datos ---
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

// --- Enums ---
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

    // Variables para el algoritmo de dibujo
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
    std::vector<DrawableNodeInfo> drawableTreeCache;
    bool positionsCalculated = false;

    // Métodos existentes
    void crawl(WebNode* currentNode, int maxDepth);
    std::string downloadHtml(const std::string& url, long* http_code);
    void extractLinks(WebNode* node);
    std::string getBaseDomain(const std::string& url);
    void search_for_links(GumboNode* node, WebNode* parentNode);
    void countNodesRecursive(WebNode* node, AnalysisResult& result);
    void collectNodesToCheck(WebNode* node, std::vector<WebNode*>& nodesToCheck);

    // Métodos para el algoritmo de dibujo
    void firstWalk(WebNode* node, std::map<int, float>& next_x_at_level);
    void secondWalk(WebNode* node, float modifier);
    void calculateNodePositions();
    void populateDrawableTree(WebNode* node, std::vector<DrawableNodeInfo>& tree);

public:
    NavigationTree();
    ~NavigationTree();

    void startCrawling(const std::string& startUrl, int maxDepth);
    WebNode* getRoot();
    AnalysisResult getAnalysisResult();
    std::vector<std::string> checkAllLinksStatus();
    PathResult findShortestPathToKeyword(const std::string& keyword);
    const std::vector<DrawableNodeInfo>& getDrawableTree();
};

