# tbox

`tbox` is an interactive terminal program that turns your keystrokes into live, colorized ASCII boxes. It manages the terminal state for you, draws each box as you type, and lets you quickly stack multiple boxes in a single session.

## Features

- **Interactive box rendering** – characters are wrapped in a bordered box that updates as you type.
- **Alternate screen buffer management** – uses the terminal's alternate buffer so the regular scrollback is preserved after you exit.
- **Raw/CBREAK input modes** – runs in raw mode by default for single-keystroke input; pass any argument to switch to CBREAK mode (signals such as <kbd>Ctrl+C</kbd> remain enabled).
- **Graceful signal handling** – restores the user's terminal settings when the process receives `SIGINT`, `SIGQUIT`, or `SIGTSTP`.
- **Simple logging** – emits diagnostic messages to a `log` file with timestamps.

## Building

The project is written in C11 and builds with `make` on any POSIX-like system that provides `gcc` (or a compatible compiler).

```sh
make            # builds the "main" executable
make clean      # removes objects and the executable
```

To rebuild from scratch, run `make clean` followed by `make`.

## Running

After building, launch the interactive session:

```sh
./main            # raw mode (no canonical processing or signal characters)
./main cbreak     # CBREAK mode (signal characters such as Ctrl+C are honored)
```

While the program is running:

- Type any printable characters to extend the current box.
- Press <kbd>Enter</kbd> to finalize the box and start a new one on the next row.
- Press <kbd>q</kbd> to quit.
- Press <kbd>Esc</kbd> to log the escape event without altering the display.

When you exit (either with `q` or EOF), the program restores your terminal to its previous state.

## Repository Layout

| Path | Description |
| ---- | ----------- |
| `main.c` | Application entry point, terminal initialization, and input loop. |
| `box.c` / `box.h` | Renders and updates the on-screen box, handles cursor placement. |
| `estring.c` / `estring.h` | Minimal dynamic string used for the live text buffer. |
| `modes.c` / `modes.h` | Helpers for switching the terminal between raw and CBREAK modes. |
| `tutils.h` | Terminal escape-sequence macros (alternate buffer, clear screen, cursor movement). |
| `log.c` / `log.h` | Timestamped logging to the `log` file. |
| `errfunc.c` / `errfunc.h` | Error-reporting helper that prints a colored message and exits. |
| `colors.h` | Macros for ANSI color escapes used while drawing boxes. |

## Logging

Runtime events (such as receiving the escape key or allocating a new row for a box) are appended to a file named `log` in the repository root. The timestamps follow the current locale's `%T` (HH:MM:SS) format. Remove the file if you want a fresh log before your next run.

## Contributing

1. Open an issue or discussion describing the change you would like to make.
2. Fork the repository and create a feature branch.
3. Run `make` to ensure the project still builds cleanly.
4. Submit a pull request with a clear description of your changes.

## License

`tbox` is distributed under the terms of the MIT License. See [LICENSE](LICENSE) for details.
