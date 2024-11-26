CXX = g++
SRCS = main.cpp src/character.cpp src/gl_util.cpp src/glad.c
OBJS = $(SRCS:.cpp=.o)
TARGET = character

INCLUDE_DIRS = -I. -Iinclude -I/opt/homebrew/include
LIBRARY_DIRS = -L/opt/homebrew/lib
LIBS = -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

CXXFLAGS = -std=c++17 $(INCLUDE_DIRS)
LDFLAGS = $(LIBRARY_DIRS) $(LIBS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/glad.o: src/glad.c
	$(CC) -Iinclude -c src/glad.c -o src/glad.o

clean:
	rm -f $(wildcard ./*.o src/*.o)
