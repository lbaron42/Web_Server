# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/14 11:22:55 by mcutura           #+#    #+#              #
#    Updated: 2024/05/17 17:10:07 by mcutura          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := webserv
MAIN := main
SRCS := Log Server Request Reply

SRCDIR := src
INCDIR := include
BINDIR := build

TESTSDIR := tests
UNITTESTDIR := $(TESTSDIR)/unit_tests
TESTS := $(addprefix $(UNITTESTDIR)/test_, $(SRCS))
TESTCOUTLOG := $(UNITTESTDIR)/test_cout.log
TESTCERRLOG := $(UNITTESTDIR)/test_cerr.log

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -Wpedantic
CXXFLAGS += -march=native -O3
CPPFLAGS := -I$(INCDIR)
debug: CXXFLAGS += -Og -ggdb3
debug: CPPFLAGS += -DDEBUG_MODE=1
static: LDFLAGS += -static -static-libstdc++
nitpicking: CPPFLAGS += -DSTRICT_EVALUATOR=1
MKDIR := mkdir -p

SHELL := /bin/bash
COLOUR_GREEN := \033[0;32m
COLOUR_RED := \033[0;31m
COLOUR_END := \033[0m

.PHONY: all clean fclean re check debug static nitpicking container

all: $(NAME)

$(NAME): $(SRCS:%=$(BINDIR)/%.o) $(MAIN:%=$(BINDIR)/%.o)
	$(CXX) $(SRCS:%=$(BINDIR)/%.o) $(MAIN:%=$(BINDIR)/%.o) -o $(NAME) $(LDFLAGS)
$(BINDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.hpp | $(BINDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BINDIR):
	$(MKDIR) $(BINDIR)

clean:
	$(RM) $(SRCS:%=$(BINDIR)/%.o)
	$(RM) $(TESTS:%=%.o)

fclean: clean
	$(RM) $(NAME)
	$(RM) -r $(BINDIR)
	$(RM) $(TESTS:%=%.out)

re: fclean all

check: $(SRCS:%=$(BINDIR)/%.o) $(TESTS:%=%.out) $(TESTS:%=%.test)
	@echo -e "$(COLOUR_GREEN)All tests passed successfully$(COLOUR_END)"

%.test: %.out
	@(./$(*:%=%.out) && echo -e "$(COLOUR_GREEN)[OK]$(COLOUR_END) $(*F)") \
	|| (echo -e "$(COLOUR_RED)[KO]$(COLOUR_END) $(*F) failed" && exit 1)

$(UNITTESTDIR)/test_%.out: $(UNITTESTDIR)/test_%.cpp
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $(@:%.out=%.o)
	@$(CXX) $(@:%.out=%.o) $(SRCS:%=$(BINDIR)/%.o) -o $@

debug: all
static: all
nitpicking: all

container:
	docker build . -t marvinx
	docker run --name c-marvinx -p 8080:8080 -it marvinx
