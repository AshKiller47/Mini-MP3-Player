#include <Wire.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DFRobotDFPlayerMini.h>

#define OLED_ADDR 0x3C

#if(SSD1306_LCDHEIGHT != 32)
#error("Height Incorrect. Please Change OLED Display!");
#endif

SoftwareSerial softwareSerial(10, 11); Adafruit_SSD1306 OLED_Display(-1); DFRobotDFPlayerMini DFPlayer;

const byte rows = 3, cols = 4; int volume = 20, eq_count = 1, lp_count = 0, rd_count = 0, lp_flag = 0, sp_flag = 0; long int track, total; bool paused = true, stopped = false;

char keys[rows][cols] = {{'A', 'B', 'C', 'D'},
                         {'E', 'F', 'G', 'H'},
                         {'I', 'J', 'K', 'L'}};
                         
byte rowPins[rows] = {2, 3, 4}, colPins[cols] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

void setup() {
  
  // put your setup code here, to run once:
  
  softwareSerial.begin(9600); OLED_Display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR); OLED_Display.clearDisplay(); OLED_Display.setTextSize(1); OLED_Display.setTextColor(WHITE);
  
  OLED_Display.setCursor(27, 5); OLED_Display.print(F("DFPlayer Mini")); OLED_Display.display(); delay(2000); OLED_Display.setCursor(14, 17); OLED_Display.print(F("(Initializing...)")); OLED_Display.display(); delay(2000);
  
  if(!DFPlayer.begin(softwareSerial)) {
    
    OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 2); OLED_Display.print(F("Unable To Begin!")); OLED_Display.setCursor(5, 12); OLED_Display.print(F("1. Recheck Connections")); OLED_Display.setCursor(5, 22); OLED_Display.print(F("2. Insert SD Card")); OLED_Display.display(); while(true);
    
  }
  
  OLED_Display.clearDisplay(); OLED_Display.setCursor(27, 5); OLED_Display.print(F("DFPlayer Mini")); OLED_Display.setCursor(39, 17); OLED_Display.print(F("(Online!)")); OLED_Display.display(); delay(2000);
  
  DFPlayer.outputDevice(DFPLAYER_DEVICE_SD); DFPlayer.EQ(DFPLAYER_EQ_NORMAL); DFPlayer.volume(volume); DFPlayer.play(1); DFPlayer.stop(); track = DFPlayer.readCurrentFileNumber(); total = DFPlayer.readFileCounts(); show();
  
}

void loop() {
  
  // put your main code here, to run repeatedly:
  
  char key = keypad.getKey(); if(key != NO_KEY) control(key); if(DFPlayer.available()) printDetail(DFPlayer.readType(), DFPlayer.read());
  
}

