import socket

ADDRESS = "0.0.0.0"
PORT = 80

VALID_REQUEST = """\
GET / HTTP/1.1\r
Host: 0.0.0.0\r
\r
"""

UNCOMPLETED_REQUEST = """\
GET /"""

def test_small_packets():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ADDRESS, PORT))
    for c in VALID_REQUEST:
        s.send(c.encode())
    data = s.recv(1024)
    assert data.decode().startswith("HTTP/1.1 200 OK")
    s.close()

# def test_timeout():
#     s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#     s.connect((ADDRESS, PORT))
#     s.send(UNCOMPLETED_REQUEST.encode())
#     data = s.recv(1024)
#     assert data.decode().startswith("HTTP/1.1 408")
#     s.close()