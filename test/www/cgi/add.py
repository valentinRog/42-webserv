import os
import sys
import urllib.parse

query_string = os.environ.get("QUERY_STRING", "")
query_dict = urllib.parse.parse_qs(query_string)

try:
    n1 = int(query_dict.get("n1")[0])
    n2 = int(query_dict.get("n2")[0])
    content = f"<output>{n1} + {n2} = {n1 + n2}</output>"
except:
    content = "<output>What the hell is this?</output>"

print("Content-Type: text/html")
print(f"Content-Length: {len(content)}")
print()
print(content)
