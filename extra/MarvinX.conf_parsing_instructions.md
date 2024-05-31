## MarvinX grammar
```
server {
	[TAB][KEY][SP][VALUE][;] // variables

	location /path {
		[TAB][TAB][KEY][SP][VALUE][;] // location variables
	}
}
```

## Valid variables
- listen
- server_name
- error_page
- index
- root
- client_max_body_size
- autoindex
- allow_methods
- alias
- cgi_path
- cgi_ext
- return

### 1. LISTEN
- Can be written multiple times per _server block_
- Specifies the IP address and port on which the server should listen for incoming connections.
- The format is listen [address]:port.
- is stored int the: 
``` 
struct Address 
{
	std::string				ip;
	std::string				port;
};

std::vector<Address> addresses;	
```

- _OUR PROTECTION_:
- _DEFAULT VALUE_

### 2. SERVER_NAME
- Can be written only once per _server block_
- A list of server names(strings) [example.com] [www.example.com] ...
- is stored in the `std::vector<std::string> hostnames;`
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_

### 3. ERROR_PAGE
- Can be written multiple times per _server block_
- First value is [INT], second is the [PATH] to the html page
- is stored in the `std::vector<std::pair<int, std::string> > error_pages;`
- _OUR PROTECTION:_
	1. check if [INT] is valid error number
- _DEFAULT VALUE_

### 4. INDEX
- Can be written only once per _server block_
- A list of paths to html pages
- Shows the first existing html page
- is stored in the `std::vector<std::string> serv_index;` for servers.
- is stored in the `std::vector<std::string> loc_index;` for locations.
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_

### 5. ROOT
- Can be written only once per _server block_
- A string to the root of the server
- is stored in the `std::string`
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_
	1. ?

### 6. CLIENT_MAX_BODY_SIZE
- Can be written only once per _server block_
- A number that indecates the max size of the HTTP request body
- is stored in the `size_t client_max_body_size;`
- _OUR PROTECTION:_
	1. check if number is valid
	2. check if number is within the boundaries(MIN, MAX)
- _DEFAULT VALUE_
	1. default to 1 megabit

### 7. AUTOINDEX
- Can be written only once per _server block_
- `on` or `off` for directory listing (If no index file is found, display a list of files) 
- is stored in the `bool`
- _OUR PROTECTION:_
	1. check if string valid
- _DEFAULT VALUE_
	1. default to 0 or false

### 8. ALLOW_METHODS
- Can be written only once per _server block_
- A list of methods(GET, POST, ...)
- is stored in the `std::vector<std::string> allow_methods;`
- _OUR PROTECTION:_
	1. check if method exists
- _DEFAULT VALUE_

### 9. ALIAS
- Can be written only once per _server block_
- A string that maps URL to specific directory
- _alias_ vs _root_. With alias, URL is replaced with the specified directory. With root, the specified directory is appended to the URL's path.
- Shows the first existing html page
- is stored in the `Location->std::string alias;`
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_

### 10. CGI_PATH
- Can be written only once per _server block_
- A string to intepreter 
- is stored in the `std::string ?`
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_

### 11. CGI_EXT
- Can be written only once per _server block_
- A list of extensions that web server accepts(.py, .php, ...) 
- is stored in the `std::vector<std::string> ?`
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_

### 11. RETURN
- Can be written only once per _server block_
- Redirects user to a certain URL 
- is stored in the `std::string> ?`
- _OUR PROTECTION:_
	1. no protection
- _DEFAULT VALUE_

## OPTIONAL?

### UPLOAD_PATH