void control(char key) {
  
  switch(key) {
    
    case 'A':
      
      OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 7); OLED_Display.print(F("Track: ")); OLED_Display.setCursor(43, 7); OLED_Display.print(DFPlayer.readCurrentFileNumber()); coil(); shuffle();
      
      if(paused) { DFPlayer.start(); OLED_Display.fillTriangle(107, 3, 107, 13, 122, 8, WHITE); paused = false;  }
      
      else {  DFPlayer.pause(); OLED_Display.fillRect(108, 3, 4, 11, WHITE); OLED_Display.fillRect(115, 3, 4, 11, WHITE); paused = true; }
      
      equalizer(); OLED_Display.setCursor(86, 19); OLED_Display.print(track); OLED_Display.setCursor(108, 19); OLED_Display.print(volume); OLED_Display.display(); stopped = 0; break;
      
    case 'B':
      
      if(paused) { DFPlayer.previous(); DFPlayer.pause();  }
      
      else DFPlayer.previous(); track = DFPlayer.readCurrentFileNumber(); show(); break;
      
    case 'C':
      
      if(paused) { DFPlayer.next(); DFPlayer.pause();  }
      
      else DFPlayer.next(); track = DFPlayer.readCurrentFileNumber(); show(); break;
      
    case 'D':
      
      DFPlayer.stop(); OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 7); OLED_Display.print(F("Track: ")); OLED_Display.setCursor(43, 7); OLED_Display.print(DFPlayer.readCurrentFileNumber()); OLED_Display.fillRect(107, 4, 14, 8, WHITE);
      
      equalizer(); OLED_Display.setCursor(86, 19); OLED_Display.print(track); OLED_Display.setCursor(108, 19); OLED_Display.print(volume); OLED_Display.display();

      lp_count = 0; lp_flag = 0; paused = true; stopped = true; if(rd_count == 1) { DFPlayer.loop(DFPlayer.readCurrentFileNumber()); DFPlayer.disableLoopAll(); rd_count = 0; }
      
      break;
      
    case 'E':
    
      eq_count++;
      
      switch(eq_count) {
        
        case 1: DFPlayer.EQ(DFPLAYER_EQ_NORMAL); show(); break;
        
        case 2: DFPlayer.EQ(DFPLAYER_EQ_POP); show(); break;
        
        case 3: DFPlayer.EQ(DFPLAYER_EQ_ROCK); show(); break;
        
        case 4: DFPlayer.EQ(DFPLAYER_EQ_JAZZ); show(); break;
        
        case 5: DFPlayer.EQ(DFPLAYER_EQ_CLASSIC); show(); break;
        
        case 6: DFPlayer.EQ(DFPLAYER_EQ_BASS); show(); eq_count = 0; break;
        
        default: break;
        
      }
      
      break;
      
    case 'F':
      
      if(volume >= 1) { DFPlayer.volumeDown(); volume--;  }
      
      show(); break;
      
    case 'G':
    
      if(volume <= 29) {  DFPlayer.volumeUp(); volume++;  }
      
      show(); break;
      
    case 'H':
    
      lp_count++; rd_count = 0;
      
      switch(lp_count) {
        
        case 1: DFPlayer.enableLoopAll(); while(DFPlayer.readCurrentFileNumber() < track) DFPlayer.next(); if(paused) DFPlayer.pause(); show(); lp_flag = 1; break;
        
        case 2: DFPlayer.loop(DFPlayer.readCurrentFileNumber()); if(paused) DFPlayer.pause(); show(); lp_flag = 2; break;
        
        case 3: DFPlayer.disableLoopAll(); if(!paused) DFPlayer.play(DFPlayer.readCurrentFileNumber()); show(); lp_count = 0; lp_flag = 0; break;
        
        default: break;
        
      }
      
      break;
      
    case 'I':
      
      if(sp_flag == 1) {
        
        while(DFPlayer.readCurrentFileNumber() != track) {  if(DFPlayer.readCurrentFileNumber() < track) DFPlayer.next(); else if(DFPlayer.readCurrentFileNumber() > track) DFPlayer.previous();  }
        
        paused = false; show(); stopped = false; sp_flag = 0;
        
      }
      
      break;
      
    case 'J': track--; if(track == 0) track = total; show(); sp_flag = 1; break;
    
    case 'K': track++; if(track == total + 1) track = 1; show(); sp_flag = 1; break;
    
    case 'L': lp_count = 0; rd_count = 1; DFPlayer.randomAll(); if(paused) DFPlayer.pause(); track = DFPlayer.readCurrentFileNumber(); show(); lp_flag = 3; break;
    
    default: break;
    
  }
  
}

void show() {
  
  OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 7); OLED_Display.print(F("Track: ")); OLED_Display.setCursor(43, 7); OLED_Display.print(DFPlayer.readCurrentFileNumber()); coil(); shuffle();
  
  if(paused) { if (stopped) OLED_Display.fillRect(107, 4, 14, 8, WHITE); else { OLED_Display.fillRect(108, 3, 4, 11, WHITE); OLED_Display.fillRect(115, 3, 4, 11, WHITE); } }
  
  else OLED_Display.fillTriangle(107, 3, 107, 13, 122, 8, WHITE); equalizer(); OLED_Display.setCursor(86, 19); OLED_Display.print(track); OLED_Display.setCursor(108, 19); OLED_Display.print(volume); OLED_Display.display();
  
}

void equalizer() {
  
  String eq; if(eq_count == 1) eq = "Normal"; else if(eq_count == 2) eq = "Pop"; else if(eq_count == 3) eq = "Rock"; else if(eq_count == 4) eq = "Jazz"; else if(eq_count == 5) eq = "Classic"; else if(eq_count == 0 || eq_count == 6) eq = "Bass";
  
  OLED_Display.setCursor(5, 19); OLED_Display.print(F("EQ: ")); OLED_Display.setCursor(25, 19); OLED_Display.print(eq);
  
}

void coil() { if(lp_count == 1) OLED_Display.drawCircle(91, 8, 5, WHITE); else if(lp_count == 2) {  OLED_Display.drawCircle(91, 8, 5, WHITE); OLED_Display.drawFastVLine(91, 5, 7, WHITE);  } }

