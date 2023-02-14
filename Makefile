NAME = webserv

SRC = $(wildcard src/*.cpp)

INCLUDE = include

CXX = clang++

CXXFLAGS = -Wall -Wextra -std=c++98 -I$(INCLUDE) -fsanitize=address

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
	./$(NAME) test/test.json
