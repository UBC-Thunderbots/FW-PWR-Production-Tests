# UART Test Bench
This repo contains projects and files to test UART communication lines between the Rasberry Pi, or with just another ESP.
This test bench validates communication between the devices, measures packet loss, data corruption, and checks latency.

### Project Structure
- receiver/ – ESP32 project for receiving UART messages.
- sender/ – ESP32 project for transmitting UART messages.
- sender.py – Python script to transmit data from a Raspberry Pi over UART for testing.
- offline_packages/ – Contains required Python packages for offline installation.

## Testing with the Rasberry Pi
1. Flash the ESP32 on the power board with the receiver project.
2. On the Raspberry Pi, copy the following into a directory:
  - sender.py
  - the offline_packages/ folder
3. Create and activate a virtual environment:
```
python3 -m venv venv
source venv/bin/activate
```
4. Install dependencies:
  If you have internet
  ```
  pip install -r requirements.txt
  ```
  If you are offline:
  ```
  pip install --no-index --find-links=offline_packages -r requirements.txt
  ```
5. Run the sender script:
   ```
   python sender.py --serial_port "/dev/ttyAMA10" --baud_rate 115200 --num_messages 100000 --timeout_ms 200 --print_to_screen True
   ```
#### Command Line Arguments

| Argument            | Type   | Default         | Description                                                     |
| ------------------- | ------ | --------------- | --------------------------------------------------------------- |
| `--serial_port`     | `str`  | `/dev/ttyAMA10` | Serial port used by the sender to communicate with the receiver |
| `--baud_rate`       | `int`  | `115200`        | Baud rate (must match receiver)                                 |
| `--timeout_ms`      | `int`  | `200`           | Time in milliseconds to wait for a response before timeout      |
| `--num_messages`    | `int`  | `100000`        | Number of messages to send                                      |
| `--print_to_screen` | `bool` | `True`          | Whether to print data to the console during collection          |


## Testing with two ESP32s
1. Flash one ESP32 with the sender project and the other with the receiver project.
2. Connect their UART TX/RX lines according to the pin definitions at the top of each project’s main source file.
3. To log received data, pipe the receiver and sender's serial output into a file. The logs are formatted as CSV, making them easy to analyze later.

## Future Developments
Implement data processing capabilities

