#!/usr/bin/env python3
import os

method = os.environ["REQUEST_METHOD"]
if (method == "DELETE"):
	if "PATH_TRANSLATED" in os.environ:
		os.remove(os.environ["PATH_TRANSLATED"])
		html = "<html><body>Resource deleted</body></html>"
		print("Status: 204")
		print("Content-Length: " + str(len(html)))
		print("Content-Type: text/html\n")
		print(html)
