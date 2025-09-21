# 📖 Linux Command Pro 使用教程

> 本教程将详细介绍Linux Command Pro中各个命令的使用方法和高级技巧。

## 目录
- [pls - 优化版 ls](#pls---优化版-ls)
- [pcat - 优化版 cat](#pcat---优化版-cat)
- [pfind - 优化版 find](#pfind---优化版-find)
- [pgrep - 优化版 grep](#pgrep---优化版-grep)
- [ptop - 优化版 top](#ptop---优化版-top)
- [pdu - 优化版 du](#pdu---优化版-du)
- [pps - 优化版 ps](#pps---优化版-ps)
- [高级用法](#高级用法)

---

## 📁 pls - 优化版 ls

### 🚀 基本使用
```bash
pls                    # 列出当前目录
pls /home              # 列出指定目录
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-l` | 详细模式，显示文件详细信息 | `pls -l` |
| `-a` | 显示隐藏文件 | `pls -a` |
| `-h` | 人类可读的文件大小 | `pls -h` |
| `-t` | 按修改时间排序 | `pls -t` |
| `-S` | 按文件大小排序 | `pls -S` |
| `--help` | 显示帮助信息 | `pls --help` |
| `--version` | 显示版本信息 | `pls --version` |

### 💡 示例
```bash
# 列出当前目录内容
pls

# 详细模式显示文件信息
pls -l

# 显示隐藏文件
pls -a

# 按大小排序显示
pls -S

# 按时间排序显示
pls -t

# 列出指定目录
pls /usr/bin

# 组合使用多个选项
pls -lah /home
```

## 📄 pcat - 优化版 cat

### 🚀 基本使用
```bash
pcat file.txt          # 显示文件内容
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-n` | 显示行号 | `pcat -n file.txt` |
| `-E` | 显示行结束符 | `pcat -E file.txt` |
| `-T` | 显示制表符 | `pcat -T file.txt` |
| `-s` | 压缩空行 | `pcat -s file.txt` |
| `-b` | 只对非空行编号 | `pcat -b file.txt` |
| `--help` | 显示帮助信息 | `pcat --help` |
| `--version` | 显示版本信息 | `pcat --version` |

### 💡 示例
```bash
# 显示文件内容
pcat main.c

# 显示带行号的内容
pcat -n main.c

# 显示行结束符
pcat -E config.txt

# 显示制表符
pcat -T data.txt

# 压缩空行
pcat -s log.txt

# 只对非空行编号
pcat -b script.sh

# 显示多个文件
pcat file1.txt file2.txt

# 组合使用多个选项
pcat -nE main.c
```

## 🔍 pfind - 优化版 find

### 🚀 基本使用
```bash
pfind . -name "*.c"    # 查找.c文件
pfind . -type d        # 查找目录
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-name` | 按文件名搜索 | `pfind . -name "*.c"` |
| `-type` | 按文件类型搜索 | `pfind . -type d` |
| `-size` | 按文件大小搜索 | `pfind . -size +1M` |
| `-mtime` | 按修改时间搜索 | `pfind . -mtime -7` |
| `-maxdepth` | 限制搜索深度 | `pfind . -maxdepth 3` |
| `-ls` | 显示详细信息 | `pfind . -ls` |
| `-regex` | 使用正则表达式 | `pfind . -regex ".*\.c$"` |
| `--help` | 显示帮助信息 | `pfind --help` |
| `--version` | 显示版本信息 | `pfind --version` |

### 💡 示例
```bash
# 查找所有C文件
pfind . -name "*.c"

# 查找所有目录
pfind . -type d

# 查找大于1MB的文件
pfind . -size +1M

# 查找7天内修改的文件
pfind . -mtime -7

# 限制搜索深度为3层
pfind . -maxdepth 3 -name "*.txt"

# 使用正则表达式搜索
pfind . -regex ".*\.(c|cpp|h)$"

# 查找空文件
pfind . -type f -empty

# 查找可执行文件
pfind . -type f -executable

# 组合多个条件
pfind . -name "*.log" -size +100M -mtime -1
```

## 🔎 pgrep - 优化版 grep

### 🚀 基本使用
```bash
pgrep "hello" file.txt # 搜索文本
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-i` | 忽略大小写 | `pgrep -i "hello"` |
| `-r` | 递归搜索目录 | `pgrep -r "hello" .` |
| `-n` | 显示行号 | `pgrep -n "hello"` |
| `-c` | 只显示匹配行数 | `pgrep -c "hello"` |
| `-v` | 显示不匹配的行 | `pgrep -v "hello"` |
| `-w` | 匹配整个单词 | `pgrep -w "hello"` |
| `-E` | 使用正则表达式 | `pgrep -E "hello\|world"` |
| `-A` | 显示匹配行后的N行 | `pgrep -A 3 "error"` |
| `-B` | 显示匹配行前的N行 | `pgrep -B 2 "error"` |
| `-C` | 显示匹配行前后的N行 | `pgrep -C 2 "error"` |
| `--include` | 指定文件类型 | `pgrep --include="*.c" "printf"` |
| `--help` | 显示帮助信息 | `pgrep --help` |
| `--version` | 显示版本信息 | `pgrep --version` |

### 💡 示例
```bash
# 在文件中搜索文本
pgrep "main" main.c

# 忽略大小写搜索
pgrep -i "HELLO" file.txt

# 递归搜索目录
pgrep -r "function" src/

# 显示行号
pgrep -n "error" log.txt

# 只显示匹配行数
pgrep -c "TODO" *.c

# 使用正则表达式
pgrep -E "int|void" *.c

# 搜索特定类型的文件
pgrep --include="*.c" "printf" .

# 显示匹配行及前后3行
pgrep -C 3 "error" log.txt

# 匹配整个单词
pgrep -w "main" *.c

# 显示不包含特定文本的行
pgrep -v "debug" *.c
```

## 📊 ptop - 优化版 top

### 🚀 基本使用
```bash
ptop                    # 显示进程监控
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-n` | 显示前N个进程 | `ptop -n 50` |
| `-d` | 刷新间隔（秒） | `ptop -d 1` |
| `-t` | 显示线程信息 | `ptop -t` |
| `-u` | 显示指定用户的进程 | `ptop -u root` |
| `-p` | 显示指定PID的进程 | `ptop -p 1234` |
| `-c` | 按CPU使用率排序 | `ptop -c` |
| `-m` | 按内存使用率排序 | `ptop -m` |
| `--help` | 显示帮助信息 | `ptop --help` |
| `--version` | 显示版本信息 | `ptop --version` |

### 💡 示例
```bash
# 实时监控进程
ptop

# 显示更多进程
ptop -n 100

# 快速刷新（每秒）
ptop -d 1

# 显示线程信息
ptop -t

# 显示root用户的进程
ptop -u root

# 按CPU使用率排序
ptop -c

# 按内存使用率排序
ptop -m

# 组合使用多个选项
ptop -n 20 -d 2 -c
```

## 📈 pdu - 优化版 du

### 🚀 基本使用
```bash
pdu                     # 分析当前目录
pdu /home               # 分析指定目录
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-h` | 人类可读格式 | `pdu -h` |
| `-d` | 限制目录深度 | `pdu -d 2` |
| `-g` | 显示图形化进度条 | `pdu -g` |
| `-o` | 显示所有者信息 | `pdu -o` |
| `-S` | 按大小排序 | `pdu -S` |
| `-H` | 显示隐藏文件 | `pdu -H` |
| `-a` | 显示所有文件 | `pdu -a` |
| `-s` | 只显示总计 | `pdu -s` |
| `-t` | 按修改时间排序 | `pdu -t` |
| `--help` | 显示帮助信息 | `pdu --help` |
| `--version` | 显示版本信息 | `pdu --version` |

### 💡 示例
```bash
# 分析当前目录
pdu

# 限制深度并显示图形
pdu -d 2 -g

# 显示所有文件和所有者
pdu -a -o

# 按大小排序
pdu -S

# 只显示总计
pdu -s

# 按修改时间排序
pdu -t

# 显示隐藏文件
pdu -H

# 组合使用多个选项
pdu -h -d 3 -g -S
```

## 🌈 pps - 优化版 ps

### 🚀 基本使用
```bash
pps                     # 显示当前用户的进程
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-a` | 显示所有进程 | `pps -a` |
| `-u` | 显示用户信息 | `pps -u` |
| `-t` | 显示线程信息 | `pps -t` |
| `-T` | 树形结构显示 | `pps -T` |
| `-f` | 显示完整命令行 | `pps -f` |
| `-c` | 按CPU使用率排序 | `pps -c` |
| `-m` | 按内存使用率排序 | `pps -m` |
| `-e` | 显示环境变量 | `pps -e` |
| `-l` | 长格式显示 | `pps -l` |
| `-x` | 显示没有终端的进程 | `pps -x` |
| `--help` | 显示帮助信息 | `pps --help` |
| `--version` | 显示版本信息 | `pps --version` |

### 💡 示例
```bash
# 显示所有进程
pps -a

# 显示用户和线程信息
pps -u -t

# 树形显示
pps -T

# 按CPU和内存排序
pps -c -m

# 显示完整命令行
pps -f

# 长格式显示
pps -l

# 显示没有终端的进程
pps -x

# 组合使用多个选项
pps -aux -c
```

## 🚀 高级用法

### 🔗 组合使用
```bash
# 查找并搜索
pfind . -name "*.c" | xargs pgrep -n "TODO"

# 列出并过滤
pls -l | pgrep "\.c$"

# 显示并搜索
pcat -n main.c | pgrep "include"

# 查找大文件并显示详细信息
pfind . -size +100M | xargs pls -lh
```

### 🔄 管道操作
```bash
# 将pls输出传递给pgrep
pls | pgrep "\.txt$"

# 将pfind结果传递给pcat
pfind . -name "*.c" | head -5 | xargs pcat -n

# 查找并监控进程
pgrep -r "nginx" . | xargs ptop -p

# 分析磁盘使用并排序
pdu -h | sort -hr
```

### 📜 脚本集成
```bash
#!/bin/bash
# 查找并显示C文件中的TODO注释
pfind . -name "*.c" | while read file; do
    echo "=== $file ==="
    pgrep -n "TODO" "$file"
done

# 监控系统资源
#!/bin/bash
while true; do
    clear
    echo "=== 系统监控 ==="
    ptop -n 10 -c
    echo ""
    pdu -s
    sleep 5
done

# 批量处理文件
#!/bin/bash
# 查找所有日志文件并压缩
pfind /var/log -name "*.log" -mtime +7 | while read file; do
    echo "压缩: $file"
    gzip "$file"
done
```

### 🎯 实用技巧

#### 1. 快速文件管理
```bash
# 快速查看大文件
pfind . -size +10M -exec pls -lh {} \;

# 查找并删除临时文件
pfind /tmp -name "*.tmp" -mtime +1 -delete

# 批量重命名文件
pfind . -name "*.txt" | while read file; do
    mv "$file" "${file%.txt}.bak"
done
```

#### 2. 系统监控
```bash
# 实时监控系统状态
watch -n 1 'ptop -n 5; echo ""; pdu -s'

# 监控特定进程
ptop -p $(pgrep -f "nginx")

# 查看系统负载
pps -aux | pgrep -c "R"
```

#### 3. 开发辅助
```bash
# 查找代码中的问题
pfind . -name "*.c" -o -name "*.h" | xargs pgrep -n "TODO\|FIXME\|BUG"

# 统计代码行数
pfind . -name "*.c" | xargs pcat | wc -l

# 查找函数定义
pgrep -r "^[a-zA-Z_][a-zA-Z0-9_]*\s*(" *.c
```
