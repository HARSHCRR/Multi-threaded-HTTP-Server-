CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
SRCDIR   = src
SRCS     = $(wildcard $(SRCDIR)/*.cpp)
OBJS     = $(SRCS:.cpp=.o)
TARGET   = http_server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)

run: all
	./$(TARGET) 8080 4 public

.PHONY: all clean run
