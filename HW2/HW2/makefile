# Makefile for gtuStudentGrades

# Compiler
CC = gcc

# Compiler flags
CFLAGS =   -std=gnu99

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = main

# Default target
all: $(TARGET)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Rule to run the program with an argument
run: $(TARGET)
	./$(TARGET) 

# Rule to clean object files and executable
clean:
	rm -f $(OBJS) $(TARGET) 
	find -type p -exec rm -f {} \;


del_txt:
	rm -f *.txt