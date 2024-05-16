/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 15:52:42 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/16 16:00:42 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <vector>

std::string itoa(int num);

int atoi(const std::string &str);

std::string trim(const std::string &str);

std::vector<std::string> split(const std::string &s, char delimiter);

#endif // UTILS_HPP
