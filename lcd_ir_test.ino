#define IR_SMALLD_NEC

#include <SoftwareSerial.h>
#include <LiquidCrystal_74HC595.h>  // used for shift register use
#include <time.h>
#include <IRsmallDecoder.h>

LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);
SoftwareSerial bt(1, 0);

IRsmallDecoder irDecoder(2);
irSmallD_t irData;

const int led_right = 5;
const int led_left = 6;


byte song[8] = {  // make music note
  B00001,
  B00001,
  B00001,
  B00001,
  B01111,
  B11111,
  B11111,
  B01110,
};

byte artist[8] {  // make microphone (or person head idk)
  B01110,
  B10001,
  B10001,
  B01110,
  B01110,
  B10001,
  B10001,
  B10001,
};

byte album[8] = {
  B00000,
  B00000,
  B01110,
  B10001,
  B10101,
  B10001,
  B01110,
  B00000,
};

byte album1[8] = {
  B00000,
  B11000,
  B10100,
  B10010,
  B10010,
  B10100,
  B11000,
  B00000,
};


void scroller(char buf[], int len, int pos, int type);          // calls scrolling function
int ir_switch(int posit, char ir_input);

// all millis variables are long unsigned to maximize space

long unsigned int total_finish = 0;
long unsigned int serial_last = millis();               // makes serial check once every .5 seconds
long unsigned int scroll_last = millis();
long unsigned int scroll_begin_time = millis();
long unsigned int current_time = millis();
long unsigned int timer[3] = {millis(), millis(), millis()};

bool scroll_begin = true;         // checks if scroll is still at first position

bool wait[] = {true, true, true};

bool hit[] = {false, false, false};

bool forward = false;
bool backward = false;
int forward_time = 0;
int backward_time = 0;

bool new_scroll = true;           // ready for new loop
bool ready_scroll = false;        // makes sure we dont start before getting a signal
bool end_scroll = false;

bool wait_save[] = {false, false, false};

bool flash = true;            // debugger led

int posit = 1;

int pos[] = {0, 0, 0};

int len[3] = {0, 0, 0};

char songbuf[100];            // song name as array
char artistbuf[100];          // artist name as array
char albumbuf[100];

char buffer1[100];
char buffer2[100];

char ir_input;

void setup() {
  // put your setup code here, to run once:
  lcd.createChar(0, song);    // makes music note as lcd character
  lcd.createChar(1, artist);  // makes microphone as lcd character
  lcd.createChar(2, album);

  lcd.begin(16, 2);           // initializes lcd
  lcd.clear();                // clears lcd
  delay(1000);

  lcd.write(byte(posit - 1));       // writes music note for song
  lcd.setCursor(0, 1);        // change cursor position
  lcd.write(byte(posit));         // writes microphone for artist
  //Serial.begin(9600);        // starts serial connection

  pinMode(led_left, OUTPUT);
  pinMode(led_right, OUTPUT);

  bt.begin(9600);
  bt.setTimeout(10);

}

