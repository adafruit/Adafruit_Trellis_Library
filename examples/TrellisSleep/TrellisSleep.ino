/*********************************************************************************
 * Sample sketch to sleep and wake-up a trellis keypad.
 *********************************************************************************/

#include <Adafruit_Trellis.h>
#include <Adafruit_SleepyDog.h>

// Trellis setup
#define NUMKEYS 16
#define INT_PIN  1

// States
#define IDLE_LIGHT_UP   1
#define IDLE_TURN_OFF   2
#define IDLE_WAIT_OFF   3

Adafruit_Trellis trellis = Adafruit_Trellis();

unsigned long nextIdleTick = millis();
unsigned long enterTimeout = millis() + (1000L * 30L); // timeout after 30s

byte state = IDLE_LIGHT_UP;
byte nextLED = 0;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  trellis.begin(0x70);
  Serial.println("Trellis initialized");
  pinMode(INT_PIN, INPUT_PULLUP);
}

void loop() {
  unsigned long now = millis();
  if (nextIdleTick <= now) {
    nextIdleTick = now + tickIdleShow();
  }
  if (enterTimeout <= now) {
    onTimeout();
  }
}

unsigned long tickIdleShow() {
  // Light up all keys in order
  if (state == IDLE_LIGHT_UP) {
    trellis.setLED(nextLED);
    trellis.writeDisplay();
    nextLED = (nextLED + 1) % NUMKEYS;
    if (nextLED == 0) {
      state = IDLE_TURN_OFF;
    }
    return 100;

  // Turn off all keys in order
  } else if (state == IDLE_TURN_OFF) {
    trellis.clrLED(nextLED);
    trellis.writeDisplay();
    nextLED = (nextLED + 1) % NUMKEYS;
    if (nextLED == 0) {
      state = IDLE_WAIT_OFF;
      return 2000;
    }
    return 100;

  // Wait in darkness
  } else if (state == IDLE_WAIT_OFF) {
    trellis.clear(); // just to clean up
    trellis.writeDisplay();
    state = IDLE_LIGHT_UP;
    return 100;
  }
}

void onTimeout() {
  trellis.clear();
  trellis.writeDisplay();
  trellis.sleep();
  attachInterrupt(digitalPinToInterrupt(INT_PIN), onButtonIsr, LOW);
  Watchdog.sleep();

  trellis.wakeup();
  trellis.setLED(0);
  trellis.writeDisplay();
  delay(15);
  trellis.clrLED(0);
  trellis.writeDisplay();
}

void onButtonIsr() {
  detachInterrupt(digitalPinToInterrupt(INT_PIN));
  nextIdleTick = millis();
  enterTimeout = millis() + (1000L * 30L); // timeout again after 30s
  state = IDLE_LIGHT_UP;
  nextLED = 0;
}
