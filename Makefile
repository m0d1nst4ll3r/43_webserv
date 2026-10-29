# Source files
CFILES		= main.cpp \
			  CGICall.cpp \
			  ClientSession.cpp \
			  ClientSocket.cpp\
			  EpollInstance.cpp \
			  FileDescriptor.cpp \
			  HTTPRequest.cpp \
			  HTTPResponse.cpp \
			  IOChannel.cpp \
			  ListenSocket.cpp \
			  WebServer.cpp

			  
SRCDIR		= src
SRC			= $(addprefix $(SRCDIR)/, $(CFILES))

# Header directories
INCDIR		= inc

# Build directory
BUILDDIR	= .build

# Object and dependency files
OBJ			= $(SRC:%.cpp=$(BUILDDIR)/%.o)
DEP			= $(OBJ:.o=.d)

# Output binary
NAME		= webserv

# Compiler settings
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98 -g -MMD -MP $(addprefix -I,$(INCDIR))

# Make settings
MAKEFLAGS	+= --no-print-directory
MAKEFLAGS	:= $(MAKEFLAGS) $(if $(filter -j%,$(MAKEFLAGS)),,-j)

# Default rule
all:		$(NAME)

# Link
$(NAME):	$(OBJ)
			$(CC) $(CFLAGS) $^ -o $@

# Compile source -> object (auto-create directories)
$(BUILDDIR)/%.o: %.cpp
			@mkdir -p $(@D)
			$(CC) $(CFLAGS) -c -o $@ $<

# Valgrind memory test
valgrind:	$(NAME)
			valgrind --leak-check=full --show-leak-kinds=all ./$(NAME)

# Cleanup
clean:
			rm -rf $(BUILDDIR)

fclean:
			rm -rf $(NAME) $(BUILDDIR)

re:
			$(MAKE) fclean
			$(MAKE) all

# Include dependency files
-include $(DEP)

.PHONY: clean fclean re