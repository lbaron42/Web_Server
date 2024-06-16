#!/usr/bin/env python3
import os

method = os.environ["REQUEST_METHOD"]
print("Method: " + str(method))
if (method == "DELETE"):
	if "PATH_TRANSLATED" in os.environ:
		cwd = os.getcwd()
		target = os.path.join(cwd, os.environ["PATH_TRANSLATED"])
		try:
			os.remove(target)
		except FileNotFoundError:
			html = f"<html><body>File not found: {target}</body></html>"
			print("Status: 404")
			print("Content-Length: " + str(len(html)))
			print("Content-Type: text/html\n")
			print(html)
		else:
			html = "<html><body>Resource deleted</body></html>"
			print("Status: 204")
			print("Content-Length: " + str(len(html)))
			print("Content-Type: text/html\n")
			print(html)
	else:
		html = "<html><body>No resource provided</body></html>"
		print("Status: 400")
		print("Content-Length: " + str(len(html)))
		print("Content-Type: text/html\n")
		print(html)
else:
	print("Status: 418\n")
	print("ligmabols")
