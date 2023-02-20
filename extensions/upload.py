import cgi
import os

form = cgi.FieldStorage()
upload_dir = os.environ["ROOT"]

if "file" in form:
    file = form["file"]
    if isinstance(file, cgi.FieldStorage) and file.filename:
        with open(f"{upload_dir}/{file.filename}", "wb") as f:
            f.write(file.file.read())
        content = "Successfully uploaded"
    else:
        content = "Not uploaded"
else:
    content = "No file specified"

print("Content-Type: text/html")
print(f"Content-Length: {len(content)}")
print()
print(content)