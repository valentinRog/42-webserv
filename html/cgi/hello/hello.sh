CONTENT=$(cat << EOF
<p>Hello from bash</p>
EOF
)

echo Content-Type: text/html
echo
echo $CONTENT