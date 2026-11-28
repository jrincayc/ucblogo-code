# R-Logo Language Specification (v0.1-MVP)

Author: Jeffrey M. Laughlin
Co-author: Google Gemini

## 1. Overview & Design Goals

**R-Logo** ("Restricted Logo") is a statically checkable, lexically scoped, ahead-of-time (AOT) compilable subset of the UCBLogo dialect designed to compile directly to **WebAssembly Garbage Collection (`wasm-gc`)** binary bytecode for headless execution inside a **Web Worker**.

Modeled on RPython (PyPy), R-Logo strips away dynamic runtime reflection, dynamic call-stack variable lookup, unstructured loops, turtle graphics, console TTY I/O, and filesystem access while retaining enough expressiveness—including tagged records and structural recursion—to implement higher-level UCBLogo semantics (such as dynamic scoping, loops, and higher-order list combinators) as a user-space layer on top of R-Logo.

Motivations include cobbling Unicode support into Web Edition, a future R-Logo compiler targeting Web Assembly, hardening the toolchain, eliminating dependencies, being strict for AI agentic coding, and perhaps implementing a future version of Logo in Logo. It's turtles, all the way down.

Warning: Code example have not been tested.

### 1.1 Core Guarantees
1. **Static Parseability:** Every valid R-Logo program parses from a UTF-8 source string into a static Abstract Syntax Tree (AST) without executing code, mutating a live workspace, or reading external files.
2. **Lexical Scope Only:** Variable resolution (`LOCAL`, `MAKE`, `THING` / `:VAR`, and procedure parameters) is strictly lexical within procedure boundaries. Dynamic caller-stack walking is excluded from the kernel and implemented as an explicit environment-passing library layer.
3. **Static Record Typing:** Static structure is provided via nominal **Tagged Records** (`RECORD`), lowering directly to `wasm-gc` `struct` types.
4. **Recursion Over Loops:** Imperative loop constructs (`REPEAT`, `WHILE`, `UNTIL`, `FOR`) are excluded from the kernel; iteration is expressed via procedure recursion (`TO` / `END` + `IF` / `IFELSE`) and lowered with WebAssembly tail calls (`return_call`) where applicable.
5. **Web Worker Native I/O:** All external communication occurs through a narrow, structured message-passing ABI (`WEB.INPUT`, `WEB.EMIT`, `WEB.ERROR`) mapped to Web Worker `postMessage` boundaries.

---

## 2. Lexical Structure

### 2.1 Character Set & Case Sensitivity
* Source files are UTF-8 encoded text.
* Identifiers, procedure names, record names, field names, and built-in keywords are **case-insensitive** and normalized to uppercase during lexing (e.g., `to`, `To`, and `TO` are identical).
* Quoted word literals preserve character content after normalization rules apply; vertical-bar escaping (`|...|`) preserves exact case and whitespace inside a word literal.

### 2.2 Comments & Whitespace
* Whitespace (`space`, `\t`, `\r`, `\n`) separates tokens.
* Line comments begin with `;` (outside `|...|`) and run to the end of the line.
* Line continuation uses `~` immediately followed by a newline.

### 2.3 Literals & Sigils
| Token Kind | Syntax | Example | Semantics |
| :--- | :--- | :--- | :--- |
| **Number** | Optional `-`, digits, optional `.digits` | `42`, `-3.14`, `0` | IEEE-754 64-bit float (`f64`) or unboxed integer index (`i32`) where statically specialized. |
| **Boolean** | `TRUE` or `FALSE` | `TRUE`, `FALSE` | Boolean primitive (`i32` `1` / `0` in `wasm-gc`). |
| **Quoted Word** | `"` followed by non-delimiter characters | `"FIZZ`, `"HELLO` | Immutable word/symbol literal. |
| **Variable Dereference** | `:` followed by identifier | `:N`, `:ACC` | Syntactic sugar for `(THING "N")`; resolves strictly in lexical/procedure scope. |
| **List Literal** | `[` ... `]` | `[]`, `[1 2 [A B]]` | Static list literal whose elements are numbers, booleans, words, or nested lists. |
| **Parenthesized Call** | `(` `PROC` `ARG1` ... `)` | `(SUM 1 2 3)` | Explicit grouping for prefix calls or variadic built-ins. |

