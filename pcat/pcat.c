#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 1024
#define TAB_WIDTH 4

// 语法高亮颜色定义
#define KEYWORD_COLOR    COLOR_BLUE
#define STRING_COLOR     COLOR_GREEN
#define COMMENT_COLOR    COLOR_YELLOW
#define NUMBER_COLOR     COLOR_MAGENTA
#define FUNCTION_COLOR   COLOR_CYAN
#define TYPE_COLOR       COLOR_RED
#define PREPROCESSOR_COLOR COLOR_MAGENTA
#define OPERATOR_COLOR   COLOR_WHITE

// 语言类型枚举
typedef enum {
    LANG_C,
    LANG_CPP,
    LANG_GO,
    LANG_PYTHON,
    LANG_CUDA,
    LANG_UNKNOWN
} LanguageType;

// C语言关键字
const char *c_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    "NULL", "true", "false", "NULL", NULL
};

// C++关键字
const char *cpp_keywords[] = {
    "auto", "break", "case", "catch", "char", "class", "const", "const_cast",
    "continue", "default", "delete", "do", "double", "dynamic_cast", "else",
    "enum", "explicit", "extern", "false", "float", "for", "friend", "goto",
    "if", "inline", "int", "long", "mutable", "namespace", "new", "operator",
    "private", "protected", "public", "register", "reinterpret_cast", "return",
    "short", "signed", "sizeof", "static", "static_cast", "struct", "switch",
    "template", "this", "throw", "true", "try", "typedef", "typeid", "typename",
    "union", "unsigned", "using", "virtual", "void", "volatile", "while",
    "and", "and_eq", "bitand", "bitor", "compl", "not", "not_eq", "or",
    "or_eq", "xor", "xor_eq", "NULL", "nullptr", NULL
};

// Go语言关键字
const char *go_keywords[] = {
    "break", "case", "chan", "const", "continue", "default", "defer", "else",
    "fallthrough", "for", "func", "go", "goto", "if", "import", "interface",
    "map", "package", "range", "return", "select", "struct", "switch", "type",
    "var", "true", "false", "nil", "iota", "make", "len", "cap", "new",
    "append", "copy", "delete", "close", "panic", "recover", NULL
};

// Python关键字
const char *python_keywords[] = {
    "and", "as", "assert", "break", "class", "continue", "def", "del",
    "elif", "else", "except", "exec", "finally", "for", "from", "global",
    "if", "import", "in", "is", "lambda", "not", "or", "pass", "print",
    "raise", "return", "try", "while", "with", "yield", "True", "False",
    "None", "self", "cls", "super", "staticmethod", "classmethod", "property",
    "setattr", "getattr", "hasattr", "delattr", "isinstance", "issubclass",
    "len", "str", "int", "float", "bool", "list", "dict", "tuple", "set",
    "range", "enumerate", "zip", "map", "filter", "reduce", "sorted", "reversed",
    "open", "file", "input", "raw_input", "print", "abs", "all", "any",
    "bin", "chr", "ord", "hex", "oct", "pow", "round", "sum", "max", "min",
    NULL
};

// CUDA关键字
const char *cuda_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    "__global__", "__device__", "__host__", "__shared__", "__constant__",
    "__restrict__", "__noinline__", "__forceinline__", "__inline__",
    "dim3", "threadIdx", "blockIdx", "blockDim", "gridDim", "warpSize",
    "atomicAdd", "atomicSub", "atomicExch", "atomicMax", "atomicMin",
    "atomicInc", "atomicDec", "atomicCAS", "atomicAnd", "atomicOr", "atomicXor",
    "syncthreads", "syncwarp", "syncgrid", "threadfence", "threadfence_block",
    "threadfence_system", "cudaMalloc", "cudaFree", "cudaMemcpy",
    "cudaMemcpyAsync", "cudaMemcpyToSymbol", "cudaMemcpyFromSymbol",
    "cudaMemset", "cudaMemsetAsync", "cudaGetLastError", "cudaDeviceSynchronize",
    "cudaStreamCreate", "cudaStreamDestroy", "cudaStreamSynchronize",
    "cudaEventCreate", "cudaEventDestroy", "cudaEventRecord", "cudaEventSynchronize",
    "cudaEventElapsedTime", "cudaGetDeviceCount", "cudaSetDevice", "cudaGetDevice",
    "cudaGetDeviceProperties", "cudaDeviceReset", "cudaDeviceSynchronize",
    "NULL", "true", "false", NULL
};

// 检测文件语言类型
LanguageType detect_language(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return LANG_UNKNOWN;
    
    ext++; // 跳过点号
    
    if (strcmp(ext, "c") == 0) return LANG_C;
    if (strcmp(ext, "h") == 0) return LANG_C;
    if (strcmp(ext, "cpp") == 0) return LANG_CPP;
    if (strcmp(ext, "cc") == 0) return LANG_CPP;
    if (strcmp(ext, "cxx") == 0) return LANG_CPP;
    if (strcmp(ext, "hpp") == 0) return LANG_CPP;
    if (strcmp(ext, "hxx") == 0) return LANG_CPP;
    if (strcmp(ext, "go") == 0) return LANG_GO;
    if (strcmp(ext, "py") == 0) return LANG_PYTHON;
    if (strcmp(ext, "pyw") == 0) return LANG_PYTHON;
    if (strcmp(ext, "cu") == 0) return LANG_CUDA;
    if (strcmp(ext, "cuh") == 0) return LANG_CUDA;
    
    return LANG_UNKNOWN;
}

