#!/usr/bin/python3

import os
import sys

def ensure_database():
	if not os.path.exists(database_path):
		os.makedirs(database_path, exist_ok=True)
	if not os.path.exists(data_file):
		with open(data_file, "w") as f:
			pass
	if not os.path.exists(users_file):
		with open(users_file, "w") as f:
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

def handle_request(form):
	action = form.get("action")

	if action == "register":
		username = form.get("username")
		password = form.get("password")
		write_data(users_file, f"{username}|{password}")
		html = "<html><body><h1>Registration Successful</h1><p>User " + username + " created</p><a href='/'>Go Back</a></body></html>"
		print("Status: 201 Created")
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

database_path = os.getcwd() + "/extra/blog/database"
data_file = os.path.join(database_path, "data.txt")
users_file = os.path.join(database_path, "users.txt")

ensure_database()
form = read_input()
handle_request(form)
