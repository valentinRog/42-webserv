local content = [[
<p>Hello from Lua</p>
]]

io.write("Content-Type: text/html\r\n")
io.write(string.format("Content-Length: %d\r\n", #content))
io.write("\r\n")
io.write(content)