// 获取关键字数组
const char **get_keywords(LanguageType lang) {
    switch (lang) {
        case LANG_C: return c_keywords;
        case LANG_CPP: return cpp_keywords;
        case LANG_GO: return go_keywords;
        case LANG_PYTHON: return python_keywords;
        case LANG_CUDA: return cuda_keywords;
        default: return c_keywords;
    }
}

// 检查是否为关键字
int is_keyword(const char *word, LanguageType lang) {
    const char **keywords = get_keywords(lang);
    
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// 检查是否为数字
int is_number(const char *str) {
    if (*str == '\0') return 0;
    
    // 检查十六进制
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        for (int i = 2; str[i]; i++) {
            if (!((str[i] >= '0' && str[i] <= '9') || 
                  (str[i] >= 'a' && str[i] <= 'f') || 
                  (str[i] >= 'A' && str[i] <= 'F'))) {
                return 0;
            }
        }
        return 1;
    }
    
    // 检查普通数字
    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            if (str[i] != '.' && str[i] != 'f' && str[i] != 'L' && str[i] != 'U') {
                return 0;
            }
        }
    }
    return 1;
}

// 检查是否为字符串
int is_string(const char *str) {
    return (str[0] == '"' && str[strlen(str)-1] == '"') ||
           (str[0] == '\'' && str[strlen(str)-1] == '\'');
}

// 检查是否为注释
int is_comment(const char *str) {
    return (str[0] == '/' && str[1] == '/') ||
           (str[0] == '/' && str[1] == '*');
}

// 检查是否为函数调用
int is_function_call(const char *str) {
    return strchr(str, '(') != NULL && strchr(str, ')') != NULL;
}

// 检查是否为预处理器指令
int is_preprocessor(const char *str) {
    return str[0] == '#';
}

