#ifndef SCRAPER_H
#define SCRAPER_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include <gumbo.h>
#include "logger.h"
#include "config.h"

class WebScraper {
public:
    WebScraper(const Config& config, Logger& logger);
    ~WebScraper();
    bool scrape();

private:
    Config config;
    Logger& logger;
    CURL* curl;

    struct ScrapedItem {
        std::string title;
        std::string price;
        std::string url;
    };

    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::string fetch_page(const std::string& url, int retries_left);
    std::vector<ScrapedItem> parse_mercado_livre(GumboNode* node);
    std::vector<ScrapedItem> parse_olx(GumboNode* node);
    void save_to_file(const std::vector<ScrapedItem>& items, const std::string& filename);
    std::string trim(const std::string& str);
    void search_node(GumboNode* node, const std::string& tag, const std::string& attribute, 
                     const std::string& value, std::vector<GumboNode*>& results);
};

#endif // SCRAPER_H