# 一阶逻辑表达式解析与量词极性分析 — 使用指南（含可选 Bison 语法层）

## 功能概述
- 读取一阶逻辑公式（逐行处理）
- 输出各阶段结果：词法 token、解析得到的 AST、IFF 展开后的 AST、量词极性判断
- 支持 ASCII/Unicode 两套符号（Windows 终端建议使用 ASCII；Unicode 需 UTF-8 输出）
- 支持将每行的完整处理结果导出为 JSONL（便于撰写实验报告）
- 可选启用 Bison 语法层（`--bison`）：在保留手写词法的前提下使用 Bison 解析；若不可用或解析失败自动回退到递归下降解析器

## 目录结构
- `syntax.h`：AST 结构、枚举与接口声明
- `src/lexer.h`、`src/lexer.c`：手写词法分析（支持 `forall/exists/!/&/|/->/<->` 与 `∀/∃/¬/∧/∨/→/↔`）
- `src/ast.c`：AST 构造与释放
- `src/rdparser.h`、`src/rdparser.c`：递归下降解析器（按优先级解析）
- `src/transform.c`：IFF 展开与深拷贝
- `src/polarity.c`：量词极性分析
- `src/print.h`、`src/print.c`：AST 可读化输出与格式化字符串
- `src/sb.h`、`src/sb.c`：简单字符串 builder（用于 JSON 输出）
- `src/main.c`：命令行程序，串起“词法→语法→IFF→极性”各阶段
- `src/parser.y`：可选 Bison 语法文件（使用 BP_* 记号前缀，避免与词法枚举冲突）
- `src/bison_adapter.h`、`src/bison_adapter.c`：Bison 适配器（用手写 `Lexer` 驱动 Bison）
- `CMakeLists.txt`：构建脚本（Windows 使用 Visual Studio 生成器）
- 测试程序：`src/test_ast.c`、`src/test_parser.c`、`src/test_transform.c`

## 构建
要求：安装 CMake 与编译器（Windows 推荐安装 Visual Studio Build Tools）。

- 生成构建文件：
  - `cmake -S . -B build`
- 编译 Release：
  - `cmake --build build --config Release`

可选启用 Bison（自动检测）：
- 若系统存在 `bison` 或 `win_bison`，构建时会自动生成并链接 `parser.c/parser.h`，主程序支持 `--bison` 开关。
- 若未安装 Bison，构建与运行不受影响，始终使用递归下降解析器。

启用并运行内置测试（可选）：
- 启用测试：
  - `cmake -S . -B build -DBUILD_TESTING=ON`
- 运行测试：
  - `ctest -C Release -VV`

## 输入语法（ASCII/Unicode）
- 量词：ASCII `forall`、`exists`；Unicode `∀`、`∃`
- 否定：ASCII `!`；Unicode `¬`
- 合取：ASCII `&`；Unicode `∧`
- 析取：ASCII `|`；Unicode `∨`
- 蕴含：ASCII `->`；Unicode `→`
- IFF：ASCII `<->`；Unicode `↔`
- 括号/逗号/点：`(`、`)`、`,`、`.`
- 标识符/数字：`[A-Za-z_][A-Za-z0-9_]*`、`[0-9]+`

优先级从低到高：`↔` < `→` < `∨` < `∧` < `¬` < 原子/括号/量词。

## 运行与阶段输出（逐行处理）
主程序可执行文件路径：`./build/Release/fol.exe`

常用开关：
- `--tokens`：打印词法 token 序列（ASCII 统一呈现）
- `--print-ast`：打印解析得到的 AST
- `--print-expanded`：打印 IFF 展开后的 AST
- `--json`：以 JSON 行输出每行的完整处理结果；字段为 `line/input/tokens/ast/expanded/polarity/valid/error`
- `--out <file>`：与 `--json` 配合使用，追加写入到指定文件
- `--bison`：优先使用 Bison 解析；若 Bison 不可用或解析失败会自动回退为递归下降解析
- `--out-text`、`--out-combined`：打印友好文本/树形结构，便于直接引用到报告

逐行处理输出格式：每行以 `== Line N ==` 为分隔，随后输出开启的各阶段结果与极性分析。

