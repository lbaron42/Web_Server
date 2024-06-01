/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 17:57:12 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/01 19:33:00 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler(){}

CGIHandler::CGIHandler(const CGIHandler &src){
	*this = src;
}

CGIHandler &CGIHandler::operator=(const CGIHandler &src){
	if (this == &src)
		return *this;
	this->scriptPath = src.scriptPath;
	this->environment = src.environment;
	this->requestBody = src.requestBody;
	return *this;
}

CGIHandler::~CGIHandler(){}

void CGIHandler::setScriptPath(const std::string& path) {
	scriptPath = path;
}

void CGIHandler::setEnvironment(const std::map<std::string, std::string>& env) {
	environment = env;
}

void CGIHandler::setRequestBody(const std::string& body) {
	requestBody = body;
}

char** CGIHandler::createEnvironmentArray() {
	char** envp = new char*[environment.size() + 1];
	int i = 0;
	std::map<std::string, std::string>::const_iterator it;
	for (it = environment.begin(); it != environment.end(); ++it) {
		std::string envVar = it->first + "=" + it->second;
		envp[i] = new char[envVar.size() + 1];
		std::strcpy(envp[i], envVar.c_str());
		++i;
	}
	envp[i] = 0;
	return envp;
}

void CGIHandler::destroyEnvironmentArray(char** envp) {
	for (int i = 0; envp[i] != 0; ++i) {
		delete[] envp[i];
	}
	delete[] envp;
}

std::string CGIHandler::determineScriptType() const {
	if (scriptPath.size() > 3 && scriptPath.substr(scriptPath.size() - 3) == ".py") {
		return "python";
	} else if (scriptPath.size() > 4 && scriptPath.substr(scriptPath.size() - 4) == ".cgi") {
		return "cgi";
	} else {
		return "unknown";
	}
}


bool CGIHandler::execute(std::string &output) {
	
	int inputPipe[2];
	int outputPipe[2];

	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
		throw std::runtime_error("Failed to create pipes");
	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("Failed to fork");
	if (pid == 0) {
		close(inputPipe[1]);
		close(outputPipe[0]);
		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);
		char** envp = createEnvironmentArray();
		
		execl(scriptPath.c_str(), scriptPath.c_str(), (char*)NULL, envp);
		destroyEnvironmentArray(envp);
		close(inputPipe[0]);
		close(outputPipe[1]);
		_exit(127);
	} else {
		close(inputPipe[0]);
		close(outputPipe[1]);
		if (!requestBody.empty()) {
			ssize_t bytesWritten = write(inputPipe[1], requestBody.c_str(), requestBody.size());
			if (bytesWritten == -1) {
				close(inputPipe[1]);
				throw std::runtime_error("Failed to write to pipe");
			}
		}
		close(inputPipe[1]);
		int epoll_fd = epoll_create1(0);
		if (epoll_fd == -1)
			throw std::runtime_error("Failed to create epoll file descriptor");
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = outputPipe[0];
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, outputPipe[0], &event) == -1)
			throw std::runtime_error("Failed to add file descriptor to epoll");
		std::vector<char> buffer(4096);
		std::ostringstream oss;
		bool done = false;
		while (!done) {
			struct epoll_event events[10];
			int n = epoll_wait(epoll_fd, events, 10, -1);
			for (int i = 0; i < n; ++i) {
				if (events[i].data.fd == outputPipe[0]) {
					ssize_t count = read(outputPipe[0], buffer.data(), buffer.size());
					if (count == -1) {
						throw std::runtime_error("Failed to read from pipe");
					} else if (count == 0) {
						done = true;
					} else {
						oss.write(buffer.data(), count);
					}
				}
			}
		}
		close(outputPipe[0]);
		close(epoll_fd);
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			output = oss.str();
			return true;
		} else {
			return false;
		}
	}
}
/* INTERFACE DESIGN SKETCH PROPOSAL */
// #include "Log.hpp"

// Class CGIHandler

// public:
// CGIHandler(Log &log);
// void setenv(std::map<std::string, std::string> envmap);
// bool execute(std::string const &command, int &pipes[2]); // status of exit

// void kill(int signal); // maybe will be done outside the class

// ssize_t write_to_pipe(std::vector<char> &buff);
// bool read_from_pipe(std::vector<char> &buff);

// /*
// 	When is child process done? how we get notified?
// */

// private:
// 	Log &log;
// 	int status;
// 	int input;
// 	int output;
// 	map <> env;
	
	
	
// CGIHandler(Log &log) : log(log), status(0) {}

// pipes.input
// pipes.output

// int pfd[2][2];
// pipe(pfd[0]);
// pipe(pfd[1]);
// fork()

// pipes[0] = pfd[1][0];
// pipes[1] = pfd[0][1];

// close(pipes[0][0]), close [1][1]
