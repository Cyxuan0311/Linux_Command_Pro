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

// 语言特定的颜色方案结构
typedef struct {
    const char *keyword;      // 关键字颜色
    const char *string;       // 字符串颜色
    const char *comment;      // 注释颜色
    const char *number;       // 数字颜色
    const char *function;     // 函数颜色
    const char *type;         // 类型颜色
    const char *preprocessor; // 预处理器颜色
    const char *operator;     // 操作符颜色
    const char *special;      // 特殊语法颜色（用于Markdown等）
} ColorScheme;

// 语言类型枚举
typedef enum {
    LANG_C,
    LANG_CPP,
    LANG_GO,
    LANG_PYTHON,
    LANG_JAVA,
    LANG_SHELL,
    LANG_MARKDOWN,
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

// Java关键字
const char *java_keywords[] = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char",
    "class", "const", "continue", "default", "do", "double", "else", "enum",
    "extends", "final", "finally", "float", "for", "goto", "if", "implements",
    "import", "instanceof", "int", "interface", "long", "native", "new", "package",
    "private", "protected", "public", "return", "short", "static", "strictfp",
    "super", "switch", "synchronized", "this", "throw", "throws", "transient",
    "try", "void", "volatile", "while", "true", "false", "null", "String",
    "Integer", "Double", "Float", "Boolean", "Character", "Byte", "Short",
    "Long", "Object", "System", "Math", "ArrayList", "HashMap", "List", "Map",
    "Set", "Iterator", NULL
};

