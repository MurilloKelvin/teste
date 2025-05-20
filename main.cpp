#include "include/scraper.h"
#include <iostream>

int main() {
    Logger logger("output/mercado_livre_data.txt");
    Config config;


    config.add_site("Mercado Livre", "https://lista.mercadolivre.com.br/iphone", "output/mercado_livre_data.txt");
    config.add_site("OLX", "https://www.olx.com.br/autos-e-pecas/corolla", "output/olx_data.txt");
    config.set_verbose(true);

    WebScraper scraper(config, logger);
    if (!scraper.scrape()) {
        std::cerr << "Erro ao executar o PDS2-20251-TF-Web-Searcher" << std::endl;
        return 1;
    }

    scraper.scrape();

    std::cout << "Scraping concluído pelo Web-Searcher. Verifique os arquivos de saída." << std::endl;
    return 0;
}

// .\vcpkg install curl:x64-windows // .\vcpkg install gumbo:x64-windows