# Yash - Yet Another Shell

Yash is a lightweight, Unix-like shell implemented in C. 

> **WARNING:** This project was created primarily for **learning purposes**. It is not intended for production use and should be used with caution. 

I am very happy to receive **contributions**! If you find a bug or have an idea for a feature, feel free to open an issue or submit a pull request.

## Current Features

Yash already supports several core shell functionalities:

- **Command Execution:** Run any standard Unix command (e.g., `ls`, `grep`, `mkdir`).
- **Pipes (`|`):** Support for multi-stage command piping.
- **Redirection:** Input (`<`), Output (`>`), and Append (`>>`).
- **Logical Operators:** Conditional execution using `&&` and `||`.
- **Built-in Commands:** `cd` and `exit`.
- **Environment Variables:** Basic expansion of variables (e.g., `$HOME`, `$PATH`).
- **Interactive Interface:** Command history and line editing via `readline`.
- **Signal Handling:** Gracefully handles `SIGINT` (Ctrl+C).

### Advanced Features

- **Lua Configuration:** Fully scriptable configuration using an embedded Lua engine. On first run, Yash generates a split configuration layout at `~/.config/JustAnotherShell/`:
  - `config.lua`: For setting the prompt, colors, and defining aliases.
  - `scripts.lua`: For running startup commands or tools (like a Python dashboard).
- **Dynamic Prompts:** Use the `%w` format specifier in your Lua config (e.g., `yash.set_prompt('%w Yash> ')`) to automatically display your current working directory.
- **Command Aliases:** Easily alias commands via Lua (e.g., `yash.add_alias('ls', 'ls --color=auto')`), fully supporting pipelines.

## Roadmap

The project is still under active development. Planned future improvements include:

- **POSIX Compliance:** Moving towards stricter adherence to POSIX standards for better compatibility.
- **Job Control:** Implementation of background and foreground process management.

## Getting Started

### Prerequisites

You will need a C compiler (like `gcc`), the `readline` library, and the `lua` development libraries.

### Building

You can build the project using the provided `Makefile`:

```bash
make
```

### Running

Once built, you can start the shell by running:

```bash
./yash
```

### Using as Default Shell (Kitty)

Since changing system shells with `chsh` might require elevated privileges, the easiest way to use Yash daily is to configure your terminal emulator to launch it. For Kitty, add this to `~/.config/kitty/kitty.conf`:

```
shell /absolute/path/to/JustAnotherShell/yash
```

## License

This project is licensed under the **GNU General Public License v3.0**. See the [LICENSE](LICENSE) file for details.
