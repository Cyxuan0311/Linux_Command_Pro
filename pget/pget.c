/**
 * pget - 文件下载工具
 * 支持 HTTP/HTTPS/FTP 协议下载文件
 * 
 * 使用方法: pget [选项] <URL>
 * 
 * 作者: Linux Command Pro Team
 * 版本: 1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "../include/common.h"

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

#define MAX_URL_LENGTH 2048
#define MAX_FILENAME 512
#define BUFFER_SIZE 8192

typedef struct {
    FILE *file;
    long total_size;
    long downloaded;
    int show_progress;
    char filename[MAX_FILENAME];
    time_t start_time;
} download_info_t;

typedef struct {
    char url[MAX_URL_LENGTH];
    char output_file[MAX_FILENAME];
    char output_dir[MAX_FILENAME];
    int show_progress;
    int resume_download;
    int quiet;
    int verbose;
    int timeout;
    int max_redirects;
    int follow_location;
    int segment_count;
    char user_agent[256];
} download_options_t;

static download_info_t download_info = {0};
static int download_interrupted = 0;

// 信号处理函数
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        download_interrupted = 1;
        printf("\n\n%s⚠️  下载被中断%s\n", COLOR_YELLOW, COLOR_RESET);
    }
}

// 从URL提取文件名
int extract_filename_from_url(const char *url, char *filename, size_t size) {
    const char *last_slash = strrchr(url, '/');
    if (last_slash && last_slash[1] != '\0') {
        const char *name = last_slash + 1;
        // 移除查询参数
        const char *query = strchr(name, '?');
        if (query) {
            size_t len = query - name;
            if (len < size) {
                strncpy(filename, name, len);
                filename[len] = '\0';
                return 1;
            }
        } else {
            if (strlen(name) < size) {
                strncpy(filename, name, size - 1);
                filename[size - 1] = '\0';
                return 1;
            }
        }
    }
    return 0;
}

int build_output_path(download_options_t *options, char *output_path, size_t size) {
    char base_name[MAX_FILENAME];
    base_name[0] = '\0';

    if (options->output_file[0] != '\0') {
        strncpy(base_name, options->output_file, sizeof(base_name) - 1);
        base_name[sizeof(base_name) - 1] = '\0';
    } else {
        if (!extract_filename_from_url(options->url, base_name, sizeof(base_name))) {
            strncpy(base_name, "download", sizeof(base_name) - 1);
            base_name[sizeof(base_name) - 1] = '\0';
        }
    }

    if (options->output_dir[0] != '\0') {
        int needs_sep = options->output_dir[strlen(options->output_dir) - 1] == '/' ? 0 : 1;
        int written = snprintf(output_path, size, "%s%s%s",
                               options->output_dir,
                               needs_sep ? "/" : "",
                               base_name);
        if (written < 0 || (size_t)written >= size) {
            return 0;
        }
    } else {
        size_t length = strlen(base_name);
        if (length >= size) {
            return 0;
        }
        strncpy(output_path, base_name, size - 1);
        output_path[size - 1] = '\0';
    }

    return 1;
}

// 进度回调函数（libcurl）
#ifdef HAVE_LIBCURL
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    download_info_t *info = (download_info_t *)userp;
    
    if (info->file) {
        size_t written = fwrite(contents, size, nmemb, info->file);
        if (written != nmemb) {
            return 0; // 写入失败
        }
    }
    
    info->downloaded += realsize;
    
    // 显示进度
    if (info->show_progress && info->total_size > 0) {
        float percent = (float)info->downloaded * 100.0 / info->total_size;
        long elapsed = time(NULL) - info->start_time;
        long speed = elapsed > 0 ? info->downloaded / elapsed : 0;
        long remaining = speed > 0 ? (info->total_size - info->downloaded) / speed : 0;
        
        // 格式化大小
        char downloaded_str[32], total_str[32], speed_str[32];
        format_size_for_download((long long)info->downloaded, downloaded_str, sizeof(downloaded_str));
        format_size_for_download((long long)info->total_size, total_str, sizeof(total_str));
        format_size_for_download((long long)speed, speed_str, sizeof(speed_str));
        
        // 进度条
        int bar_width = 50;
        int filled = (int)(percent * bar_width / 100.0);
        
        printf("\r%s[", COLOR_CYAN);
        for (int i = 0; i < filled; i++) {
            printf("█");
        }
        for (int i = filled; i < bar_width; i++) {
            printf("░");
        }
        printf("%s] %.1f%% %s/%s %s/s ETA: %lds%s",
               COLOR_RESET, percent, downloaded_str, total_str, speed_str, remaining,
               COLOR_RESET);
        fflush(stdout);
    }
    
    return realsize;
}

static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                             curl_off_t ultotal, curl_off_t ulnow) {
    (void)ultotal;
    (void)ulnow;
    
    download_info_t *info = (download_info_t *)clientp;
    
    if (dltotal > 0) {
        info->total_size = (long)dltotal;
    }
    info->downloaded = (long)dlnow;
    
    if (download_interrupted) {
        return 1; // 中断下载
    }
    
    return 0;
}
#endif

// 格式化文件大小（用于下载进度显示）
void format_size_for_download(long long size, char *buffer, size_t buffer_size) {
    if (size < 1024) {
        snprintf(buffer, buffer_size, "%lld B", size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.2f KB", size / 1024.0);
    } else if (size < 1024LL * 1024LL * 1024LL) {
        snprintf(buffer, buffer_size, "%.2f MB", size / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_size, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
    }
}

// 使用 libcurl 下载文件
#ifdef HAVE_LIBCURL

typedef struct {
    CURL *handle;
    FILE *file;
    char temp_path[MAX_FILENAME];
    curl_off_t start;
    curl_off_t end;
    curl_off_t downloaded;
    curl_off_t total;
} segment_info_t;

static size_t segment_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    segment_info_t *segment = (segment_info_t *)userp;
    size_t realsize = size * nmemb;
    if (!segment->file) {
        return 0;
    }
    
    size_t written = fwrite(contents, size, nmemb, segment->file);
    if (written != nmemb) {
        return 0;
    }
    
    segment->downloaded += realsize;
    return realsize;
}

static void cleanup_segments(segment_info_t *segments, int segment_count, int remove_temp_files) {
    if (!segments) return;
    
    for (int i = 0; i < segment_count; ++i) {
        if (segments[i].file) {
            fclose(segments[i].file);
            segments[i].file = NULL;
        }
        if (segments[i].handle) {
            curl_easy_cleanup(segments[i].handle);
            segments[i].handle = NULL;
        }
        if (remove_temp_files && segments[i].temp_path[0] != '\0') {
            remove(segments[i].temp_path);
        }
    }
}

static void display_multi_progress(segment_info_t *segments,
                                   int segment_count,
                                   curl_off_t total_size,
                                   time_t start_time) {
    if (total_size <= 0 || !segments) {
        return;
    }
    
    curl_off_t total_downloaded = 0;
    for (int i = 0; i < segment_count; ++i) {
        total_downloaded += segments[i].downloaded;
    }
    
    double percent = (double)total_downloaded * 100.0 / (double)total_size;
    if (percent > 100.0) percent = 100.0;
    if (percent < 0.0) percent = 0.0;
    
    int bar_width = 50;
    int filled = (int)(percent * bar_width / 100.0);
    if (filled > bar_width) filled = bar_width;
    
    printf("\r%s[", COLOR_CYAN);
    for (int i = 0; i < filled; ++i) {
        printf("█");
    }
    for (int i = filled; i < bar_width; ++i) {
        printf("░");
    }
    printf("%s] %.1f%% ", COLOR_RESET, percent);
    
    char downloaded_str[32], total_str[32], speed_str[32];
    format_size_for_download((long long)total_downloaded, downloaded_str, sizeof(downloaded_str));
    format_size_for_download((long long)total_size, total_str, sizeof(total_str));
    
    time_t now = time(NULL);
    time_t elapsed = now > start_time ? now - start_time : 0;
    long long speed = 0;
    long long remaining_time = 0;
    if (elapsed > 0) {
        speed = (long long)(total_downloaded / elapsed);
        long long remaining = (long long)total_size - (long long)total_downloaded;
        if (speed > 0 && remaining > 0) {
            remaining_time = remaining / speed;
        }
    }
    
    format_size_for_download(speed, speed_str, sizeof(speed_str));
    printf("%s/%s %s/s", downloaded_str, total_str, speed_str);
    if (remaining_time > 0) {
        printf(" ETA: %llds", remaining_time);
    }
    
    fflush(stdout);
}

static int merge_segments(const char *output_path, segment_info_t *segments, int segment_count) {
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        fprintf(stderr, "%s❌ 错误: 无法创建输出文件 '%s': %s%s\n",
                COLOR_RED, output_path, strerror(errno), COLOR_RESET);
        return 0;
    }
    
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < segment_count; ++i) {
        FILE *part = fopen(segments[i].temp_path, "rb");
        if (!part) {
            fprintf(stderr, "%s❌ 错误: 无法读取分段文件 '%s'%s\n",
                    COLOR_RED, segments[i].temp_path, COLOR_RESET);
            fclose(out);
            return 0;
        }
        
        size_t read_size;
        while ((read_size = fread(buffer, 1, sizeof(buffer), part)) > 0) {
            size_t written = fwrite(buffer, 1, read_size, out);
            if (written != read_size) {
                fprintf(stderr, "%s❌ 错误: 合并分段数据失败%s\n", COLOR_RED, COLOR_RESET);
                fclose(part);
                fclose(out);
                return 0;
            }
        }
        fclose(part);
        
        // 删除临时文件
        remove(segments[i].temp_path);
    }
    
    fclose(out);
    return 1;
}

static int query_remote_file_size(download_options_t *options, curl_off_t *content_length) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return 0;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, options->url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, options->follow_location ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, (long)options->max_redirects);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)options->timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    if (options->user_agent[0] != '\0') {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, options->user_agent);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "pget/1.0");
    }
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return 0;
    }
    
    double length = -1.0;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    
    if (http_code >= 400 || length <= 0) {
        return 0;
    }
    
    *content_length = (curl_off_t)length;
    return 1;
}

static int prepare_segments(download_options_t *options,
                            const char *output_path,
                            segment_info_t *segments,
                            int segment_count,
                            curl_off_t total_size,
                            CURLM *multi_handle) {
    curl_off_t base_size = total_size / segment_count;
    curl_off_t remainder = total_size % segment_count;
    curl_off_t offset = 0;
    
    for (int i = 0; i < segment_count; ++i) {
        segments[i].start = offset;
        segments[i].end = offset + base_size - 1;
        if (i < remainder) {
            segments[i].end += 1;
        }
        if (segments[i].end >= total_size) {
            segments[i].end = total_size - 1;
        }
        segments[i].total = segments[i].end - segments[i].start + 1;
        segments[i].downloaded = 0;
        
        offset = segments[i].end + 1;
        
        snprintf(segments[i].temp_path, sizeof(segments[i].temp_path), "%s.part%d", output_path, i);
        segments[i].file = fopen(segments[i].temp_path, "wb");
        if (!segments[i].file) {
            fprintf(stderr, "%s❌ 错误: 无法创建分段文件 '%s': %s%s\n",
                    COLOR_RED, segments[i].temp_path, strerror(errno), COLOR_RESET);
            return 0;
        }
        
        segments[i].handle = curl_easy_init();
        if (!segments[i].handle) {
            fprintf(stderr, "%s❌ 错误: 无法初始化分段连接%s\n", COLOR_RED, COLOR_RESET);
            return 0;
        }
        
        char range_header[128];
        snprintf(range_header, sizeof(range_header), "%lld-%lld",
                 (long long)segments[i].start, (long long)segments[i].end);
        
        curl_easy_setopt(segments[i].handle, CURLOPT_URL, options->url);
        curl_easy_setopt(segments[i].handle, CURLOPT_WRITEFUNCTION, segment_write_callback);
        curl_easy_setopt(segments[i].handle, CURLOPT_WRITEDATA, &segments[i]);
        curl_easy_setopt(segments[i].handle, CURLOPT_RANGE, range_header);
        curl_easy_setopt(segments[i].handle, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(segments[i].handle, CURLOPT_FOLLOWLOCATION, options->follow_location ? 1L : 0L);
        curl_easy_setopt(segments[i].handle, CURLOPT_MAXREDIRS, (long)options->max_redirects);
        curl_easy_setopt(segments[i].handle, CURLOPT_TIMEOUT, (long)options->timeout);
        curl_easy_setopt(segments[i].handle, CURLOPT_CONNECTTIMEOUT, 30L);
        if (options->user_agent[0] != '\0') {
            curl_easy_setopt(segments[i].handle, CURLOPT_USERAGENT, options->user_agent);
        } else {
            curl_easy_setopt(segments[i].handle, CURLOPT_USERAGENT, "pget/1.0");
        }
        if (options->verbose) {
            curl_easy_setopt(segments[i].handle, CURLOPT_VERBOSE, 1L);
        }
        
        CURLMcode mc = curl_multi_add_handle(multi_handle, segments[i].handle);
        if (mc != CURLM_OK) {
            fprintf(stderr, "%s❌ 错误: 无法添加分段任务: %s%s\n",
                    COLOR_RED, curl_multi_strerror(mc), COLOR_RESET);
            return 0;
        }
    }
    
    return 1;
}

static int download_with_curl_single(download_options_t *options, const char *output_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "%s❌ 错误: 无法初始化 libcurl%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    FILE *file = NULL;
    long file_size = 0;
    
    if (options->resume_download) {
        struct stat st;
        if (stat(output_path, &st) == 0) {
            file_size = st.st_size;
            file = fopen(output_path, "ab");
            if (file) {
                curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)file_size);
                printf("%s📥 继续下载: %s (已下载: %ld 字节)%s\n",
                       COLOR_YELLOW, output_path, file_size, COLOR_RESET);
            }
        }
    }
    
    if (!file) {
        file = fopen(output_path, "wb");
        if (!file) {
            fprintf(stderr, "%s❌ 错误: 无法创建文件 '%s': %s%s\n",
                    COLOR_RED, output_path, strerror(errno), COLOR_RESET);
            curl_easy_cleanup(curl);
            return 1;
        }
    }
    
    download_info.file = file;
    download_info.total_size = 0;
    download_info.downloaded = file_size;
    download_info.show_progress = options->show_progress;
    strncpy(download_info.filename, output_path, sizeof(download_info.filename) - 1);
    download_info.filename[sizeof(download_info.filename) - 1] = '\0';
    download_info.start_time = time(NULL);
    
    curl_easy_setopt(curl, CURLOPT_URL, options->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &download_info);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &download_info);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, options->follow_location ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, (long)options->max_redirects);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)options->timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    if (options->user_agent[0] != '\0') {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, options->user_agent);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "pget/1.0");
    }
    if (options->verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    
    if (!options->quiet) {
        printf("%s📥 开始下载:%s\n", COLOR_CYAN, COLOR_RESET);
        printf("  URL: %s\n", options->url);
        printf("  保存到: %s\n", output_path);
        if (options->show_progress) {
            printf("\n");
        }
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (options->show_progress) {
        printf("\n");
    }
    
    if (res != CURLE_OK) {
        if (res == CURLE_ABORTED_BY_CALLBACK) {
            printf("%s⚠️  下载被用户中断%s\n", COLOR_YELLOW, COLOR_RESET);
        } else {
            fprintf(stderr, "%s❌ 下载失败: %s%s\n",
                    COLOR_RED, curl_easy_strerror(res), COLOR_RESET);
        }
        fclose(file);
        curl_easy_cleanup(curl);
        return 1;
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    fclose(file);
    curl_easy_cleanup(curl);
    
    if (!options->quiet) {
        if (http_code >= 200 && http_code < 300) {
            char size_str[32];
            format_size_for_download((long long)download_info.downloaded, size_str, sizeof(size_str));
            printf("%s✅ 下载完成!%s\n", COLOR_GREEN, COLOR_RESET);
            printf("  文件: %s\n", output_path);
            printf("  大小: %s\n", size_str);
            
            long elapsed = time(NULL) - download_info.start_time;
            if (elapsed > 0) {
                long speed = download_info.downloaded / elapsed;
                char speed_str[32];
                format_size_for_download((long long)speed, speed_str, sizeof(speed_str));
                printf("  速度: %s/s\n", speed_str);
                printf("  耗时: %ld 秒\n", elapsed);
            }
        } else {
            fprintf(stderr, "%s⚠️  HTTP 状态码: %ld%s\n",
                    COLOR_YELLOW, http_code, COLOR_RESET);
        }
    }
    
    return 0;
}

static int download_with_curl_multi(download_options_t *options,
                                    const char *output_path,
                                    curl_off_t total_size) {
    if (options->resume_download) {
        if (!options->quiet) {
            printf("%s⚠️  提示: 多连接下载暂不支持断点续传，已切换为单连接模式%s\n",
                   COLOR_YELLOW, COLOR_RESET);
        }
        return -1;
    }
    
    int segment_count = options->segment_count;
    if (segment_count < 2) {
        return -1;
    }
    if (segment_count > 16) {
        segment_count = 16;
    }
    if (total_size < (curl_off_t)segment_count) {
        segment_count = (int)total_size;
    }
    if (segment_count < 2) {
        if (!options->quiet) {
            printf("%s⚠️  文件过小，继续使用单连接下载%s\n", COLOR_YELLOW, COLOR_RESET);
        }
        return -1;
    }
    
    segment_info_t *segments = calloc(segment_count, sizeof(segment_info_t));
    if (!segments) {
        fprintf(stderr, "%s❌ 错误: 无法分配内存用于分段下载%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    CURLM *multi_handle = curl_multi_init();
    if (!multi_handle) {
        fprintf(stderr, "%s❌ 错误: 无法初始化多连接下载%s\n", COLOR_RED, COLOR_RESET);
        free(segments);
        return 1;
    }
    
    if (!prepare_segments(options, output_path, segments, segment_count, total_size, multi_handle)) {
        cleanup_segments(segments, segment_count, 1);
        curl_multi_cleanup(multi_handle);
        free(segments);
        return 1;
    }
    
    if (!options->quiet) {
        printf("%s⚡ 启动多连接下载 (%d 路)%s\n", COLOR_CYAN, segment_count, COLOR_RESET);
        printf("  URL: %s\n", options->url);
        printf("  保存到: %s\n", output_path);
        if (options->show_progress) {
            printf("\n");
        }
    }
    
    int still_running = 0;
    CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
    if (mc != CURLM_OK) {
        fprintf(stderr, "%s❌ 错误: %s%s\n", COLOR_RED, curl_multi_strerror(mc), COLOR_RESET);
        cleanup_segments(segments, segment_count, 1);
        curl_multi_cleanup(multi_handle);
        free(segments);
        return 1;
    }
    
    time_t start_time = time(NULL);
    time_t last_update = start_time;
    int result = 0;
    
    while (still_running && !download_interrupted) {
        mc = curl_multi_wait(multi_handle, NULL, 0, 1000, NULL);
        if (mc != CURLM_OK) {
            fprintf(stderr, "%s❌ 错误: %s%s\n", COLOR_RED, curl_multi_strerror(mc), COLOR_RESET);
            result = 1;
            break;
        }
        
        mc = curl_multi_perform(multi_handle, &still_running);
        if (mc != CURLM_OK) {
            fprintf(stderr, "%s❌ 错误: %s%s\n", COLOR_RED, curl_multi_strerror(mc), COLOR_RESET);
            result = 1;
            break;
        }
        
        if (options->show_progress) {
            time_t now = time(NULL);
            if (now != last_update || !still_running) {
                display_multi_progress(segments, segment_count, total_size, start_time);
                last_update = now;
            }
        }
    }
    
    if (options->show_progress) {
        display_multi_progress(segments, segment_count, total_size, start_time);
        printf("\n");
    }
    
    if (download_interrupted) {
        result = 1;
    }
    
    CURLMsg *msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE && msg->data.result != CURLE_OK) {
            fprintf(stderr, "%s❌ 分段下载失败: %s%s\n",
                    COLOR_RED, curl_easy_strerror(msg->data.result), COLOR_RESET);
            result = 1;
        }
    }
    
    int range_supported = 1;
    if (result == 0) {
        for (int i = 0; i < segment_count; ++i) {
            long response_code = 0;
            curl_easy_getinfo(segments[i].handle, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code != 206) {
                range_supported = 0;
                break;
            }
        }
    }
    
    curl_multi_cleanup(multi_handle);
    
    if (!range_supported) {
        if (!options->quiet) {
            printf("%s⚠️  服务端不支持分块下载，已切换为单连接模式%s\n",
                   COLOR_YELLOW, COLOR_RESET);
        }
        cleanup_segments(segments, segment_count, 1);
        free(segments);
        return -1;
    }
    
    if (result != 0) {
        cleanup_segments(segments, segment_count, 1);
        free(segments);
        return 1;
    }
    
    cleanup_segments(segments, segment_count, 0);
    
    if (!merge_segments(output_path, segments, segment_count)) {
        cleanup_segments(segments, segment_count, 1);
        free(segments);
        return 1;
    }
    
    if (!options->quiet) {
        char size_str[32];
        format_size_for_download((long long)total_size, size_str, sizeof(size_str));
        printf("%s✅ 下载完成!%s\n", COLOR_GREEN, COLOR_RESET);
        printf("  文件: %s\n", output_path);
        printf("  大小: %s\n", size_str);
        
        time_t elapsed = time(NULL) - start_time;
        if (elapsed > 0) {
            long long speed = (long long)(total_size / elapsed);
            char speed_str[32];
            format_size_for_download(speed, speed_str, sizeof(speed_str));
            printf("  平均速度: %s/s\n", speed_str);
            printf("  耗时: %lds\n", (long)elapsed);
            printf("  使用连接: %d\n", segment_count);
        }
    }
    
    free(segments);
    return 0;
}

int download_with_curl(download_options_t *options) {
    char output_path[MAX_FILENAME];
    if (!build_output_path(options, output_path, sizeof(output_path))) {
        fprintf(stderr, "%s❌ 错误: 无法构建输出路径%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    if (options->segment_count > 1) {
        curl_off_t content_length = 0;
        if (query_remote_file_size(options, &content_length)) {
            int multi_result = download_with_curl_multi(options, output_path, content_length);
            if (multi_result == 0) {
                return 0;
            } else if (multi_result > 0) {
                return multi_result;
            }
        } else if (!options->quiet) {
            printf("%s⚠️  无法获取文件大小，已切换为单连接模式%s\n",
                   COLOR_YELLOW, COLOR_RESET);
        }
    }
    
    return download_with_curl_single(options, output_path);
}
#else
// 如果没有 libcurl，显示错误信息
int download_with_curl(download_options_t *options) {
    (void)options;
    fprintf(stderr, "%s❌ 错误: pget 需要 libcurl 库支持%s\n", COLOR_RED, COLOR_RESET);
    fprintf(stderr, "请安装 libcurl 开发库:\n");
    fprintf(stderr, "  Ubuntu/Debian: sudo apt-get install libcurl4-openssl-dev\n");
    fprintf(stderr, "  CentOS/RHEL:   sudo yum install libcurl-devel\n");
    fprintf(stderr, "  Arch Linux:    sudo pacman -S curl\n");
    return 1;
}
#endif

// 显示帮助信息
void print_help(const char *program_name) {
    printf("🐧 pget - 文件下载工具\n");
    printf("====================\n\n");
    printf("使用方法: %s [选项] <URL>\n\n", program_name);
    printf("选项:\n");
    printf("  -h, --help           显示此帮助信息\n");
    printf("  -v, --version        显示版本信息\n");
    printf("  -o, --output FILE    指定输出文件名\n");
    printf("  -O, --output-dir DIR 指定输出目录\n");
    printf("  -c, --continue       断点续传\n");
    printf("  -q, --quiet          静默模式\n");
    printf("  -V, --verbose        详细输出\n");
    printf("  -P, --progress       显示进度条（默认）\n");
    printf("  -s, --segments N    启用多连接分段下载（默认: 1）\n");
    printf("  -t, --timeout SEC    超时时间（秒，默认: 0=无限制）\n");
    printf("  -L, --location       跟随重定向（默认）\n");
    printf("  -m, --max-redirects N 最大重定向次数（默认: 5）\n");
    printf("  -U, --user-agent STR 设置 User-Agent\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s https://example.com/file.zip\n", program_name);
    printf("  %s -o myfile.zip https://example.com/file.zip\n", program_name);
    printf("  %s -O ~/Downloads https://example.com/file.zip\n", program_name);
    printf("  %s -c https://example.com/largefile.zip\n", program_name);
    printf("  %s -s 4 https://example.com/largefile.iso\n", program_name);
    printf("  %s -P https://example.com/file.zip\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pget version 1.0.0\n");
    printf("Copyright (c) 2025 Linux Command Pro Team\n");
    printf("MIT License\n");
#ifdef HAVE_LIBCURL
    printf("\nBuilt with libcurl support\n");
#else
    printf("\nBuilt without libcurl support\n");
#endif
}

int main(int argc, char *argv[]) {
    download_options_t options = {0};
    char *url = NULL;
    
    // 默认值
    options.show_progress = 1;
    options.resume_download = 0;
    options.quiet = 0;
    options.verbose = 0;
    options.timeout = 0;
    options.max_redirects = 5;
    options.follow_location = 1;
    options.segment_count = 1; // 默认单个分段
    strncpy(options.user_agent, "", sizeof(options.user_agent));
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                strncpy(options.output_file, argv[++i], sizeof(options.output_file) - 1);
                options.output_file[sizeof(options.output_file) - 1] = '\0';
            } else {
                fprintf(stderr, "%s❌ 错误: --output 需要指定文件名%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else if (strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 < argc) {
                strncpy(options.output_dir, argv[++i], sizeof(options.output_dir) - 1);
                options.output_dir[sizeof(options.output_dir) - 1] = '\0';
            } else {
                fprintf(stderr, "%s❌ 错误: --output-dir 需要指定目录%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--continue") == 0) {
            options.resume_download = 1;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            options.quiet = 1;
            options.show_progress = 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0) {
            options.verbose = 1;
        } else if (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--progress") == 0) {
            options.show_progress = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--segments") == 0) {
            if (i + 1 < argc) {
                options.segment_count = atoi(argv[++i]);
                if (options.segment_count < 1) {
                    options.segment_count = 1;
                }
            } else {
                fprintf(stderr, "%s❌ 错误: --segments 需要指定连接数%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) {
            if (i + 1 < argc) {
                options.timeout = atoi(argv[++i]);
            } else {
                fprintf(stderr, "%s❌ 错误: --timeout 需要指定秒数%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else if (strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "--location") == 0) {
            options.follow_location = 1;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--max-redirects") == 0) {
            if (i + 1 < argc) {
                options.max_redirects = atoi(argv[++i]);
            } else {
                fprintf(stderr, "%s❌ 错误: --max-redirects 需要指定次数%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else if (strcmp(argv[i], "-U") == 0 || strcmp(argv[i], "--user-agent") == 0) {
            if (i + 1 < argc) {
                strncpy(options.user_agent, argv[++i], sizeof(options.user_agent) - 1);
                options.user_agent[sizeof(options.user_agent) - 1] = '\0';
            } else {
                fprintf(stderr, "%s❌ 错误: --user-agent 需要指定字符串%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else if (argv[i][0] != '-') {
            if (!url) {
                url = argv[i];
            } else {
                fprintf(stderr, "%s❌ 错误: 只能指定一个 URL%s\n",
                        COLOR_RED, COLOR_RESET);
                return 1;
            }
        } else {
            fprintf(stderr, "%s❌ 错误: 未知选项 '%s'%s\n",
                    COLOR_RED, argv[i], COLOR_RESET);
            fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
            return 1;
        }
    }
    
    // 检查是否指定了URL
    if (!url) {
        fprintf(stderr, "%s❌ 错误: 请指定要下载的 URL%s\n",
                COLOR_RED, COLOR_RESET);
        fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
        return 1;
    }
    
    // 验证URL格式
    if (strncmp(url, "http://", 7) != 0 && 
        strncmp(url, "https://", 8) != 0 &&
        strncmp(url, "ftp://", 6) != 0) {
        fprintf(stderr, "%s⚠️  警告: URL 格式可能不正确（应包含 http://, https:// 或 ftp://）%s\n",
                COLOR_YELLOW, COLOR_RESET);
    }
    
    strncpy(options.url, url, sizeof(options.url) - 1);
    options.url[sizeof(options.url) - 1] = '\0';
    
    // 检查输出目录是否存在
    if (options.output_dir[0] != '\0') {
        struct stat st;
        if (stat(options.output_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s❌ 错误: 输出目录不存在: %s%s\n",
                    COLOR_RED, options.output_dir, COLOR_RESET);
            return 1;
        }
    }
    
#ifdef HAVE_LIBCURL
    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // 执行下载
    int result = download_with_curl(&options);
    
    // 清理 libcurl
    curl_global_cleanup();
    
    return result;
#else
    return download_with_curl(&options);
#endif
}


