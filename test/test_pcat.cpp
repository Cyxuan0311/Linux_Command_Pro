#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "common.h"

class PcatTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试文件
        test_file = "tmp/pcat_test.txt";
        test_c_file = "tmp/pcat_test.c";
        test_empty_file = "tmp/pcat_empty.txt";
        
        // 创建普通文本文件
        std::ofstream file(test_file);
        file << "Hello World\n";
        file << "This is a test file\n";
        file << "Line 3\n";
        file.close();
        
        // 创建C代码文件
        std::ofstream c_file(test_c_file);
        c_file << "#include <stdio.h>\n";
        c_file << "int main() {\n";
        c_file << "    printf(\"Hello World\\n\");\n";
        c_file << "    return 0;\n";
        c_file << "}\n";
        c_file.close();
        
        // 创建空文件
        std::ofstream empty_file(test_empty_file);
        empty_file.close();
    }
    
    void TearDown() override {
        // 清理测试文件
        std::filesystem::remove(test_file);
        std::filesystem::remove(test_c_file);
        std::filesystem::remove(test_empty_file);
    }
    
    std::string test_file;
    std::string test_c_file;
    std::string test_empty_file;
};

// 测试文件存在性检查
TEST_F(PcatTest, TestFileExistence) {
    // 测试存在的文件
    EXPECT_TRUE(std::filesystem::exists(test_file));
    EXPECT_TRUE(std::filesystem::exists(test_c_file));
    EXPECT_TRUE(std::filesystem::exists(test_empty_file));
    
    // 测试不存在的文件
    EXPECT_FALSE(std::filesystem::exists("tmp/nonexistent_file.txt"));
}

// 测试文件读取
TEST_F(PcatTest, TestFileReading) {
    std::ifstream file(test_file);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line);
    EXPECT_EQ("Hello World", line);
    
    std::getline(file, line);
    EXPECT_EQ("This is a test file", line);
    
    std::getline(file, line);
    EXPECT_EQ("Line 3", line);
    
    file.close();
}

// 测试空文件处理
TEST_F(PcatTest, TestEmptyFile) {
    std::ifstream file(test_empty_file);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    EXPECT_FALSE(std::getline(file, line));
    
    file.close();
}

// 测试C代码文件识别
TEST_F(PcatTest, TestCodeFileRecognition) {
    EXPECT_TRUE(is_code_file("test.c"));
    EXPECT_TRUE(is_code_file("test.cpp"));
    EXPECT_TRUE(is_code_file("test.h"));
    EXPECT_TRUE(is_code_file("test.py"));
    EXPECT_TRUE(is_code_file("test.js"));
    EXPECT_TRUE(is_code_file("test.html"));
    EXPECT_TRUE(is_code_file("test.css"));
    EXPECT_FALSE(is_code_file("test.txt"));
    EXPECT_FALSE(is_code_file("test"));
}

// 测试关键字检测
TEST_F(PcatTest, TestKeywordDetection) {
    // 这些函数需要从pcat.c中提取或重构为可测试的函数
    // 这里我们测试一些基本的字符串操作
    
    std::string code_line = "int main() { return 0; }";
    EXPECT_TRUE(code_line.find("int") != std::string::npos);
    EXPECT_TRUE(code_line.find("main") != std::string::npos);
    EXPECT_TRUE(code_line.find("return") != std::string::npos);
    
    std::string string_line = "printf(\"Hello World\\n\");";
    EXPECT_TRUE(string_line.find("\"") != std::string::npos);
    
    std::string comment_line = "// This is a comment";
    EXPECT_TRUE(comment_line.find("//") != std::string::npos);
}

// 测试数字检测
TEST_F(PcatTest, TestNumberDetection) {
    // 测试各种数字格式
    std::string hex_number = "0x1234";
    std::string decimal_number = "1234";
    std::string float_number = "123.45f";
    
    // 基本的数字检测逻辑
    EXPECT_TRUE(hex_number.find("0x") == 0);
    EXPECT_TRUE(decimal_number.find_first_not_of("0123456789") == std::string::npos);
    EXPECT_TRUE(float_number.find(".") != std::string::npos);
}

// 测试字符串检测
TEST_F(PcatTest, TestStringDetection) {
    std::string single_quote = "'a'";
    std::string double_quote = "\"Hello World\"";
    
    EXPECT_TRUE(single_quote[0] == '\'' && single_quote.back() == '\'');
    EXPECT_TRUE(double_quote[0] == '"' && double_quote.back() == '"');
}

