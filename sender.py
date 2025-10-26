import serial
import time
import crcmod
import csv
import os, sys

def crc16(data: bytes | list[int]) -> int:
    crc = 0xFFFF
    for byte in data:
        x = ((crc >> 8) ^ byte) & 0xFF
        x ^= (x >> 4)
        crc = ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xFFFF
    return crc

def main(serial_port="/dev/ttyAMA10", 
         print_to_screen=True, 
         timeout_ms=200, 
         baud_rate=115200,
         num_messages=100000):

# Message Number
    seq = 0
    filepath = 'out/sender_data.csv'

# --- Setup UART ---
    uart = serial.Serial(
        port=serial_port,
        baudrate=baud_rate,
        timeout=0.01  # 10ms read timeout
    )

# --- CRC setup (CRC-16-CCITT-FALSE) ---
# Polynomial 0x1021, initial value 0xFFFF, no reverse, no final XOR
#    crc16 = crcmod.predefined.Crc('ccitt-false')

    print("Raspberry Pi UART Transmitter Ready")

# Create an output directory to store the data if it doesn't exist
    file_exists = os.path.isfile(filepath)

# Add the header if the file doesn't exist
    if not file_exists:
        with open(filepath, mode='w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['Status','Time (ms)', 'Response Time (ms)', 'Seq'])
    csv_data = []
    while seq < num_messages:
        # --- Build payload ---
        payload = f"SEQ:{seq}"
#        crc16.update(payload.encode('utf-8'))
        crc = crc16(payload.encode('utf-8'))
#        crc = crc16.crcValue

        msg = f"{payload}:{crc:04X}\n"

        # --- Send message ---
        uart.write(msg.encode('utf-8'))
        uart.flush()
        t0 = time.time() * 1000  # in ms
        if print_to_screen:
            print(f"Sent,{int(t0)},{msg.strip()}")
        # No "return time", so it is signaled with -1
        csv_data.append(['SENT', int(t0), -1 , msg.strip()])
        # --- Wait for ACK ---
        response = ""
        got_ack = False
        buffer = ""
        response_data = []

        while (time.time() * 1000 - t0) < timeout_ms:
            data = uart.read().decode(errors='ignore')
            if data:
                for c in data:
                    if c == '\n':
                        response = buffer.strip()
                        buffer = ""
                        tf = time.time() * 1000
                        if response.startswith("ACK:"):
                            parts = response.split(":")
                            if len(parts) == 3:
                                try:
                                    rseq = int(parts[1])
                                    rchk = int(parts[2], 16)
                                    validate = f"SEQ:{rseq}"
                                    calc = crc16(validate.encode('utf-8'))
                                   # crc16.update(validate.encode('utf-8'))
                                    # calc = crc16.crcValue

                                    if calc == rchk:
                                        if rseq == seq:
                                            if print_to_screen:
                                                print(f"ACK_OK,{int(tf)},{int(tf - t0)},{rseq}")
                                            response_data = ['ACK_OK', int(tf), int(tf-t0), rseq]
                                        else: 
                                            if print_to_screen:
                                                print(f"ACK_OK/LATE,{int(tf)},{int(tf - t0)},{rseq}")
                                            response_data = ['ACK_OK/LATE', int(tf), int(tf-t0), rseq, seq]
                                    else:
                                        if print_to_screen:
                                            print(f"ACK_BAD,{int(tf)},{int(tf - t0)},{rseq}")
                                        response_data = ['ACK_BAD', int(tf), int(tf-t0), rseq]
                                    got_ack = True
                                    break
                                except ValueError:
                                    pass
                    else:
                        buffer += c
            if got_ack:
                break

        if not got_ack:
            if print_to_screen:
                print(f"TIMEOUT,{int(time.time()*1000)},{seq}")
            response_data = ['TIMEOUT', int(time.time()*1000), -1, seq]
        csv_data.append(response_data)
        seq += 1
        
        # Display a progress bar to monitor the messages being sent
        progress = seq / num_messages
        bar_length = 40
        filled_length = int(bar_length * progress)
        bar = "#" * filled_length + "-" * (bar_length - filled_length)

        # Write the progress line
        sys.stdout.write(f"\rSending messages: |{bar}| {seq}/{num_messages} ({progress*100:.1f}%)")
        sys.stdout.flush()

    print("Finished Sending Messages")
    with open(filepath, mode='a', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(csv_data)

    print(f"Data written to {filepath}")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="UART Sender Data Collector")
    parser.add_argument(
        "--serial_port", 
        type=str, 
        default="/dev/ttyAMA10",
        help= "serial port which the sender is using to communicate with the receiver",
    )
    parser.add_argument(
        "--print_to_screen", 
        type=bool, 
        default=True,
        help="prints the data to screen as it collects it",
    )
    parser.add_argument(
        "--timeout_ms",
        type=int,
        default=200,
        help="Number of milliseconds to wait for a response before a timeout",
    )
    parser.add_argument(
        "--baud_rate", 
        type=int, 
        default=115200,
        help="Communication rate between the sender and receiver. Must be the same as receiver",
    )
    parser.add_argument(
        "--num_messages", 
        type=int, 
        default=100000,
        help="Number of messages to send to the receiver",
    )
    args = parser.parse_args()
    main(
        serial_port=args.serial_port,
        print_to_screen=args.print_to_screen,
        timeout_ms=args.timeout_ms,
        baud_rate=args.baud_rate,
        num_messages=args.num_messages,
    )

