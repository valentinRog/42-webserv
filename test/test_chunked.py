import requests

URL = "http://0.0.0.0"

headers = {
    "Transfer-Encoding": "chunked",
    "Content-Type": "text/plain",
}

def chunker(data, size):
    for i in range(0, len(data), size):
        chunk = data[i:i+size]
        yield chunk.encode()

def test_chunked():
    data = "This is the data to be sent in chunks"
    response = requests.post(f"{URL}/echo", headers=headers, data=chunker(data, 5))
    assert response.text == data
