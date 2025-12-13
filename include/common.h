#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

// 颜色定义
#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// 背景颜色
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

// 样式
#define BOLD      "\033[1m"
#define DIM       "\033[2m"
#define ITALIC    "\033[3m"
#define UNDERLINE "\033[4m"

// 文件类型图标
#define ICON_DIRECTORY     "📁"
#define ICON_FILE          "📄"
#define ICON_EXECUTABLE    "⚡"
#define ICON_SYMLINK       "🔗"
#define ICON_ARCHIVE       "📦"
#define ICON_IMAGE         "🖼️"
#define ICON_VIDEO         "🎬"
#define ICON_AUDIO         "🎵"
#define ICON_DOCUMENT      "📝"
#define ICON_CODE          "💻"

// C/C++ 兼容
#ifdef __cplusplus
extern "C" {
#endif

// 工具函数
void print_error(const char *message);
void print_success(const char *message);
void print_warning(const char *message);
void print_info(const char *message);

// 文件类型检测
int is_executable(const char *path);
int is_archive(const char *filename);
int is_image(const char *filename);
int is_video(const char *filename);
int is_audio(const char *filename);
int is_document(const char *filename);
int is_code_file(const char *filename);

// 格式化函数
char* format_size(off_t size);
char* format_time(time_t time);
char* get_file_icon(const char *filename, mode_t mode);

#ifdef __cplusplus
}
#endif

#endif // COMMON_H
