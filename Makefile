CXX = g++
SRCS = main.cpp character.cpp glad/src/glad.c
OBJS = $(SRCS:.cpp=.o)
TARGET = character

INCLUDE_DIRS = -I. -Iglad/include -I/opt/homebrew/include
LIBRARY_DIRS = -L/opt/homebrew/lib
LIBS = -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

CXXFLAGS = -std=c++11 $(INCLUDE_DIRS)
LDFLAGS = $(LIBRARY_DIRS) $(LIBS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

glad/src/glad.o: glad/src/glad.c
	$(CC) -Iglad/include -c glad/src/glad.c -o glad/src/glad.o

clean:
	rm -f $(TARGET) $(OBJS) glad/src/glad.o
