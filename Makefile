# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mcutura <mcutura@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/05/14 11:22:55 by mcutura           #+#    #+#              #
#    Updated: 2024/06/16 15:45:53 by mcutura          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := webserv
MAIN := main
SRCS := Log Config Cluster Headers Reply Request Server Utils CGIHandler
SRCS += Cookie ChunkNorris

SRCDIR := src
INCDIR := include
BINDIR := build

TESTSDIR := tests
UNITTESTDIR := $(TESTSDIR)/unit_tests
TESTS := $(addprefix $(UNITTESTDIR)/test_, $(SRCS))
UNITTESTSLOG := $(UNITTESTDIR)/tests.log
UNITTESTSERRLOG := $(UNITTESTDIR)/tests_error.log

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -Wpedantic
CXXFLAGS += -march=native -O3
CPPFLAGS := -I$(INCDIR)
debug: CXXFLAGS += -Og -ggdb3
debug: CPPFLAGS += -DDEBUG_MODE=1
static: LDFLAGS += -static -static-libstdc++
nitpicking: CPPFLAGS += -DSTRICT_EVALUATOR=1
MKDIR := mkdir -p

CONTAINER_NAME := marvinx
PORT_MAPPING ?= -p "8080:8080"
MOUNT_VOLUME ?= -v $(shell pwd)/extra/www:/var/www/html:rw

SUBMODPAGE := extra/mc-putchar.github.io

COLOUR_END := \033[0m
COLOUR_GREEN := \033[0;32m
COLOUR_RED := \033[0;31m
COLOUR_MAG := \033[0;35m
COLOUR_MAGB := \033[1;35m
COLOUR_CYN := \033[0;36m
COLOUR_CYNB := \033[1;36m

.PHONY: all clean fclean re check debug static nitpicking container help

all: $(NAME)	# Compile all targets

$(NAME): $(SRCS:%=$(BINDIR)/%.o) $(MAIN:%=$(BINDIR)/%.o)
	$(CXX) $(SRCS:%=$(BINDIR)/%.o) $(MAIN:%=$(BINDIR)/%.o) -o $(NAME) $(LDFLAGS)
$(BINDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.hpp | $(BINDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BINDIR):
	$(MKDIR) $(BINDIR)

clean:			# Clean binary object files
	$(RM) $(SRCS:%=$(BINDIR)/%.o)
	$(RM) $(TESTS:%=%.o) $(UNITTESTSLOG) $(UNITTESTSERRLOG)

fclean: clean	# Clean all compiled binaries
	$(RM) $(NAME)
	$(RM) -r $(BINDIR)
	$(RM) $(TESTS:%=%.out)

re: fclean all	# Recompile all targets

check: debug $(TESTS:%=%.out) $(TESTS:%=%.test)	# Run tests
	@echo "$(COLOUR_GREEN)All tests passed successfully$(COLOUR_END)"
%.test: %.out
	@echo "[TESTING]: $(*F)" >> $(UNITTESTSLOG)
	@(timeout --preserve-status --signal=INT 4.2s ./$(*:%=%.out) \
	2>$(UNITTESTSERRLOG) && echo "$(COLOUR_GREEN)[OK]$(COLOUR_END) $(*F)") \
	|| (echo "$(COLOUR_RED)[KO]$(COLOUR_END) $(*F) failed" \
	&& cat $(UNITTESTSLOG) && exit 1)
$(UNITTESTDIR)/test_%.out: $(UNITTESTDIR)/test_%.cpp
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $(@:%.out=%.o)
	@$(CXX) $(@:%.out=%.o) $(SRCS:%=$(BINDIR)/%.o) -o $@

debug: re		# Build for debugging
static: all		# Compile statically linked executable
nitpicking: re	# Insist on blindly following subject.pdf to the letter... silly

container:		# Build and run a Docker container running target executable
	docker build . -t marvinx --progress=plain
	docker run --rm --name $(CONTAINER_NAME) \
		$(MOUNT_VOLUME) $(PORT_MAPPING) -it marvinx

submodule:		# Pull git submodule webpage manually for testing
	rm -rf $(SUBMODPAGE)
	git clone git@github.com:mc-putchar/mc-putchar.github.io.git $(SUBMODPAGE)

help:			# Print help on Makefile
	@awk 'BEGIN { \
	FS = ":.*#"; printf "Usage:\n\t$(COLOUR_CYNB)make $(COLOUR_MAGB)<target> \
	$(COLOUR_END)\n\nTargets:\n"; } \
	/^[a-zA-Z_0-9-]+:.*?#/ { \
	printf "$(COLOUR_MAGB)%-16s$(COLOUR_CYN)%s$(COLOUR_END)\n", $$1, $$2 } ' \
	Makefile
