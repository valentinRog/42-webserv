import requests

URL = "http://0.0.0.0"

def test_invalid():
    r = requests.get(URL + "/aaaaaa")
    assert r.status_code == 404

    r = requests.get(URL + "/index.htmlll")
    assert r.status_code == 404

def test_existing():
    r = requests.get(URL)
    assert r.status_code == 200

    r = requests.get(URL + "/index.html")
    assert r.status_code == 200
