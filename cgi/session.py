#! /usr/bin/python3
import os
import secrets

def make_token():
	return secrets.token_urlsafe(24)

def make_session():
	sessionid = make_token()
	html = "<html><body><h2>Session created</h2></body></html>"
	print("Status: 200 OK")
	print("Set-Cookie: session=" + str(sessionid))
	print("Content-Length: " + str(len(html)))
	print("Content-Type: text/html\n")
	print(html)
	return sessionid

method = os.environ["REQUEST_METHOD"]
if (method != 'GET'):
	print("Status 405 Method Not Allowed")
	print("Connection: close\n")

sid = make_session()
