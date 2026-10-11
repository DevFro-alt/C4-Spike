#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int redLedPin = 10;
const int yellowLedPin = 11;
const int buzzerPin = 12;
const int defuseBtn = A0; 

String inputCode = "";
const String secretCode = "7355608";
int currentState = 0; 

unsigned long plantTime = 0;
unsigned long lastBeepTime = 0;
unsigned long ledOffTime = 0; 

bool isDefusing = false;
unsigned long defuseStartTime = 0;

int lastRemaining = -1;
int lastProgress = -1;
bool yellowLedState = false;

void setup() {
  lcd.init();
  lcd.backlight();
  
  pinMode(redLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(defuseBtn, INPUT_PULLUP); 

  digitalWrite(redLedPin, HIGH); 
  digitalWrite(yellowLedPin, LOW);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C4 Activated");
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");
}

void loop() {
  unsigned long now = millis();

  if (currentState == 0) {
    char key = keypad.getKey();
    
    if (key) {
      tone(buzzerPin, 2000, 50); 
      inputCode += key;
      
      lcd.setCursor(5 + inputCode.length() - 1, 1);
      lcd.print("*");
      
      if (inputCode.length() == secretCode.length()) {
        if (inputCode == secretCode) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Bomb has been");
          lcd.setCursor(0, 1);
          lcd.print("planted!");
          delay(1500); 
          
          currentState = 1;
          plantTime = millis();
          digitalWrite(redLedPin, LOW); 
          lcd.clear();
          lastRemaining = -1; 
        } else {
          inputCode = "";
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("C4 Activated");
          lcd.setCursor(0, 1);
          lcd.print("PIN: ");
        }
      }
    }
  } 
  else if (currentState == 1) {
    unsigned long elapsed = now - plantTime;
    int remaining = 40 - (elapsed / 1000); 
    
    if (remaining <= 0) {
      currentState = 2; 
      return;
    }

    int beepInterval;
    if (remaining > 20)      beepInterval = 1000;
    else if (remaining > 10) beepInterval = 500;
    else if (remaining > 5)  beepInterval = 250;
    else                     beepInterval = 125;
    
    int currentPitch = map(remaining, 40, 1, 1000, 3000);

    if (digitalRead(defuseBtn) == LOW) { 
      if (!isDefusing) {
        isDefusing = true;
        defuseStartTime = now;
        lastProgress = -1;
        digitalWrite(redLedPin, HIGH); 
        tone(buzzerPin, 1500);         
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DEFUSING...     ");
      }
      
      if (now - defuseStartTime >= 5000) {
        noTone(buzzerPin);
        currentState = 3; 
        return;
      }

      int progress = (now - defuseStartTime) / 300; 
      if (progress != lastProgress) {
        lastProgress = progress;
        lcd.setCursor(0, 1);
        for(int i = 0; i < progress && i < 16; i++) {
          lcd.print("=");
        }
      }

    } else { 
      if (isDefusing) {
        isDefusing = false;
        digitalWrite(redLedPin, LOW); 
        noTone(buzzerPin);            
        digitalWrite(yellowLedPin, LOW);
        yellowLedState = false;
        lcd.clear(); 
        lastRemaining = -1; 
      }
      
      if (remaining != lastRemaining) {
        lastRemaining = remaining;
        lcd.setCursor(0, 0);
        lcd.print("BOMB PLANTED    ");
        lcd.setCursor(0, 1);
        lcd.print("DETONATION: ");
        
        if (remaining >= 10) {
          lcd.print("**");
        } else {
          lcd.print("0");
          lcd.print(remaining);
        }
        lcd.print(" ");
      }
    }

    if (now - lastBeepTime >= beepInterval) {
      lastBeepTime = now;
      tone(buzzerPin, currentPitch, 50); 
      digitalWrite(yellowLedPin, HIGH); 
      yellowLedState = true;
      ledOffTime = now + 50; 
    }

    if (now >= ledOffTime && yellowLedState) {
      digitalWrite(yellowLedPin, LOW);
      yellowLedState = false;
      if (isDefusing) {
        tone(buzzerPin, 1500);
      }
    }
  }
  else if (currentState == 2) {
    digitalWrite(yellowLedPin, HIGH); 
    digitalWrite(redLedPin, HIGH);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CRITICAL ALARM!");
    lcd.setCursor(0, 1);
    lcd.print("DETONATING...   ");
    
    tone(buzzerPin, 4000); 
    delay(3000);           
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Terrorist Win!");
    lcd.setCursor(0, 1);
    lcd.print("Bomb Detonated  ");
    
    tone(buzzerPin, 400);  
    delay(5000);
    noTone(buzzerPin);
    
    while (true); 
  }
  else if (currentState == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("COUNTER-TERRORIST"); 
    lcd.setCursor(0, 1);
    lcd.print("WIN! DEFUSED.   ");
    
    digitalWrite(redLedPin, HIGH);
    digitalWrite(yellowLedPin, HIGH);
    
    while (true); 
  }
}