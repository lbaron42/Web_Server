/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_CGIHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 17:56:05 by plandolf          #+#    #+#             */
/*   Updated: 2024/06/02 19:50:41 by mcutura          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <map>
#include <pthread.h>
#include "CGIHandler.hpp"


// struct ThreadParams {
//     CGIHandler* handler;
//     std::string* output;
// };


// void* executeCGI(void* arg) {
//     ThreadParams* params = static_cast<ThreadParams*>(arg);
//     if (params->handler->execute(*(params->output))) {
//         std::cout << "Script Output: " << *(params->output) << std::endl;
//     } else {
//         std::cerr << "Failed to execute script." << std::endl;
//     }
//     return NULL;
// }

// void runCGI(const std::string& scriptPath) {
//     CGIHandler handler;
//     handler.setScriptPath(scriptPath);
//     std::map<std::string, std::string> env;
//     env["REQUEST_METHOD"] = "GET";
//     handler.setEnvironment(env);
//     handler.setRequestBody("");

//     std::string output;
//     if (handler.execute(output)) {
//         std::cout << "Script executed successfully:\n" << output << std::endl;
//     } else {
//         std::cerr << "Script execution failed" << std::endl;
//     }
// }

int main() {
	// std::string testScriptPath = "../../cgi-bin/test_parallel.py";
	// std::string testScriptPath2 = "../../cgi-bin/test.cgi";
	// std::map<std::string, std::string> env;
	// env["REQUEST_METHOD"] = "GET";
	// env["QUERY_STRING"] = "";
	// env["CONTENT_LENGTH"] = "0";
	// env["CONTENT_TYPE"] = "text/plain";

	// CGIHandler cgiHandler1, cgiHandler2;
	// cgiHandler1.setScriptPath(testScriptPath);
	// cgiHandler2.setScriptPath(testScriptPath2);
	// cgiHandler1.setEnvironment(env);
	// cgiHandler2.setEnvironment(env);
	// cgiHandler1.setRequestBody("");
	// cgiHandler2.setRequestBody("");

	// std::string output1, output2;
	// std::cout << "Executing parallel CGI scripts..." << std::endl;

	// pthread_t thread1, thread2;
	// ThreadParams params1 = {&cgiHandler1, &output1};
	// ThreadParams params2 = {&cgiHandler2, &output2};

	// // Create threads
	// if (pthread_create(&thread1, NULL, executeCGI, &params1) != 0) {
	// 	std::cerr << "Failed to create thread 1" << std::endl;
	// 	return 1;
	// }
	// if (pthread_create(&thread2, NULL, executeCGI, &params2) != 0) {
	// 	std::cerr << "Failed to create thread 2" << std::endl;
	// 	return 1;
	// }

	// pthread_join(thread1, NULL);
	// pthread_join(thread2, NULL);

	return 0;
}
