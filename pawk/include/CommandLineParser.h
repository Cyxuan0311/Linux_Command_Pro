#pragma once

#include <string>
#include <vector>
#include <map>

namespace pawk {

struct CommandLineOptions {
    std::string program;
    std::string field_separator;
    std::string output_separator;
    std::string record_separator;
    std::string output_record_separator;
    bool color_output;
    bool show_line_numbers;
    bool show_field_numbers;
    bool show_help;
    bool show_version;
    bool verbose;
    bool debug;
    std::vector<std::string> input_files;
    std::map<std::string, std::string> variables;
    std::string program_file;
};

class CommandLineParser {
private:
    std::vector<std::string> args_;
    CommandLineOptions options_;
    
public:
    CommandLineParser(int argc, char* argv[]);
    
    void parse();
    const CommandLineOptions& getOptions() const;
    
    // 帮助和版本信息
    void printHelp() const;
    void printVersion() const;
    
private:
    void parseOption(const std::string& arg, size_t& index);
    void parseVariableAssignment(const std::string& assignment);
    void validateOptions();
};

} // namespace pawk
