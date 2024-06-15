#! /usr/bin/python3
import os

html = "<html><body><h2>Go away</h2></body></html>"
print("Status: 307")
print("Location: /action.html")
print("Content-Type: text/html\n")
print(html)
