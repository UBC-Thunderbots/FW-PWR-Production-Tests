#include <Arduino.h>


// pin assignments
const int PIN_CHARGE = 26; // CHARGE
const int PIN_DONE = 25; // DONE
const int PIN_CHIP = 32; // CHIP
const int PIN_KICK = 33; // KICK

#define MAX_PULSE_WIDTH 4000

hw_timer_t* timer = NULL
// put function declarations here:
void capCharge();
void IRAM_ATTR pin_done();
void IRAM_ATTR stopPulse();
void sendPulse(int pulse_length, String action);
volatile bool cap_charge = false;

void setup() {

  pinMode(PIN_CHARGE OUTPUT);
  pinMode(PIN_DONE INPUT_PULLDOWN);
  pinMode(PIN_CHIP OUTPUT);
  pinMode(PIN_KICK OUTPUT);

  attachInterrupt(PIN_DONE, pin_done , RISING);
  digitalWrite(PIN_KICK, LOW);
  digitalWrite(PIN_CHIP, LOW);
  digitalWrite(PIN_CHARGE LOW);
  Serial.begin(9600);

  timer = timer.begin(0, 80, true);
  timerAttachInterrupt(timer, &stopPulse, true);

}

void loop() {

  while (Serial.available()){
    String input = Serial.readString();
    input.trim();
    if (input == "Charge"){
      Serial.println("Charging the capacitors");
      capCharge();
    } else if (input == "Kick" || input == "Chip"){
      if (cap_charge == false){
        Serial.println("You need to charge the capacitors first or let them finish charging");
      } else {
        pulse_width = MAX_PULSE_WIDTH + 1;
        while (pulse_width > MAX_PULSE_WIDTH || pulse_width < 1) {
          Serial.println("Enter your pulse length");
          int pulse_width = Serial.parseInt();
        }
        sendPulse(pulse_width,input);
      }
    } else {
      Serial.println("INVALID INPUT");
    }
  }

}

// put function definitions here:
void capCharge() {
  digitalWrite(PIN_CHARGE, HIGH);
}

void IRAM_ATTR pin_done{
  digitalWrite(PIN_CHARGE LOW);
  cap_charge = true;
  Serial.println("Capacitors are charged");
}
void IRAM_ATTR stopPulse() {
  digitalWrite(PIN_KICK, LOW);
  digitalWrite(PIN_CHIP, LOW);
}

void sendPulse(int pulse_length, String action) {
  timerWrite(timer, 0);
  timerAlarmWrite(timer, pulse_length, false);
  if (action.equals("Chip")){
    digitalWrite(PIN_CHIP, HIGH);
    
  } else if (action.equals("Kick")){
    digitalWrite(PIN_KICK, HIGH);
  }
  timerAlarmEnable(timer);
}