void loop() {

  current_time = millis();                          // keeps track of time

  if (current_time - forward_time > 500 && forward == true) {
    digitalWrite(led_right, LOW);
    forward = false;
  }
  if (current_time - backward_time > 500 && backward == true) {
    digitalWrite(led_left, LOW);
    backward = false;
  }

  if (irDecoder.dataAvailable(irData)) {
    switch (irData.cmd) {
      case 0x9:
        ir_input = 'u';   // scroll up
        break;
      case 0x7:
        ir_input = 'd';  // scroll down
        break;
      case 0x40:
        ir_input = 'p';   //pause or play
        break;
      case 0x43:
        ir_input = 'f';   // skip forward
        break;
      case 0x44:
        ir_input = 'b';   // skip back
        break;
      default:
        ir_input = 'q';
    }
    if (ir_input != 'q') {
      posit = ir_switch(posit, ir_input);
      lcd.clear();                                // clears screen (can get funky stuff if this doesnt happen)
      lcd.setCursor(0, 0);
      lcd.write(byte(posit - 1));                     // shows music note
      lcd.setCursor(0, 1);                        // change cursor position
      lcd.write(byte(posit));                         // shows microphone

      for (int i = 0; i < 3; i++) {
        pos[i] = 0;
      }
      switch (posit) {
        case 1:
          for (int i = 0; i < len[0]; i++) {
            buffer1[i] = songbuf[i];
          }
          for (int i = 0; i < len[1]; i++) {
            buffer2[i] = artistbuf[i];
          }
          break;
        case 2:
          for (int i = 0; i < len[1]; i++) {
            buffer1[i] = artistbuf[i];
          }
          for (int i = 0; i < len[2]; i++) {
            buffer2[i] = albumbuf[i];
          }
          break;
      }
      for (int i = 0; i < len[posit - 1]; i++) {
        lcd.setCursor(i + 2, 0);
        lcd.write(buffer1[i]);
      }
      for (int i = 0; i < len[posit]; i++) {
        lcd.setCursor(i + 2, 1);
        lcd.write(buffer2[i]);
      }
    }
  }

  if (current_time - serial_last > 200) {           // checks if its been .5 seconds since last serial read
    flash = !flash;
    if (bt.available()) {                  // checks if new serial info to read
      int x = bt.available();                  // checks length of serial info
      lcd.clear();                                // clears screen (can get funky stuff if this doesnt happen)
      lcd.setCursor(0, 0);
      lcd.write(byte(posit - 1));                     // shows music note
      lcd.setCursor(0, 1);                        // change cursor position
      lcd.write(byte(posit));                         // shows microphone
      /*
              format of serial input should be 'song:artist:'

      */
      ready_scroll = true;
      scroll_begin_time = millis();
      len[0] = bt.readBytesUntil(':', songbuf, 100);     // reads length of song

      len[1] = bt.readBytesUntil(':', artistbuf, 100);  // reads length of artist

      len[2] = bt.readBytesUntil(':', albumbuf, 100);

      //Serial.println("ready:");
      //Serial.println(albumbuf);

      bt.read();                                           // throw away extra colon

      for (int i = 0; i < 3; i++)
        pos[i] = 0;

      switch (posit) {
        case 1:
          for (int i = 0; i < len[0]; i++) {
            buffer1[i] = songbuf[i];
          }
          for (int i = 0; i < len[1]; i++) {
            buffer2[i] = artistbuf[i];
          }
          break;
        case 2:
          for (int i = 0; i < len[1]; i++) {
            buffer1[i] = artistbuf[i];
          }
          for (int i = 0; i < len[2]; i++) {
            buffer2[i] = albumbuf[i];
          }
          break;


      }
      //      for (int i = 0; i < len[2]; i++) {                     // for each character
      //        for (int j = 0; j < 2; j++) {
      //          lcd.setCursor(i + 2, posit + j);                              // lcd offset by 2
      //          lcd.write(albumbuf[i]);                                // write char to lcd
      //          pos[posit - 1 + j] = 0;                                    // scroll pos 0
      //        }
      //      }
    }
  }

  if (new_scroll == true && current_time - scroll_begin_time > 2500 && ready_scroll == true) {    // if scroll is at the start position and has waited a while
    new_scroll = false;                                                   // allow code to enter scroller call
  }

  if (current_time - scroll_last > 400 && new_scroll == false) {    // if .2s and not at start
    scroll_last = millis();                                         // resets timer


    if (pos[posit - 1] + 14 >= len[posit - 1] && hit[posit - 1] == false) {
      hit[posit - 1] = true;
      timer[posit - 1] = millis();
    }

    if (pos[posit - 1] + 14 <= len[posit - 1]) {                                   // if song hasn't reached end
      scroller(buffer1, len[posit - 1], pos[posit - 1], 1);         // call scroller, check if end
      pos[posit - 1]++;                                                 // increment song position
    }

    else if (pos[posit - 1] + 14 >= len[posit - 1] && hit[posit - 1] == true) {                            // if song position is at end but not reverted back to start
      if (current_time - timer[posit - 1] > 2000) {
        hit[posit - 1] = false;
        for (int i = 0; i < len[posit - 1]; i++) {                    // for each character
          lcd.setCursor(i + 2, 0);                             // lcd offseet by 2 for music note
          lcd.write(buffer1[i]);                               // write char to lcd
        }
      }
      pos[posit - 1] = 0;
      wait[posit - 1] = false;
    }

    if (pos[posit] + 14 >= len[posit] && hit[posit] == false) {
      hit[posit] = true;
      timer[posit] = millis();
    }

    if (pos[posit] + 14 <= len[posit]) {                           // if artist isn't at end
      scroller(buffer2, len[posit], pos[posit], 2);              // call scroller, return end
      pos[posit]++;                                               // increment artist position
    }

    else if (pos[posit] + 14 >= len[posit] && hit[posit] == true) {                               // if artist position is at end but not reverted to start
      if (current_time - timer[posit] > 2000) {
        hit[posit] = false;

        for (int i = 0; i < len[posit]; i++) {                       // for each character
          lcd.setCursor(i + 2, 1);                                // lcd offset by 2
          lcd.write(buffer2[i]);                                // write char to lcd
        }
        pos[posit] = 0;
        wait[posit] = false;
      }

    }

    if (wait[posit - 1] == false && wait[posit] == false) {             // once both song and artist have scrolled all the way
      new_scroll = true;                                          // get ready for new scroll iteration
      scroll_begin_time = millis();                               // reset scroll start time

      end_scroll = false;
      wait[posit - 1] = true;
      wait[posit] = true;

      //Serial.println("123");
    }
  }
}

void scroller(char buf[], int len, int pos, int type) {       // scroller function, input buffer, length of buffer, and position in scroll, returns if at end

  for (int i = pos; i < pos + 14; i++) {            // iterates through 14 times
    switch (type) {
      case 1:
        lcd.setCursor(i - pos + 2, 0);                  // sets cursor to pos 1-14
        break;

      case 2:
        lcd.setCursor(i - pos + 2, 1);                  // sets cursor to pos 1-14
        break;
    }

    lcd.write(buf[i]);                              // writes buffer to lcd
  }
}

int ir_switch(int posit, char i) {
  switch (i) {
    case 'u':
      posit--;
      break;
    case 'd':
      posit++;
      break;
    case 'p':
      bt.write("p");
      break;
    case 'f':
      bt.write("f");
      analogWrite(led_right, 255 * 0.5);
      forward = true;
      forward_time = millis();
      break;
    case 'b':
      bt.write("b");
      analogWrite(led_left, 255 * 0.5);
      backward = true;
      backward_time = millis();
      break;

  }
  constrain(posit, 1, 2);
  return posit;
}
