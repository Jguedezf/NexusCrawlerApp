#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <gumbo.h>

class WebNode;

struct AnalysisResult {
    int totalNodes = 0;
    int internalLinks = 0;
    int externalLinks = 0;
    int maxDepth = 0;
};

enum class LinkType {
    Internal,
    External
};

enum class LinkStatus {
    OK,
    Broken,
    NotChecked
};

class WebNode {
public:
    std::string url;
    LinkType type;
    LinkStatus status;
    int depth;
    std::vector<WebNode*> children;

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

    void crawl(WebNode* currentNode, int maxDepth);
    std::string downloadHtml(const std::string& url);
    void extractLinks(WebNode* node);
    std::string getBaseDomain(const std::string& url);
    void search_for_links(GumboNode* node, WebNode* parentNode);
    void countNodesRecursive(WebNode* node, AnalysisResult& result);

public:
    NavigationTree();
    ~NavigationTree();

    void startCrawling(const std::string& startUrl, int maxDepth);
    WebNode* getRoot();
    AnalysisResult getAnalysisResult();
};