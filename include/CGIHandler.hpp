/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plandolf <plandolf@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 11:58:31 by plandolf          #+#    #+#             */
/*   Updated: 2024/05/29 18:25:13 by plandolf         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/epoll.h>
#include <map>
#include <string>
#include <fstream>

class CGIHandler{
	
	private:
		std::string scriptPath;
		std::map<std::string, std::string> environment;
		std::string requestBody;

		char** createEnvironmentArray();
		void destroyEnvironmentArray(char** envp);
		void setupEnvironmentVariables(char* envp[]);
  		std::string determineScriptType() const;

	public:
		CGIHandler();
		CGIHandler(const CGIHandler &src);
		CGIHandler &operator=(const CGIHandler &src);
		~CGIHandler();

		void setScriptPath(const std::string &path);
		void setEnvironment(const std::map<std::string, std::string>& env);
    	void setRequestBody(const std::string& body);

		bool execute(std::string &output);
};

#endif
