CXX = gcc
EMCC = emcc

# Path to Raylib (change this to your local Raylib directory)
RAYLIB_PATH = /usr/local

# Raylib source and include paths
RAYLIB_INCLUDE = $(RAYLIB_PATH)/include
RAYLIB_LIB = $(RAYLIB_PATH)/lib

# Raylib library name
RAYLIB_LIB_NAME = raylib

# Flags for compiling
CXXFLAGS = -I$(RAYLIB_INCLUDE) -O2
LDFLAGS = -L$(RAYLIB_LIB) -l$(RAYLIB_LIB_NAME)

# Source files
SRC = memorex.c
OBJ = $(SRC:.cpp=.o)

# Output files
DESKTOP_OUTPUT = memorex
WEB_OUTPUT = memorex.html

# Web-specific flags for Emscripten
EMCC_FLAGS = -s USE_GLFW=3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s FULL_ES3=1

# Default target is to build for desktop
all: desktop

# Compile for desktop
desktop: $(OBJ)
	$(CXX) $(OBJ) -o $(DESKTOP_OUTPUT) $(LDFLAGS)

# Compile for web
web: $(OBJ)
	$(EMCC) $(OBJ) -o $(WEB_OUTPUT) $(EMCC_FLAGS) $(LDFLAGS)

# Compile the source files into object files
%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean the generated files
clean:
	rm -f $(OBJ) $(DESKTOP_OUTPUT) $(WEB_OUTPUT)