### PowerShell 多行示例（可视化输出）
```
$s = @'
(forall x. P(x))
((forall y. Q(y)) & (exists z. R(z)))
P(x) <-> Q(x)
'@
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$s | ./build/Release/fol.exe --tokens --print-ast --print-expanded
```

Bison 路径示例（可选）：
```
$s = @'
(forall x. P(x))
((forall y. Q(y)) & (exists z. R(z)))
P(x) <-> Q(x)
'@
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$s | ./build/Release/fol.exe --bison --tokens --print-ast --print-expanded
```

示例输出（节选）：
- Line 1：
  - tokens：`( FORALL IDENT(x) . IDENT(P) ( IDENT(x) ) )`
  - AST：`forall x. P(x)`
  - expanded：`forall x. P(x)`
  - polarity：`forall x : positive`
- Line 2：
  - tokens：`( ( FORALL IDENT(y) . Q ( y ) ) & ( EXISTS IDENT(z) . R ( z ) ) )`
  - AST：`(forall y. Q(y) & exists z. R(z))`
  - expanded：`(forall y. Q(y) & exists z. R(z))`
  - polarity：`forall y : positive;exists z : positive`
- Line 3：
  - tokens：`P ( x ) <-> Q ( x )`
  - AST：`(P(x) <-> Q(x))`
  - expanded：`((P(x) -> Q(x)) & (Q(x) -> P(x)))`
  - polarity：`（此行无量词，空）`

### 从文件批量处理
- 将公式逐行写入 `tests.txt`
- 可视化输出：
  - `./build/Release/fol.exe --tokens --print-ast --print-expanded < tests.txt`
- 导出 JSONL 报告（便于实验报告引用/分析）：
  - `./build/Release/fol.exe --json --out report.jsonl < tests.txt`
- JSON 行示例：
  ```json
  {"line":2,"input":"P(x) <-> Q(x)","tokens":"P ( x ) <-> Q ( x ) ","ast":"(P(x) <-> Q(x))","expanded":"((P(x) -> Q(x)) & (Q(x) -> P(x)))","polarity":"","valid":true,"error":""}
  ```
- 若语句解析失败也会写入 JSON（`valid:false,error:"syntax error"`），便于定位问题行
- 查看前几条记录（PowerShell）：
  - `Get-Content -Encoding UTF8 report.jsonl | Select-Object -First 5`

## 已集成测试程序（可选）
- `test_ast`：验证 AST 构造与释放
- `test_parser`：验证解析优先级与量词、连接词处理
- `test_transform`：验证 IFF 展开

运行示例：
- 启用测试后，执行：`ctest -C Release -VV`

## Windows 使用提示
- PowerShell 中输入含 `<`、`&` 等符号时，建议使用 here-string 或文件重定向；否则可能被当成重定向/运算符。
- 若需显示 Unicode 运算符（`∀`、`∃`、`¬`、`∧`、`∨`、`→`、`↔`），建议设置控制台输出为 UTF-8：
  - `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8`
- 为避免编码警告，源码已尽量使用 ASCII；功能上同时支持 ASCII 与 Unicode。

## Bison 混合方案说明
- 启用条件：系统检测到 `bison` 或 `win_bison`，构建时自动生成并链接 `parser.c/parser.h`。
- 记号前缀：Bison 语法文件使用 `BP_*` 作为记号前缀，避免与手写词法的枚举值冲突。
- 适配器：`bison_adapter.c` 使用手写 `Lexer` 提供记号给 Bison；`--bison` 开关启用此路径。
- 回退机制：若 Bison 不可用或 `yyparse` 返回错误，主程序自动回退为递归下降解析。

## 注意与限制
- 常规模式下解析错误会提示 `PARSE_ERROR` 并跳过该行；在 `--json` 模式下同样会输出一条带 `valid:false` 的记录。
- IFF 展开后结构符合 `(φ -> ψ) & (ψ -> φ)`，极性分析按根正、否定翻转、蕴含左负右正、合/析保持的规则执行。

## 许可证
- 课程实验用途，按课程要求使用。

本项目实现对一阶逻辑公式的词法分析、递归下降语法分析、抽象语法树（AST）构建、`↔` 等价展开以及量词正/负极性输出，接口结构与 `syntax.h` 对应。
