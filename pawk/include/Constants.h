#pragma once

#include <string>

namespace pawk {

// 颜色定义
namespace Colors {
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";
    const std::string UNDERLINE = "\033[4m";
}

// 默认值
namespace Defaults {
    const std::string FIELD_SEPARATOR = "\\s+";
    const std::string OUTPUT_SEPARATOR = " ";
    const std::string RECORD_SEPARATOR = "\n";
    const std::string OUTPUT_RECORD_SEPARATOR = "\n";
    const bool COLOR_OUTPUT = true;
    const bool SHOW_LINE_NUMBERS = false;
    const bool SHOW_FIELD_NUMBERS = false;
}

// 版本信息
namespace Version {
    const std::string VERSION = "1.0.0";
    const std::string DESCRIPTION = "优化版 awk 命令";
    const std::string AUTHOR = "pawk team";
}

} // namespace pawk
