#include "common.h"

void print_error(const char *message) {
    printf("%s[错误]%s %s\n", COLOR_RED, COLOR_RESET, message);
}

void print_success(const char *message) {
    printf("%s[成功]%s %s\n", COLOR_GREEN, COLOR_RESET, message);
}

void print_warning(const char *message) {
    printf("%s[警告]%s %s\n", COLOR_YELLOW, COLOR_RESET, message);
}

void print_info(const char *message) {
    printf("%s[信息]%s %s\n", COLOR_CYAN, COLOR_RESET, message);
}

int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return (st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH);
    }
    return 0;
}

int is_archive(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *archives[] = {".zip", ".tar", ".gz", ".bz2", ".xz", ".7z", ".rar", ".deb", ".rpm"};
    for (int i = 0; i < 9; i++) {
        if (strcasecmp(ext, archives[i]) == 0) return 1;
    }
    return 0;
}

int is_image(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *images[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".svg", ".ico", ".webp"};
    for (int i = 0; i < 8; i++) {
        if (strcasecmp(ext, images[i]) == 0) return 1;
    }
    return 0;
}

int is_video(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *videos[] = {".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm", ".m4v"};
    for (int i = 0; i < 8; i++) {
        if (strcasecmp(ext, videos[i]) == 0) return 1;
    }
    return 0;
}

int is_audio(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *audios[] = {".mp3", ".wav", ".flac", ".aac", ".ogg", ".wma", ".m4a"};
    for (int i = 0; i < 7; i++) {
        if (strcasecmp(ext, audios[i]) == 0) return 1;
    }
    return 0;
}

int is_document(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *docs[] = {".pdf", ".doc", ".docx", ".txt", ".rtf", ".odt", ".ppt", ".pptx"};
    for (int i = 0; i < 8; i++) {
        if (strcasecmp(ext, docs[i]) == 0) return 1;
    }
    return 0;
}

int is_code_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *code[] = {".c", ".cpp", ".h", ".hpp", ".py", ".js", ".html", ".css", ".java", ".go", ".rs", ".php"};
    for (int i = 0; i < 12; i++) {
        if (strcasecmp(ext, code[i]) == 0) return 1;
    }
    return 0;
}

char* format_size(off_t size) {
    static char buffer[32];
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double fsize = size;
    
    while (fsize >= 1024 && unit < 4) {
        fsize /= 1024;
        unit++;
    }
    
    if (unit == 0) {
        snprintf(buffer, sizeof(buffer), "%ld %s", size, units[unit]);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f %s", fsize, units[unit]);
    }
    
    return buffer;
}

char* format_time(time_t time) {
    static char buffer[64];
    struct tm *tm_info = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm_info);
    return buffer;
}

char* get_file_icon(const char *filename, mode_t mode) {
    if (S_ISDIR(mode)) {
        return ICON_DIRECTORY;
    } else if (S_ISLNK(mode)) {
        return ICON_SYMLINK;
    } else if (is_executable(filename)) {
        return ICON_EXECUTABLE;
    } else if (is_archive(filename)) {
        return ICON_ARCHIVE;
    } else if (is_image(filename)) {
        return ICON_IMAGE;
    } else if (is_video(filename)) {
        return ICON_VIDEO;
    } else if (is_audio(filename)) {
        return ICON_AUDIO;
    } else if (is_document(filename)) {
        return ICON_DOCUMENT;
    } else if (is_code_file(filename)) {
        return ICON_CODE;
    } else {
        return ICON_FILE;
    }
}
