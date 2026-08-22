#include <fstream>
#include <iostream>
#include <logger.hpp>
#include <string>

std::string level_prefix(Level level) {
    switch (level) {
    case 0:
        return "[Debug] ";
    case 1:
        return "[Info] ";
    case 2:
        return "[Warning] ";
    case 3:
        return "[Error] ";
    }
    return "[Info] ";
}

std::string Logger::form_message(std::string message, Level level) {
    std::string level_s = level_prefix(level);
    return level_s + message;
}

void Logger::log(std::string message, Level level) {
    if (min_level > level) {
        return;
    }
    if (log_file_enable) {
        std::ofstream outFile(log_file_path);
        outFile << form_message(message, level) << std::endl;
    }
    std::cout << form_message(message, level) << std::endl;
    return;
};
