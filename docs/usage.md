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
- [dsa - 图片查看器](#dsa---图片查看器)
- [pdate - 日期时间](#pdate---日期时间)
- [pecho - 文本输出](#pecho---文本输出)
- [pcal - 日历显示](#pcal---日历显示)
- [pwhich - 命令查找](#pwhich---命令查找)
- [pbc - 基础计算器](#pbc---基础计算器)
- [pseq - 数字序列](#pseq---数字序列)
- [pman - 手册页查看](#pman---手册页查看)
- [pwhereis - 文件定位](#pwhereis---文件定位)
- [pwhatis - 命令描述](#pwhatis---命令描述)
- [pshuf - 随机排序](#pshuf---随机排序)
- [pyes - 重复输出](#pyes---重复输出)
- [ptee - 分流输出](#ptee---分流输出)
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

## 🖼️ dsa - 图片查看器

### 🚀 基本使用
```bash
dsa image.jpg          # 显示图片
dsa image.png 120      # 指定宽度显示图片
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-h, --help` | 显示帮助信息 | `dsa --help` |
| `-v, --version` | 显示版本信息 | `dsa --version` |
| `-c, --color` | 启用颜色显示 (默认) | `dsa -c image.jpg` |
| `-n, --no-color` | 禁用颜色显示 | `dsa -n image.jpg` |
| `-w, --width` | 指定显示宽度 | `dsa --width 100 image.png` |

### 💡 示例
```bash
# 显示图片（默认宽度80）
dsa photo.jpg

# 指定宽度显示图片
dsa image.png 120

# 启用颜色显示
dsa -c colorful.jpg

# 组合使用多个选项
dsa -c --width 100 landscape.png

# 显示帮助信息
dsa --help

# 显示版本信息
dsa --version
```

### 🎨 特色功能
- **ASCII艺术转换**：将图片转换为ASCII字符艺术
- **颜色支持**：可选的彩色显示模式
- **格式支持**：支持JPEG和PNG格式
- **可调宽度**：自定义显示宽度
- **无依赖**：使用stb_image库，无需外部依赖

### 📝 使用技巧
```bash
# 查看大图片时使用较小宽度
dsa large_image.jpg 60

# 查看细节时使用较大宽度
dsa detail.png 150

# 启用颜色查看彩色图片
dsa -c colorful_image.jpg

# 批量查看多张图片
for img in *.jpg; do
    echo "=== $img ==="
    dsa "$img" 80
    echo ""
done
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

---

## 📅 pdate - 日期时间

### 🚀 基本使用
```bash
pdate                    # 显示当前日期时间
pdate +%Y-%m-%d         # 显示日期 (YYYY-MM-DD)
pdate +%H:%M:%S         # 显示时间 (HH:MM:SS)
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-I, --iso-8601` | ISO 8601格式 | `pdate --iso-8601` |
| `-R, --rfc-email` | RFC 5322格式 | `pdate --rfc-email` |
| `-u, --utc` | 显示UTC时间 | `pdate --utc` |
| `--timestamp` | 显示时间戳 | `pdate --timestamp` |
| `--epoch` | 显示Unix时间戳 | `pdate --epoch` |
| `--weekday` | 显示星期几 | `pdate --weekday` |
| `--timezone=TZ` | 指定时区 | `pdate --timezone=UTC` |

### 💡 示例
```bash
# 显示当前时间
pdate

# 显示ISO格式时间
pdate --iso-8601

# 显示UTC时间
pdate --utc

# 显示时间戳
pdate --timestamp

# 显示星期几
pdate --weekday

# 自定义格式
pdate +"%Y年%m月%d日 %H:%M:%S"
```

### 🎨 特色功能
- **多格式支持**：支持多种日期时间格式
- **时区支持**：支持UTC和本地时区
- **彩色输出**：美观的彩色显示
- **格式灵活**：支持自定义格式字符串

---

## 🎨 pecho - 文本输出

### 🚀 基本使用
```bash
pecho "Hello World"     # 输出文本
pecho -n "Hello"        # 不换行输出
pecho -e "Hello\nWorld" # 解释转义字符
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-n, --no-newline` | 不输出换行符 | `pecho -n "Hello"` |
| `-e, --enable-escape` | 启用转义字符解释 | `pecho -e "Hello\nWorld"` |
| `-E, --disable-escape` | 禁用转义字符解释 | `pecho -E "Hello\nWorld"` |
| `--help-escapes` | 显示转义字符帮助 | `pecho --help-escapes` |

### 💡 示例
```bash
# 基本输出
pecho "Hello World"

# 不换行输出
pecho -n "Hello "
pecho "World"

# 转义字符
pecho -e "Hello\nWorld\tTest"

# 十六进制字符
pecho -e "Hello\x20World"

# 八进制字符
pecho -e "Hello\041"
```

### 🎨 特色功能
- **转义字符支持**：完整的转义字符支持
- **彩色输出**：美观的彩色显示
- **格式灵活**：支持多种输出格式

---

## 📆 pcal - 日历显示

### 🚀 基本使用
```bash
pcal                    # 显示当前月份
pcal 2024               # 显示2024年
pcal 3 2024             # 显示2024年3月
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-3, --three-months` | 显示前月、当月、下月 | `pcal -3` |
| `-y, --year` | 显示整年日历 | `pcal -y` |
| `-h, --help` | 显示帮助信息 | `pcal --help` |

### 💡 示例
```bash
# 显示当前月份
pcal

# 显示2024年
pcal 2024

# 显示2024年3月
pcal 3 2024

# 显示三个月
pcal -3

# 显示整年
pcal -y
```

### 🎨 特色功能
- **美观显示**：彩色日历显示
- **今天高亮**：突出显示今天
- **多月份支持**：支持显示多个月份
- **整年显示**：支持显示整年日历

---

## 🔍 pwhich - 命令查找

### 🚀 基本使用
```bash
pwhich ls               # 查找ls命令路径
pwhich -a gcc           # 显示所有gcc路径
pwhich -v python        # 详细显示python信息
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-a, --all` | 显示所有匹配的路径 | `pwhich -a gcc` |
| `-i, --read-alias` | 读取别名 | `pwhich -i ls` |
| `-f, --read-functions` | 读取函数 | `pwhich -f myfunc` |
| `-b, --read-builtins` | 读取内置命令 | `pwhich -b cd` |
| `-q, --quiet` | 静默模式 | `pwhich -q bash` |
| `-v, --verbose` | 详细模式 | `pwhich -v python` |

### 💡 示例
```bash
# 查找命令路径
pwhich ls

# 显示所有匹配路径
pwhich -a gcc

# 详细模式
pwhich -v python

# 静默模式
pwhich -q bash

# 查找多个命令
pwhich ls cat grep
```

### 🎨 特色功能
- **路径查找**：在PATH中查找命令
- **详细信息**：显示详细的查找信息
- **多路径支持**：支持显示所有匹配路径
- **彩色输出**：美观的彩色显示

---

## 🧮 pbc - 基础计算器

### 🚀 基本使用
```bash
pbc                     # 启动交互模式
pbc "2 + 3 * 4"         # 计算表达式
pbc "sqrt(16)"          # 计算平方根
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-i, --interactive` | 交互模式 (默认) | `pbc -i` |
| `-p, --precision=N` | 设置精度 | `pbc -p 4` |
| `-h, --history` | 显示计算历史 | `pbc -h` |
| `-q, --quiet` | 静默模式 | `pbc -q` |

### 💡 示例
```bash
# 交互模式
pbc

# 计算表达式
pbc "2 + 3 * 4"
pbc "sqrt(16)"
pbc "sin(pi/2)"

# 设置精度
pbc -p 4 "22/7"

# 显示历史
pbc --history
```

### 🎨 特色功能
- **数学运算**：支持基本数学运算
- **函数支持**：支持数学函数
- **交互模式**：友好的交互界面
- **计算历史**：保存计算历史
- **精度控制**：可设置输出精度

---

## 🔢 pseq - 数字序列

### 🚀 基本使用
```bash
pseq 5                  # 1 2 3 4 5
pseq 2 5                # 2 3 4 5
pseq 1 2 10             # 1 3 5 7 9
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-f, --format=FORMAT` | 使用printf格式 | `pseq -f "%02d" 1 5` |
| `-s, --separator=STR` | 使用指定分隔符 | `pseq -s "," 1 5` |
| `-w, --equal-width` | 等宽格式 | `pseq -w 1 100` |
| `--help-format` | 显示格式帮助 | `pseq --help-format` |
| `--help-separator` | 显示分隔符帮助 | `pseq --help-separator` |

### 💡 示例
```bash
# 基本序列
pseq 5

# 指定范围
pseq 2 5

# 指定步长
pseq 1 2 10

# 格式化输出
pseq -f "%02d" 1 5

# 自定义分隔符
pseq -s "," 1 5

# 等宽格式
pseq -w 1 100
```

### 🎨 特色功能
- **序列生成**：生成数字序列
- **格式支持**：支持printf格式
- **分隔符**：支持自定义分隔符
- **等宽格式**：支持等宽数字显示

---

## 📚 pman - 手册页查看

### 🚀 基本使用
```bash
pman ls                  # 查看ls命令手册
pman -s 2 open           # 查看open系统调用
pman -f ls               # 显示ls的简短描述
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-s, --section=SECTION` | 指定手册页章节 | `pman -s 2 open` |
| `-a, --all` | 显示所有匹配的手册页 | `pman -a ls` |
| `-f, --whatis` | 显示命令的简短描述 | `pman -f ls` |
| `-k, --apropos` | 搜索关键词 | `pman -k "file system"` |
| `-w, --whereis` | 显示手册页位置 | `pman -w ls` |
| `-l, --list-sections` | 列出所有章节 | `pman -l` |

### 💡 示例
```bash
# 查看命令手册
pman ls
pman -s 2 open

# 显示命令描述
pman -f ls

# 搜索相关命令
pman -k "file system"

# 显示手册页位置
pman -w ls

# 列出所有章节
pman -l
```

### 🎨 特色功能
- **章节支持**：支持指定手册页章节
- **搜索功能**：支持关键词搜索
- **位置查找**：显示手册页文件位置
- **彩色输出**：美观的彩色显示

---

## 🔍 pwhereis - 文件定位

### 🚀 基本使用
```bash
pwhereis ls              # 查找ls的所有文件
pwhereis -b gcc          # 只查找gcc的二进制文件
pwhereis -m ls           # 只查找ls的手册页
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-b, --binaries` | 只查找二进制文件 | `pwhereis -b gcc` |
| `-m, --manuals` | 只查找手册页 | `pwhereis -m ls` |
| `-s, --sources` | 只查找源代码 | `pwhereis -s gcc` |
| `-a, --all` | 查找所有类型 | `pwhereis -a gcc` |
| `-v, --verbose` | 详细模式 | `pwhereis -v python` |

### 💡 示例
```bash
# 查找所有文件
pwhereis ls

# 只查找二进制文件
pwhereis -b gcc

# 只查找手册页
pwhereis -m ls

# 只查找源代码
pwhereis -s gcc

# 详细模式
pwhereis -v python
```

### 🎨 特色功能
- **多类型支持**：支持查找二进制、手册页、源代码
- **详细模式**：显示详细的查找信息
- **彩色输出**：美观的彩色显示

---

## 📝 pwhatis - 命令描述

### 🚀 基本使用
```bash
pwhatis ls               # 显示ls的描述
pwhatis -w "*grep*"      # 使用通配符搜索
pwhatis -e ls            # 精确匹配ls
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-w, --wildcard` | 使用通配符匹配 | `pwhatis -w "*grep*"` |
| `-e, --exact` | 精确匹配 | `pwhatis -e ls` |
| `-v, --verbose` | 详细模式 | `pwhatis -v python` |

### 💡 示例
```bash
# 显示命令描述
pwhatis ls

# 使用通配符搜索
pwhatis -w "*grep*"

# 精确匹配
pwhatis -e ls

# 详细模式
pwhatis -v python
```

### 🎨 特色功能
- **通配符支持**：支持通配符搜索
- **精确匹配**：支持精确匹配
- **详细模式**：显示详细信息
- **彩色输出**：美观的彩色显示

---

## 🎲 pshuf - 随机排序

### 🚀 基本使用
```bash
pshuf file.txt           # 打乱文件行
pshuf -e A B C           # 打乱指定字符串
pshuf -i 1-10            # 打乱数字1-10
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-e, --echo` | 将每个ARG视为输入行 | `pshuf -e A B C` |
| `-i, --input-range=LO-HI` | 将LO到HI的每个数字视为输入行 | `pshuf -i 1-10` |
| `-n, --head-count=COUNT` | 最多输出COUNT行 | `pshuf -n 5 file.txt` |

### 💡 示例
```bash
# 打乱文件行
pshuf file.txt

# 打乱指定字符串
pshuf -e A B C D E

# 打乱数字范围
pshuf -i 1-10

# 随机选择5行
pshuf -n 5 file.txt
```

### 🎨 特色功能
- **随机排序**：使用Fisher-Yates洗牌算法
- **多种输入**：支持文件、字符串、数字范围
- **随机选择**：支持随机选择指定行数
- **彩色输出**：美观的彩色显示

---

## 🔄 pyes - 重复输出

### 🚀 基本使用
```bash
pyes                     # 重复输出 y
pyes hello               # 重复输出 hello
pyes "hello world"       # 重复输出 hello world
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-v, --version` | 显示版本信息 | `pyes -v` |
| `-h, --help` | 显示帮助信息 | `pyes -h` |
| `--help-usage` | 显示使用说明 | `pyes --help-usage` |
| `--help-examples` | 显示使用示例 | `pyes --help-examples` |
| `--help-signals` | 显示信号处理说明 | `pyes --help-signals` |

### 💡 示例
```bash
# 基本用法
pyes
pyes hello

# 与管道结合
pyes | rm -i file
pyes | cp -i src dest

# 重定向输出
pyes hello > output.txt
pyes | head -n 100
```

### 🎨 特色功能
- **重复输出**：无限重复输出指定字符串
- **信号处理**：优雅处理中断信号
- **管道支持**：与管道命令结合使用
- **彩色输出**：美观的彩色显示

---

## 📤 ptee - 分流输出

### 🚀 基本使用
```bash
ptee file.txt            # 输出到文件
ptee -a file.txt         # 追加到文件
ptee file1 file2         # 输出到多个文件
```

### ⚙️ 选项
| 选项 | 说明 | 示例 |
|------|------|------|
| `-a, --append` | 追加到文件而不是覆盖 | `ptee -a file.txt` |
| `-i, --ignore-interrupts` | 忽略中断信号 | `ptee -i file.txt` |
| `-p, --preserve` | 保留文件权限 | `ptee -p file.txt` |
| `--help-usage` | 显示使用说明 | `ptee --help-usage` |
| `--help-examples` | 显示使用示例 | `ptee --help-examples` |

### 💡 示例
```bash
# 基本用法
echo 'hello' | ptee file.txt
echo 'world' | ptee -a file.txt

# 多文件输出
echo 'test' | ptee file1.txt file2.txt

# 与管道结合
ls -la | ptee filelist.txt
ps aux | ptee process.txt | grep 'python'
```

### 🎨 特色功能
- **分流输出**：同时输出到标准输出和文件
- **多文件支持**：支持输出到多个文件
- **追加模式**：支持追加到现有文件
- **彩色输出**：美观的彩色显示
