/**************************************************************************************
  *****                                                                           *****
  ***** Atmega328P UART & PS/2 Terminal Interface written by Carsten Herting 2021 *****
  *****                   Version 2.2, last update: 02.06.2021                    *****
  *****               US-International version by Augusto Baffa Dec/21            *****
  *****                                                                           *****
  *************************************************************************************
  This Atmega328P reads serial data from it's UART RX and transfers it to the 2nd
  ATmega328P handling VGA. PS/2 data is converted into ASCII and sent away via UART TX.
  C0-C3: UART SPEED SCALER
  C4: PS/2 clock input, triggers a pin change interrupt
  C5: PS/2 dtat input, sampled at the rising edge of the PS/2 clock
  D0,1: UART RX, TX
  D2-7: bits 0-5 of transfer output to C0-5
  B0,1: bits 6-7 of transfer output to D6-7 via 1k resistors
  B2: DATA_NEW output to B0 input of 2nd µC
  See license at the end of this file.

 Fuse: FF | D9 | FF | D5

 ** Added Caps lock support jan, 6 2022 Augusto Baffa
 ** Added Num lock support jan, 8 2022 Augusto Baffa
*/
//#define DEBUG
//#define DEBUG_SCANCODE

volatile byte ps2_scan = 0;                     // holds valid PS/2 scancode (set this to 0 after processing!)

volatile bool ps2Keyboard_caps_lock = false; // remembers shift lock has been pressed
volatile bool ps2Keyboard_num_lock = false; // remembers shift lock has been pressed
volatile boolean cmd_in_progress = false;
         byte    cmd_value = 0;
volatile byte    cmd_ack_value = 1;
         byte    cmd_parity = 0;
volatile boolean cmd_ack_byte_ok = false;
volatile int     cmd_count = 0;

volatile byte    cmd_last = 0;
volatile bool    cmd_resend = false;
volatile int     cmd_resend_count = 0;

int vga_count = 0;
bool vga_show_title = true;
const char vga_title[81] PROGMEM = "\e[24;0HBaffa-Terminal v1.2\e[25;0HBased on Minimal-Terminal by Herting 2021\e[0;0H";
const char vga_clear[5] PROGMEM = "\e[2J";

const byte LookupScanToASCII[4][128] PROGMEM =  // lookup table (in: SHIFT/ALTGR/CTRL keystate and PS/2 scancode, out: ASCII code)
{ 
  { 
    // plain key
    0,120,0,116,114,112,113,123,0,121,119,117,115,9,96,0,0,0,0,0,0,113,49,0,0,0,122,115,97,119,50,0,0,99,120,100,101,52,51,0,0,32,118,102,116,114,53,0,0,110,98,104,103,121,54,0,0,0,109,106,117,55,56,0,0,44,107,105,111,48,57,0,0,46,47,108,59,112,45,0,0,0,39,0,91,61,0,0,0,0,10,93,0,92,0,0,0,92,0,0,0,0,8,0,0,0,0,0,0,46,0,0,48,127,68,53,82,85,27,78,122,43,51,45,42,0,0,0
    },
/* //OLD    
    0,0,0,0,0,0,0,0,          0,0,0,0,0,0,94,0,           0,0,0,0,0,113,49,0,         0,0,122,115,97,119,50,0,     // plain key
    0,99,120,100,101,52,51,0, 0,32,118,102,116,114,53,0,  0,110,98,104,103,121,54,0,  0,0,109,106,117,55,56,0,
    0,44,107,105,111,48,57,0, 0,46,45,108,0,112,0,0,      0,0,0,0,0,96,0,0,           0,0,10,43,0,35,0,0,
    0,60,0,0,0,0,8,0,         0,0,0,0,0,0,0,0,            0,127,0,0,0,0,27,0,         0,0,0,0,0,0,0,0          },
*/
  { 
    // with SHIFT
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,0,0,0,0,0,0,81,33,0,0,0,90,83,65,87,64,0,0,67,88,68,69,36,35,0,0,0,86,70,84,82,37,0,0,78,66,72,71,89,94,0,0,0,77,74,85,38,42,0,0,60,75,73,79,41,40,0,0,62,63,76,58,80,95,0,0,0,34,0,123,43,0,0,0,0,0,125,0,124,0,0,0,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,0,0,0,0,0,43,35,45,42,0,0,0
    },
/* //OLD        
    0,0,0,0,0,0,0,0,          0,0,0,0,0,0,248,0,          0,0,0,0,0,81,33,0,          0,0,90,83,65,87,34,0,        // with SHIFT
    0,67,88,68,69,36,0,0,     0,0,86,70,84,82,37,0,       0,78,66,72,71,89,38,0,      0,0,77,74,85,47,40,0,
    0,59,75,73,79,61,41,0,    0,58,95,76,0,80,63,0,       0,0,0,0,0,0,0,0,            0,0,0,42,0,39,0,0,
    0,62,0,0,0,0,0,0,         0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0          },
*/
  { 0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,            0,0,0,0,0,64,0,0,           0,0,0,0,0,0,0,0,             // with ALT(GR)
    0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0,            0,0,0,0,0,123,91,0,
    0,0,0,0,0,125,93,0,       0,0,0,0,0,0,92,0,           0,0,0,0,0,0,0,0,            0,0,0,126,0,0,0,0,
    0,124,0,0,0,0,0,0,        0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0          },
  {
    // with CTRL(Strg)
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,0,0,0,0,26,19,1,23,0,0,0,3,24,4,5,0,0,0,0,32,22,6,20,18,0,0,0,14,2,8,7,25,0,0,0,0,13,10,21,0,0,0,0,0,11,9,15,0,0,0,0,0,0,12,0,16,0,0,0,0,0,0,27,0,0,0,0,0,0,29,0,28,0,0,0,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  }
};
/*    
    0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,            0,0,0,0,0,0x11,0,0,         0,0,0,0x13,0x01,0,0,0,             // with CTRL(Strg)
    0,0x03,0x18,0,0,0,0,0,    0,0,0x16,0,0,0,0,0,         0,0x0e,0,0,0,0x1a,0,0,      0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,          0,0,0,0x0c,0,0,0,0,         0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,          0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0,            0,0,0,0,0,0,0,0          }
};
*/
  
