#include "CommandLineParser.h"
#include "AwkEngine.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <fstream>

using namespace pawk;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        CommandLineParser parser(0, nullptr);
        parser.printHelp();
        return 1;
    }
    
    try {
        // 解析命令行参数
        CommandLineParser parser(argc, argv);
        const auto& options = parser.getOptions();
        
        if (options.show_help) {
            parser.printHelp();
            return 0;
        }
        
        if (options.show_version) {
            parser.printVersion();
            return 0;
        }
        
        // 创建AWK引擎
        AwkEngine engine(options);
        
        // 解析程序
        if (!options.program_file.empty()) {
            engine.parseProgramFile(options.program_file);
        } else if (!options.program.empty()) {
            engine.parseProgram(options.program);
        } else {
            Utils::printError("没有指定程序或程序文件");
            return 1;
        }
        
        // 处理输入文件
        if (options.input_files.empty()) {
            engine.processStdin();
        } else {
            for (const auto& file : options.input_files) {
                engine.processFile(file);
            }
        }
        
        // 显示统计信息（调试模式）
        if (options.debug) {
            std::cout << Utils::colorize("统计信息:", Colors::CYAN) << std::endl;
            std::cout << "  总记录数: " << engine.getTotalRecords() << std::endl;
            std::cout << "  处理记录数: " << engine.getProcessedRecords() << std::endl;
            std::cout << "  匹配记录数: " << engine.getMatchedRecords() << std::endl;
        }
        
    } catch (const std::exception& e) {
        Utils::printError("运行时错误: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Utils::printError("未知错误");
        return 1;
    }
    
    return 0;
}
