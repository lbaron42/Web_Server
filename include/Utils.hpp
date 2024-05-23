/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 15:52:42 by lbaron            #+#    #+#             */
/*   Updated: 2024/05/23 18:22:28 by lbaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <map>
# include <string>
# include <sstream>
# include <vector>

# include <sys/stat.h>
# include <sys/types.h>

std::string itoa(int num);
int atoi(const std::string &str);
std::string trim(const std::string &str);
std::vector<std::string> split(const std::string &s, char delimiter);

#endif // UTILS_HPP
