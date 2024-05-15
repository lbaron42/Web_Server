**Readme.txt**

<img src="./extra/MarvinX.png" alt="MarvinX" align="right" style="width:400px; margin:0 0 10px 10px;">

# MarvinX Server

Welcome to the MarvinX Server, a powerful and efficient C++ server inspired by Nginx and named after the iconic Marvin the Paranoid Android from "The Hitchhiker's Guide to the Galaxy."

[![build](https://github.com/lbaron42/Web_Server/actions/workflows/build.yml/badge.svg)](https://github.com/lbaron42/Web_Server/actions/workflows/build.yml)

## Deploy

```
docker build . -t webserv \
&& docker run --name c-webserv -p 8080:8080 -it webserv
```
Server now available at 127.0.0.1:8080

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

- C++98
- Makefile

### Steps

1. Clone the repository:..
