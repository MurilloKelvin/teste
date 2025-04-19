#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex> // Adicionado para std::mutex

class Logger {
public:
    enum class LogLevel { INFO, WARNING, ERR }; // ERROR foi renomeado para ERR

    Logger(const std::string& filename);
    ~Logger();

    void log(LogLevel level, const std::string& message);

private:
    std::ofstream log_file; // Renomeado de 'file' para 'log_file'
    std::mutex log_mutex;   // Adicionado para sincronização

    std::string get_timestamp(); // Declaração da função get_timestamp
    std::string level_to_string(LogLevel level);
};

#endif // LOGGER_H