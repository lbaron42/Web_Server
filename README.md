**Readme.txt**

<img src="./extra/MarvinX.png" alt="MarvinX" align="right" style="width:400px; margin:0 0 10px 10px;">

# MarvinX Server

Welcome to the MarvinX Server, a powerful and efficient C++ server inspired by Nginx and named after the iconic Marvin the Paranoid Android from "The Hitchhiker's Guide to the Galaxy."

[![build](https://github.com/lbaron42/Web_Server/actions/workflows/build.yml/badge.svg)](https://github.com/lbaron42/Web_Server/actions/workflows/build.yml)

## Deploy

```
make container
```
Servers now available at localhost:8080 and localhost:8081 with directory
`./extra` mounted on `/var/www/html`  

To expose different ports:  
`PORT_MAPPING=' -p "8080:8080" -p "8081:8081"'`  

To specify bind mount:  
`MOUNT_VOLUME=' -v extra/www:/usr/share/www:ro' `

Followed by `make container` (same command line)

## Table of Contents

1. Introduction
2. Features
3. Installation
4. Configuration
5. Usage
6. Contributing
7. License

## Introduction

MarvinX is a high-performance, lightweight server designed to handle concurrent connections efficiently. Drawing inspiration from the character Marvin and the reliable performance of Nginx, MarvinX aims to provide a robust solution for your server needs.

## Features

- High concurrency handling
- Efficient resource usage
- Easy configuration
- Modular architecture
- Robust error handling
- Customizable modules

## Installation

### Prerequisites

- Makefile
- Linux kernel 2.6.27+ OR Docker

### Steps

1. Clone the repository
2. Run `make`

### Usage

1. add [.conf file](https://[link-url-here.org](https://github.com/lbaron42/Web_Server/wiki/Configuration.md))
2. run ./webser path_to_conf_file/.conf
