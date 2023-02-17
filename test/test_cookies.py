import requests

URL = "http://0.0.0.0"

s = requests.Session() 

def test_cookies():
    assert requests.get(f"{URL}/cookies").cookies.get("visit_count") == "1"
    N = 5
    for _ in range(N):
        s.get(f"{URL}/cookies")
    assert s.cookies.get("visit_count") == str(N)
