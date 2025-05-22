#include "logger.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <ctime>

Logger::Logger(const std::string& output_path) {
    log_file.open(output_path, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "LOGGER_CONSTRUTOR_ERRO: Falha ao abrir o arquivo de log EM: '" << output_path << "'" << std::endl;

        std::cerr << "LOGGER_CONSTRUTOR_ERRO: Codigo do erro (errno): " << errno << " - Mensagem do sistema: " << strerror(errno) << std::endl;
    } else {

        std::cerr << "LOGGER_CONSTRUTOR_INFO: Arquivo de log ABERTO COM SUCESSO (ou ja estava aberto) EM: '" << output_path << "'" << std::endl;


        log_file << "[" << get_timestamp() << "] [CONSTRUTOR] Arquivo de log aberto/criado." << std::endl;
        log_file.flush();
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
        log_file.flush();
    }
    std::cout << "[" << get_timestamp() << "] [" << level_to_string(level) << "] " << message << std::endl;
}