#include "BusinessLogic.h"
#include <iostream>
#include <curl/curl.h>
#include <algorithm>

// --- Implementación de WebNode ---
WebNode::WebNode(const std::string& url, LinkType type, int depth)
    : url(url), type(type), depth(depth), status(LinkStatus::NotChecked) {
}

WebNode::~WebNode() {
    for (WebNode* child : children) {
        delete child;
    }
}
void WebNode::addChild(WebNode* child) {
    children.push_back(child);
}

// --- Implementación de NavigationTree ---
NavigationTree::NavigationTree() : root(nullptr) {}
NavigationTree::~NavigationTree() {
    delete root;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string NavigationTree::downloadHtml(const std::string& url) {
    CURL* curl_handle = curl_easy_init();
    std::string readBuffer;
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK) {
            readBuffer = "";
        }
        curl_easy_cleanup(curl_handle);
    }
    return readBuffer;
}

void NavigationTree::search_for_links(GumboNode* node, WebNode* parentNode) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href && href->value && strlen(href->value) > 0) {
            std::string link_url = href->value;
            std::string parent_base_url = parentNode->url.substr(0, parentNode->url.find_last_of('/') + 1);

            if (link_url.rfind("//", 0) == 0) link_url = this->scheme + ":" + link_url;
            else if (link_url.rfind("/", 0) == 0) link_url = this->scheme + "://" + this->baseDomain + link_url;
            else if (link_url.rfind("http", 0) != 0) link_url = parent_base_url + link_url;

            if (link_url.length() > 1 && link_url.find('#') == (link_url.length() - 1)) link_url.pop_back();
            if (link_url.empty() || link_url.find("mailto:") == 0 || link_url.find("tel:") == 0) return;

            LinkType type = (link_url.find(this->baseDomain) != std::string::npos) ? LinkType::Internal : LinkType::External;
            WebNode* new_node = new WebNode(link_url, type, parentNode->depth + 1);
            parentNode->addChild(new_node);
        }
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_for_links(static_cast<GumboNode*>(children->data[i]), parentNode);
    }
}

void NavigationTree::extractLinks(WebNode* node) {
    std::string html = downloadHtml(node->url);
    if (html.empty()) {
        node->status = LinkStatus::Broken;
        return;
    }
    node->status = LinkStatus::OK;

    GumboOutput* output = gumbo_parse(html.c_str());
    if (output && output->root) {
        search_for_links(output->root, node);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
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

void NavigationTree::startCrawling(const std::string& startUrl, int maxDepth) {
    delete root;
    root = nullptr;
    visitedUrls.clear();

    size_t scheme_end = startUrl.find("://");
    this->scheme = (scheme_end != std::string::npos) ? startUrl.substr(0, scheme_end) : "http";

    baseDomain = getBaseDomain(startUrl);
    root = new WebNode(startUrl, LinkType::Internal, 0);
    crawl(root, maxDepth);
}

#include <algorithm> // Asegúrate de incluir esta cabecera para usar std::max

void NavigationTree::countNodesRecursive(WebNode* node, AnalysisResult& result) {
    if (!node) return;

    // Solo contamos como nodo válido si no está roto
    if (node->status != LinkStatus::Broken) {
        result.totalNodes++;
        if (node->depth > 0) {
            if (node->type == LinkType::Internal) result.internalLinks++;
            else result.externalLinks++;
        }
    }

    // Asegúrate de que std::max esté correctamente referenciado
    result.maxDepth = (std::max)(result.maxDepth, node->depth);

    for (WebNode* child : node->children) {
        countNodesRecursive(child, result);
    }
}

AnalysisResult NavigationTree::getAnalysisResult() {
    AnalysisResult result = {}; // Inicialización a ceros
    if (root) {
        countNodesRecursive(root, result);
    }
    return result;
}

WebNode* NavigationTree::getRoot() {
    return this->root;
}