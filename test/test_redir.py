import socket

ADDRESS = "0.0.0.0"
PORT = 80

def test_redir():
    REQUEST = """\
GET /redir HTTP/1.1\r
Host: 0.0.0.0\r
\r
"""
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ADDRESS, PORT))
    s.send(REQUEST.encode())
    data = s.recv(1024)
    assert data.decode().startswith("HTTP/1.1 301")
    s.close()

