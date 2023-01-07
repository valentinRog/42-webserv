import sys
import socket

HOST = "127.0.0.1"

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, int(sys.argv[1]) if len(sys.argv) == 2 else 80))
        for line in sys.stdin:
            s.sendall(line.encode())
        s.sendall(b"\r\n\r\n")
