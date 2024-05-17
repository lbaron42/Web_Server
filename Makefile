# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/14 11:22:55 by mcutura           #+#    #+#              #
#    Updated: 2024/05/17 03:12:23 by mcutura          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := webserv
MAIN := main
SRCS := Log Server

SRCDIR := src
INCDIR := include
BINDIR := bin

TESTSDIR := tests
UNITTESTDIR := $(TESTSDIR)/unit_tests
TESTS := $(addprefix $(UNITTESTDIR)/test_, $(SRCS))
TESTLOG := $(UNITTESTDIR)/tests.log

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -O2
CPPFLAGS := -I$(INCDIR)
debug: CXXFLAGS += -Og -ggdb3
deploy: LDFLAGS += -static -static-libstdc++
nitpicking: CPPFLAGS += -DSTRICT_EVALUATOR=1
MKDIR := mkdir -p

COLOUR_GREEN := \033[0;32m
COLOUR_RED := \033[0;31m
COLOUR_END := \033[0m

.PHONY: all clean fclean re check debug deploy nitpicking

all: $(NAME)

$(NAME): $(SRCS:%=$(BINDIR)/%.o) $(MAIN:%=$(BINDIR)/%.o)
	$(CXX) $(SRCS:%=$(BINDIR)/%.o) $(MAIN:%=$(BINDIR)/%.o) -o $(NAME) $(LDFLAGS)
$(BINDIR)/%.o: $(SRCDIR)/%.cpp | $(BINDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^ -o $@
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
	@echo "$(COLOUR_GREEN)All tests passed successfully$(COLOUR_END)"

%.test: %.out
	@(./$(*:%=%.out) >$(TESTLOG) 2>&1 \
	&& echo "$(COLOUR_GREEN)[OK]$(COLOUR_END) $(*F)") \
	|| (echo "$(COLOUR_RED)[KO]$(COLOUR_END) $(*F) failed" && exit 1)

$(UNITTESTDIR)/test_%.out: $(UNITTESTDIR)/test_%.cpp
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $(@:%.out=%.o)
	@$(CXX) $(@:%.out=%.o) $(SRCS:%=$(BINDIR)/%.o) -o $@

debug: all
deploy: all
nitpicking: all
