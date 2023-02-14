import cgi
import cgitb

cgitb.enable()
input_data = cgi.FieldStorage()

try:
    n1 = int(input_data["n1"].value)
    n2 = int(input_data["n2"].value)
    content = f"<output>{n1} + {n2} = {n1 + n2}</output>"
except:
    content = "<output>What the hell is this?</output>"

print("Content-Type: text/html")
print(f"Content-Length: {len(content)}")
print()
print(content)