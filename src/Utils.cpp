/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 15:59:51 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/23 18:21:57 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils.hpp"
# include <string>

std::string itoa(int num) {
	std::stringstream ss;
	ss << num;
	return ss.str();
}

int atoi(const std::string &str) {
	std::istringstream ss(str);
	int num;
	ss >> num;
	return num;
}

std::string trim(const std::string &str) {
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) {
		return "";
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> split(const std::string &s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(trim(token));
	}
	return tokens;
}
