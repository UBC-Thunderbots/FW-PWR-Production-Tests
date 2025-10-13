# UART Testing

A simple test bench to capture information about packet loss, data corruption,
error rate between two communicating ESP32's over UART.

One ESP32 acts as sender sending incrementing sequences with a checksum.
The other ESP32 is the receiver which parses the messages and validates the checksum.
The receiving ESP32 then sends an acknowledgement message back to the sending ESP32,
which is then again validated.

The output information for both is printed to the UART0 serial ports for both ESPs.

### Usage

Flash the sending ESP32 with the sending project and the receiving ESP32 with the
receiving project. Connect the RX1 and TX1 pins together. The pins are defined at the top
of each main file.

Connect a serial monitor to each ESP32 (if using pio: pio device monitor -b 115200).

