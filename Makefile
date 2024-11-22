CXX = g++
SRCS = render.cpp glad/src/glad.c
TARGET = character

INCLUDE_DIRS = -I. -Iglad/include -I/opt/homebrew/include
LIBRARY_DIRS = -L/opt/homebrew/lib
LIBS = -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

CXXFLAGS = -std=c++11 $(INCLUDE_DIRS)
LDFLAGS = $(LIBRARY_DIRS) $(LIBS)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
