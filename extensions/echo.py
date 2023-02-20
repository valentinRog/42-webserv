import sys

print("Content-Type: text/plain")
print()

print(open(0, 'rb').read().decode(), end="")