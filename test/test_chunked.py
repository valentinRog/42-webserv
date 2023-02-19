import requests

URL = "http://0.0.0.0"
data = "This is the data to be sent in chunks"
chunk_size = 10

headers = {
    "Transfer-Encoding": "chunked",
    "Content-Type": "text/plain",
}

# Use a generator to create chunks of data
def chunker(data, size):
    for i in range(0, len(data), size):
        chunk = data[i:i+size]
        yield chunk.encode()

# Send the request with chunks
response = requests.post(f"{URL}/echo", headers=headers, data=chunker(data, 5))

print(response.text)
