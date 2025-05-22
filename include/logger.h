#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    enum class LogLevel { INFO, WARNING, ERR };

    Logger(const std::string& log_file);
    ~Logger();

    void log(LogLevel level, const std::string& message);

private:
    std::ofstream log_file;
    std::mutex log_mutex;

    std::string get_timestamp();
    std::string level_to_string(LogLevel level);
};

#endif // LOGGER_H