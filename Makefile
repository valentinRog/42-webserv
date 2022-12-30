NAME = webserv

SRC = $(wildcard src/*.cpp)

INCLUDE = include

CXX = clang++

CXXFLAGS = -Wall -Wextra -I$(INCLUDE)

OBJ = $(SRC:.cpp=.o)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

all: $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

run: $(NAME)
	./$(NAME)
