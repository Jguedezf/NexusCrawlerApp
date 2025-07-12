#define NOMINMAX
#include "BusinessLogic.h"
#include <iostream>
#include <curl/curl.h>
#include <algorithm>
#include <vector>
#include <list>
#include <queue>
#include <map>

struct RequestData {
    WebNode* node;
    CURL* handle;
};

// --- Implementacion de WebNode ---
WebNode::WebNode(const std::string& url, LinkType type, int depth)
    : url(url), type(type), depth(depth), status(LinkStatus::NotChecked), ancestor(this), thread(nullptr), modifier(0.0f), x_pos(0.0f), y_pos(0.0f) {
}

WebNode::~WebNode() {
    for (WebNode* child : children) {
        delete child;
    }
}
void WebNode::addChild(WebNode* child) {
    children.push_back(child);
}

// --- Implementacion de NavigationTree ---
NavigationTree::NavigationTree() : root(nullptr) {}
NavigationTree::~NavigationTree() {
    if (root) {
        delete root;
    }
}

static size_t write_callback_to_string(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t write_callback_discard(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

// Ejemplo para limitar enlaces y poner timeout (ajusta según tu implementación real)
std::string NavigationTree::downloadHtml(const std::string& url, long* http_code) {
    CURL* curl_handle = curl_easy_init();
    std::string readBuffer;
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback_to_string);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L); // 10 segundos de timeout
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "NexusCrawler/1.0");
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode res = curl_easy_perform(curl_handle);
        if (http_code != nullptr) {
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, http_code);
        }
        if (res != CURLE_OK) {
            readBuffer = "";
            if (http_code != nullptr) *http_code = -1;
        }
        curl_easy_cleanup(curl_handle);
    }
    return readBuffer;
}

// Limitar la cantidad de enlaces por página
void NavigationTree::extractLinks(WebNode* node) {
    long http_code = 0;
    std::string html = downloadHtml(node->url, &http_code);
    if (http_code >= 400 || http_code < 0 || html.empty()) {
        node->status = LinkStatus::Broken;
        return;
    }
    node->status = LinkStatus::OK;
    GumboOutput* output = gumbo_parse(html.c_str());
    if (output && output->root) {
        std::vector<std::string> links;
        search_for_links(output->root, node);
        gumbo_destroy_output(&kGumboDefaultOptions, output);

        int maxLinks = 20; // máximo 20 enlaces por página
        int count = 0;
        for (WebNode* child : node->children) {
            if (count++ >= maxLinks) break;
            links.push_back(child->url);
        }
    }
}

void NavigationTree::search_for_links(GumboNode* node, WebNode* parentNode) {
    if (node->type != GUMBO_NODE_ELEMENT) return;
    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href && href->value && strlen(href->value) > 0) {
            std::string link_url_raw = href->value;
            if (link_url_raw.rfind("#", 0) == 0 || link_url_raw.rfind("javascript:", 0) == 0) {
                return;
            }
            std::string link_url = link_url_raw;
            std::string parent_base_url = parentNode->url.substr(0, parentNode->url.find_last_of('/') + 1);
            if (link_url.rfind("//", 0) == 0) link_url = this->scheme + ":" + link_url;
            else if (link_url.rfind("/", 0) == 0) link_url = this->scheme + "://" + this->baseDomain + link_url;
            else if (link_url.rfind("http", 0) != 0) link_url = parent_base_url + link_url;
            if (link_url.find('#') != std::string::npos) {
                link_url = link_url.substr(0, link_url.find('#'));
            }
            if (link_url.empty() || link_url.find("mailto:") == 0 || link_url.find("tel:") == 0) return;
            if (link_url == parentNode->url) {
                return;
            }
            std::string newLinkHost = getBaseDomain(link_url); 
            LinkType type = LinkType::External; 

            if (this->bIncludeSubdomains) {
                if (newLinkHost.length() >= this->baseDomain.length() &&
                    newLinkHost.compare(newLinkHost.length() - this->baseDomain.length(), this->baseDomain.length(), this->baseDomain) == 0) {
                    type = LinkType::Internal;
                }
            }
            else {
                if (newLinkHost == this->baseDomain) {
                    type = LinkType::Internal;
                }
            }

            WebNode* new_node = new WebNode(link_url, type, parentNode->depth + 1);
            parentNode->addChild(new_node);

        }
    }
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_for_links(static_cast<GumboNode*>(children->data[i]), parentNode);
    }
}