// Shell脚本关键字
const char *shell_keywords[] = {
    "if", "then", "else", "elif", "fi", "case", "esac", "for", "select",
    "while", "until", "do", "done", "in", "function", "time", "coproc",
    "[", "[[", "]]", "test", "true", "false", "break", "continue", "return",
    "exit", "export", "local", "readonly", "declare", "typeset", "alias",
    "unalias", "set", "unset", "shift", "getopts", "eval", "exec", "command",
    "builtin", "let", "echo", "printf", "read", "cd", "pwd", "pushd", "popd",
    "dirs", "hash", "bind", "help", "enable", "disable", "source", "dot",
    "type", "trap", "kill", "wait", "jobs", "fg", "bg", "suspend", "logout",
    "history", "fc", NULL
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

// 获取语言特定的颜色方案
ColorScheme get_color_scheme(LanguageType lang) {
    ColorScheme scheme;
    
    switch (lang) {
        case LANG_C:
            scheme.keyword = COLOR_BLUE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        case LANG_CPP:
            scheme.keyword = COLOR_BLUE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        case LANG_GO:
            scheme.keyword = COLOR_BLUE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        case LANG_PYTHON:
            scheme.keyword = COLOR_BLUE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        case LANG_JAVA:
            scheme.keyword = COLOR_BLUE BOLD;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED BOLD;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        case LANG_SHELL:
            scheme.keyword = COLOR_BLUE BOLD;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA BOLD;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        case LANG_MARKDOWN:
            scheme.keyword = COLOR_BLUE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN BOLD;
            scheme.type = COLOR_RED BOLD;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN BOLD;
            break;
        case LANG_CUDA:
            scheme.keyword = COLOR_BLUE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
        default:
            scheme.keyword = COLOR_WHITE;
            scheme.string = COLOR_GREEN;
            scheme.comment = COLOR_YELLOW;
            scheme.number = COLOR_MAGENTA;
            scheme.function = COLOR_CYAN;
            scheme.type = COLOR_RED;
            scheme.preprocessor = COLOR_MAGENTA;
            scheme.operator = COLOR_WHITE;
            scheme.special = COLOR_CYAN;
            break;
    }
    
    return scheme;
}

// 检测文件语言类型
LanguageType detect_language(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) {
        // 检查是否有shebang行来判断shell脚本
        FILE *f = fopen(filename, "r");
        if (f) {
            char first_line[256];
            if (fgets(first_line, sizeof(first_line), f)) {
                if (strstr(first_line, "#!/bin/bash") || strstr(first_line, "#!/bin/sh") ||
                    strstr(first_line, "#!/usr/bin/bash") || strstr(first_line, "#!/usr/bin/sh")) {
                    fclose(f);
                    return LANG_SHELL;
                }
            }
            fclose(f);
        }
        return LANG_UNKNOWN;
    }
    
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
    if (strcmp(ext, "java") == 0) return LANG_JAVA;
    if (strcmp(ext, "sh") == 0) return LANG_SHELL;
    if (strcmp(ext, "bash") == 0) return LANG_SHELL;
    if (strcmp(ext, "md") == 0) return LANG_MARKDOWN;
    if (strcmp(ext, "markdown") == 0) return LANG_MARKDOWN;
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
        case LANG_JAVA: return java_keywords;
        case LANG_SHELL: return shell_keywords;
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
int is_comment(const char *str, LanguageType lang) {
    if (lang == LANG_PYTHON || lang == LANG_SHELL || lang == LANG_MARKDOWN) {
        return str[0] == '#';
    }
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
    
    // Java特有类型
    if (lang == LANG_JAVA) {
        const char *java_types[] = {
            "String", "Integer", "Double", "Float", "Boolean", "Character", "Byte",
            "Short", "Long", "Object", "System", "Math", "ArrayList", "HashMap",
            "List", "Map", "Set", "Iterator", "Collection", "Exception", "Error",
            "Throwable", "Runnable", "Thread", NULL
        };
        
        for (int i = 0; java_types[i] != NULL; i++) {
            if (strcmp(str, java_types[i]) == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

// 检查是否为Markdown语法元素
int is_markdown_syntax(const char *line, int *start, int *end) {
    int len = strlen(line);
    *start = -1;
    *end = -1;
    
    // 检查标题 (# ## ### 等)
    if (len > 0 && line[0] == '#') {
        int i = 0;
        while (i < len && line[i] == '#') i++;
        if (i > 0 && i <= 6 && (line[i] == ' ' || line[i] == '\0')) {
            *start = 0;
            *end = i;
            return 1; // 标题标记
        }
    }
    
    // 检查代码块标记 (```)
    if (strncmp(line, "```", 3) == 0) {
        *start = 0;
        *end = 3;
        return 2; // 代码块标记
    }
    
    // 检查粗体 (**text**)
    const char *bold_start = strstr(line, "**");
    if (bold_start) {
        *start = bold_start - line;
        const char *bold_end = strstr(bold_start + 2, "**");
        if (bold_end) {
            *end = bold_end - line + 2;
            return 3; // 粗体
        }
    }
    
    // 检查斜体 (*text*)
    const char *italic_start = strstr(line, "*");
    if (italic_start && italic_start[1] != '*') {
        *start = italic_start - line;
        const char *italic_end = strstr(italic_start + 1, "*");
        if (italic_end && italic_end[1] != '*') {
            *end = italic_end - line + 1;
            return 4; // 斜体
        }
    }
    
    // 检查链接 [text](url)
    const char *link_start = strstr(line, "[");
    if (link_start) {
        const char *link_mid = strstr(link_start + 1, "]");
        if (link_mid && link_mid[1] == '(') {
            const char *link_end = strstr(link_mid + 2, ")");
            if (link_end) {
                *start = link_start - line;
                *end = link_end - line + 1;
                return 5; // 链接
            }
        }
    }
    
    return 0;
}

// Markdown语法高亮
void highlight_markdown_line(const char *line, int line_num, int show_numbers, ColorScheme scheme) {
    if (show_numbers) {
        printf("%s%4d%s | ", COLOR_CYAN, line_num, COLOR_RESET);
    }
    
    int len = strlen(line);
    int i = 0;
    
    // 检查整行是否为标题
    if (len > 0 && line[0] == '#') {
        int j = 0;
        while (j < len && j < 6 && line[j] == '#') j++;
        if (j > 0 && (line[j] == ' ' || line[j] == '\0')) {
            printf("%s%.*s%s", scheme.type, j, line, COLOR_RESET);
            if (line[j] == ' ') {
                printf("%s%s%s", scheme.special, line + j, COLOR_RESET);
            }
            printf("\n");
            return;
        }
    }
    
    // 检查代码块标记
    if (strncmp(line, "```", 3) == 0) {
        printf("%s%s%s\n", scheme.special, line, COLOR_RESET);
        return;
    }
    
    // 逐个字符处理，识别Markdown语法
    while (i < len) {
        // 检查粗体 **text**
        if (i < len - 3 && line[i] == '*' && line[i+1] == '*') {
            int start = i;
            i += 2;
            while (i < len - 1 && !(line[i] == '*' && line[i+1] == '*')) {
                i++;
            }
            if (i < len - 1) {
                printf("%s**%.*s**%s", scheme.special, i - start - 2, line + start + 2, COLOR_RESET);
                i += 2;
                continue;
            } else {
                i = start;
            }
        }
        
        // 检查斜体 *text*
        if (i < len - 1 && line[i] == '*' && (i == 0 || line[i-1] != '*') && 
            (i == len - 1 || line[i+1] != '*')) {
            int start = i;
            i++;
            while (i < len && line[i] != '*') i++;
            if (i < len && (i == len - 1 || line[i+1] != '*')) {
                printf("%s*%.*s*%s", scheme.special, i - start - 1, line + start + 1, COLOR_RESET);
                i++;
                continue;
            } else {
                i = start;
            }
        }
        
        // 检查链接 [text](url)
        if (line[i] == '[') {
            int link_start = i;
            i++;
            while (i < len && line[i] != ']') i++;
            if (i < len && i < len - 1 && line[i+1] == '(') {
                int text_end = i;
                i += 2;
                while (i < len && line[i] != ')') i++;
                if (i < len) {
                    printf("%s%.*s%s", scheme.special, text_end - link_start + 1, line + link_start, COLOR_RESET);
                    printf("%s%.*s%s", COLOR_GREEN, i - text_end - 1, line + text_end + 1, COLOR_RESET);
                    printf("%s", COLOR_RESET);
                    i++;
                    continue;
                }
            }
            i = link_start;
        }
        
        // 检查代码内联 `code`
        if (line[i] == '`') {
            int start = i;
            i++;
            while (i < len && line[i] != '`') i++;
            if (i < len) {
                printf("%s`%.*s`%s", scheme.special, i - start - 1, line + start + 1, COLOR_RESET);
                i++;
                continue;
            } else {
                i = start;
            }
        }
        
        // 检查注释
        if (line[i] == '#') {
            printf("%s%s%s", scheme.comment, line + i, COLOR_RESET);
            break;
        }
        
        // 普通字符
        printf("%c", line[i]);
        i++;
    }
    
    printf("\n");
}

// 高级语法高亮
void highlight_line(const char *line, int line_num, int show_numbers, LanguageType lang) {
    ColorScheme scheme = get_color_scheme(lang);
    
    // Markdown使用特殊的处理方式
    if (lang == LANG_MARKDOWN) {
        highlight_markdown_line(line, line_num, show_numbers, scheme);
        return;
    }
    
    if (show_numbers) {
        printf("%s%4d%s | ", COLOR_CYAN, line_num, COLOR_RESET);
    }
    
    int len = strlen(line);
    if (len == 0) {
        printf("\n");
        return;
    }
    
    // 检查整行是否为注释（Python/Shell/Markdown）
    if (lang == LANG_PYTHON || lang == LANG_SHELL) {
        int i = 0;
        while (i < len && (line[i] == ' ' || line[i] == '\t')) i++;
        if (i < len && line[i] == '#') {
            printf("%s%s%s\n", scheme.comment, line, COLOR_RESET);
            return;
        }
    }
    
    // 检查整行是否为注释（C/C++/Java/Go）
    if (lang == LANG_C || lang == LANG_CPP || lang == LANG_JAVA || lang == LANG_GO || lang == LANG_CUDA) {
        int i = 0;
        while (i < len && (line[i] == ' ' || line[i] == '\t')) i++;
        if (i < len - 1 && line[i] == '/' && line[i+1] == '/') {
            printf("%s%s%s\n", scheme.comment, line, COLOR_RESET);
            return;
        }
    }
    
    // 逐字符处理，识别字符串、注释、关键字等
    int i = 0;
    int in_string = 0;
    char string_char = 0;
    int in_single_comment = 0;
    int in_multi_comment = 0;
    
    char word_buf[256] = {0};
    int word_pos = 0;
    
    while (i < len) {
        // 检查多行注释开始 (/*)
        if (!in_string && !in_single_comment && !in_multi_comment &&
            i < len - 1 && line[i] == '/' && line[i+1] == '*') {
            if (word_pos > 0) {
                word_buf[word_pos] = '\0';
                // 处理之前的单词
                char clean_token[256];
                int j = 0;
                for (int k = 0; word_buf[k] && j < 255; k++) {
                    if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                        clean_token[j++] = word_buf[k];
                    }
                }
                clean_token[j] = '\0';
                
                if (is_keyword(clean_token, lang)) {
                    printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
                } else if (is_type_definition(clean_token, lang)) {
                    printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
                } else if (is_number(word_buf)) {
                    printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
                } else {
                    printf("%s", word_buf);
                }
                word_pos = 0;
            }
            in_multi_comment = 1;
            printf("%s/*", scheme.comment);
            i += 2;
            continue;
        }
        
        // 检查多行注释结束 (*/)
        if (in_multi_comment && i < len - 1 && line[i] == '*' && line[i+1] == '/') {
            printf("*/%s", COLOR_RESET);
            in_multi_comment = 0;
            i += 2;
            continue;
        }
        
        // 处理多行注释内容
        if (in_multi_comment) {
            printf("%c", line[i]);
            i++;
            continue;
        }
        
        // 检查单行注释 (//)
        if (!in_string && !in_single_comment && i < len - 1 && 
            line[i] == '/' && line[i+1] == '/') {
            if (word_pos > 0) {
                word_buf[word_pos] = '\0';
                char clean_token[256];
                int j = 0;
                for (int k = 0; word_buf[k] && j < 255; k++) {
                    if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                        clean_token[j++] = word_buf[k];
                    }
                }
                clean_token[j] = '\0';
                
                if (is_keyword(clean_token, lang)) {
                    printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
                } else if (is_type_definition(clean_token, lang)) {
                    printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
                } else if (is_number(word_buf)) {
                    printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
                } else {
                    printf("%s", word_buf);
                }
                word_pos = 0;
            }
            printf("%s%s%s", scheme.comment, line + i, COLOR_RESET);
            break;
        }
        
        // 检查字符串
        if (!in_single_comment && (line[i] == '"' || line[i] == '\'')) {
            if (!in_string) {
                if (word_pos > 0) {
                    word_buf[word_pos] = '\0';
                    char clean_token[256];
                    int j = 0;
                    for (int k = 0; word_buf[k] && j < 255; k++) {
                        if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                            clean_token[j++] = word_buf[k];
                        }
                    }
                    clean_token[j] = '\0';
                    
                    if (is_keyword(clean_token, lang)) {
                        printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
                    } else if (is_type_definition(clean_token, lang)) {
                        printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
                    } else if (is_number(word_buf)) {
                        printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
                    } else {
                        printf("%s", word_buf);
                    }
                    word_pos = 0;
                }
                in_string = 1;
                string_char = line[i];
                printf("%s%c", scheme.string, line[i]);
            } else if (line[i] == string_char && (i == 0 || line[i-1] != '\\')) {
                printf("%c%s", line[i], COLOR_RESET);
                in_string = 0;
            } else {
                printf("%c", line[i]);
            }
            i++;
            continue;
        }
        
        // 处理字符串内容
        if (in_string) {
            printf("%c", line[i]);
            i++;
            continue;
        }
        
        // 检查预处理器指令
        if (i == 0 && line[i] == '#') {
            printf("%s%s%s", scheme.preprocessor, line, COLOR_RESET);
            break;
        }
        
        // 检查操作符
        if (strchr("+-*/%=<>!&|^~()[]{}.,;?:", line[i])) {
            if (word_pos > 0) {
                word_buf[word_pos] = '\0';
                char clean_token[256];
                int j = 0;
                for (int k = 0; word_buf[k] && j < 255; k++) {
                    if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                        clean_token[j++] = word_buf[k];
                    }
                }
                clean_token[j] = '\0';
                
                if (is_keyword(clean_token, lang)) {
                    printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
                } else if (is_type_definition(clean_token, lang)) {
                    printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
                } else if (is_number(word_buf)) {
                    printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
                } else if (is_function_call(word_buf)) {
                    printf("%s%s%s", scheme.function, word_buf, COLOR_RESET);
                } else {
                    printf("%s", word_buf);
                }
                word_pos = 0;
            }
            
            // 检查是否为操作符
            char op_buf[4] = {0};
            if (i < len - 1) {
                op_buf[0] = line[i];
                op_buf[1] = line[i+1];
                op_buf[2] = '\0';
                if (is_operator(op_buf)) {
                    printf("%s%s%s", scheme.operator, op_buf, COLOR_RESET);
                    i += 2;
                    continue;
                }
            }
            op_buf[0] = line[i];
            op_buf[1] = '\0';
            if (is_operator(op_buf)) {
                printf("%s%s%s", scheme.operator, op_buf, COLOR_RESET);
            } else {
                printf("%c", line[i]);
            }
            i++;
            continue;
        }
        
        // 收集单词字符
        if (isalnum(line[i]) || line[i] == '_') {
            if (word_pos < 255) {
                word_buf[word_pos++] = line[i];
            }
        } else if (line[i] == ' ' || line[i] == '\t') {
            if (word_pos > 0) {
                word_buf[word_pos] = '\0';
                char clean_token[256];
                int j = 0;
                for (int k = 0; word_buf[k] && j < 255; k++) {
                    if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                        clean_token[j++] = word_buf[k];
                    }
                }
                clean_token[j] = '\0';
                
                if (is_keyword(clean_token, lang)) {
                    printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
                } else if (is_type_definition(clean_token, lang)) {
                    printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
                } else if (is_number(word_buf)) {
                    printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
                } else if (is_function_call(word_buf)) {
                    printf("%s%s%s", scheme.function, word_buf, COLOR_RESET);
                } else {
                    printf("%s", word_buf);
                }
                word_pos = 0;
            }
            printf("%c", line[i]);
        } else {
            if (word_pos > 0) {
                word_buf[word_pos] = '\0';
                char clean_token[256];
                int j = 0;
                for (int k = 0; word_buf[k] && j < 255; k++) {
                    if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                        clean_token[j++] = word_buf[k];
                    }
                }
                clean_token[j] = '\0';
                
                if (is_keyword(clean_token, lang)) {
                    printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
                } else if (is_type_definition(clean_token, lang)) {
                    printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
                } else if (is_number(word_buf)) {
                    printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
                } else if (is_function_call(word_buf)) {
                    printf("%s%s%s", scheme.function, word_buf, COLOR_RESET);
                } else {
                    printf("%s", word_buf);
                }
                word_pos = 0;
            }
            printf("%c", line[i]);
        }
        
        i++;
    }
    
    // 处理剩余的单词
    if (word_pos > 0) {
        word_buf[word_pos] = '\0';
        char clean_token[256];
        int j = 0;
        for (int k = 0; word_buf[k] && j < 255; k++) {
            if (isalnum(word_buf[k]) || word_buf[k] == '_') {
                clean_token[j++] = word_buf[k];
            }
        }
        clean_token[j] = '\0';
        
        if (is_keyword(clean_token, lang)) {
            printf("%s%s%s", scheme.keyword, word_buf, COLOR_RESET);
        } else if (is_type_definition(clean_token, lang)) {
            printf("%s%s%s", scheme.type, word_buf, COLOR_RESET);
        } else if (is_number(word_buf)) {
            printf("%s%s%s", scheme.number, word_buf, COLOR_RESET);
        } else if (is_function_call(word_buf)) {
            printf("%s%s%s", scheme.function, word_buf, COLOR_RESET);
        } else {
            printf("%s", word_buf);
        }
    }
    
    printf("\n");
}

// 获取语言名称
const char *get_language_name(LanguageType lang) {
    switch (lang) {
        case LANG_C: return "C";
        case LANG_CPP: return "C++";
        case LANG_GO: return "Go";
        case LANG_PYTHON: return "Python";
        case LANG_JAVA: return "Java";
        case LANG_SHELL: return "Shell";
        case LANG_MARKDOWN: return "Markdown";
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
    printf("  Java      (.java)\n");
    printf("  Shell     (.sh, .bash, 或带shebang的脚本)\n");
    printf("  Markdown  (.md, .markdown)\n");
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
    printf("  %s Main.java         # 显示Java文件内容\n", program_name);
    printf("  %s script.sh         # 显示Shell脚本内容\n", program_name);
    printf("  %s README.md         # 显示Markdown文件内容\n", program_name);
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
            printf("pcat - 多语言语法高亮版 cat 命令 v3.0\n");
            printf("支持语言: C, C++, Go, Python, Java, Shell, Markdown, CUDA\n");
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
