# Minimal-Terminal / Supported Escape Sequences

Standard escape codes are prefixed with `Escape`:

- Ctrl-Key: `^[`
- Octal: `\033`
- Unicode: `\u001b`
- Hexadecimal: `\x1B`
- Decimal: `27`

- `CSI` - Control Sequence Introducer: sequence starting with `ESC [` or CSI (`\x9B`)

Followed by the command, somtimes delimited by opening square bracket (`[`), known as a Control Sequence Introducer (CSI), optionally followed by arguments and the command itself.

Arguments are delimeted by semi colon (`;`).

For example:

```sh
\x1b[1;31m  # Set style to bold, red foreground.
```
* Any whitespaces between sequences and arguments should be ignored. They are present for improved readability.

## Supported Cursor Controls

| ESC Code Sequence                                  | Description                                              |
| :------------------------------------------------- | :------------------------------------------------------- |
| `ESC[H`                                            | moves cursor to home position (0, 0)                     |
| `ESC[{line};{column}H` <br> `ESC[{line};{column}f` | moves cursor to line #, column #                         |
| `ESC[#A`                                           | moves cursor up # lines                                  |
| `ESC[#B`                                           | moves cursor down # lines                                |
| `ESC[#C`                                           | moves cursor right # columns                             |
| `ESC[#d`                                           | moves cursor to an absolute # line                       |
| `ESC[#D`                                           | moves cursor left # columns                              |
| `ESC[#E`                                           | moves cursor to beginning of next line, # lines down     |
| `ESC[#F`                                           | moves cursor to beginning of previous line, # lines up   |
| `ESC[#G`                                           | moves cursor to column #                                 |
| `ESC[#S`                                           | scrolls down # lines                                     |
| `ESC[#T`                                           | scrolls up # lines                                       |

## Supported Erase Functions

| ESC Code Sequence | Description                               |
| :---------------- | :---------------------------------------- |
| `ESC[J`           | erase in display (same as ESC\[0J)        |
| `ESC[0J`          | erase from cursor until end of screen     |
| `ESC[1J`          | erase from cursor to beginning of screen  |
| `ESC[2J`          | erase entire screen                       |
| `ESC[K`           | erase in line (same as ESC\[0K)           |
| `ESC[0K`          | erase from cursor to end of line          |
| `ESC[1K`          | erase start of line to the cursor         |
| `ESC[2K`          | erase the entire line                     |

## Support Colors / Graphics Mode

| ESC Code Sequence | Reset Sequence | Description                                                |
| :---------------- | :------------- | :--------------------------------------------------------- |
| `ESC[1;34;{...}m` |                | Set graphics modes for cell, separated by semicolon (`;`). |
| `ESC[0m`          |                | reset all modes (styles and colors)                        |
| `ESC[7m`          | `ESC[27m`      | set inverse/reverse mode                                   |

> **Note:** Colors are not supported and are ignored by terminal.

## References

- [Main Reference](https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797)
- [Wikipedia: ANSI escape code](https://en.wikipedia.org/wiki/ANSI_escape_code)
- [Build your own Command Line with ANSI escape codes](http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html)
- [ascii-table: ANSI Escape sequences](http://ascii-table.com/ansi-escape-sequences.php)
- [bluesock: ansi codes](https://bluesock.org/~willkg/dev/ansi.html)
- [bash-hackers: Terminal Codes (ANSI/VT100) introduction](http://wiki.bash-hackers.org/scripting/terminalcodes)
- [XTerm Control Sequences](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html)
- [VT100 – Various terminal manuals](https://vt100.net/)
- [xterm.js – Supported Terminal Sequences](https://xtermjs.org/docs/api/vtfeatures/)
