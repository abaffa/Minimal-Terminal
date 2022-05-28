# Baffa Terminal (minimal-terminal clone) [![Hackaday.io](https://img.shields.io/badge/Hackaday.io--blue.svg)](https://hackaday.io/project/184235-60x25-minimal-terminal)

This is a Simple Terminal board based on "Minimal-Terminal" project by Carsten Herting (oct/2021 https://github.com/slu4coder/Minimal-Terminal).

Current version supports US-International keyboard mapping, four basic serial speeds (115200, 57600, 38400, 9600 BPS) and [supports some basic CSI/Ansi escapes](supported_escapes.md).
 
The prototype board has been tested with my CP/M machine (see http://baffa-2.baffasoft.com.br) but there is another board compatible with RC2014 pinout. 

### Video
[![minimal_terminal_video](http://img.youtube.com/vi/mmQTUNvmSpk/0.jpg)](http://www.youtube.com/watch?v=mmQTUNvmSpk "Baffa-2 Homebrew Microcomputer + Terminal Card")

## Baffa-2 Terminal Board:

[Order Baffa-2 Minimal Terminal PCB from PCBWay](https://www.pcbway.com/project/shareproject/Baffa_2_Minimal_Terminal_f85704c4.html)

![minimal_terminalv2](_images/baffa2_minimal_terminal_rev1.png)

## RC2014 Minimal-Terminal Module

[Order RC2014 Minimal Terminal PCB from PCBWay](https://www.pcbway.com/project/shareproject/RC2014_Minimal_Terminal_41593f8c.html)

![minimal_terminalv2_rc2014](_images/minimal_terminalv2_rc2014.png)

## Bill of Materials

| Qty | Value | Parts | Description |
|:---:|:----------:|:-----|:-----|
| 1 |  PINHD-1X20 | JP4 |  PIN HEADER |
| 1 |  PINHD-1X40 | JP3 |  PIN HEADER |
| 6 |  100nf | C2, C6, C9, C10, C11, C12 | CAPACITOR |
| 2 |  10k | R10, R16 | RESISTOR |
| 1 | 150 | R13 | RESISTOR |
| 2 |  1k | R11, R12 | RESISTOR |
| 1 |  24Mhz XTAL |  XTAL |  XTAL SOCKET-14 |
| 1 |  330nf | C1 | CAPACITOR |
| 1 |  47uf | C7 | POLARIZED CAPACITOR |
| 1 |  74HC166N |  IC2 |  8-bit parallel load SHIFT REGISTER |
| 2 | 75 | R14, R15 | RESISTOR, American symbol |
| 1 |  7806T | IC1 |  Positive VOLTAGE REGULATOR |
| 2 |  ATMEGA328P-PU | UART_PS2, VGA |  PDIP-28 Atmel |
| 1 |  BUS UART | JP2 |  JUMPER |
| 1 |  MINI-DIN6PTH |  PS2_DIN |  mini DIN 6 Connector |
| 1 |  Power 9-12v | JP1 |  PIN HEADER |
| 1 |  RESET | S5 | MOM SWITCH |
| 1 |  SPEED | JP7 |  JUMPER |
| 1 | PINHD-1X6 | UART |  PIN HEADER |
| 1 | PINHD-1X6 |  VGA |  PIN HEADER |
| 1 | DSUB | VGA Output | SUB-D |
 
## License

Copyright (c) 2021 Augusto Baffa, Carsten Herting

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

