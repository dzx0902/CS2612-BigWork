# 一阶逻辑表达式解析与量词极性分析——提交文档

本文档根据课程要求编写，系统说明本项目的实现与验证情况。内容覆盖：编译与运行方式、词法设计、语法（BNF）、AST 结构、IFF 展开方法、量词极性分析方法以及完整测试样例与输出展示。

---

## 1. 编译与运行说明

### 1.1 环境依赖
- CMake ≥ 3.15
- C 编译器（GCC/Clang 均可，源码使用 C99）
- 可选：Bison（存在时可启用 `--bison` 解析路径）

### 1.2 构建步骤
```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
```

生成的可执行文件：
- `build/fol`：主程序
- `build/test_ast`、`build/test_parser`、`build/test_transform`、`build/test_polarity`、`build/test_errors`：自测程序

### 1.3 运行主程序
```bash
./build/fol --tokens --print-ast --print-expanded < tests.txt
```
常用开关：
- `--tokens`：打印词法 Token 序列
- `--print-ast` / `--print-tree`：输出原始 AST 文字或树形结构
- `--print-expanded`：输出 IFF 展开后的 AST
- `--json --out <file>`：每行结果编码为 JSONL，字段为 `line/input/tokens/ast/expanded/polarity/valid/error`
- `--bison`：若可用，优先通过 Bison 解析，失败时自动回退递归下降
- `--out-text`/`--out-combined`：将可视化结果写入文件，便于实验报告引用

若解析失败，`--json` 与 `--out-combined` 仍会写出对应行并将 `valid` 置为 `false`（`error` 字段写入错误原因），方便脚本自动排查。

### 1.4 JSON 报告示例
`tests.txt` 中第二行通过 `--json --out report.jsonl` 生成的记录如下，可直接用于实验报告或脚本分析：
```json
{"line":2,"input":"P(x) <-> Q(x)","tokens":"P ( x ) <-> Q ( x ) ","ast":"(P(x) <-> Q(x))","expanded":"((P(x) -> Q(x)) & (Q(x) -> P(x)))","polarity":"","valid":true,"error":""}
```
当输入非法时会得到 `valid:false` 的记录：
```json
{"line":61,"input":"P(x,","tokens":"P ( x , ","ast":"","expanded":"","polarity":"","valid":false,"error":"syntax error"}
```
`report_all.txt` 则展示了 `--out-combined` 生成的完整可视化文本（含 AST/树形输出）以及对应 JSON 片段，便于直接引用到文档。

---

## 2. Token 列表与词法设计

词法分析位于 `src/lexer.c`，核心枚举 `TokenType` 定义在 `src/lexer.h`。支持 ASCII 与 Unicode 混合表示，逐字符扫描、优先识别多字符运算符，必要时通过 UTF‑8 字节模式匹配。

| Token | 说明 | 典型字面量 |
|-------|------|------------|
| `TOK_IDENT` | 变量、函数、谓词名 | `P`、`f1`、`_x` |
| `TOK_NUMBER` | 常量数字 | `0`、`42` |
| `TOK_LPAREN` / `TOK_RPAREN` | 括号 | `(` / `)` |
| `TOK_COMMA` | 参数分隔符 | `,` |
| `TOK_DOT` | 量词与主体之间的点号 | `.` |
| `TOK_NOT` | 否定 | `!`、`¬` |
| `TOK_AND` | 合取 | `&`、`∧` |
| `TOK_OR` | 析取 | `|`、`∨` |
| `TOK_IMPLY` | 蕴含 | `->`、`→` |
| `TOK_IFF` | 双条件 | `<->`、`↔` |
| `TOK_FORALL` / `TOK_EXISTS` | 量词 | `forall/∃` 等 |
| `TOK_EOF` | 输入结束 | — |
| `TOK_INVALID` | 未知字符，用于报错 | — |

设计要点：
- 使用 `lexer_next` 单次返回一个 Token，所有开关均由主程序根据 Token 序列输出。
- `ident` 支持 `forall`/`exists` 关键词识别，便于 ASCII 量词。
- UTF‑8 运算符通过 `starts_with_bytes` 判断固定字节序列，保证 Windows/Linux 一致。

---

## 3. 语法说明（BNF）

递归下降解析器位于 `src/rdparser.c`，Bison 语法 `src/parser.y` 与其保持完全一致。优先级从低到高：`↔` < `→` < `∨` < `∧` < `¬` < 原子/量词/括号。

```
Formula   ::= IffExpr
IffExpr   ::= ImplyExpr | IffExpr IFF ImplyExpr
ImplyExpr ::= OrExpr | ImplyExpr IMPLY OrExpr
OrExpr    ::= AndExpr | OrExpr OR AndExpr
AndExpr   ::= UnaryExpr | AndExpr AND UnaryExpr
UnaryExpr ::= NOT UnaryExpr | QuantExpr | Atom
QuantExpr ::= FORALL IDENT DOT Formula
            | EXISTS IDENT DOT Formula
Atom      ::= Predicate | "(" Formula ")"
Predicate ::= IDENT "(" TermList ")"
TermList  ::= Term | TermList "," Term
Term      ::= IDENT | NUMBER | IDENT "(" TermList ")"
```

所有产生式直接映射到 AST 构造函数，解析错误会标记 `Parser.error` 并返回 `NULL`。