void NavigationTree::crawl(WebNode* currentNode, int maxDepth) {
    if (currentNode == nullptr || currentNode->depth >= maxDepth || visitedUrls.count(currentNode->url)) return;
    visitedUrls.insert(currentNode->url);
    extractLinks(currentNode);
    for (WebNode* child : currentNode->children) {
        if (child->type == LinkType::Internal) {
            crawl(child, maxDepth);
        }
    }
}

std::string NavigationTree::getBaseDomain(const std::string& url) {
    std::string domain;
    size_t start = url.find("://");
    start = (start == std::string::npos) ? 0 : start + 3;
    size_t end = url.find('/', start);
    domain = (end == std::string::npos) ? url.substr(start) : url.substr(start, end - start);
    if (domain.rfind("www.", 0) == 0) domain = domain.substr(4);
    return domain;
}

void NavigationTree::startCrawling(const std::string& startUrl, int maxDepth, bool includeSubdomains) {
    if (root) {
        delete root;
        root = nullptr;
    }
    visitedUrls.clear();
    positionsCalculated = false;
    drawableTreeCache.clear();

    this->bIncludeSubdomains = includeSubdomains;

    size_t scheme_end = startUrl.find("://");
    this->scheme = (scheme_end != std::string::npos) ? startUrl.substr(0, scheme_end) : "http";
    baseDomain = getBaseDomain(startUrl);
    root = new WebNode(startUrl, LinkType::Internal, 0);
    crawl(root, maxDepth);
}

void NavigationTree::countNodesRecursive(WebNode* node, AnalysisResult& result) {
    if (!node) return;
    if (node->status != LinkStatus::Broken) {
        result.totalNodes++;
        if (node->depth > 0) {
            if (node->type == LinkType::Internal) result.internalLinks++;
            else result.externalLinks++;
        }
    }
    result.maxDepth = (std::max)(result.maxDepth, node->depth);
    for (WebNode* child : node->children) {
        countNodesRecursive(child, result);
    }
}

AnalysisResult NavigationTree::getAnalysisResult() {
    AnalysisResult result = {};
    if (root) {
        countNodesRecursive(root, result);
    }
    return result;
}

WebNode* NavigationTree::getRoot() const {
    return this->root;
}

void NavigationTree::collectNodesToCheck(WebNode* node, std::vector<WebNode*>& nodesToCheck) {
    if (!node) return;
    if (node->depth > 0 && node->status == LinkStatus::NotChecked) {
        nodesToCheck.push_back(node);
    }
    for (WebNode* child : node->children) {
        collectNodesToCheck(child, nodesToCheck);
    }
}

std::vector<std::string> NavigationTree::checkAllLinksStatus() {
    std::vector<std::string> brokenLinks;
    if (!root) return brokenLinks;
    std::vector<WebNode*> nodesToCheck;
    collectNodesToCheck(root, nodesToCheck);
    if (nodesToCheck.empty()) return brokenLinks;
    CURLM* multi_handle = curl_multi_init();
    std::list<RequestData> requests;
    const int MAX_PARALLEL = 50;
    for (WebNode* node : nodesToCheck) {
        CURL* eh = curl_easy_init();
        if (eh) {
            curl_easy_setopt(eh, CURLOPT_URL, node->url.c_str());
            curl_easy_setopt(eh, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(eh, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(eh, CURLOPT_USERAGENT, "NexusCrawler/1.0 (LinkChecker)");
            curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(eh, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_callback_discard);
            curl_easy_setopt(eh, CURLOPT_WRITEDATA, NULL);
            curl_easy_setopt(eh, CURLOPT_PRIVATE, node);
            RequestData req = { node, eh };
            requests.push_back(req);
        }
    }
    auto it = requests.begin();
    int handles_to_add = std::min((int)requests.size(), MAX_PARALLEL);
    for (int i = 0; i < handles_to_add; ++i) {
        curl_multi_add_handle(multi_handle, it->handle);
        ++it;
    }
    int still_running;
    do {
        curl_multi_perform(multi_handle, &still_running);
        curl_multi_wait(multi_handle, NULL, 0, 100, NULL);
        CURLMsg* msg;
        int msgs_left;
        while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
            if (msg->msg == CURLMSG_DONE) {
                CURL* easy_handle = msg->easy_handle;
                WebNode* node_ptr = nullptr;
                curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &node_ptr);
                if (node_ptr) {
                    long http_code = 0;
                    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
                    if (msg->data.result != CURLE_OK || http_code >= 400) {
                        node_ptr->status = LinkStatus::Broken;
                        brokenLinks.push_back(node_ptr->url);
                    }
                    else {
                        node_ptr->status = LinkStatus::OK;
                    }
                }
                curl_multi_remove_handle(multi_handle, easy_handle);
                curl_easy_cleanup(easy_handle);
                if (it != requests.end()) {
                    curl_multi_add_handle(multi_handle, it->handle);
                    ++it;
                }
            }
        }
    } while (still_running > 0);
    curl_multi_cleanup(multi_handle);
    return brokenLinks;
}

