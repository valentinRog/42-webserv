import cgi
import os
import http.cookies as Cookies

print("Content-Type: text/html")

# Get the cookies
cookies = Cookies.SimpleCookie(os.environ.get("HTTP_COOKIE"))

# Check if the cookie "visits" exists
if "visits" in cookies:
    # If it does, retrieve the value and increment it
    visits = int(cookies["visits"].value)
    visits += 1
else:
    # If it doesn't, set it to 1
    visits = 1

# Set the cookie "visits" with the updated value
cookie = Cookies.SimpleCookie()
cookie["visits"] = visits

# Set the cookie expiration to 1 day
cookie["visits"]["expires"] = 86400

# Add the cookie to the response header
print(cookie.output())

# Get the form data
form = cgi.FieldStorage()

# HTML page content
html = """
<html>
  <head>
    <title>CGI Cookie Example</title>
  </head>
  <body>
    <h1>CGI Cookie Example</h1>
    <p>You have visited this page {visits} times.</p>
  </body>
</html>
""".format(visits=visits)

print("\n")
print(html)
