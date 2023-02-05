content = """
<p>Hello from Python</p>
"""

print("Content-Type: text/html")
print(f"Content-Length: {len(content)}")
print()
print(content)