void setup()
{
  DDRC = 0b00000000;       // C4-5: input CLK und DAT of PS/2 keyboard
  PORTC = 0b00001111;      // 20k pull-ups ON
  DDRB = 0b00000111;       // B0-1: transfer bits 6-7, B2: DATA_NEW output, B5: DATA_ACK input
  PORTB = 0;
  DDRD = 0b11111100;       // D0-1: UART RX/TX, D2-7: transfers bits 0-5
  PORTD = 0;
  noInterrupts();
    PCICR  |= 0b00000010;  // enable Port C 0b10 PCINT1
    PCMSK1 |= 0b00010000;  // input pin C4 (PS/2 CLK) löst PCI aus
  interrupts();   

  Serial.begin(230400, SERIAL_8N2);    // @24MHz 230400 can exactly be reached!
  

  // 24MHZ    
  // ((230400/Bauds)*12)+((230400/Bauds)-1)    
      
  //1   230400  12 
  //2   115200  25
  //4   57600   51
  //6   38400   77
  //8   28800   103
  //12  19200   155
  //16  14400   207
  //24  9600    311
  //32  7200    415
  //64  3600    831
  //128 1800    1663
  //256 900     3327

  //int rate[4] = { 103, 51, 25, 12 };  
  //
  // O  O|   230400 bps
  // O  O|
  // 
  // O==O|   115200 bps
  // O  O|
  //   
  // O  O|    57600 bps
  // O==O|
  // 
  // O==O|    28800 bps
  // O==O|
  // 
  
  int rate[4] = { 311, 77, 51, 25};   // Current Settings
  //
  // O  O|   115200 bps
  // O  O|
  // 
  // O==O|    57600 bps
  // O  O|
  //   
  // O  O|    38400 bps
  // O==O|
  // 
  // O==O|     9600 bps
  // O==O|
  // 

  // 16Mhz
  // 300  3332  0.0
  // 600 1666  0.0
  // 1200  832 0.0
  // 2400  416 0.1
  // 4800  207 0.2
  // 9600  103 0.2
  // 14400 68  0.6
  // 19200 51  0.2
  // 28800 34  0.8
  // 38400 25  0.2
  // 57600 16  2.1
  // 76800 12  0.2
  // 115200  8 3.7
  //int rate[4] = { 107, 53, 35, 17};  
  
  bitWrite(UCSR0A, 1, HIGH);// switch off UART rate multiplier
  UBRR0 = rate[PINC & 0b00000011];

  delay(1000);

  for(vga_count = 0; vga_count < 80; vga_count++){
    delayMicroseconds(500);
    send_vga(pgm_read_byte(&vga_title[vga_count]));
  }
}

