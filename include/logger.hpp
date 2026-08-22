#include <string>

enum Level { DEBUG, INFO, WARNING, ERROR };

struct Logger {
    Level min_level = INFO;
    bool log_file_enable = false;
    std::string log_file_path = "";

    std::string level_prefix(Level level);
    std::string form_message(std::string message, Level level);
    void log(std::string message, Level level = INFO);
};