void shuffle() {  if(rd_count == 1) { OLED_Display.drawFastHLine(84, 7, 14, WHITE); OLED_Display.drawFastHLine(84, 9, 14, WHITE); } }

void printDetail(uint8_t type, int value) {
  
  OLED_Display.clearDisplay();
  
  switch(type) {
    
    case TimeOut: OLED_Display.setCursor(38, 12); OLED_Display.print(F("Time Out!")); OLED_Display.display(); break;
    
    case WrongStack: OLED_Display.setCursor(32, 12); OLED_Display.print(F("Stack Error!")); OLED_Display.display(); break;
    
    case DFPlayerCardInserted: OLED_Display.setCursor(25, 12); OLED_Display.print(F("Card Inserted!")); OLED_Display.display(); break;
    
    case DFPlayerCardRemoved: OLED_Display.setCursor(27, 12); OLED_Display.print(F("Card Removed!")); OLED_Display.display(); break;
    
    case DFPlayerCardOnline: OLED_Display.setCursor(30, 12); OLED_Display.print(F("Card Online!")); OLED_Display.display(); break;
    
    case DFPlayerPlayFinished:
      
      if(lp_flag == 0) {
        
        OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 7); OLED_Display.print(F("Track: ")); OLED_Display.setCursor(43, 7); OLED_Display.print(value); OLED_Display.fillRect(108, 4, 15, 8, WHITE); equalizer();
        
        OLED_Display.setCursor(86, 19); OLED_Display.print(track); OLED_Display.setCursor(108, 19); OLED_Display.print(volume); OLED_Display.display(); paused = true;
        
      }
      
      else if(lp_flag == 1) {
        
        OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 7); OLED_Display.print(F("Track: ")); OLED_Display.setCursor(43, 7);
        
        if(value == 14) { track = 1; OLED_Display.print(track); }
        
        else {  track = value + 1; OLED_Display.print(track); }
        
        coil(); shuffle(); OLED_Display.fillTriangle(107, 3, 107, 13, 122, 8, WHITE); equalizer(); OLED_Display.setCursor(86, 19); OLED_Display.print(track); OLED_Display.setCursor(108, 19); OLED_Display.print(volume); OLED_Display.display();
        
      }
      
      else if(lp_flag == 3) {
        
        DFPlayer.next(); OLED_Display.clearDisplay(); OLED_Display.setCursor(5, 7); OLED_Display.print(F("Track: ")); OLED_Display.setCursor(43, 7); OLED_Display.print(value); shuffle(); OLED_Display.fillTriangle(107, 3, 107, 13, 122, 8, WHITE);
        
        equalizer(); OLED_Display.setCursor(86, 19); OLED_Display.print(value); OLED_Display.setCursor(108, 19); OLED_Display.print(volume); OLED_Display.display(); track = value;
        
      }
      
      total = DFPlayer.readFileCounts(); break;
      
    case DFPlayerError:
      
      OLED_Display.setCursor(5, 2); OLED_Display.print(F("DFPlayerError!"));
      
      switch(value) {
        
        case Busy: OLED_Display.setCursor(5, 12); OLED_Display.print(F("Card Not Found")); OLED_Display.display(); break;
        
        case Sleeping: OLED_Display.setCursor(5, 12); OLED_Display.print(F("Sleeping")); OLED_Display.display(); break;
        
        case SerialWrongStack: OLED_Display.setCursor(5, 12); OLED_Display.print(F("Get Wrong Stack")); OLED_Display.display(); break;
        
        case CheckSumNotMatch: OLED_Display.setCursor(5, 12); OLED_Display.print(F("Check Sum Not Match")); OLED_Display.display(); break;
        
        case FileIndexOut: OLED_Display.setCursor(5, 12); OLED_Display.print(F("File Index Out of")); OLED_Display.setCursor(5, 22); OLED_Display.print(F("Bound")); OLED_Display.display(); break;
        
        case FileMismatch: OLED_Display.setCursor(5, 12); OLED_Display.print(F("Cannot Find File")); OLED_Display.display(); break;
        
        case Advertise: OLED_Display.setCursor(5, 12); OLED_Display.print(F("In Advertise")); OLED_Display.display(); break;
        
        default: break;
        
      }
      
      break;
      
    default: break;
    
  }
  
}