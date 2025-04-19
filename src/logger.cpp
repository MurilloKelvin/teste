#include "logger.h"
#include <iostream>
#include <mutex> // Adicionado para std::mutex e std::lock_guard
#include <ctime>

Logger::Logger(const std::string& filename) {
    log_file.open(filename, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Falha ao abrir o arquivo de log: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (log_file.is_open()) log_file.close();
}

std::string Logger::get_timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERR:     return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_file.is_open()) {
        log_file << "[" << get_timestamp() << "] [" << level_to_string(level) << "] " << message << std::endl;
    }
    std::cout << "[" << get_timestamp() << "] [" << level_to_string(level) << "] " << message << std::endl;
}