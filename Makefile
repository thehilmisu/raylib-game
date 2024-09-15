# Makefile for compiling all C source files in the directory

# Determine the operating system
ifeq ($(OS),Windows_NT)
    # Windows settings
    CFLAGS = -I. -Wall -std=c99
    LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm
    EXECUTABLE = game.exe
    RM = del /Q
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        # Linux settings
        CFLAGS = -I. -Wall -std=c99
        LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
        EXECUTABLE = game
        RM = rm -f
    endif
    ifeq ($(UNAME_S),Darwin)
        # macOS settings
        CFLAGS = -I. -Wall -std=c99
        LDFLAGS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit
        EXECUTABLE = game
        RM = rm -f
    endif
endif

# Source and object files
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

# Default target
all: $(EXECUTABLE)

# Link the executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files into object files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)