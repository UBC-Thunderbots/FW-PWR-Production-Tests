// Sender Side
#include <Arduino.h>
#include <cstdint> 
#include <stdio.h>
#include "CRC.h"

#define TXD1 19
#define RXD1 22

int BAUD_RATE = 115200;

// Use Serial1 for UART communication
HardwareSerial uart_port(1);
unsigned long timoutms = 200;
uint32_t seq = 0;


void setup() {
  Serial.begin(BAUD_RATE);
  uart_port.begin(BAUD_RATE, SERIAL_8N1, RXD1, TXD1);  // UART setup
  Serial.println("ESP32 UART Transmitter");
}


void loop() {
  // Building a message of the form SEQ:Payload:Checksum\n
  // Creates a string with the label and the payload
  char payload[32];
  int len = snprintf(payload, sizeof(payload), "SEQ:%lu", (unsigned long) seq);

  // Create the checksum for the message
  uint16_t crc = CRC::Calculate(payload, len, CRC::CRC_16_CCITTFALSE());
  
  //Append the checksum to the message
  char msg[42];
  int msglen = snprintf(msg, sizeof(msg), "%s:%04X\n", payload, crc);

  // Send the message
  uart_port.write(msg);
  // Wait for message to be sent
  uart_port.flush();
  unsigned long t0 = millis();
  Serial.printf("Sent,%lu,%s\n", (unsigned long)t0, msg);

  // Wait for an acknowledgment of the message for up to timeoums millis
  String response = "";
  bool gotAcknowledgment = false;
  while(millis() - t0 < timoutms) {
      while(uart_port.available()) {
        char c = uart_port.read();
        if (c == '\n') {
          response.trim();
          unsigned long tf = millis();
          if (response.length() > 0) {
            if (response.startsWith("ACK:")) {
              int sequenceStart = response.indexOf(":");
              int checksumStart = response.indexOf(":", sequenceStart + 1);
              
              if (checksumStart > sequenceStart) {
                String returnedSeq = response.substring(sequenceStart + 1, checksumStart);
                String returnedChksum = response.substring(checksumStart + 1);
                
                unsigned long rseq = returnedSeq.toInt();
                uint16_t rchecksum = (uint16_t)strtoul(returnedChksum.c_str(), NULL, 16);
                
                char validate[32];
                int len_v = snprintf(validate, sizeof(validate), "SEQ:%lu", rseq);
                uint16_t calc = CRC::Calculate(validate, sizeof(validate), CRC::CRC_16_CCITTFALSE());

                if (calc == crc && rseq == seq) {
                  Serial.printf("ACK_OK,%lu,%lu,%lu\n", (unsigned long)millis(), tf - t0, rseq);
                
              } else {
                Serial.printf("ACK_BAD,%lu,%lu,%lu\n", (unsigned long)millis(), tf - t0, rseq);
              }
              gotAcknowledgment = true;
              break;
            }
          }
        } else {
          response = "";
          break;
        }
      } else {
        response += c;
      }
    }
    if (gotAcknowledgment) {
      break;
    }
  }
  if(!gotAcknowledgment) {
    Serial.printf("TIMEOUT,%lu,%lu\n", (unsigned long)millis(), seq);
  }
  seq++;
}
