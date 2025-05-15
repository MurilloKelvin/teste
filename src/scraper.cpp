#include "scraper.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>



WebScraper::WebScraper(const Config& cfg, Logger& log) : config(cfg), logger(log) {
    curl = curl_easy_init(); // Inicializa o handle do libcurl
    if (!curl) {
        logger.log(Logger::LogLevel::ERR, "Falha ao inicializar libcurl");
    }
}

WebScraper::~WebScraper() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

// Callback para armazenar o conteúdo da página baixada
size_t WebScraper::write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t realsize = size * nmemb;
    userp->append((char*)contents, realsize);
    return realsize;
}

// Função para baixar uma página web com tentativas de retry
std::string WebScraper::fetch_page(const std::string& url, int retries_left) {
    std::string html_content;
    if (!curl) return "";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_content);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string error = "Falha ao baixar " + url + ": " + curl_easy_strerror(res);
        logger.log(Logger::LogLevel::ERR, error);
        if (retries_left > 0) {
            logger.log(Logger::LogLevel::WARNING, "Tentando novamente... Restam " + std::to_string(retries_left) + " tentativas");
            return fetch_page(url, retries_left - 1);
        }
        return "";
    }
    logger.log(Logger::LogLevel::INFO, "Página baixada com sucesso: " + url);
    return html_content;
}

// Função auxiliar para remover espaços em branco
std::string WebScraper::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, last - first + 1);
}

// Função recursiva para buscar nós no HTML (ajustada para lidar com múltiplas classes)
void WebScraper::search_node(GumboNode* node, const std::string& tag, const std::string& attribute,
                             const std::string& value, std::vector<GumboNode*>& results) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    const char* tag_name = gumbo_normalized_tagname(node->v.element.tag);
    if (tag_name && tag == tag_name) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attribute.c_str());
        if (attr) {
            std::string attr_value = attr->value;
            if (value.empty() || attr_value.find(value) != std::string::npos) { // Verifica se o valor contém a classe
                results.push_back(node);
            }
        }
    }

    for (unsigned int i = 0; i < node->v.element.children.length; ++i) {
        search_node(static_cast<GumboNode*>(node->v.element.children.data[i]), tag, attribute, value, results);
    }
}

// Parsing específico para Mercado Livre
std::vector<WebScraper::ScrapedItem> WebScraper::parse_mercado_livre(GumboNode* node) {
    std::vector<ScrapedItem> items;
    std::vector<GumboNode*> product_nodes;

    // Busca por elementos h3 com a classe poly-component__title-wrapper
    search_node(node, "h3", "class", "poly-component__title-wrapper", product_nodes);
    logger.log(Logger::LogLevel::INFO, "Número de itens encontrados no Mercado Livre: " + std::to_string(product_nodes.size()));

    for (auto* product : product_nodes) {
        ScrapedItem item;
        std::vector<GumboNode*> link_nodes, price_nodes;

        // Encontra o link dentro do h3
        search_node(product, "a", "class", "poly-component__title", link_nodes);
        if (!link_nodes.empty()) {
            // Extrai o título do conteúdo do link
            std::string title;
            for (unsigned int i = 0; i < link_nodes[0]->v.element.children.length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(link_nodes[0]->v.element.children.data[i]);
                if (child->type == GUMBO_NODE_TEXT) {
                    title += child->v.text.text;
                }
            }
            item.title = trim(title);
            logger.log(Logger::LogLevel::INFO, "Titulo encontrado: " + item.title);

            // Extrai o link do atributo href
            GumboAttribute* href = gumbo_get_attribute(&link_nodes[0]->v.element.attributes, "href");
            if (href) {
                item.url = href->value;
                logger.log(Logger::LogLevel::INFO, "Link encontrado: " + item.url);
            } else {
                logger.log(Logger::LogLevel::WARNING, "Link nao encontrado para um item no Mercado Livre");
                item.url = "N/A";
            }
        } else {
            logger.log(Logger::LogLevel::WARNING, "Link nao encontrado para um item no Mercado Livre");
            item.url = "N/A";
            item.title = "N/A";
        }

        // Encontra o preço (span com classe andes-money-amount__fraction)
        search_node(product->parent, "span", "class", "andes-money-amount__fraction", price_nodes);
        if (!price_nodes.empty()) {
            std::string price;
            for (unsigned int i = 0; i < price_nodes[0]->v.element.children.length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(price_nodes[0]->v.element.children.data[i]);
                if (child->type == GUMBO_NODE_TEXT) {
                    price += child->v.text.text;
                }
            }
            item.price = trim(price);
            logger.log(Logger::LogLevel::INFO, "Preco encontrado: " + item.price);
        } else {
            logger.log(Logger::LogLevel::WARNING, "Preco nao encontrado para um item no Mercado Livre");
            item.price = "N/A";
        }

        if (!item.title.empty() || !item.price.empty() || !item.url.empty()) {
            items.push_back(item);
            logger.log(Logger::LogLevel::INFO, "Item encontrado no Mercado Livre: " + item.title + " | " + item.price + " | " + item.url);
        }
    }
    return items;
}

