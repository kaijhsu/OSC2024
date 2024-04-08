import argparse
import serial
import os
from tqdm import tqdm, trange
import math
import time



def main():
    # parsing argument
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file", help="file to be transfered", type=str)
    parser.add_argument("-p", "--port", help="transfer portination", type=str)
    args = parser.parse_args()
    if args.file is None or args.port is None:
        print("Usage: python3 -f <file> -d <port>")

    # read kernel file
    with open(args.file, "rb") as file:
        bytecodes = file.read()
    
    # Describe kernel size
    size = os.path.getsize(args.file)
    print(f"File {args.file}: {size} bytes")

    # Open serial port to device
    try:
        port = serial.Serial(args.port, 115200)
    except:
        print("Serial Error!")
        exit(1)
    
    port.write(b'reboot\n')
    text = port.read_until(b'$ ').decode(encoding='ascii')

    start_time = time.time()
    chunk_size = 2048
    for i in range(0, len(bytecodes), chunk_size):
        while True:
            addr = hex(int("0x80000", 16)+i)
            byte = min(len(bytecodes)-i, chunk_size)
            port.write(f'w {addr} {byte}\n'.encode('ascii'))
            # print(f'w {addr} {byte}\n')
            port.write(bytecodes[i:i+byte])
            port.flush()
            check_sum = sum(bytecodes[i:i+byte])
            text = port.read_until(b'$ ').decode().split()
            if check_sum == int(text[6]):
                break
            print(f"{addr} transmit error!")
        left_icons = round(10*i/len(bytecodes))
        right_icons = 10 - left_icons
        print(f"{addr} {i: >6}/{len(bytecodes)}", end="")
        print(f" progress: {'🌌'*left_icons}{'🌠' if right_icons > 0 else '🌟'}{'⬛'*right_icons}", end="\n")
        print('\033[F', end="")

    print(f"\nFinish after {time.time() - start_time} sec!")
    port.write(f'jump 0x80000\n'.encode('ascii'))
    port.flush()

if __name__ == "__main__":
    main()
