import requests

def test_get():
    url = "http://localhost/"
    response = requests.get(url)
    assert response.status_code == 200