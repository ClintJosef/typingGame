CC = cc
# last 2 flags make sure to recompile the source if its header was modified
CFLAGS = -Wall -MMD -MP

# Wildcard expansion
sources = main.c $(wildcard lib/*.c)
# Replaces .c suffix with .o (object files)
objects = $(sources:.c=.o)
# same thing but for dependency files
dependencies = $(objects:.o=.d)
# Compiled game location
result = output/typingGame

# In case i have files literally called "all" and "clean"
.PHONY: all clean

# Default target
all: $(result)

# result requires all object files
$(result): $(objects) | output
	# Link object files and compile into $(result)
	$(CC) $(CFLAGS) $(objects) -o $(result)

# Compile source files into the required object files
# $@ is the target (.o), $< is its first prerequisite (.c)
%.o: %.c
	# Compile $<
	$(CC) -c $(CFLAGS) $< -o $@

# Create output dir if it doesnt exist
output:
	# Create output directory
	mkdir -p output

clean:
	# Delete all object files
	rm -rf $(objects)
	# Delete all dependency files
	rm -rf $(dependencies)
	# Delete output
	rm -rf $(result)
