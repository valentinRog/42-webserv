CONTENT=$(cat << EOF
<p>Hello from bash</p>
EOF
)

echo Content-Length: $(echo -n $CONTENT | wc -c)
echo Content-Type: text/html
echo
echo $CONTENT