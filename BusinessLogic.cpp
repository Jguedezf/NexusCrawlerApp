#define NOMINMAX
#include "BusinessLogic.h"
#include <iostream>
#include <curl/curl.h>
#include <algorithm>
#include <vector>
#include <list>

struct RequestData {
    WebNode* node;
    CURL* handle;
};

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

std::string NavigationTree::downloadHtml(const std::string& url, long* http_code) {
    CURL* curl_handle = curl_easy_init();
    std::string readBuffer;
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback_to_string);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
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
    long http_code = 0;
    std::string html = downloadHtml(node->url, &http_code);
    if (http_code >= 400 || http_code < 0) {
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
    if (root) {
        delete root;
        root = nullptr;
    }
    visitedUrls.clear();
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

WebNode* NavigationTree::getRoot() {
    return this->root;
}

void NavigationTree::collectNodesToCheck(WebNode* node, std::vector<WebNode*>& nodesToCheck) {
    if (!node) return;
    if (node->status == LinkStatus::NotChecked) {
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

    if (nodesToCheck.empty()) {
        return brokenLinks;
    }

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
                auto req_it = std::find_if(requests.begin(), requests.end(), [easy_handle](const RequestData& r) { return r.handle == easy_handle; });
                if (req_it != requests.end()) {
                    long http_code = 0;
                    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
                    if (msg->data.result != CURLE_OK || http_code >= 400) {
                        req_it->node->status = LinkStatus::Broken;
                        brokenLinks.push_back(req_it->node->url);
                    }
                    else {
                        req_it->node->status = LinkStatus::OK;
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