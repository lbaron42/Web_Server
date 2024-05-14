NAME := webserv
SRCS := main Server

SRCDIR := src
INCDIR := include
BINDIR := bin

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -O2 -I$(INCDIR)
debug: CXXFLAGS += -Og -ggdb3

deploy: CXX := g++
deploy: LDFLAGS += -static -static-libstdc++

MKDIR := mkdir

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