### 2.4 Canonicalization Rule
Aliases from standard UCBLogo (such as `OP` for `OUTPUT`, `BF` for `BUTFIRST`, `+` for `SUM`) are normalized to their canonical names during frontend parsing or rejected if outside the R-Logo canonical set.

---

## 3. Grammar (EBNF)

```ebnf
program        = { top_level_decl } ;

top_level_decl = record_decl
               | proc_decl ;

record_decl    = "RECORD" , quoted_word , "[" , { field_spec } , "]" ;
field_spec     = identifier , type_tag ;
type_tag       = "NUM" | "WORD" | "BOOL" | "LIST" | "ANY" | identifier ;

proc_decl      = "TO" , identifier , { param } ,
                 { stmt } ,
                 "END" ;
param          = ":" , identifier ;

stmt           = local_stmt
               | make_stmt
               | record_set_stmt
               | if_stmt
               | ifelse_stmt
               | output_stmt
               | stop_stmt
               | web_emit_stmt
               | web_error_stmt
               | proc_call ;

local_stmt     = "LOCAL" , quoted_word ;
make_stmt      = "MAKE" , quoted_word , expr ;
record_set_stmt= identifier , expr , expr ;     (* SET.<RECORD>.<FIELD> target value *)

if_stmt        = "IF" , expr , "[" , { stmt } , "]" ;
ifelse_stmt    = "IFELSE" , expr , "[" , { stmt } , "]" , "[" , { stmt } , "]" ;

output_stmt    = "OUTPUT" , expr ;
stop_stmt      = "STOP" ;

web_emit_stmt  = "WEB.EMIT" , expr ;
web_error_stmt = "WEB.ERROR" , expr ;

expr           = number_literal
               | bool_literal
               | quoted_word
               | var_ref
               | list_literal
               | "WEB.INPUT"
               | builtin_expr
               | record_ctor_expr
               | record_get_expr
               | proc_call
               | "(" , expr , ")" ;

var_ref        = ":" , identifier
               | "THING" , quoted_word ;

list_literal   = "[" , { list_item } , "]" ;
list_item      = number_literal | bool_literal | identifier | quoted_word | list_literal ;

proc_call      = identifier , { expr }
               | "(" , identifier , { expr } , ")" ;
```

---

## 4. Static Type System & Tagged Records

R-Logo uses **nominal tagged records** with **local static type inference** across procedure bodies.

### 4.1 Core Types & `wasm-gc` Lowering
| R-Logo Type Tag | Description | `wasm-gc` Representation |
| :--- | :--- | :--- |
| `NUM` | Numeric value | `f64` (unboxed in local math/parameters; boxed `$Num` struct when stored in `ANY` / `LIST`) |
| `BOOL` | Boolean truth value (`TRUE` / `FALSE`) | `i32` (`1` / `0`; boxed `$Bool` when stored in `ANY` / `LIST`) |
| `WORD` | Immutable character sequence / symbol | `(ref $Word)` wrapping `(array i16)` UTF-16 code units |
| `LIST` | Persistent singly-linked cons list or empty sentinel | `(ref null $Pair)` where `$Pair = (struct (field $head eqref) (field $tail (ref null $Pair)))` |
| `<RECORD_NAME>` | User-declared nominal tagged record | `(ref $<RECORD_NAME>)` concrete `wasm-gc` `struct` |
| `ANY` | Dynamically typed / polymorphic value | `eqref` (supertype checked via `ref.test` / `ref.cast` at runtime) |

### 4.2 Record Declarations & Generated Procedures
A top-level declaration:

```logo
RECORD "FB.ENTRY [INDEX NUM LABEL WORD]
```

statically defines a nominal record type `FB.ENTRY` and synthesizes the following typed built-in procedures at compile time:

1. **Constructor:** `MAKE.<RECORD>`
   * Signature: `(MAKE.FB.ENTRY index:NUM label:WORD) -> FB.ENTRY`
   * Lowers to: `struct.new $FB.ENTRY`
2. **Field Getters:** `<RECORD>.<FIELD>`
   * Signatures:
     * `(FB.ENTRY.INDEX rec:FB.ENTRY) -> NUM`
     * `(FB.ENTRY.LABEL rec:FB.ENTRY) -> WORD`
   * Lowers to: `struct.get $FB.ENTRY $field_idx`
