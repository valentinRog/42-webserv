import socket

ADDRESS = "0.0.0.0"
PORT = 80

VALID_REQUEST = """\
GET / HTTP/1.1\r
Host: 0.0.0.0\r
\r
"""

BAD_METHOD_REQUEST = """\
gET / HTTP/1.1\r
Host: 0.0.0.0\r
\r
"""
def test_small_packets():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ADDRESS, PORT))
    for c in VALID_REQUEST:
        s.send(c.encode())
    data = s.recv(1024)
    assert data.decode().startswith("HTTP/1.1 200 OK")
    s.close()

def  test_bad_method():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ADDRESS, PORT))
    s.send(BAD_METHOD_REQUEST.encode())
    data = s.recv(1024)
    assert data.decode().startswith("HTTP/1.1 400 Bad Request")
    s.close()