// Parsing específico para OLX
std::vector<WebScraper::ScrapedItem> WebScraper::parse_olx(GumboNode* node) {
    std::vector<ScrapedItem> items;
    std::vector<GumboNode*> product_nodes;

    // Encontra os itens (li com classe sc-1fcmfeb-2 ou outra classe específica)
    search_node(node, "li", "class", "sc-1fcmfeb-2", product_nodes);
    logger.log(Logger::LogLevel::INFO, "Numero de itens encontrados na OLX: " + std::to_string(product_nodes.size()));

    if (product_nodes.empty()) {
        // Tenta outro seletor para itens (ex.: li com data-lid)
        search_node(node, "li", "data-lid", "", product_nodes);
        logger.log(Logger::LogLevel::INFO, "Numero de itens encontrados na OLX (usando data-lid): " + std::to_string(product_nodes.size()));
    }

    for (auto* product : product_nodes) {
        ScrapedItem item;
        std::vector<GumboNode*> title_nodes, price_nodes, link_nodes;

        // Encontra o título (h2 ou h6 com classe específica)
        search_node(product, "h2", "class", "sc-1fcmfeb-0", title_nodes);
        if (title_nodes.empty()) {
            search_node(product, "h6", "class", "ad-card-title", title_nodes);
        }
        if (!title_nodes.empty() && title_nodes[0]->v.element.children.length > 0) {
            std::string title;
            for (unsigned int i = 0; i < title_nodes[0]->v.element.children.length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(title_nodes[0]->v.element.children.data[i]);
                if (child->type == GUMBO_NODE_TEXT) {
                    title += child->v.text.text;
                }
            }
            item.title = trim(title);
            logger.log(Logger::LogLevel::INFO, "Titulo encontrado: " + item.title);
        } else {
            logger.log(Logger::LogLevel::WARNING, "Titulo nao encontrado para um item na OLX");
            item.title = "N/A";
        }

        // Encontra o preço (span com classe específica)
        search_node(product, "span", "class", "price", price_nodes);
        if (price_nodes.empty()) {
            search_node(product, "span", "class", "ad-card-price", price_nodes);
        }
        if (!price_nodes.empty() && price_nodes[0]->v.element.children.length > 0) {
            std::string price;
            for (unsigned int i = 0; i < price_nodes[0]->v.element.children.length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(price_nodes[0]->v.element.children.data[i]);
                if (child->type == GUMBO_NODE_TEXT) {
                    price += child->v.text.text;
                }
            }
            item.price = trim(price);
            logger.log(Logger::LogLevel::INFO, "Preco encontrado: " + item.price);
        } else {
            logger.log(Logger::LogLevel::WARNING, "Preco não encontrado para um item na OLX");
            item.price = "N/A";
        }

        // Encontra o link (procura por um elemento 'a' ancestral)
        GumboNode* link_node = product;
        while (link_node && link_node->type == GUMBO_NODE_ELEMENT && link_node->v.element.tag != GUMBO_TAG_A) {
            link_node = link_node->parent;
        }
        if (link_node && link_node->type == GUMBO_NODE_ELEMENT) {
            GumboAttribute* href = gumbo_get_attribute(&link_node->v.element.attributes, "href");
            if (href) {
                item.url = href->value;
                logger.log(Logger::LogLevel::INFO, "Link encontrado: " + item.url);
            } else {
                logger.log(Logger::LogLevel::WARNING, "Link nao encontrado para um item na OLX");
                item.url = "N/A";
            }
        } else {
            logger.log(Logger::LogLevel::WARNING, "Link nao encontrado para um item na OLX");
            item.url = "N/A";
        }

        // Adiciona o item mesmo que o título ou preço estejam faltando
        items.push_back(item);
        logger.log(Logger::LogLevel::INFO, "Item encontrado na OLX: " + item.title + " | " + item.price + " | " + item.url);
    }
    return items;
}