3. **Field Setters (Mutable Record Fields):** `SET.<RECORD>.<FIELD>`
   * Signatures:
     * `(SET.FB.ENTRY.INDEX rec:FB.ENTRY val:NUM) -> VOID`
     * `(SET.FB.ENTRY.LABEL rec:FB.ENTRY val:WORD) -> VOID`
   * Lowers to: `struct.set $FB.ENTRY $field_idx`
4. **Type Predicate:** `<RECORD>P`
   * Signature: `(FB.ENTRYP val:ANY) -> BOOL`
   * Lowers to: `ref.test (ref $FB.ENTRY)`

### 4.3 Static Type-Checking Rules
1. **Top-Level Record Visibility:** All `RECORD` declarations are collected in Pass 1 before procedure bodies are type-checked, allowing mutually recursive record types (e.g., `ENVFRAME` referencing `BINDING` and `ENVFRAME`).
2. **Fixed Procedure Arity:** Every `TO` procedure has a fixed parameter count. Calling a procedure with too few or too many arguments is a compile-time error.
3. **Uniform Return Kind:** Within a single `TO` procedure, all `OUTPUT` statements must return compatible types, and a procedure cannot mix value-returning `OUTPUT <expr>` with bare `STOP` unless its inferred return type is `ANY` (where `STOP` lowers to a canonical `$Void` singleton).
4. **Record Field Safety:** Passing a non-`FB.ENTRY` value to `FB.ENTRY.INDEX` or assigning a `WORD` to a `NUM` field via `SET.FB.ENTRY.INDEX` fails static type-checking before bytecode emission.
5. **Polymorphic Boundaries (`ANY`):** Values extracted from `LIST` elements (`FIRST`) or `WEB.INPUT` have static type `ANY` (`eqref`) and must either be inspected with predicates (`WORDP`, `LISTP`, `<RECORD>P`) or passed to `ANY`-accepting operations.

---

## 5. Scoping & Variable Semantics

* **No Dynamic Scope Walk:** R-Logo does **not** traverse caller activation frames at runtime.
* **Procedure Scope:** Each `TO` procedure defines a single flat lexical scope containing:
  1. Formal parameters (`:P1`, `:P2`, ...), initialized on entry.
  2. Local variables explicitly declared via `LOCAL "VARNAME`.
* **Binding Rules:**
  * `LOCAL "X` declares a local variable `X` in the current procedure (must appear before `MAKE "X ...` or `:X` reads).
  * `MAKE "X <expr>` mutates the local variable `X` in the current procedure. Writing to an undeclared variable is a compile-time error in R-Logo (preventing accidental implicit global mutation).
  * `:X` and `THING "X` read the local variable `X` in the current procedure. Reading an undeclared variable is a compile-time error.
* **`wasm-gc` Lowering:** Every parameter and `LOCAL` variable maps 1-to-1 to a WebAssembly function local (`local.get` / `local.set`).

---

## 6. R-Logo Kernel Built-in Reference

### 6.1 Procedures, Variables & Records
| Built-in | Arity | Signature / Behavior |
| :--- | :--- | :--- |
| `TO` / `END` | Special | Defines a named procedure with fixed formal parameters. |
| `OUTPUT` | 1 | Exits the current procedure returning `expr`. |
| `STOP` | 0 | Exits the current procedure with no value (`VOID`). |
| `LOCAL` | 1 (quoted word) | Declares a lexical local variable in the current procedure. |
| `MAKE` | 2 (quoted word, expr) | Assigns `expr` to a declared local variable in the current procedure. |
| `THING` / `:var` | 1 (quoted word) | Reads a declared local variable in the current procedure. |
| `RECORD` | 2 (quoted word, field list) | Declares a nominal tagged record type and its constructor, getters, setters, and predicate. |

### 6.2 Data Structure Constructors & Selectors
| Built-in | Arity | Input Types | Return Type | Semantics |
| :--- | :--- | :--- | :--- | :--- |
| `WORD` | 2 (or variadic in `(...)`) | `WORD, WORD` | `WORD` | Concatenates two words into a new `$Word`. |
| `LIST` | 2 (or variadic in `(...)`) | `ANY, ANY` | `LIST` | Allocates a 2-element list `[a b]`. |
| `SENTENCE` | 2 (or variadic in `(...)`) | `ANY, ANY` | `LIST` | Concatenates elements into a flat list, flattening top-level `LIST` arguments by one level. |
| `FPUT` | 2 | `(ANY, LIST)` or `(WORD, WORD)` | `LIST` or `WORD` | Prepends an item to a list, or prepends a 1-character word to a word. |
| `FIRST` | 1 | `LIST` or `WORD` | `ANY` or `WORD` | Returns the first element of a non-empty list, or the first character of a non-empty word as a 1-char word. Traps via `WEB.ERROR` if empty. |
| `BUTFIRST` | 1 | `LIST` or `WORD` | `LIST` or `WORD` | Returns the tail of a non-empty list, or all characters after the first of a non-empty word. Traps via `WEB.ERROR` if empty. |
| `COUNT` | 1 | `LIST` or `WORD` | `NUM` | Returns the number of elements in a list or UTF-16 code units in a word. |

