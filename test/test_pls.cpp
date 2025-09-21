#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <filesystem>
#include "common.h"

class PlsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录结构
        test_dir = "tmp/pls_test_dir";
        std::filesystem::create_directories(test_dir);
        
        // 创建测试文件
        std::ofstream file1(test_dir + "/test.txt");
        file1 << "Hello World";
        file1.close();
        
        std::ofstream file2(test_dir + "/test.c");
        file2 << "#include <stdio.h>\nint main() { return 0; }";
        file2.close();
        
        std::ofstream file3(test_dir + "/test.jpg");
        file3 << "fake image data";
        file3.close();
        
        // 创建子目录
        std::filesystem::create_directories(test_dir + "/subdir");
        std::ofstream file4(test_dir + "/subdir/nested.txt");
        file4 << "nested file";
        file4.close();
        
        // 创建隐藏文件
        std::ofstream hidden_file(test_dir + "/.hidden");
        hidden_file << "hidden content";
        hidden_file.close();
    }
    
    void TearDown() override {
        // 清理测试目录
        std::filesystem::remove_all(test_dir);
    }
    
    std::string test_dir;
};

// 测试文件类型识别
TEST_F(PlsTest, TestFileTypeRecognition) {
    // 测试普通文件
    struct stat st;
    stat((test_dir + "/test.txt").c_str(), &st);
    const char* icon = get_file_icon("test.txt", st.st_mode);
    EXPECT_STREQ(ICON_FILE, icon);
    
    // 测试代码文件
    stat((test_dir + "/test.c").c_str(), &st);
    icon = get_file_icon("test.c", st.st_mode);
    EXPECT_STREQ(ICON_CODE, icon);
    
    // 测试图片文件
    stat((test_dir + "/test.jpg").c_str(), &st);
    icon = get_file_icon("test.jpg", st.st_mode);
    EXPECT_STREQ(ICON_IMAGE, icon);
    
    // 测试目录
    stat((test_dir + "/subdir").c_str(), &st);
    icon = get_file_icon("subdir", st.st_mode);
    EXPECT_STREQ(ICON_DIRECTORY, icon);
}

// 测试文件大小格式化
TEST_F(PlsTest, TestSizeFormatting) {
    char* result;
    
    // 测试小文件
    result = format_size(0);
    EXPECT_STREQ("0 B", result);
    
    result = format_size(1023);
    EXPECT_STREQ("1023 B", result);
    
    // 测试KB
    result = format_size(1024);
    EXPECT_STREQ("1.0 KB", result);
    
    result = format_size(1536);
    EXPECT_STREQ("1.5 KB", result);
    
    // 测试MB
    result = format_size(1024 * 1024);
    EXPECT_STREQ("1.0 MB", result);
    
    // 测试GB
    result = format_size(1024 * 1024 * 1024);
    EXPECT_STREQ("1.0 GB", result);
}

// 测试时间格式化
TEST_F(PlsTest, TestTimeFormatting) {
    time_t test_time = 1609459200; // 2021-01-01 00:00:00 UTC
    char* result = format_time(test_time);
    
    EXPECT_NE(nullptr, result);
    EXPECT_TRUE(strstr(result, "2021") != nullptr);
}

// 测试文件存在性检查
TEST_F(PlsTest, TestFileExistence) {
    // 测试存在的文件
    struct stat st;
    EXPECT_EQ(0, stat((test_dir + "/test.txt").c_str(), &st));
    EXPECT_TRUE(S_ISREG(st.st_mode));
    
    EXPECT_EQ(0, stat((test_dir + "/test.c").c_str(), &st));
    EXPECT_TRUE(S_ISREG(st.st_mode));
    
    EXPECT_EQ(0, stat((test_dir + "/test.jpg").c_str(), &st));
    EXPECT_TRUE(S_ISREG(st.st_mode));
    
    // 测试目录
    EXPECT_EQ(0, stat((test_dir + "/subdir").c_str(), &st));
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    
    // 测试不存在的文件
    EXPECT_NE(0, stat((test_dir + "/nonexistent.txt").c_str(), &st));
}

