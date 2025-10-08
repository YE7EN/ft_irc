NAME	:= ircserv
SRCS	:= main.cpp 
OBJS	:= $(SRCS:%.cpp=obj/%.o)
CXX		:= c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

obj/%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

debug:
	clear && make re && valgrind  --leak-check=full ./$(NAME) input.txt && make fclean

.PHONY: all clean fclean re debug
