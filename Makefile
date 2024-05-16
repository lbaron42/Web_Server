# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lbaron <lbaron@student.42berlin.de>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/14 11:22:55 by mcutura           #+#    #+#              #
#    Updated: 2024/05/16 16:01:48 by lbaron           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := webserv
SRCS := main Server Config Utils

SRCDIR := src
INCDIR := include
BINDIR := bin

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -O2 -I$(INCDIR)
debug: CXXFLAGS += -Og -ggdb3
deploy: LDFLAGS += -static -static-libstdc++
MKDIR := mkdir -p

.PHONY: all clean fclean re debug deploy

all: $(NAME)

$(NAME): $(SRCS:%=$(BINDIR)/%.o)
	$(CXX) $(SRCS:%=$(BINDIR)/%.o) -o $(NAME) $(LDFLAGS)
$(BINDIR)/%.o: $(SRCDIR)/%.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -c $^ -o $@
$(BINDIR):
	$(MKDIR) $(BINDIR)

clean:
	$(RM) $(SRCS:%=$(BINDIR)/%.o)
fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(BINDIR)
re: fclean all
debug: all
deploy: all
