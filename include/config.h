#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

class Config {
public:
    Config();
    struct SiteConfig {
        std::string name;
        std::string url;
        std::string output_file;
    };

    std::vector<SiteConfig> get_sites() const;
    int get_max_retries() const;
    bool get_verbose() const;
    void add_site(const std::string& name, const std::string& url, const std::string& output_file);
    void set_max_retries(int retries);
    void set_verbose(bool verbose);

private:
    std::vector<SiteConfig> sites;
    int max_retries;
    bool verbose;
};

#endif // CONFIG_H