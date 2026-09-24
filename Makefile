CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -I./mock_sdk -c
SRCS = c4_fx_mod.cpp shake_hook.cpp explosion_hook.cpp
OBJS = $(SRCS:.cpp=.o)
all: $(OBJS)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
clean:
rm -f $(OBJS)