// 测试文件权限
TEST_F(PlsTest, TestFilePermissions) {
    struct stat st;
    stat((test_dir + "/test.txt").c_str(), &st);
    
    // 检查文件权限
    EXPECT_TRUE(st.st_mode & S_IRUSR); // 用户读权限
    EXPECT_TRUE(st.st_mode & S_IWUSR); // 用户写权限
}

// 测试文件类型检测函数
TEST_F(PlsTest, TestFileTypeDetection) {
    // 测试代码文件检测
    EXPECT_TRUE(is_code_file("test.c"));
    EXPECT_TRUE(is_code_file("test.cpp"));
    EXPECT_TRUE(is_code_file("test.h"));
    EXPECT_TRUE(is_code_file("test.py"));
    EXPECT_TRUE(is_code_file("test.js"));
    EXPECT_FALSE(is_code_file("test.txt"));
    
    // 测试图片文件检测
    EXPECT_TRUE(is_image("test.jpg"));
    EXPECT_TRUE(is_image("test.png"));
    EXPECT_TRUE(is_image("test.gif"));
    EXPECT_TRUE(is_image("test.svg"));
    EXPECT_FALSE(is_image("test.txt"));
    
    // 测试视频文件检测
    EXPECT_TRUE(is_video("test.mp4"));
    EXPECT_TRUE(is_video("test.avi"));
    EXPECT_TRUE(is_video("test.mkv"));
    EXPECT_FALSE(is_video("test.txt"));
    
    // 测试音频文件检测
    EXPECT_TRUE(is_audio("test.mp3"));
    EXPECT_TRUE(is_audio("test.wav"));
    EXPECT_TRUE(is_audio("test.flac"));
    EXPECT_FALSE(is_audio("test.txt"));
    
    // 测试文档文件检测
    EXPECT_TRUE(is_document("test.pdf"));
    EXPECT_TRUE(is_document("test.doc"));
    EXPECT_TRUE(is_document("test.txt"));
    EXPECT_FALSE(is_document("test.mp3"));
    
    // 测试压缩文件检测
    EXPECT_TRUE(is_archive("test.zip"));
    EXPECT_TRUE(is_archive("test.tar.gz"));
    EXPECT_TRUE(is_archive("test.7z"));
    EXPECT_FALSE(is_archive("test.txt"));
}

// 测试边界条件
TEST_F(PlsTest, TestBoundaryConditions) {
    // 测试空字符串
    EXPECT_FALSE(is_code_file(""));
    EXPECT_FALSE(is_image(""));
    EXPECT_FALSE(is_video(""));
    EXPECT_FALSE(is_audio(""));
    EXPECT_FALSE(is_document(""));
    EXPECT_FALSE(is_archive(""));
    
    // 测试只有扩展名的文件
    EXPECT_FALSE(is_code_file(".c"));
    EXPECT_FALSE(is_image(".jpg"));
    
    // 测试大小格式化边界值
    char* result = format_size(1024 * 1024 * 1024 - 1);
    EXPECT_NE(nullptr, result);
    
    result = format_size(1024 * 1024 * 1024);
    EXPECT_NE(nullptr, result);
}

// 测试性能
TEST_F(PlsTest, TestPerformance) {
    const char* test_files[] = {
        "test.c", "test.cpp", "test.h", "test.py", "test.js",
        "test.html", "test.css", "test.jpg", "test.png", "test.gif",
        "test.mp4", "test.avi", "test.mp3", "test.wav", "test.pdf",
        "test.doc", "test.zip", "test.tar.gz", "test.txt", "test"
    };
    
    const int num_files = sizeof(test_files) / sizeof(test_files[0]);
    
    // 测试文件类型检测性能
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < num_files; j++) {
            is_code_file(test_files[j]);
            is_image(test_files[j]);
            is_video(test_files[j]);
            is_audio(test_files[j]);
            is_document(test_files[j]);
            is_archive(test_files[j]);
        }
    }
}

// 测试错误处理
TEST_F(PlsTest, TestErrorHandling) {
    // 测试打印函数不崩溃
    EXPECT_NO_THROW(print_error("Test error"));
    EXPECT_NO_THROW(print_success("Test success"));
    EXPECT_NO_THROW(print_warning("Test warning"));
    EXPECT_NO_THROW(print_info("Test info"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
