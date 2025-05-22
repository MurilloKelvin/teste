#include "config.h"

Config::Config() : max_retries(3), verbose(false) {

    add_site("Mercado Livre", "https://lista.mercadolivre.com.br/celulares", "output/mercado_livre_data.txt");
    add_site("OLX", "https://www.olx.com.br/autos-e-pecas/carros-vans-e-utilitarios", "output/olx_data.txt");
}

std::vector<Config::SiteConfig> Config::get_sites() const { return sites; }
int Config::get_max_retries() const { return max_retries; }
bool Config::get_verbose() const { return verbose; }

void Config::add_site(const std::string& name, const std::string& url, const std::string& output) {
    sites.push_back({name, url, output});
}

void Config::set_max_retries(int retries) { max_retries = retries; }
void Config::set_verbose(bool v) { verbose = v; }