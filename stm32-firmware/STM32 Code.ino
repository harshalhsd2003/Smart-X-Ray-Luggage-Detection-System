#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== PINS =====
const int togglePin       = PB12;
const int estopPin        = PB10;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int beltRelayPin    = PA4;   // NC relay: LOW = motor ON, HIGH = motor OFF
const int solenoidPin     = PA5;   // NO solenoid: HIGH = ON, LOW = OFF

const int redLedPin       = PB1;
const int greenLedPin     = PA7;
const int yellowLedPin    = PB0;
const int buzzerPin       = PA0;

// ===== SOLENOID TIMING =====
const unsigned long BELT_STOP_DELAY     = 500;
const unsigned long SOLENOID_EXTEND_MS  = 1000;
const unsigned long SOLENOID_RETRACT_MS = 500;

// ===== STATES =====
bool systemOn     = false;
bool dangerActive = false;
bool estopActive  = false;

enum SolenoidState { SOL_IDLE, SOL_WAITING, SOL_EXTENDED, SOL_RETRACTING };
SolenoidState solState = SOL_IDLE;
unsigned long solTimer = 0;

// ===== HELPERS =====
void motorOn()     { digitalWrite(beltRelayPin, LOW);  }  // NC: LOW = motor ON
void motorOff()    { digitalWrite(beltRelayPin, HIGH); }  // NC: HIGH = motor OFF
void solenoidOn()  { digitalWrite(solenoidPin, HIGH);  }  // NO: HIGH = extended
void solenoidOff() { digitalWrite(solenoidPin, LOW);   }  // NO: LOW = retracted

// ===== SETUP =====
void setup() {
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  pinMode(solenoidPin, OUTPUT);
  solenoidOff();

  Serial.begin(9600);

  pinMode(togglePin, INPUT_PULLUP);
  pinMode(estopPin,  INPUT_PULLUP);

  pinMode(beltRelayPin,  OUTPUT);
  pinMode(redLedPin,     OUTPUT);
  pinMode(greenLedPin,   OUTPUT);
  pinMode(yellowLedPin,  OUTPUT);

  motorOff();

  lcd.init();
  lcd.backlight();
  lcd.print("SYSTEM READY");
  delay(800);

  systemOff();
}

// ===== LOOP =====
void loop() {
  checkEStop();

  if (estopActive) {
    blinkAllFast();
    return;
  }

  checkToggle();

  if (systemOn) {
    handleSerial();
    handleSolenoidFSM();
    blinkYellow();

    if (dangerActive) {
      dangerBuzzerPattern();
    }
  }
}

// ===== E-STOP =====
void checkEStop() {
  static unsigned long t = 0;
  static bool last = HIGH;

  bool r = digitalRead(estopPin);
  if (r != last) t = millis();

  if (millis() - t > 50) {
    if (r == LOW && !estopActive)
      activateEStop();

    if (r == HIGH && estopActive) {
      estopActive = false;
      lcd.clear();
      lcd.print("RESET");
      delay(500);
      systemOff();
    }
  }
  last = r;
}

void activateEStop() {
  estopActive  = true;
  systemOn     = false;
  dangerActive = false;
  solState     = SOL_IDLE;

  motorOff();
  solenoidOff();
  noTone(buzzerPin);

  lcd.clear();
  lcd.print("EMERGENCY STOP");
  lcd.setCursor(0,1);
  lcd.print("BELT+SOL OFF");
}

// ===== TOGGLE =====
void checkToggle() {
  if (digitalRead(togglePin) == LOW && !systemOn) {
    systemOn = true;
    activateSafe();
    delay(300);
  }

  if (digitalRead(togglePin) == HIGH && systemOn) {
    systemOn = false;
    systemOff();
  }
}

// ===== SERIAL =====
void handleSerial() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '1') activateDanger();
    if (c == '0') activateSafe();
  }
}

// ===== DANGER MODE =====
void activateDanger() {
  if (estopActive || dangerActive) return;

  dangerActive = true;

  motorOff();
  digitalWrite(redLedPin, HIGH);
  digitalWrite(greenLedPin, LOW);

  lcd.clear();
  lcd.print("STATUS:DANGER");
  lcd.setCursor(0,1);
  lcd.print("BELT STOPPED");

  solState = SOL_WAITING;
  solTimer = millis();
}

// ===== SAFE MODE =====
void activateSafe() {
  if (estopActive) return;

  dangerActive = false;
  solenoidOff();

  if (solState == SOL_IDLE) {
    motorOn();
  }

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);
  noTone(buzzerPin);

  lcd.clear();
  lcd.print("STATUS SAFE");
}

// ===== SOLENOID FSM =====
void handleSolenoidFSM() {
  switch (solState) {

    case SOL_IDLE:
      break;

    case SOL_WAITING:
      if (millis() - solTimer >= BELT_STOP_DELAY) {
        solenoidOn();
        lcd.setCursor(0,1);
        lcd.print("EJECTING...   ");
        solState = SOL_EXTENDED;
        solTimer = millis();
      }
      break;

    case SOL_EXTENDED:
      if (millis() - solTimer >= SOLENOID_EXTEND_MS) {
        solenoidOff();
        lcd.setCursor(0,1);
        lcd.print("RETRACTED     ");
        solState = SOL_RETRACTING;
        solTimer = millis();
      }
      break;

    case SOL_RETRACTING:
      if (millis() - solTimer >= SOLENOID_RETRACT_MS) {
        if (dangerActive && !estopActive) {
          solState = SOL_WAITING;
          solTimer = millis();
          lcd.setCursor(0,1);
          lcd.print("RETRYING...   ");
        } else {
          solState = SOL_IDLE;
          if (!estopActive) {
            motorOn();
            lcd.clear();
            lcd.print("STATUS SAFE");
          }
        }
      }
      break;
  }
}

// ===== SYSTEM OFF =====
void systemOff() {
  dangerActive = false;
  solState = SOL_IDLE;

  motorOff();
  solenoidOff();
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);

  noTone(buzzerPin);

  lcd.clear();
  lcd.print("SYSTEM OFF");
}

// ===== YELLOW BLINK =====
void blinkYellow() {
  static unsigned long t;

  if (dangerActive && millis() - t > 150) {
    t = millis();
    digitalWrite(yellowLedPin, !digitalRead(yellowLedPin));
  }

  if (!dangerActive)
    digitalWrite(yellowLedPin, HIGH);
}

// ===== DANGER BUZZER PATTERN =====
void dangerBuzzerPattern() {
  static unsigned long t;
  static bool state = false;

  if (millis() - t > 300) {
    t = millis();
    state = !state;
    if (state) tone(buzzerPin, 1000);
    else       noTone(buzzerPin);
  }
}

// ===== ESTOP BLINK =====
void blinkAllFast() {
  static unsigned long t;

  if (millis() - t > 120) {
    t = millis();
    bool s = !digitalRead(redLedPin);
    digitalWrite(redLedPin, s);
    digitalWrite(greenLedPin, s);
    digitalWrite(yellowLedPin, s);
    if (s) tone(buzzerPin, 1000);
    else   noTone(buzzerPin);
  }
}