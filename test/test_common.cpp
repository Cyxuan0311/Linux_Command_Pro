#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "common.h"

class CommonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的设置
    }
    
    void TearDown() override {
        // 测试后的清理
    }
};

// 测试文件类型检测函数
TEST_F(CommonTest, TestIsExecutable) {
    // 测试可执行文件检测
    EXPECT_FALSE(is_executable("test.txt"));
    EXPECT_FALSE(is_executable("nonexistent_file"));
}

TEST_F(CommonTest, TestIsArchive) {
    // 测试压缩文件检测
    EXPECT_TRUE(is_archive("test.zip"));
    EXPECT_TRUE(is_archive("test.tar.gz"));
    EXPECT_TRUE(is_archive("test.7z"));
    EXPECT_FALSE(is_archive("test.txt"));
    EXPECT_FALSE(is_archive("test"));
}

TEST_F(CommonTest, TestIsImage) {
    // 测试图片文件检测
    EXPECT_TRUE(is_image("test.jpg"));
    EXPECT_TRUE(is_image("test.png"));
    EXPECT_TRUE(is_image("test.gif"));
    EXPECT_TRUE(is_image("test.svg"));
    EXPECT_FALSE(is_image("test.txt"));
    EXPECT_FALSE(is_image("test"));
}

TEST_F(CommonTest, TestIsVideo) {
    // 测试视频文件检测
    EXPECT_TRUE(is_video("test.mp4"));
    EXPECT_TRUE(is_video("test.avi"));
    EXPECT_TRUE(is_video("test.mkv"));
    EXPECT_TRUE(is_video("test.webm"));
    EXPECT_FALSE(is_video("test.txt"));
    EXPECT_FALSE(is_video("test"));
}

TEST_F(CommonTest, TestIsAudio) {
    // 测试音频文件检测
    EXPECT_TRUE(is_audio("test.mp3"));
    EXPECT_TRUE(is_audio("test.wav"));
    EXPECT_TRUE(is_audio("test.flac"));
    EXPECT_TRUE(is_audio("test.ogg"));
    EXPECT_FALSE(is_audio("test.txt"));
    EXPECT_FALSE(is_audio("test"));
}

TEST_F(CommonTest, TestIsDocument) {
    // 测试文档文件检测
    EXPECT_TRUE(is_document("test.pdf"));
    EXPECT_TRUE(is_document("test.doc"));
    EXPECT_TRUE(is_document("test.docx"));
    EXPECT_TRUE(is_document("test.txt"));
    EXPECT_FALSE(is_document("test.mp3"));
    EXPECT_FALSE(is_document("test"));
}

TEST_F(CommonTest, TestIsCodeFile) {
    // 测试代码文件检测
    EXPECT_TRUE(is_code_file("test.c"));
    EXPECT_TRUE(is_code_file("test.cpp"));
    EXPECT_TRUE(is_code_file("test.h"));
    EXPECT_TRUE(is_code_file("test.py"));
    EXPECT_TRUE(is_code_file("test.js"));
    EXPECT_TRUE(is_code_file("test.html"));
    EXPECT_FALSE(is_code_file("test.txt"));
    EXPECT_FALSE(is_code_file("test"));
}

TEST_F(CommonTest, TestFormatSize) {
    // 测试大小格式化
    char* result;
    
    // 测试字节
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
    
    result = format_size(1024 * 1024 * 2);
    EXPECT_STREQ("2.0 MB", result);
    
    // 测试GB
    result = format_size(1024 * 1024 * 1024);
    EXPECT_STREQ("1.0 GB", result);
}

TEST_F(CommonTest, TestFormatTime) {
    // 测试时间格式化
    time_t test_time = 1609459200; // 2021-01-01 00:00:00 UTC
    char* result = format_time(test_time);
    
    // 检查结果不为空
    EXPECT_NE(nullptr, result);
    
    // 检查格式是否正确（应该包含年份）
    EXPECT_TRUE(strstr(result, "2021") != nullptr);
}

TEST_F(CommonTest, TestGetFileIcon) {
    // 测试文件图标获取
    const char* result;
    
    // 测试目录
    result = get_file_icon("test_dir", S_IFDIR);
    EXPECT_STREQ(ICON_DIRECTORY, result);
    
    // 测试符号链接
    result = get_file_icon("test_link", S_IFLNK);
    EXPECT_STREQ(ICON_SYMLINK, result);
    
    // 测试压缩文件
    result = get_file_icon("test.zip", S_IFREG);
    EXPECT_STREQ(ICON_ARCHIVE, result);
    
    // 测试图片文件
    result = get_file_icon("test.jpg", S_IFREG);
    EXPECT_STREQ(ICON_IMAGE, result);
    
    // 测试视频文件
    result = get_file_icon("test.mp4", S_IFREG);
    EXPECT_STREQ(ICON_VIDEO, result);
    
    // 测试音频文件
    result = get_file_icon("test.mp3", S_IFREG);
    EXPECT_STREQ(ICON_AUDIO, result);
    
    // 测试文档文件
    result = get_file_icon("test.pdf", S_IFREG);
    EXPECT_STREQ(ICON_DOCUMENT, result);
    
    // 测试代码文件
    result = get_file_icon("test.c", S_IFREG);
    EXPECT_STREQ(ICON_CODE, result);
    
    // 测试普通文件
    result = get_file_icon("test.txt", S_IFREG);
    EXPECT_STREQ(ICON_FILE, result);
}

// 测试错误处理函数
TEST_F(CommonTest, TestPrintFunctions) {
    // 这些函数主要测试是否能够正常调用而不崩溃
    // 在实际测试中，我们可能需要重定向stdout来捕获输出
    
    // 测试各种打印函数
    EXPECT_NO_THROW(print_error("Test error message"));
    EXPECT_NO_THROW(print_success("Test success message"));
    EXPECT_NO_THROW(print_warning("Test warning message"));
    EXPECT_NO_THROW(print_info("Test info message"));
}

// 性能测试
TEST_F(CommonTest, TestPerformance) {
    // 测试大量文件类型检测的性能
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

// 边界条件测试
TEST_F(CommonTest, TestBoundaryConditions) {
    // 测试空字符串
    EXPECT_FALSE(is_archive(""));
    EXPECT_FALSE(is_image(""));
    EXPECT_FALSE(is_video(""));
    EXPECT_FALSE(is_audio(""));
    EXPECT_FALSE(is_document(""));
    EXPECT_FALSE(is_code_file(""));
    
    // 测试只有扩展名的文件
    EXPECT_FALSE(is_archive(".zip"));
    EXPECT_FALSE(is_image(".jpg"));
    
    // 测试大小格式化边界值
    char* result = format_size(1024 * 1024 * 1024 - 1);
    EXPECT_NE(nullptr, result);
    
    result = format_size(1024 * 1024 * 1024);
    EXPECT_NE(nullptr, result);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