void send_vga(byte a){
    PORTD = (a << 2);                          // D2-7 hold bits 0-5
    PORTB = ((PORTB & 0b00000100) ^ 0b00000100) | (a >> 6); // B0-1 hold bits 6-7, toggle B2
}

void loop()
{
  if (Serial.available())
  {
      if(vga_show_title){
        for(vga_count = 0; vga_count < 4; vga_count++){
          delayMicroseconds(500);
          send_vga(pgm_read_byte(&vga_clear[vga_count]));
        }
        delayMicroseconds(500);
        vga_show_title = false;
      }
  
    byte a = Serial.read();
    send_vga(a);
  }

  if(cmd_resend){
    cmd_resend = false;
    Serial.print(" Resend last cmd. ");
    kbd_send_command(cmd_last);     // now send the data
  }


  if (ps2_scan != 0)                           // check for PS2 input
  {    
    byte scan = ps2_scan; ps2_scan = 0;        // take the PS2 scan result and clear it
    static bool ALT = false;                   // state of some important keys of the PS2 keyboard
    static bool SHIFT = false;
    static bool CTRL = false;
    static bool released = false;              // indicating that the next key counts as 'released'
    
    #ifdef DEBUG_SCANCODE
    int code = scan;
    Serial.print(code);
    Serial.print(" >> ");
    #endif
    
    switch (scan)
    {
      case 17:  ALT = !released; released = false; break;             // ALT, ALTGR
      case 18: case 89: SHIFT = !released; released = false; break;  // SHIFT LEFT, SHIFT RIGHT     
      case 20:  CTRL = !released; released = false; break;            // CTRL LEFT, CTRL RIGHT
      case 240: released = true; break;                              // key release indicator
            
      case 0x58: {
        if(released){
          ps2Keyboard_caps_lock = ps2Keyboard_caps_lock? false : true;
          
          kbd_set_lights(ps2Keyboard_caps_lock<<2 | ps2Keyboard_num_lock <<1);

          released = false; 
        }
                
        break;
      }

      case 0x77: {
        if(released){
          ps2Keyboard_num_lock = ps2Keyboard_num_lock? false : true;
          
          kbd_set_lights(ps2Keyboard_caps_lock<<2 | ps2Keyboard_num_lock <<1);

          released = false; 
        }
                
        break;
      }


      default:                                                       // PROCESS ANY OTHER KEYS
      {
        if (released == true) released = false;                      // ignore released keys
        else                                                         // key was pressed
        {
          switch (scan)
          {
            case 117: Serial.print(ps2Keyboard_num_lock ? "8" : "\e[A"); break;   // cursor up
            case 114: Serial.print(ps2Keyboard_num_lock ? "2" : "\e[B"); break;   // cursor down
            case 116: Serial.print(ps2Keyboard_num_lock ? "6" : "\e[C"); break;   // cursor right              
            case 107: Serial.print(ps2Keyboard_num_lock ? "4" : "\e[D"); break;   // cursor left
            case 108: Serial.print(ps2Keyboard_num_lock ? "7" : "\e[1~"); break;  // pos1 / home
            case 105: Serial.print(ps2Keyboard_num_lock ? "1" : "\e[4~"); break;  // end
            case 125: Serial.print(ps2Keyboard_num_lock ? "9" : "\e[5~"); break;  // page up
            case 122: Serial.print(ps2Keyboard_num_lock ? "3" : "\e[6~"); break;  // page dn        
            case 13:  Serial.print("  "); break;       // TAB = 2 SPACES 
            case 90:  Serial.print("\r"); break;       // Enter CRLF
            
            default:
            {
              // select bank of lookup table according to the states of the special keys
              byte s=0; if (SHIFT) s = 1; else if (ALT) s = 2; else if (CTRL) s = 3;
              char p = pgm_read_byte(&LookupScanToASCII[s][scan & 127]);

              if(ps2Keyboard_caps_lock){
                if(p >= 'a' && p <= 'z') p += ('A'-'a'); 
                else if(p >= 'A' && p <= 'Z')  p -= ('A'-'a'); 
              }
              if(ps2Keyboard_num_lock ){
                switch (scan)
                {
                  case 113: p = '.'; break;
                }
              }
              
              if (p != 0) Serial.print(p);
            }            
          }
        }
        break;
      }
    }  
  }  
}


