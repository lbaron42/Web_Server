#!/usr/bin/env python3

import os
import sys
import secrets
from urllib.parse import unquote

def get_html(title, content):
	HTML_HEAD = f'''<!DOCTYPE html><html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>{title}</title>
		<link href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css" rel="stylesheet">
	</head>
	<body>'''
	HTML_NAVBAR = '''<nav class="navbar navbar-expand-lg navbar-light bg-light">
		<a class="navbar-brand" href="/">MarvinX Blog</a>
		<div class="collapse navbar-collapse" id="navbarNav">
			<ul class="navbar-nav ml-auto">
				<li class="nav-item">
					<a class="nav-link" href="/create-post">Create Post(404 example)</a>
				</li>
				<li class="nav-item">
					<a class="nav-link" href="/logout">Logout</a>
				</li>
			</ul>
		</div>
	</nav>'''
	HTML_FOOT = "</body></html>"
	return HTML_HEAD + HTML_NAVBAR + content + HTML_FOOT

def ensure_database():
	if not os.path.exists(database_path):
		os.makedirs(database_path, exist_ok=True)
	if not os.path.exists(data_file):
		with open(data_file, "w") as f:
			pass
	if not os.path.exists(users_file):
		with open(users_file, "w") as f:
			pass
	if not os.path.exists(sessions_file):
		with open(sessions_file, "w") as f:
			pass

def read_input():
	form = {}
	for line in sys.stdin:
		form_data = line.split('&')
		form = dict(s.split('=') for s in form_data)
	return form

def read_data(file_path):
	data = []
	with open(file_path, "r") as f:
		for line in f:
			data.append(line.strip())
	return data

def write_data(file_path, line):
	with open(file_path, "a") as f:
		f.write(line + "\n")

def get_entry(file_path, key):
	entry = ''
	with open(file_path, "r") as file:
		for line in file:
			if line.startswith(key):
				entry = line
				break
	return entry

def delete_entry(file_path, key):
	with open(file_path, "r") as file:
		lines = file.readlines()
	with open(file_path, "w") as file:
		for line in lines:
			if not line.startswith(key):
				file.write(line)

def valid_credentials(username, password):
	creds = get_entry(users_file, username).split('|')
	return (len(creds) > 1 and creds[1] == password)

def make_session(username):
	sessionid = secrets.token_urlsafe(32)
	write_data(sessions_file, f"{sessionid}|{username}")
	return sessionid

def get_session():
	sessionid = ''
	if "HTTP_COOKIE" in os.environ:
		cookies = os.environ["HTTP_COOKIE"].split("; ")
		for cookie in cookies:
			key, value = cookie.split('=', 1)
			if key == "session":
				sessionid = value
				break
	if not sessionid:
		return ('', '')
	entries = get_entry(sessions_file, sessionid).split('|', 1)
	if len(entries) == 2:
		return (entries[0], entries[1].strip())
	return ('','')

def delete_session():
	sessionid = ''
	if "HTTP_COOKIE" in os.environ:
		cookies = os.environ["HTTP_COOKIE"].split("; ")
		for cookie in cookies:
			key, value = cookie.split('=', 1)
			if key == "session":
				sessionid = value
				break
	if sessionid:
		delete_entry(sessions_file, sessionid)

def get_user_email(username):
	entry = get_entry(users_file, username)
	if not entry:
		return str(None)
	fields = entry.split('|')
	if len(fields) < 3:
		return str(None)
	return fields[2].strip()