// Salva os dados extraídos em arquivo
void WebScraper::save_to_file(const std::vector<ScrapedItem>& items, const std::string& output) {
    // Caminho fixo de saída

    // Garante que o diretório de saída exista
    create_output_directory("output");

    // Verifica se há itens para salvar
    if (items.empty()) {
        logger.log(Logger::LogLevel::WARNING, "Nenhum item para salvar em: " + output);
        return;
    }

    // Tenta abrir o arquivo
    std::ofstream file(output, std::ios::out);
    if (!file.is_open()) {
        logger.log(Logger::LogLevel::ERR, "Falha ao abrir arquivo: " + output);
        return;
    }

    // Escreve os dados no arquivo
    for (const auto& item : items) {
        if (!item.title.empty() || !item.price.empty() || !item.url.empty()) {
            file << "Título: " << item.title << "\n"
                 << "Preço: " << item.price << "\n"
                 << "URL: " << item.url << "\n"
                 << "----------------------------------------\n";
            file.flush(); // Força escrita a cada item, opcional
        }
    }

    file.close();

    // Confirmação de sucesso
    if (std::filesystem::exists(output) && std::filesystem::file_size(output) > 0) {
        logger.log(Logger::LogLevel::INFO, "Dados salvos em: " + output);
    } else {
        logger.log(Logger::LogLevel::ERR, "Arquivo não foi salvo corretamente: " + output);
    }
}


void WebScraper::create_output_directory(const std::string& output) {
    try {
        if (!std::filesystem::exists(output)) {
            if (std::filesystem::create_directory(output)) {
                logger.log(Logger::LogLevel::INFO, "Diretorio criado: " + output);
            } else {
                logger.log(Logger::LogLevel::ERR, "Falha ao criar diretório: " + output);
            }
        } else {
            logger.log(Logger::LogLevel::INFO, "Diretorio ja existe: " + output);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        logger.log(Logger::LogLevel::ERR, "Erro ao manipular diretorio: " + std::string(e.what()));
    }
}

// Função principal de scraping
bool WebScraper::scrape() {
    if (!curl) return false;

    create_output_directory("output");

    for (const auto& site : config.get_sites()) {
        logger.log(Logger::LogLevel::INFO, "Iniciando scraping em: " + site.name);
        std::string html = fetch_page(site.url, config.get_max_retries());
        if (html.empty()) continue;

        GumboOutput* output = gumbo_parse(html.c_str());
        std::vector<ScrapedItem> items;

        if (site.name == "Mercado Livre") {
            items = parse_mercado_livre(output->root);
            save_to_file(items, "output/mercado_livre_data.txt");
        } else if (site.name == "OLX") {
            items = parse_olx(output->root);
            save_to_file(items, "output/olx_data.txt");
        }

        if (items.empty()) {
            logger.log(Logger::LogLevel::WARNING, "Nenhum item encontrado em: " + site.name);
        } else {
            save_to_file(items, site.output_file);
        }

        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }
    return true;
}