### 6.3 Predicates & Comparisons
| Built-in | Arity | Input Types | Return Type | Semantics |
| :--- | :--- | :--- | :--- | :--- |
| `WORDP` | 1 | `ANY` | `BOOL` | `TRUE` if value is a `$Word` (or boxed `$Num`). |
| `LISTP` | 1 | `ANY` | `BOOL` | `TRUE` if value is a `LIST` (`$Pair` or empty list `[]`). |
| `EMPTYP` | 1 | `LIST` or `WORD` | `BOOL` | `TRUE` if value is `[]` or the empty word `""`. |
| `EQUALP` | 2 | `ANY, ANY` | `BOOL` | Deep structural equality across numbers, booleans, words, lists, and records. |
| `LESSP` | 2 | `NUM, NUM` | `BOOL` | `TRUE` if `arg1 < arg2`. |
| `GREATERP` | 2 | `NUM, NUM` | `BOOL` | `TRUE` if `arg1 > arg2`. |

### 6.4 Arithmetic & Logic
| Built-in | Arity | Input Types | Return Type | Semantics |
| :--- | :--- | :--- | :--- | :--- |
| `SUM` | 2 (or variadic in `(...)`) | `NUM, NUM` | `NUM` | Addition (`f64.add`). |
| `DIFFERENCE` | 2 | `NUM, NUM` | `NUM` | Subtraction (`f64.sub`). |
| `PRODUCT` | 2 (or variadic in `(...)`) | `NUM, NUM` | `NUM` | Multiplication (`f64.mul`). |
| `QUOTIENT` | 2 | `NUM, NUM` | `NUM` | Floating-point division (`f64.div`). Traps on division by zero. |
| `REMAINDER` | 2 | `NUM, NUM` | `NUM` | Truncated integer remainder (`a - trunc(a / b) * b`). |
| `AND` | 2 (or variadic in `(...)`) | `BOOL, BOOL` | `BOOL` | Short-circuiting logical AND. |
| `OR` | 2 (or variadic in `(...)`) | `BOOL, BOOL` | `BOOL` | Short-circuiting logical OR. |
| `NOT` | 1 | `BOOL` | `BOOL` | Logical negation. |

### 6.5 Control Flow & Web Worker Native I/O
| Built-in | Arity | Input Types | Return Type | Semantics |
| :--- | :--- | :--- | :--- | :--- |
| `IF` | 2 | `BOOL, [stmt...]` | `VOID` | Executes the statement block when condition is `TRUE`. |
| `IFELSE` | 3 | `BOOL, [stmt...], [stmt...]` | `VOID` | Executes the first statement block when `TRUE`, else the second block. |
| `WEB.INPUT` | 0 | — | `ANY` | Retrieves the structured input payload provided to the Web Worker invocation. |
| `WEB.EMIT` | 1 | `ANY` | `VOID` | Serializes and emits a structured value to the Web Worker host via `postMessage`. |
| `WEB.ERROR` | 1 | `ANY` | `VOID` (aborts) | Reports a structured error payload to the Web Worker host and terminates execution. |

---

## 7. Control Flow & Tail-Call Lowering in `wasm-gc`

1. **Procedure Exit Blocks (`OUTPUT` / `STOP`):**
   * Every `TO` procedure compiles to a WebAssembly function containing an outer labeled `block $exit (result <ret_type>)`.
   * Any `OUTPUT <expr>` inside nested `IF` or `IFELSE` blocks evaluates `<expr>` and executes `br $exit`.
   * Any `STOP` pushes the `$Void` singleton (or returns void) and executes `br $exit`.
