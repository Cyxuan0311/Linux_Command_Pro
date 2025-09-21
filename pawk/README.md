# pawk - 优化版 awk 命令

## 项目结构

### 模块化设计

pawk 采用模块化设计，将功能分解为独立的模块，提高代码的可维护性和可扩展性。

```
pawk/
├── include/                 # 头文件
│   ├── Constants.h         # 常量和配置
│   ├── Utils.h             # 工具函数
│   ├── Field.h             # 字段类
│   ├── Record.h            # 记录类
│   ├── BuiltinFunctions.h  # 内置函数
│   ├── Variables.h         # 变量管理
│   ├── Pattern.h           # 模式匹配
│   ├── Action.h            # 动作执行
│   ├── CommandLineParser.h # 命令行解析
│   └── AwkEngine.h         # 主引擎
├── src/                    # 源文件
│   ├── pawk_new.cpp        # 新的主程序
│   ├── Utils.cpp           # 工具函数实现
│   ├── Field.cpp           # 字段类实现
│   ├── Record.cpp          # 记录类实现
│   ├── BuiltinFunctions.cpp # 内置函数实现
│   ├── Variables.cpp       # 变量管理实现
│   ├── Pattern.cpp         # 模式匹配实现
│   ├── Action.cpp          # 动作执行实现
│   ├── CommandLineParser.cpp # 命令行解析实现
│   └── AwkEngine.cpp       # 主引擎实现
├── modules/                # 扩展模块（预留）
├── pawk.cpp               # 原始实现（向后兼容）
└── CMakeLists.txt         # 构建配置
```

### 核心模块

#### 1. 常量和工具模块 (Constants.h, Utils.h)
- **Constants.h**: 定义颜色、默认值、版本信息等常量
- **Utils.h**: 提供字符串处理、类型检查、格式化等工具函数

#### 2. 核心数据类 (Field.h, Record.h)
- **Field**: 表示记录中的单个字段，支持类型转换、匹配、替换等操作
- **Record**: 表示一行数据，包含多个字段，支持字段访问、解析、输出等操作

#### 3. 内置函数模块 (BuiltinFunctions.h)
- 数学函数: sqrt, sin, cos, log, exp 等
- 字符串函数: length, substr, toupper, tolower 等
- 时间函数: systime, strftime 等
- 类型转换函数: sprintf 等

#### 4. 变量管理模块 (Variables.h)
- **Variable**: 支持字符串、数值、数组三种类型的变量
- **Variables**: 管理所有变量，包括内置变量和用户定义变量

#### 5. 模式匹配模块 (Pattern.h)
- **Pattern**: 抽象基类
- **RegexPattern**: 正则表达式模式
- **ExpressionPattern**: 表达式模式
- **RangePattern**: 范围模式
- **FieldPattern**: 字段模式
- **CompoundPattern**: 复合模式

#### 6. 动作执行模块 (Action.h)
- **Action**: 抽象基类
- **PrintAction**: 打印动作
- **PrintfAction**: 格式化打印动作
- **AssignmentAction**: 赋值动作
- **IfAction**: 条件动作
- **ForAction**: 循环动作
- **WhileAction**: while循环动作
- **FunctionCallAction**: 函数调用动作
- **BlockAction**: 代码块动作

#### 7. 命令行解析模块 (CommandLineParser.h)
- 解析命令行参数
- 提供帮助和版本信息
- 支持各种选项和变量设置

#### 8. 主引擎模块 (AwkEngine.h)
- 协调各个模块
- 处理文件和数据流
- 执行模式-动作对
- 管理统计信息

### 设计优势

#### 1. 模块化
- 每个模块职责单一，易于理解和维护
- 模块间依赖关系清晰，便于测试和调试
- 支持独立开发和测试

#### 2. 可扩展性
- 易于添加新的模式类型
- 易于添加新的动作类型
- 易于添加新的内置函数
- 支持插件系统（预留）

#### 3. 可维护性
- 代码结构清晰，易于阅读
- 错误处理集中，便于调试
- 文档完善，便于理解

#### 4. 性能优化
- 智能指针管理内存
- 避免不必要的拷贝
- 高效的字符串处理

### 使用方法

#### 编译
```bash
mkdir build && cd build
cmake ..
make
```

#### 运行
```bash
# 基本使用
./pawk '{print $1}' file.txt

# 使用字段分隔符
./pawk -F: '{print $1, $3}' /etc/passwd

# 使用模式
./pawk '/pattern/ {print $0}' file.txt

# 使用变量
./pawk -v var=value '{print var}' file.txt

# 从文件读取程序
./pawk -f program.awk file.txt
```

### 扩展开发

#### 添加新的模式类型
1. 继承 `Pattern` 类
2. 实现 `matches` 和 `getDescription` 方法
3. 在 `PatternFactory` 中注册

#### 添加新的动作类型
1. 继承 `Action` 类
2. 实现 `execute` 和 `getDescription` 方法
3. 在 `ActionFactory` 中注册

#### 添加新的内置函数
1. 在 `BuiltinFunctions` 类中添加静态方法
2. 在 `Action` 中调用相应的函数

### 向后兼容性

项目保留了原始的 `pawk.cpp` 文件，编译为 `pawk_old` 可执行文件，确保向后兼容性。

### 未来计划

1. 实现完整的AWK语法解析器
2. 添加更多内置函数
3. 支持用户自定义函数
4. 添加插件系统
5. 性能优化和内存管理改进
6. 添加单元测试和集成测试
