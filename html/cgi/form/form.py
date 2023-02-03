import cgi, cgitb

cgitb.enable()

input_data = cgi.FieldStorage()

print("Content-Type: text/html")
print("")
print("<h1>Addition Results</h1>")
try:
    n1 = int(input_data["n1"].value)
    n2 = int(input_data["n2"].value)
except:
    print("<output>What the hell is this?</output>")
    raise SystemExit(1)
print(f"<output>{n1} + {n2} = {n1 + n2}</output>")
