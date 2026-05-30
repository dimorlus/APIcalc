# Architecture of Variable Scopes and Calculator Contexts

The expression engine utilizes a hierarchical structure of nested calculators to handle variable scopes, data isolation, and context management.

## 1. Context Hierarchy and Nesting Limits

Every time the engine executes a built-in solver (`solve`, `calc`, `inv`, `for`, `integr`, `sum`, `diff`, `extr`), a plotting utility (`plot*`), a user-defined function, or initializes a matrix using an expression, it instantiates a **child calculator**.

* **Nesting Depth:** Child calculators can be nested within other child calculators (for example, a `solve` routine inside another `solve` routine). The engine supports up to **10 levels** of nested contexts. Exceeding this limit triggers a runtime error.

## 2. Context Inheritance and Data Isolation

Depending on how a child calculator handles inherited variables and registers functions, the execution environment operates within three strict scope models:

### A. Local Isolation with Transient Context

*Applies to:* Solvers (`solve`, `calc`, `inv`, `for`, `integr`, `sum`, `diff`, `extr`) and matrix initialization expressions.

* **Inheritance:** The child calculator automatically inherits the parent calculator's context (clones all variables available at the moment of invocation).
* **Behavior:** The child calculator can read and modify inherited variables, as well as instantiate new variables during its execution loop.
* **Teardown:** When the operation terminates, the entire child context is **completely destroyed**. The parent context remains unmodified; no changes or new variables leak upstream.
* **Function Unavailability (`plot*` and `fdlg`):** High-level GUI functions are not registered in the child calculator's function lookup table. Attempting to call them produces an "Unknown function" error (identical to calling an undefined identifier like `abrakadabra(10)`).
* **`plot*`** functions are excluded because they return a bitmap (BMP) handle designed for image operations, storage, or main GUI rendering, making them meaningless inside a mathematical solver.
* **`fdlg`** is excluded because it triggers a modal GUI file dialog, which cannot resolve input context when executed from within a nested background evaluation.



### B. Full Isolation

*Applies to:* User-Defined Functions declared via the `{fn(x, y) expr}` syntax.

* **Behavior:** The child calculator instantiated for a user-defined function **does not inherit variables** from the parent context. It is completely isolated from the parent workspace, recognizing only the explicit arguments passed to it during the call.
* **Example:**
```pascal
{summa(x, y) x+y}
x := 10; y := 20;
res := summa(1, 2)  ;; 'res' evaluates to 3, completely ignoring parent x and y

```


* **Teardown:** Upon termination, the local context is wiped from memory, returning only the final computed scalar or matrix to the parent.

### C. Persistent Context (Environment Modification)

*Applies to:* The `eval("expression")` function and the Interactive Debug Console (activated via `F7` during script traceback).

* **Behavior:** Instead of discarding the execution state upon completion, these features **commit and transfer** their execution context back into the parent calculator.
* **The `eval()` Function:** Evaluates a dynamic string directly within the active local scope. Any variable reassignments or new variables created inside `eval()` persist in the main context after the function returns. User-defined functions `{fn()}` declared or modified via `eval()` also remain updated in the main context.
* **Interactive Debugger (`F7`):** Allows viewing and changing variables on the fly. Modifications persist within the active **script context** for the remainder of the script's execution. However, these changes do not leak back into the top-level GUI calculator instance that originally spawned the script.

---

## 3. Scope and Feature Reference Matrix

| Target Operation | Instantiates Child Calculator? | Inherits Parent Variables? | Commits Changes Back to Parent? | High-Level GUI Functions Available? (`plot*`, `fdlg`) |
| --- | --- | --- | --- | --- |
| **Solvers** (`solve`, `calc`...) | **Yes** | Yes | No (Discarded) | **No** (Triggers "Unknown function" error) |
| **Matrix Initialization** | **Yes** | Yes | No (Discarded) | **No** (Triggers "Unknown function" error) |
| **Plotting Utilities** (`plot*`) | **Yes** | **No** | No (Discarded) | **No** (Triggers "Unknown function" error) |
| **User-Defined Functions** | **Yes** | **No** | No (Discarded) | **No** (Triggers "Unknown function" error) |
| **Dynamic Evaluation** (`eval`) | **No** | Yes | **Yes (To Main Context)** | **No** (Triggers "Unknown function" error) |
| **Debug Console (`F7`)** | **No** | Yes | **Yes (To Script Context)** | **No** (Triggers "Unknown function" error) |

---

## 4. Operational Notes

* **The `fdlg("*.txt")` Routine:** This is a dedicated GUI wrapper function that opens a standard Windows file selection dialog and returns the chosen filename as a string. It carries no variable context and does not utilize a child calculator.
* **The Purpose of `eval()`:** This function is primarily an architecture utility intended for internal engine debugging and dynamic code injection. It is not designed for standard user-facing mathematical modeling.
* **Automatic Debug Interface Cleanup:** In the GUI application, the debug console interface is bound to the lifecycle of the script. It automatically terminates and closes as soon as the script completes execution or encounters a `RET` instruction.