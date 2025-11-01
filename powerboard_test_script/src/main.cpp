#include <Arduino.h>

// pin assignments
const int PIN_CHARGE = 26; // CHARGE
const int PIN_DONE = 25; // DONE
const int PIN_CHIP = 32; // CHIP
const int PIN_KICK = 33; // KICK

const int MAX_PULSE_WIDTH = 4000; // Max Kick and Chip Pulse in [us] 
const int BAUD_RATE = 115200;

void IRAM_ATTR capsCharged();
void IRAM_ATTR stopPulse();
void sendPulse(const int pulse_length, const String& action);
void chargeCaps();
volatile bool caps_charged = false;
hw_timer_t *timer = NULL;
SemaphoreHandle_t timer_lock = xSemaphoreCreateBinary();

void setup() {

  pinMode(PIN_CHARGE, OUTPUT);
  pinMode(PIN_DONE, INPUT_PULLDOWN);
  pinMode(PIN_CHIP, OUTPUT);
  pinMode(PIN_KICK, OUTPUT);

  attachInterrupt(PIN_DONE, capsCharged , RISING);
  digitalWrite(PIN_KICK, LOW);
  digitalWrite(PIN_CHIP, LOW);
  digitalWrite(PIN_CHARGE, LOW);
  Serial.begin(BAUD_RATE);
  
  // Set timer frequency to 1MHz and attach to stop pulse
  // Assuming clock speed is 80MHz
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &stopPulse, true);
  xSemaphoreGive(timer_lock);
}

void loop() {

  while (Serial.available()){
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.equalsIgnoreCase("charge")){
      Serial.println("Charging the capacitors...");
      chargeCaps();
    } else if (input.equalsIgnoreCase("kick") || input.equalsIgnoreCase("chip")){
      if (caps_charged){
        int pulse_width = MAX_PULSE_WIDTH + 1;
        while (pulse_width > MAX_PULSE_WIDTH || pulse_width < 1) {
          Serial.println("Enter your pulse length");
          int pulse_width = Serial.parseInt();
        }
        caps_charged = false;
        sendPulse(pulse_width, input);
      } else {
        Serial.println("You need to charge the capacitors first or let them finish charging");
      }
    } else {
      Serial.println("INVALID INPUT");
    }
  }

}

void chargeCaps() {
  digitalWrite(PIN_CHARGE,HIGH);
}

void IRAM_ATTR capsCharged() {
  digitalWrite(PIN_CHARGE, LOW);
  caps_charged = true;
  Serial.println("Capacitors are charged");
}

void IRAM_ATTR stopPulse() {
  digitalWrite(PIN_KICK, LOW);
  digitalWrite(PIN_CHIP, LOW);
  xSemaphoreGive(timer_lock);
}


void sendPulse(const int pulse_length, const String& action) {
  if(xSemaphoreTake(timer_lock, portMAX_DELAY)) {
    timerWrite(timer, 0);
    timerAlarmWrite(timer, pulse_length, false); 

    if (action.equalsIgnoreCase("Chip")){
      digitalWrite(PIN_CHIP, HIGH);

    } else if (action.equalsIgnoreCase("kick")){
      digitalWrite(PIN_KICK, HIGH);
    }
    // Start the timer to last for the pulse length starting at 0. Do not repeat.
    timerAlarmEnable(timer);
  } else {
    Serial.println("Pulse failed. Existing pulse");
  }
}

