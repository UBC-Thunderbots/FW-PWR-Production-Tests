#include <Arduino.h>
#include <cstdint> 
#include <stdio.h>
#include "CRC.h"

// Define TX and RX pins for UART (change if needed)
#define TXD1 19 
#define RXD1 22 

int BAUD_RATE = 115200;
// Use Serial1 for UART communication
HardwareSerial uart_port(1);
uint32_t previousMsg = 0xFFFFFFFF;

void setup() {
  Serial.begin(BAUD_RATE);
  uart_port.begin(BAUD_RATE, SERIAL_8N1, RXD1, TXD1);  // UART setup
  Serial.println("ESP32 UART Receiver");
  delay(5000);
}

void loop() {
  String message = "";
  while(uart_port.available()) {
    // Read data
    char c = uart_port.read();
    if (c == '\n'){
      message.trim();
      
      if (message.length() > 0) {
        if (message.startsWith("SEQ:")) {
          int sequenceStart = message.indexOf(":");
          int checksumStart = message.indexOf(":", sequenceStart + 1);
                
          if (checksumStart > sequenceStart) {
            // Parse the message SEQ:<Message>:<Checksum>\n
            String srseq = message.substring(sequenceStart + 1, checksumStart);
            String srcsm = message.substring(checksumStart + 1);
            unsigned long rseq = srseq.toInt();
            uint16_t rcsm = (uint16_t)strtoul(srcsm.c_str(), NULL, 16);
            
            // Validate if the checksum matches the message received
            char validate[32];
            int len_v = snprintf(validate, sizeof(validate), "SEQ:%lu", rseq);
            uint16_t calc = CRC::Calculate(validate, len_v, CRC::CRC_16_CCITTFALSE());


            if (calc != rcsm) {
              Serial.printf("CORRUPT,%lu,%lu,Expected:%04X, got:%04X\n", (unsigned long)millis(),rseq,calc, rcsm);
                  
            } else if (previousMsg != 0xFFFFFFFF && rseq != previousMsg + 1) {
              // Check for missing packets
                // under the assumption that the messages are incremented 1 at a time
                if (rseq > previousMsg + 1) {
                  uint32_t missing = rseq - (previousMsg + 1);
                  Serial.printf("MISSING,%lu, Expected: %lu, got: %lu, missed: %lu\n",
                                (unsigned long) millis(),
                                (unsigned long)(previousMsg + 1),
                                (unsigned long) rseq, (unsigned long) missing);

                } else 
                  // Messages were sent out of order
                  Serial.printf("OUT_OF_ORDER,%lu, Prev: %lu, got: %lu\n",
                                (unsigned long) millis(),
                                (unsigned long) previousMsg,
                                (unsigned long) rseq);
            } else {
              Serial.printf("RECV_OK, %lu, %lu, %lu\n",
                            (unsigned long) millis(),
                            (unsigned long) rseq,
                            (unsigned long) rcsm);
            }
            previousMsg = rseq;
            char ackPayload[48];
            int alen = snprintf(ackPayload, sizeof(ackPayload), "ACK:%lu:%04X\n", (unsigned long)rseq, rcsm);
            uart_port.write(ackPayload);
            uart_port.flush();
            break;
          }
        }
      } else {
        message = "";
      }
    } else {
      message += c;
    }
  }
}