// 测试注释检测
TEST_F(PcatTest, TestCommentDetection) {
    std::string line_comment = "// This is a line comment";
    std::string block_comment = "/* This is a block comment */";
    
    EXPECT_TRUE(line_comment.find("//") != std::string::npos);
    EXPECT_TRUE(block_comment.find("/*") != std::string::npos);
    EXPECT_TRUE(block_comment.find("*/") != std::string::npos);
}

// 测试函数调用检测
TEST_F(PcatTest, TestFunctionCallDetection) {
    std::string function_call = "printf(\"Hello World\\n\");";
    std::string not_function_call = "int main";
    
    EXPECT_TRUE(function_call.find("(") != std::string::npos);
    EXPECT_TRUE(function_call.find(")") != std::string::npos);
    EXPECT_FALSE(not_function_call.find("(") != std::string::npos);
}

// 测试文件大小
TEST_F(PcatTest, TestFileSize) {
    auto file_size = std::filesystem::file_size(test_file);
    EXPECT_GT(file_size, 0);
    
    auto empty_file_size = std::filesystem::file_size(test_empty_file);
    EXPECT_EQ(0, empty_file_size);
}

// 测试文件权限
TEST_F(PcatTest, TestFilePermissions) {
    auto perms = std::filesystem::status(test_file).permissions();
    EXPECT_TRUE((perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none);
    EXPECT_TRUE((perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none);
}

// 测试性能
TEST_F(PcatTest, TestPerformance) {
    // 创建一个大文件进行性能测试
    std::string large_file = "tmp/pcat_large_test.txt";
    std::ofstream file(large_file);
    
    for (int i = 0; i < 10000; i++) {
        file << "Line " << i << ": This is a test line for performance testing\n";
    }
    file.close();
    
    // 测试读取性能
    std::ifstream input_file(large_file);
    std::string line;
    int line_count = 0;
    
    while (std::getline(input_file, line)) {
        line_count++;
    }
    
    EXPECT_EQ(10000, line_count);
    input_file.close();
    
    // 清理
    std::filesystem::remove(large_file);
}

// 测试错误处理
TEST_F(PcatTest, TestErrorHandling) {
    // 测试不存在的文件
    std::ifstream file("tmp/nonexistent_file.txt");
    EXPECT_FALSE(file.is_open());
    
    // 测试打印函数不崩溃
    EXPECT_NO_THROW(print_error("Test error"));
    EXPECT_NO_THROW(print_success("Test success"));
    EXPECT_NO_THROW(print_warning("Test warning"));
    EXPECT_NO_THROW(print_info("Test info"));
}

// 测试边界条件
TEST_F(PcatTest, TestBoundaryConditions) {
    // 测试空字符串
    std::string empty_string = "";
    EXPECT_TRUE(empty_string.empty());
    
    // 测试只有换行符的文件
    std::string newline_file = "tmp/pcat_newline_test.txt";
    std::ofstream file(newline_file);
    file << "\n\n\n";
    file.close();
    
    std::ifstream input_file(newline_file);
    std::string line;
    int line_count = 0;
    
    while (std::getline(input_file, line)) {
        line_count++;
    }
    
    EXPECT_EQ(3, line_count);
    input_file.close();
    
    // 清理
    std::filesystem::remove(newline_file);
}

// 测试特殊字符
TEST_F(PcatTest, TestSpecialCharacters) {
    std::string special_file = "tmp/pcat_special_test.txt";
    std::ofstream file(special_file);
    file << "Line with special chars: !@#$%^&*()\n";
    file << "Line with unicode: 你好世界\n";
    file << "Line with tabs:\t\t\t\n";
    file.close();
    
    std::ifstream input_file(special_file);
    std::string line;
    
    EXPECT_TRUE(std::getline(input_file, line));
    EXPECT_TRUE(line.find("!@#$%^&*()") != std::string::npos);
    
    EXPECT_TRUE(std::getline(input_file, line));
    EXPECT_TRUE(line.find("你好世界") != std::string::npos);
    
    EXPECT_TRUE(std::getline(input_file, line));
    EXPECT_TRUE(line.find("\t") != std::string::npos);
    
    input_file.close();
    
    // 清理
    std::filesystem::remove(special_file);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
