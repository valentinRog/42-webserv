import requests
import os

URL = "http://0.0.0.0:80"

def upload_and_delete(filename):
    path = os.path.join(os.path.dirname(__file__), f"data/{filename}")
    r = requests.post(URL + "/form/upload", files={"file": open(path, "rb")})

    assert r.status_code == 200

    assert open(path, "rb").read() == open(os.path.join(os.path.dirname(__file__), f"www/upload/{filename}"), "rb").read()

    r = requests.delete(f"{URL}/upload/{filename}")

    assert r.status_code == 200

    assert not os.path.exists(os.path.join(os.path.dirname(__file__), f"www/upload/{filename}"))

def upload_too_large(filename):
    path = os.path.join(os.path.dirname(__file__), f"data/{filename}")
    r = requests.post(URL + "/form/upload", files={"file": open(path, "rb")})

    assert r.status_code == 413

def test_upload_small():
    upload_and_delete("test.txt")

# def test_upload_binary():
#     upload_and_delete("fractol_compressed.png")

# def test_upload_too_large():
#     upload_too_large("fractol_less_compressed.png")