2. **Tail-Recursion Optimization (`return_call`):**
   * When a procedure call appears in tail position—specifically as the direct operand of `OUTPUT (<proc> ...)` or as the final statement of a procedure/branch in a `VOID` procedure—the `wasm-gc` backend emits `return_call $proc` instead of `call $proc` followed by `br $exit`.
   * This guarantees bounded stack usage for accumulator-style recursion in place of imperative loops.

---

## 8. Web Worker Host ABI & Serialization

The compiled `.wasm` module imports its Web-native I/O primitives from the `"env"` module namespace:

```wat
(import "env" "web_input" (func $web_input (result eqref)))
(import "env" "web_emit"  (func $web_emit  (param eqref)))
(import "env" "web_error" (func $web_error (param eqref)))
```

### 8.1 Data Marshaling Rules
| R-Logo Value | JavaScript / `postMessage` Representation |
| :--- | :--- |
| `NUM` | `number` |
| `BOOL` | `boolean` |
| `WORD` | `string` |
| `LIST` | `Array<any>` |
| Tagged `RECORD` | Plain JS object `{ $record: "<RECORD_NAME>", <field1>: ..., <field2>: ... }` |

---

## 9. Explicitly Rejected Features (Compile-Time Errors)

The static validator rejects any program containing constructs outside the R-Logo kernel with structured diagnostics (`ERR_UNSUPPORTED_BUILTIN`, `ERR_UNDECLARED_VAR`, `ERR_TYPE_MISMATCH`, `ERR_ARITY_MISMATCH`):

* **Imperative Loops:** `REPEAT`, `REPCOUNT`, `WHILE`, `UNTIL`, `FOR`
* **Dynamic Scope & Reflection:** `LOCALMAKE`, `NAMEP`, `GLOBAL`, `DEFINE`, `TEXT`, `FULLTEXT`, `RUN`, `RUNRESULT`, `APPLY`, `MAP`, `FILTER`, `FIND`, `REDUCE`, `THROW`, `CATCH`, `TEST`, `IFTRUE`, `IFFALSE`
* **Turtle & Graphics:** `FORWARD`, `BACK`, `LEFT`, `RIGHT`, `PENUP`, `PENDOWN`, `SETPOS`, `SETXY`, `CLEARSCREEN`, etc.
* **Console & Filesystem:** `PRINT`, `TYPE`, `SHOW`, `READLIST`, `READWORD`, `READCHAR`, `OPENREAD`, `OPENWRITE`, `LOAD`, `SAVE`, etc.

---

## 10. Reference Example: FizzBuzz in R-Logo

```logo
RECORD "FB.ENTRY [INDEX NUM LABEL WORD]

TO FIZZBUZZ.WORD :N
  IFELSE EQUALP (REMAINDER :N 15) 0 [
    OUTPUT "FIZZBUZZ
  ] [
    IFELSE EQUALP (REMAINDER :N 3) 0 [
      OUTPUT "FIZZ
    ] [
      IFELSE EQUALP (REMAINDER :N 5) 0 [
        OUTPUT "BUZZ
      ] [
        OUTPUT :N
      ]
    ]
  ]
END

TO FIZZBUZZ.ACC :CURRENT :LIMIT :ACC
  IFELSE GREATERP :CURRENT :LIMIT [
    OUTPUT :ACC
  ] [
    LOCAL "ENTRY
    MAKE "ENTRY (MAKE.FB.ENTRY :CURRENT (FIZZBUZZ.WORD :CURRENT))
    OUTPUT FIZZBUZZ.ACC (SUM :CURRENT 1) :LIMIT (FPUT :ENTRY :ACC)
  ]
END

TO REVERSE.LIST :ITEMS :ACC
  IFELSE EMPTYP :ITEMS [
    OUTPUT :ACC
  ] [
    OUTPUT REVERSE.LIST (BUTFIRST :ITEMS) (FPUT (FIRST :ITEMS) :ACC)
  ]
END

TO MAIN
  LOCAL "LIMIT
  MAKE "LIMIT WEB.INPUT

  IFELSE GREATERP 1 :LIMIT [
    WEB.ERROR [INVALID_LIMIT "LIMIT_MUST_BE_AT_LEAST_1]
    STOP
  ] [
    LOCAL "REVERSED
    MAKE "REVERSED FIZZBUZZ.ACC 1 :LIMIT []
    WEB.EMIT (REVERSE.LIST :REVERSED [])
  ]
END
```