def handle_request(form):
	action = form.get("action")

	if action == "register":
		html = ""
		username = form.get("username")
		password = form.get("password")
		email = form.get("email")
		exist = get_entry(users_file, username)
		if not exist:
			write_data(users_file, f"{username}|{password}|{email}")
			html = "<html><body><h1>Registration Successful</h1>"
			html += f"<p>User {username} created</p>"
			html += "<a href='/../login.html'>Login</a></body></html>"
			print("Status: 201 Created")
		else:
			html = f"<html><body><h2>Username {username} already exists</h2>"
			html += "<p><a href='/../register.html'>Try again</a></p>"
			html += "<p><a href='/../'>Go home</a></p>"
		print("Content-Length: " + str(len(html)))
		print("Content-Type: text/html\n")
		print(html)

	elif action == "login":
		username = form.get("username")
		password = form.get("password")
		if not valid_credentials(username, password):
			html = """<html><body>
				<h3>Wrong username or password</h3>
				<p><a href='/../login.html'>Try again</a></p>
				<p><a href='/../index.html'>Go Home</a></p>
				</body></html>"""
			print("Content-Length: " + str(len(html)))
			print("Content-Type: text/html\n")
			print(html)
			return
		html = f"<html><body><h3>Welcome {username}</h3>"
		html += "<p><a href='/../dashboard'>Dashboard</a></p></body></html>"
		sid = make_session(username)
		print(f"Set-Cookie: session={sid}; Path=/")
		print("Content-Length: " + str(len(html)))
		print("Content-Type: text/html\n")
		print(html)

	elif action == "create_post":
		post_id = form.get("post_id")
		title = form.get("title")
		content = form.get("content")
		write_data(data_file, f"post|{post_id}|{title}|{content}")
		html = "<html><body><h1>Post Created</h1><a href='/'>Go Back</a></body></html>"
		print("Status: 201 Created")
		print("Content-Length: " + str(len(html)))
		print("Content-type: text/html\n")
		print(html)

	# if action == "get_post":
	# 	posts = read_data(data_file)
	# 	post_id = form.get("post_id")
	# 	title = form.get("title")
	# 	content = form.get("content")
	# 	write_data(data_file, f"post|{post_id}|{title}|{content}")
	# 	html = "<html><body><h1>Post Created</h1><a href='/'>Go Back</a></body></html>"
	# 	print("Status: 201 Created")
	# 	print("Content-Length: " + str(len(html)))
	# 	print("Content-type: text/html\n")
	# 	print(html)

	elif action == "add_comment":
		post_id = form.get("post_id")
		commenter = form.get("commenter")
		comment = form.get("comment")
		write_data(data_file, f"comment|{post_id}|{commenter}|{comment}")
		html = "<html><body><h1>Comment Added</h1><a href='/'>Go Back</a></body></html>"
		print("Status: 201 Created")
		print("Content-Length: " + str(len(html)))
		print("Content-type: text/html\n")
		print(html)

	else:
		print("Content-type: text/html\n")
		print("<html><body><h1>Unknown Action</h1></body></html>")

def personal_page():
	session, user = get_session()
	if not user:
		html = "<p>Unathorized user</p>"
		html += "<p>Please <a href='/../login.html'>login</a></p>"
		return html
	email = unquote(get_user_email(user))
	html = f"<h2>Welcome {user}</h2><p>Your Profile:</p>"
	html += f"<ul><li>Username: {user}</li><li>Email: {email}</li></ul>"
	return html

database_path = os.getcwd() + "/database"
data_file = os.path.join(database_path, "data.txt")
users_file = os.path.join(database_path, "users.txt")
sessions_file = os.path.join(database_path, "sessions.txt")

ensure_database()
method = os.environ["REQUEST_METHOD"]
if (method == "POST"):
	form = read_input()
	handle_request(form)
elif (method == "GET"):
	action = ''
	if "QUERY_STRING" in os.environ:
		queries = os.environ["QUERY_STRING"].split("&")
		for query in queries:
			if query.startswith('action'):
				action = query.split('=')[1]
		if action == "logout":
			delete_session()
			html = f"<html><body><h3>Logged out</h3>"
			html += "<p><a href='/'>Return Home</a></p></body></html>"
			print(f"Set-Cookie: session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT")
			print("Content-Length: " + str(len(html)))
			print("Content-Type: text/html\n")
			print(html)
	if not action:
		html = get_html("Dashboard", personal_page())
		print("Content-Length: " + str(len(html)))
		print("Content-Type: text/html\n")
		print(html)
else:
	print("Status: 405")