ISR(PCINT1_vect)
{  
  static uint16_t dat = 0;                        // shift the received bits from PS/2 keyboard in
  static uint8_t clk = 0;
  static uint32_t last = 0;

  // This is the code to send a byte to the keyboard. Actually its 12 bits:
  // a start bit, 8 data bits, 1 parity, 1 stop bit, 1 ack bit (from the kbd)
  if (cmd_in_progress && cmd_count < 12) {

    if (bitRead(PINC, 4) == HIGH)                   // sampling keyboard data at the rising edge of the PS/2 clock                    
    {
      switch (cmd_count) {
      case 0: // start bit
        PORTC &= ~(1<<DDC5);                //digitalWrite(ps2Keyboard_DataPin,LOW);

        #ifdef DEBUG
        Serial.print("|");
        Serial.print(0);
        Serial.print("_");
        #endif
        
        break;
      case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        // data bits to shift
        if((cmd_value & 1) == 0) PORTC &= ~(1<<DDC5); else PORTC |= (1<<DDC5);    // digitalWrite(ps2Keyboard_DataPin,cmd_value&1);
        
        #ifdef DEBUG
        if((cmd_value & 1) == 0) Serial.print(0); else Serial.print(1);
        #endif
        
        cmd_value = cmd_value>>1;
        break;
      case 9:  // parity bit
        if((cmd_parity & 1) == 0) PORTC &= ~(1<<DDC5); else PORTC |= (1<<DDC5);   // digitalWrite(ps2Keyboard_DataPin,cmd_parity);
        
        #ifdef DEBUG
        Serial.print("_");
        if((cmd_parity & 1) == 0) Serial.print(0); else Serial.print(1);
        #endif
        
        break;
      case 10:  // stop bit
        // release the data pin, so stop bit actually relies on pull-up
        // but this ensures the data pin is ready to be driven by the kbd for
        // for the next bit.
        PORTC |= (1<<DDC5);               //digitalWrite(ps2Keyboard_DataPin, HIGH);
        DDRC &=  ~(1<<DDC5);              //pinMode(ps2Keyboard_DataPin, INPUT);

        #ifdef DEBUG
        Serial.print("_");
        Serial.print(1);
        Serial.print("|");
        #endif
        
        break;
      case 11: // ack bit - driven by the kbd, so we read its value
        cmd_ack_value = bitRead(PINC, 5); //digitalRead(ps2Keyboard_DataPin);

        #ifdef DEBUG
        Serial.print(" minor ack: ");
        if((cmd_ack_value & 1) == 0) Serial.print(0); else Serial.print(1);
        Serial.print("! ");
        #endif
        
        cmd_in_progress = false;  // done shifting out
      }
  
      cmd_count++;
    }
    return; // don't fall through to the receive section of the ISR
  }


  if (bitRead(PINC, 4) == HIGH)                   // sampling keyboard data at the rising edge of the PS/2 clock                    
  {

    if (clk < 11)                                 // wait for an entire PS2 datum (start, stop, parits, 8 data bits)
    {
      uint32_t t = micros();
      if (t - last > 200) clk = 0;                // count from zero after a longer pause
      last = t;
      dat = dat>>1;                               // shift existing bits to the right
      if (bitRead(PINC, 5)) dat |= 0b10000000000;
      clk++;                                      // count the received bits
      if (clk == 11) { 
        ps2_scan = byte(dat>>1);
        clk = 0; 

        
        switch (ps2_scan) {
          case 0xFA: { // command acknowlegde byte
            cmd_ack_byte_ok = true;
            
            cmd_resend_count = 0;

            #ifdef DEBUG
            Serial.print(" >> ACK! ");
            #endif
            
            ps2_scan = 0;
            break;
          }

          #ifdef DEBUG
          case 0x00: { 
            Serial.print(" >> Key detection error or internal buffer overrun.");

            ps2_scan = 0;
            break;
          }
          case 0xAA: { 
            Serial.print(" >> Self test passed (sent after \"0xFF (reset)\" command or keyboard power up).");

            ps2_scan = 0;
            break;
          }
          case 0xEE: { 
            Serial.print(" >> Response to \"0xEE (echo)\" command.");

            ps2_scan = 0;
            break;
          }
          
          
          case 0xFC: case 0xFD: { 
            
            Serial.print(" >> Self test failed (sent after \"0xFF (reset)\" command or keyboard power up).");

            ps2_scan = 0;
            break;
          }
          #endif
          
          case 0xFE: { 
            #ifdef DEBUG
            Serial.print(" >> Resend (keyboard wants controller to repeat last command it sent).");
            #endif
            
            if(cmd_resend_count < 3){
              cmd_resend_count++;
              cmd_resend = true;
            }
            else
              cmd_resend_count = 0;

            ps2_scan = 0;
            break;
          }
          
          #ifdef DEBUG
          case 0xFF: { 
            
            Serial.print(" >>   Key detection error or internal buffer overrun.");
            
            ps2_scan = 0;
            break;
          }
          #endif
        }
      }   // full datum has been received => strip start, stop parity and store it
    }
  }
}

