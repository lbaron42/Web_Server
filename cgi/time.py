#!/usr/bin/python3
from datetime import datetime

html = "<html><body><h1>"
html += str(datetime.now().strftime('%a, %d %b %Y  %H:%M:%S'))
html += "</h1></body></html>"
print("Status: 200 OK")
print("Content-Length: " + str(len(html)))
print("Content-Type: text/html\n")
print(html)
