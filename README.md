# 一阶逻辑表达式解析与量词极性分析 — 使用指南

## 功能概述
- 读取一阶逻辑公式（逐行处理）
- 输出各阶段结果：词法 token、解析得到的 AST、IFF 展开后的 AST、量词极性判断
- 支持 ASCII/Unicode 两套符号（Windows 终端建议使用 ASCII；Unicode 需 UTF-8 输出）
- 支持将每行的完整处理结果导出为 JSONL（便于撰写实验报告）

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
- `CMakeLists.txt`：构建脚本（Windows 使用 Visual Studio 生成器）
- 测试程序：`src/test_ast.c`、`src/test_parser.c`、`src/test_transform.c`

## 构建
要求：安装 CMake 与编译器（Windows 推荐安装 Visual Studio Build Tools）。

- 生成构建文件：
  - `cmake -S . -B build`
- 编译 Release：
  - `cmake --build build --config Release`

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
- `--json`：以 JSON 行输出每行的完整处理结果
- `--out <file>`：与 `--json` 配合使用，追加写入到指定文件

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

## 注意与限制
- 解析错误会提示 `PARSE_ERROR` 并跳过该行；建议使用 ASCII 形式确保跨平台稳定性。
- IFF 展开后结构符合 `(φ -> ψ) & (ψ -> φ)`，极性分析按根正、否定翻转、蕴含左负右正、合/析保持的规则执行。

## 许可证
- 课程实验用途，按课程要求使用。

本项目实现对一阶逻辑公式的词法分析、递归下降语法分析、抽象语法树（AST）构建、`↔` 等价展开以及量词正/负极性输出，接口结构与 `syntax.h` 对应。

---

#### 编译与运行

依赖：`flex`、`bison`、`gcc`。

```bash
make
./fol
```

手动编译示例：

```bash
flex fol.l
bison -d fol.y
gcc -o fol lex.yy.c fol.tab.c ast.c analyze.c main.c -Wall
```

---

#### 文件结构

| 文件 / 目录 | 说明 |
|-------------|------|
| `syntax.h`  | 抽象语法树结构定义 |
| `fol.l`     | Flex 词法规则 |
| `fol.y`     | Bison 语法规则，生成 AST |
| `ast.c`     | AST 节点构造与复制、释放 |
| `analyze.c` | 量词极性分析 |
| `main.c`    | 程序入口，完成解析与分析 |
| `Makefile`  | 构建脚本 |
| `tests.txt` | 示例与边界测试用例 |

---

#### 词法规则与 Token

| Token | 形式 |
|-------|------|
| `IDENT` | `[A-Za-z_][A-Za-z0-9_]*`（变量、函数、谓词名） |
| `NUMBER` | `[0-9]+` |
| 量词 | `∀`、`∃` |
| 逻辑符号 | `¬`/`!`/`~`，`∧`/`&`，`∨`/`|`，`->`/`→`，`<->`/`↔` |
| 界定符 | `(` `)` `,` `.` |
| 空白 | 忽略 |

未知字符会触发解析错误。

---

#### 语法（BNF）

```
Formula    ::= IffExpr
IffExpr    ::= ImplyExpr | IffExpr IFF ImplyExpr
ImplyExpr  ::= OrExpr | ImplyExpr IMPLY OrExpr
OrExpr     ::= AndExpr | OrExpr OR AndExpr
AndExpr    ::= UnaryExpr | AndExpr AND UnaryExpr
UnaryExpr  ::= NOT UnaryExpr | QuantExpr | Atom
QuantExpr  ::= FORALL IDENT DOT Formula | EXISTS IDENT DOT Formula
Atom       ::= IDENT "(" TermList ")" | "(" Formula ")"
TermList   ::= Term | TermList "," Term
Term       ::= IDENT "(" TermList ")" | IDENT | NUMBER
```

优先级（低→高）：`↔` < `→` < `∨` < `∧` < `¬/量词/原子`。

---

#### AST 映射（syntax.h）

- `Term`：变量 `Term_VarName`、常量 `Term_ConstNum`、函数项 `Term_UFTerm`
- `UFunction` / `UPredicate`：保存名称与参数数组
- `Prop`：
  - 二元：`Prop_AND`、`Prop_OR`、`Prop_IMPLY`、`Prop_IFF`
  - 一元：`Prop_NOT`
  - 量词：`Prop_FORALL`、`Prop_EXISTS`
  - 原子：`Prop_Atom`

`ast.c` 提供 `new_*` 系列构造函数、`clone_prop` 与递归 `free_prop`。

---

#### IFF 等价展开

`expand_iff` 将 `φ ↔ ψ` 转换为 `(φ → ψ) ∧ (ψ → φ)`，对子公式递归展开，避免树中残留 IFF 便于极性分析。

---

#### 量词极性分析

`analyze_polarity` 深度优先遍历 AST：

- 根为正极性；`¬` 翻转极性。
- `→`：左支翻转，右支保持；`∧/∨`：两支保持。
- 量词继承当前极性并即时输出，如 `∀x : positive`。

---

#### 测试

`tests.txt` 覆盖原子公式、嵌套函数、合取/析取/蕴含/双条件、ASCII 与 Unicode 混用、量词极性以及错误输入（缺括号、逗号错误、缺少点号等）。可用：

```bash
./fol < tests.txt
```

或交互输入单条公式检验词法、语法与极性分析结果。