PathResult NavigationTree::findShortestPathToKeyword(const std::string& keyword) {
    PathResult result;
    if (!root) {
        return result;
    }
    std::queue<WebNode*> q;
    std::map<WebNode*, WebNode*> parentMap;
    std::unordered_set<WebNode*> visitedNodes;
    q.push(root);
    visitedNodes.insert(root);
    parentMap[root] = nullptr;
    WebNode* foundNode = nullptr;
    while (!q.empty()) {
        WebNode* current = q.front();
        q.pop();
        std::string urlLower = current->url;
        std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        std::string keywordLower = keyword;
        std::transform(keywordLower.begin(), keywordLower.end(), keywordLower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        if (urlLower.find(keywordLower) != std::string::npos) {
            foundNode = current;
            break;
        }
        for (WebNode* child : current->children) {
            if (visitedNodes.find(child) == visitedNodes.end()) {
                visitedNodes.insert(child);
                parentMap[child] = current;
                q.push(child);
            }
        }
    }
    if (foundNode) {
        result.found = true;
        WebNode* curr = foundNode;
        while (curr != nullptr) {
            result.path.insert(result.path.begin(), curr->url);
            curr = parentMap[curr];
        }
    }
    return result;
}


// --- Lógica de Visualización del Árbol (Algoritmo de Reingold-Tilford CORREGIDO) ---

static WebNode* get_next_right(WebNode* n) {
    if (!n->thread) return nullptr;
    WebNode* t = n->thread;
    while (t->children.size() > 1 && t->depth == n->depth) t = t->children[t->children.size() - 1];
    return t;
}

static WebNode* get_next_left(WebNode* n) {
    if (!n->thread) return nullptr;
    WebNode* t = n->thread;
    while (t->children.size() > 0 && t->depth == n->depth) t = t->children[0];
    return t;
}

void NavigationTree::firstWalk(WebNode* node) {
    if (node->children.empty()) {
        return;
    }

    firstWalk(node->children[0]);
    WebNode* iyl = node->children[0];
    WebNode* iyr = node->children[0];

    for (size_t i = 1; i < node->children.size(); ++i) {
        WebNode* current_child = node->children[i];
        firstWalk(current_child);
        WebNode* oyl = node->children[i - 1];
        WebNode* oyr = node->children[i];

        float shift = (iyl->x_pos + 1.5f) - iyr->x_pos;
        if (shift > 0) {
            oyr->modifier += shift;
            oyr->x_pos += shift;
        }

        iyl = get_next_right(iyl) ? get_next_right(iyl) : oyl;
        iyr = get_next_left(iyr) ? get_next_left(iyr) : oyr;
    }

    float midpoint = (node->children.front()->x_pos + node->children.back()->x_pos) / 2.0f;
    node->x_pos = midpoint;
}

void NavigationTree::secondWalk(WebNode* node, float modifier) {
    node->x_pos += modifier;
    node->y_pos = (float)node->depth;
    for (WebNode* child : node->children) {
        secondWalk(child, modifier + node->modifier);
    }
}

void NavigationTree::calculateNodePositions() {
    if (!root || positionsCalculated) return;
    firstWalk(root);
    secondWalk(root, -root->x_pos);
    positionsCalculated = true;
}

void NavigationTree::populateDrawableTree(WebNode* node, std::vector<DrawableNodeInfo>& tree) {
    if (!node) return;

    DrawableNodeInfo dNode = {}; // Inicializar estructura
    dNode.nodePtr = node;
    dNode.x = node->x_pos;
    dNode.y = node->y_pos;
    tree.push_back(dNode);

    for (WebNode* child : node->children) {
        populateDrawableTree(child, tree);
    }
}

const std::vector<DrawableNodeInfo>& NavigationTree::getDrawableTree() const {
    if (!positionsCalculated) {
        const_cast<NavigationTree*>(this)->calculateNodePositions();
        const_cast<NavigationTree*>(this)->drawableTreeCache.clear();
        const_cast<NavigationTree*>(this)->populateDrawableTree(root, const_cast<NavigationTree*>(this)->drawableTreeCache);
    }
    return drawableTreeCache;
}