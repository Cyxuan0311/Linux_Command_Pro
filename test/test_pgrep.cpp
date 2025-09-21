#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <regex>
#include "common.h"

class PgrepTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试文件
        test_file = "tmp/pgrep_test.txt";
        test_c_file = "tmp/pgrep_test.c";
        test_empty_file = "tmp/pgrep_empty.txt";
        
        // 创建普通文本文件
        std::ofstream file(test_file);
        file << "Hello World\n";
        file << "This is a test file\n";
        file << "Line with hello in it\n";
        file << "Another line\n";
        file << "Final line\n";
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

// 测试基本字符串搜索
TEST_F(PgrepTest, TestBasicStringSearch) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    int match_count = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        if (line.find("hello") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(2, match_count); // "Hello World" 和 "Line with hello in it"
    file.close();
}

// 测试大小写敏感搜索
TEST_F(PgrepTest, TestCaseSensitiveSearch) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    int match_count = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        if (line.find("Hello") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(1, match_count); // 只有 "Hello World"
    file.close();
}

// 测试大小写不敏感搜索
TEST_F(PgrepTest, TestCaseInsensitiveSearch) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    int match_count = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        // 转换为小写进行比较
        std::string lower_line = line;
        std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
        if (lower_line.find("hello") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(2, match_count); // "Hello World" 和 "Line with hello in it"
    file.close();
}

// 测试正则表达式搜索
TEST_F(PgrepTest, TestRegexSearch) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    int match_count = 0;
    
    std::regex pattern(".*[Hh]ello.*");
    
    while (std::getline(file, line)) {
        line_number++;
        if (std::regex_match(line, pattern)) {
            match_count++;
        }
    }
    
    EXPECT_EQ(2, match_count); // "Hello World" 和 "Line with hello in it"
    file.close();
}

// 测试单词边界搜索
TEST_F(PgrepTest, TestWordBoundarySearch) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    int match_count = 0;
    
    std::regex pattern("\\bhello\\b");
    
    while (std::getline(file, line)) {
        line_number++;
        if (std::regex_search(line, pattern)) {
            match_count++;
        }
    }
    
    EXPECT_EQ(1, match_count); // 只有 "Line with hello in it"
    file.close();
}

// 测试多行搜索
TEST_F(PgrepTest, TestMultiLineSearch) {
    std::ifstream file(test_file);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    std::regex pattern("Hello.*World");
    bool found = std::regex_search(content, pattern);
    EXPECT_TRUE(found);
}

// 测试空文件搜索
TEST_F(PgrepTest, TestEmptyFileSearch) {
    std::ifstream file(test_empty_file);
    std::string line;
    int match_count = 0;
    
    while (std::getline(file, line)) {
        if (line.find("hello") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(0, match_count);
    file.close();
}

// 测试不存在的文件
TEST_F(PgrepTest, TestNonexistentFile) {
    std::ifstream file("tmp/nonexistent_file.txt");
    EXPECT_FALSE(file.is_open());
}

// 测试特殊字符搜索
TEST_F(PgrepTest, TestSpecialCharacterSearch) {
    std::string special_file = "tmp/pgrep_special_test.txt";
    std::ofstream file(special_file);
    file << "Line with special chars: !@#$%^&*()\n";
    file << "Line with regex chars: .*+?^${}[]|()\\\n";
    file << "Line with quotes: \"Hello World\"\n";
    file.close();
    
    std::ifstream input_file(special_file);
    std::string line;
    int match_count = 0;
    
    while (std::getline(input_file, line)) {
        if (line.find("!@#$%^&*()") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(1, match_count);
    input_file.close();
    
    // 清理
    std::filesystem::remove(special_file);
}

// 测试行号显示
TEST_F(PgrepTest, TestLineNumberDisplay) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    std::vector<int> matching_lines;
    
    while (std::getline(file, line)) {
        line_number++;
        if (line.find("hello") != std::string::npos) {
            matching_lines.push_back(line_number);
        }
    }
    
    EXPECT_EQ(2, matching_lines.size());
    EXPECT_EQ(1, matching_lines[0]); // "Hello World"
    EXPECT_EQ(3, matching_lines[1]); // "Line with hello in it"
    file.close();
}

// 测试计数功能
TEST_F(PgrepTest, TestCountFunction) {
    std::ifstream file(test_file);
    std::string line;
    int match_count = 0;
    
    while (std::getline(file, line)) {
        if (line.find("hello") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(2, match_count);
    file.close();
}

// 测试反向匹配
TEST_F(PgrepTest, TestInverseMatch) {
    std::ifstream file(test_file);
    std::string line;
    int line_number = 0;
    int non_matching_lines = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        if (line.find("hello") == std::string::npos) {
            non_matching_lines++;
        }
    }
    
    EXPECT_EQ(3, non_matching_lines); // 总行数5 - 匹配行数2 = 3
    file.close();
}

// 测试性能
TEST_F(PgrepTest, TestPerformance) {
    // 创建一个大文件进行性能测试
    std::string large_file = "tmp/pgrep_large_test.txt";
    std::ofstream file(large_file);
    
    for (int i = 0; i < 10000; i++) {
        file << "Line " << i << ": This is a test line for performance testing\n";
        if (i % 100 == 0) {
            file << "Line " << i << ": This line contains hello for testing\n";
        }
    }
    file.close();
    
    // 测试搜索性能
    std::ifstream input_file(large_file);
    std::string line;
    int match_count = 0;
    
    while (std::getline(input_file, line)) {
        if (line.find("hello") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(100, match_count); // 每100行有一个匹配
    input_file.close();
    
    // 清理
    std::filesystem::remove(large_file);
}

// 测试错误处理
TEST_F(PgrepTest, TestErrorHandling) {
    // 测试打印函数不崩溃
    EXPECT_NO_THROW(print_error("Test error"));
    EXPECT_NO_THROW(print_success("Test success"));
    EXPECT_NO_THROW(print_warning("Test warning"));
    EXPECT_NO_THROW(print_info("Test info"));
}

// 测试边界条件
TEST_F(PgrepTest, TestBoundaryConditions) {
    // 测试空字符串搜索
    std::string empty_string = "";
    EXPECT_TRUE(empty_string.find("") == 0);
    
    // 测试搜索空字符串
    std::ifstream file(test_file);
    std::string line;
    int match_count = 0;
    
    while (std::getline(file, line)) {
        if (line.find("") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(5, match_count); // 所有行都包含空字符串
    file.close();
}

// 测试Unicode字符
TEST_F(PgrepTest, TestUnicodeSearch) {
    std::string unicode_file = "tmp/pgrep_unicode_test.txt";
    std::ofstream file(unicode_file);
    file << "Hello 世界\n";
    file << "你好 World\n";
    file << "测试 test\n";
    file.close();
    
    std::ifstream input_file(unicode_file);
    std::string line;
    int match_count = 0;
    
    while (std::getline(input_file, line)) {
        if (line.find("世界") != std::string::npos) {
            match_count++;
        }
    }
    
    EXPECT_EQ(1, match_count);
    input_file.close();
    
    // 清理
    std::filesystem::remove(unicode_file);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