---

## 4. AST 节点结构说明（`syntax.h`）

核心结构定义在 `syntax.h`，由 `ast.c` 提供构造/释放函数。

### Term
```c
typedef enum { Term_UFTerm, Term_ConstNum, Term_VarName } TermType;
struct Term {
    TermType type;
    union { int ConstNum; char* Variable; UFunction* UFTerm; } term;
};
```

### UFunction / UPredicate
```c
struct UFunction { char* name; int numArgs; Term** args; };
struct UPredicate { char* name; int numArgs; Term** args; };
```
函数与谓词共享“名称 + 动态参数数组”的设计，支持任意嵌套。

### Prop（逻辑公式）
```c
typedef enum { Prop_Binop, Prop_Unop, Prop_Quant, Prop_Atom } PropType;
struct Prop {
    PropType type;
    union {
        struct { PropBop op; Prop *prop1, *prop2; } Binary_prop;
        struct { PropUop op; Prop *prop1; } Unary_prop;
        struct { PropQuant op; char* Variable; Prop *prop1; } Quant_prop;
        UPredicate *Atomic_prop;
    } prop;
};
```
- `PropBop`：`Prop_AND` / `Prop_OR` / `Prop_IMPLY` / `Prop_IFF`
- `PropUop`：目前只支持 `Prop_NOT`
- `PropQuant`：`Prop_FORALL` / `Prop_EXISTS`

所有节点通过 `new_prop_*` 构造，配合 `free_prop` 递归释放，避免内存泄漏。

---

## 5. IFF 变换方法说明

IFF 展开实现于 `src/transform.c` 的 `expand_iff`：

1. 遇到 `Prop_IFF` 时，先递归展开两侧 `φ`、`ψ`。
2. 构造 `(φ -> ψ)` 与 `(ψ -> φ)` 两棵新树（使用 `clone_prop`，确保原 AST 不被共享）。
3. 返回 `new_prop_binop(Prop_AND, ab, ba)`，即 `(φ -> ψ) ∧ (ψ -> φ)`。
4. 其他节点类型保持结构，只对孩子递归展开。

该函数保证：
- 返回的新树不含 `Prop_IFF`，便于后续极性分析。
- 所有新建节点独立，`free_prop` 不会造成 double free。

---

## 6. 量词极性分析方法说明

量词极性遍历定义在 `src/polarity.c`，算法规则：

1. 根节点默认正极性（`positive=true`）。
2. `¬φ`：对子公式传入反转后的极性。
3. `φ → ψ`：左子树传入“当前极性的取反”，右子树保持（保证左支在负环境下逐层累积）。
4. `φ ∧ ψ` / `φ ∨ ψ`：两子树继承当前极性。
5. 量词节点：即时输出 `forall/exists name : positive/negative`，然后对子公式递归，保持极性。

主程序有两条使用路径：
- `%stdout%` 展示：直接调用 `analyze_polarity(expanded_ast, true)`，逐条打印量词极性；
- JSON/报告场景：调用 `format_polarity(expanded_ast, true)`，将所有结果拼接成 `forall x : positive;exists y : negative` 形式的字符串写入记录。

---

## 7. 测试样例与输出展示

### 7.1 内置测试
`CMakeLists.txt` 将以下用例纳入 `ctest`：
- `test_ast`：手动构造复杂 AST、释放验证
- `test_parser`：解析蕴含/双条件与优先级组合
- `test_transform`：`expand_iff` 功能
- `test_polarity`：多量词链路、`<->` 嵌套场景
- `test_errors`：覆盖合法/非法输入、括号/逗号/量词误用等边界

运行：
```bash
ctest -C Release -VV
```
示例输出节选：
```
test 4
    Start 4: test_polarity
4: POLARITY_OK
```

### 7.2 主程序演示
输入 `tests.txt` 片段：
```
(forall x. P(x)) -> ((forall y. Q(y)) & (exists z. R(z)))
P(x) <-> Q(x)
∀x. (P(x) ∧ ∃y. Q(y))
```
命令与输出（开启 tokens + AST + IFF 展开）：
```
== Line 1 ==
IDENT(P) ( IDENT(x) ) -> ...
(forall x. P(x) -> ((forall y. Q(y)) & (exists z. R(z))))
(forall x. P(x) -> ((forall y. Q(y)) & (exists z. R(z))))
forall x : negative
forall y : positive
exists z : positive

== Line 2 ==
P ( x ) <-> Q ( x )
(P(x) <-> Q(x))
((P(x) -> Q(x)) & (Q(x) -> P(x)))

== Line 3 ==
forall x. (P(x) & exists y. Q(y))
forall x : positive
exists y : positive
```

`--json --out report.jsonl` 可生成如下记录：
```json
{"line":2,"input":"P(x) <-> Q(x)","tokens":"P ( x ) <-> Q ( x ) ","ast":"(P(x) <-> Q(x))","expanded":"((P(x) -> Q(x)) & (Q(x) -> P(x)))","polarity":""}
```

---

## 结语

本项目完成了计划书中的全部阶段：词法、递归下降/可选 Bison 解析、AST 构造、IFF 展开、量词极性分析、JSON/可视输出以及覆盖边界的自测集合。文档中列出的编译、设计与验证信息可用于使用参考。