// sending command bytes to the keybaord needs proper parity (otherwise the keyboard
// just asks you to repeat the byte)
byte odd_parity(byte val) {
  int i, count = 1;  // start with 0 for even parity
  for (i=0; i<8; i++) {
    if (val&1) count++;
    val = val>>1;
  }
  return count & 1; // bottom bit of count is parity bit
}


void kbd_send_command(byte val) {

   cmd_last = val;

  // stop interrupt routine from receiving characters so that we can use it
  // to send a byte

    
  // set up the byte to shift out and initialise the ack bit
  cmd_value      = val;
  cmd_ack_value  = 1;    // the kbd will clear this bit on receiving the byte
  cmd_parity     = odd_parity(val);

    
  // set the data pin as an output, ready for driving
  
  PORTC |= (1<<PC5);    //digitalWrite(ps2Keyboard_DataPin, HIGH);
  DDRC |=  (1<<DDC5);   //pinMode(ps2Keyboard_DataPin, OUTPUT);
  

  // drive clock pin low - this is going to generate the first
  // interrupt of the shifting out process
  DDRC |=  (1<<DDC4);   //pinMode(PS2_INT_PIN, OUTPUT);
  PORTC &= ~(1<<PC4);   //digitalWrite(PS2_INT_PIN, LOW);

  // wait at least one clock cycle (in case the kbd is mid transmission)
  delayMicroseconds(60);

  cmd_in_progress = true;
  cmd_count       = 0;

  // set up the 0 start bit
  PORTC &= ~(1<<PC5);   //digitalWrite(ps2Keyboard_DataPin, LOW);

  // let go of clock - the kbd takes over driving the clock from here
  PORTC |= (1<<DDC4);   //digitalWrite(PS2_INT_PIN, HIGH);
  DDRC &=  ~(1<<DDC4); //pinMode(PS2_INT_PIN, INPUT);

  // wait for interrupt routine to shift out byte, parity and receive ack bit
  //while (cmd_ack_value!=0) delayMicroseconds(60);
  while (cmd_in_progress) delayMicroseconds(60);


  // switch back to the interrupt routine receiving characters from the kbd
  cmd_in_progress = false;

}

/*
void kbd_reset() {

  kbd_send_command(0xFF);   // send the kbd reset code to the kbd: 3 lights
                            // should flash briefly on the kbd

  // reset all the global variables
  ps2Keyboard_shift         = false;
  ps2Keyboard_ctrl          = false;
  ps2Keyboard_alt           = false;
  ps2Keyboard_extend        = false;
  ps2Keyboard_release       = false;
  ps2Keyboard_caps_lock     = false;
  cmd_in_progress           = false;
  cmd_count                 = 0;
  cmd_value                 = 0;
  cmd_ack_value             = 1;
  cmd_last =                = 0;
  cmd_resend_count          = 0;
  cmd_resend                = false;
}
*/


void kbd_set_lights(byte val) {

  // When setting the lights with the 0xED command the keyboard responds
  // with an "ack byte", 0xFA. This is NOT the same as the "ack bit" that
  // follows the succesful shifting of each command byte. See this web
  // page for a good description of all this:
  // http://www.beyondlogic.org/keyboard/keybrd.htm
  //https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf
  //https://wiki.osdev.org/PS/2_Keyboard

#ifdef DEBUG
Serial.println();
Serial.print("send val:");
Serial.println(val);
#endif

  cmd_ack_byte_ok = false;   // initialise the ack byte flag
  kbd_send_command(0xED);    // send the command byte
  
  while (!cmd_ack_byte_ok) delayMicroseconds(60) ; // ack byte from keyboard sets this flag

  kbd_send_command(val);     // now send the data
  
}



/*
-----------
MIT License
-----------
Copyright (c) 2021 Carsten Herting
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