// 检查是否为操作符
int is_operator(const char *str) {
    const char *operators[] = {
        "+", "-", "*", "/", "%", "=", "==", "!=", "<", ">", "<=", ">=",
        "&&", "||", "!", "&", "|", "^", "~", "<<", ">>", "++", "--",
        "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
        "->", ".", "::", "?", ":", "->*", ".*", NULL
    };
    
    for (int i = 0; operators[i] != NULL; i++) {
        if (strcmp(str, operators[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// 检查是否为类型定义
int is_type_definition(const char *str, LanguageType lang) {
    const char *types[] = {
        "int", "char", "float", "double", "void", "short", "long", "signed", "unsigned",
        "struct", "union", "enum", "typedef", "const", "static", "extern", "volatile",
        "register", "auto", "inline", "restrict", NULL
    };
    
    for (int i = 0; types[i] != NULL; i++) {
        if (strcmp(str, types[i]) == 0) {
            return 1;
        }
    }
    
    // C++特有类型
    if (lang == LANG_CPP) {
        const char *cpp_types[] = {
            "class", "template", "typename", "namespace", "using", "public", "private",
            "protected", "virtual", "friend", "mutable", "explicit", "operator",
            "new", "delete", "this", "throw", "try", "catch", "const_cast",
            "dynamic_cast", "reinterpret_cast", "static_cast", "typeid", NULL
        };
        
        for (int i = 0; cpp_types[i] != NULL; i++) {
            if (strcmp(str, cpp_types[i]) == 0) {
                return 1;
            }
        }
    }
    
    // Go特有类型
    if (lang == LANG_GO) {
        const char *go_types[] = {
            "string", "bool", "byte", "rune", "int8", "int16", "int32", "int64",
            "uint8", "uint16", "uint32", "uint64", "float32", "float64",
            "complex64", "complex128", "uintptr", "interface", "chan", "map",
            "slice", "array", "func", "error", NULL
        };
        
        for (int i = 0; go_types[i] != NULL; i++) {
            if (strcmp(str, go_types[i]) == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

// 高级语法高亮
void highlight_line(const char *line, int line_num, int show_numbers, LanguageType lang) {
    if (show_numbers) {
        printf("%s%4d%s | ", COLOR_CYAN, line_num, COLOR_RESET);
    }
    
    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " \t\n");
    
    while (token != NULL) {
        // 清理token中的标点符号
        char clean_token[256];
        int j = 0;
        for (int i = 0; token[i] && j < 255; i++) {
            if (isalnum(token[i]) || token[i] == '_') {
                clean_token[j++] = token[i];
            }
        }
        clean_token[j] = '\0';
        
        if (is_preprocessor(token)) {
            printf("%s%s%s ", PREPROCESSOR_COLOR, token, COLOR_RESET);
        } else if (is_comment(token)) {
            printf("%s%s%s ", COMMENT_COLOR, token, COLOR_RESET);
        } else if (is_string(token)) {
            printf("%s%s%s ", STRING_COLOR, token, COLOR_RESET);
        } else if (is_number(token)) {
            printf("%s%s%s ", NUMBER_COLOR, token, COLOR_RESET);
        } else if (is_operator(token)) {
            printf("%s%s%s ", OPERATOR_COLOR, token, COLOR_RESET);
        } else if (is_type_definition(clean_token, lang)) {
            printf("%s%s%s ", TYPE_COLOR, token, COLOR_RESET);
        } else if (is_keyword(clean_token, lang)) {
            printf("%s%s%s ", KEYWORD_COLOR, token, COLOR_RESET);
        } else if (is_function_call(token)) {
            printf("%s%s%s ", FUNCTION_COLOR, token, COLOR_RESET);
        } else {
            printf("%s ", token);
        }
        token = strtok(NULL, " \t\n");
    }
    
    free(line_copy);
    printf("\n");
}

// 获取语言名称
const char *get_language_name(LanguageType lang) {
    switch (lang) {
        case LANG_C: return "C";
        case LANG_CPP: return "C++";
        case LANG_GO: return "Go";
        case LANG_PYTHON: return "Python";
        case LANG_CUDA: return "CUDA";
        default: return "Unknown";
    }
}

// 显示文件内容
int display_file(const char *filename, int show_numbers, int show_ends) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        print_error("无法打开文件");
        return 1;
    }
    
    char line[MAX_LINE_LENGTH];
    int line_num = 1;
    LanguageType lang = detect_language(filename);
    
    printf("%s文件: %s%s", COLOR_CYAN, filename, COLOR_RESET);
    if (lang != LANG_UNKNOWN) {
        printf(" %s[%s]%s", COLOR_YELLOW, get_language_name(lang), COLOR_RESET);
    }
    printf("\n");
    printf("%s%s%s\n", COLOR_YELLOW, "=" + strlen(filename) + 6, COLOR_RESET);
    
    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\n")] = '\0';
        
        if (show_ends) {
            // 显示行结束符
            printf("%s%4d%s | %s$%s\n", 
                   COLOR_CYAN, line_num, COLOR_RESET, 
                   line, COLOR_YELLOW);
        } else {
            highlight_line(line, line_num, show_numbers, lang);
        }
        line_num++;
    }
    
    fclose(file);
    return 0;
}

// 显示多个文件
int display_files(char *filenames[], int count, int show_numbers, int show_ends) {
    for (int i = 0; i < count; i++) {
        if (count > 1) {
            printf("\n");
        }
        if (display_file(filenames[i], show_numbers, show_ends) != 0) {
            return 1;
        }
    }
    return 0;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [文件...]\n", program_name);
    printf("优化版的 cat 命令，提供多语言语法高亮和行号显示\n\n");
    printf("支持的语言:\n");
    printf("  C/C++     (.c, .h, .cpp, .cc, .cxx, .hpp, .hxx)\n");
    printf("  Go        (.go)\n");
    printf("  Python    (.py, .pyw)\n");
    printf("  CUDA      (.cu, .cuh)\n\n");
    printf("选项:\n");
    printf("  -n, --number        显示行号\n");
    printf("  -E, --show-ends     显示行结束符\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -v, --version       显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s file.c            # 显示C文件内容（带语法高亮）\n", program_name);
    printf("  %s -n main.cpp       # 显示C++文件内容（带行号）\n", program_name);
    printf("  %s -E script.py      # 显示Python文件内容（显示行结束符）\n", program_name);
    printf("  %s *.go              # 显示多个Go文件\n", program_name);
    printf("  %s kernel.cu         # 显示CUDA文件内容\n", program_name);
}

int main(int argc, char *argv[]) {
    int show_numbers = 0;
    int show_ends = 0;
    char *files[argc];
    int file_count = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number") == 0) {
            show_numbers = 1;
        } else if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "--show-ends") == 0) {
            show_ends = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pcat - 多语言语法高亮版 cat 命令 v2.0\n");
            printf("支持语言: C, C++, Go, Python, CUDA\n");
            return 0;
        } else if (argv[i][0] != '-') {
            files[file_count++] = argv[i];
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 如果没有指定文件，从标准输入读取
    if (file_count == 0) {
        char line[MAX_LINE_LENGTH];
        int line_num = 1;
        
        printf("%s从标准输入读取%s\n", COLOR_CYAN, COLOR_RESET);
        printf("%s====================%s\n", COLOR_YELLOW, COLOR_RESET);
        
        while (fgets(line, sizeof(line), stdin)) {
            line[strcspn(line, "\n")] = '\0';
            highlight_line(line, line_num, show_numbers, LANG_UNKNOWN);
            line_num++;
        }
        return 0;
    }
    
    return display_files(files, file_count, show_numbers, show_ends);
}
