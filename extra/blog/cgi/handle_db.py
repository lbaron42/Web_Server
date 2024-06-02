#!/usr/bin/python3

import os
import cgi
import cgitb
import html

cgitb.enable()

database_path = "/home/mcutura/42cursus/Web_Server/extra/blog/database"
data_file = os.path.join(database_path, "data.txt")
users_file = os.path.join(database_path, "users.txt")

def ensure_database():
	if not os.path.exists(database_path):
		os.makedirs(database_path, exist_ok=True)
	if not os.path.exists(data_file):
		with open(data_file, "w") as f:
			pass
	if not os.path.exists(users_file):
		with open(users_file, "w") as f:
			pass

ensure_database()

def read_data(file_path):
	data = []
	with open(file_path, "r") as f:
		for line in f:
			data.append(line.strip())
	return data

def write_data(file_path, line):
	with open(file_path, "a") as f:
		f.write(line + "\n")

def handle_request():
	form = cgi.FieldStorage()
	action = form.getvalue("action")
		
	if action == "create_post":
		post_id = form.getvalue("post_id")
		title = form.getvalue("title")
		content = form.getvalue("content")
		write_data(data_file, f"post|{post_id}|{title}|{content}")
		print("Content-type: text/html\n")
		print("<html><body><h1>Post Created</h1><a href='/'>Go Back</a></body></html>")
		
	elif action == "add_comment":
		post_id = form.getvalue("post_id")
		commenter = form.getvalue("commenter")
		comment = form.getvalue("comment")
		write_data(data_file, f"comment|{post_id}|{commenter}|{comment}")
		print("Content-type: text/html\n")
		print("<html><body><h1>Comment Added</h1><a href='/'>Go Back</a></body></html>")

	elif action == "register":
		username = form.getvalue("username")
		password = form.getvalue("password")
		write_data(users_file, f"{username}|{password}")
		print("Content-type: text/html\n")
		print("<html><body><h1>Registration Successful</h1><a href='/'>Go Back</a></body></html>")

	else:
		print("Content-type: text/html\n")
		print("<html><body><h1>Unknown Action</h1></body></html>")

handle_request()
