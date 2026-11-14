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
outputDir = output
result = $(outputDir)/typingGame

# In case i have files literally called "all" and "clean"
.PHONY: all clean

# Default target
all: $(result)

# result requires all object files
# "@" before a command just makes sure the command itself isnt outputted as a string
$(result): $(objects) | output
	@echo "Compiling with flags $(CFLAGS)"
	@$(CC) $(CFLAGS) $(objects) -o $(result)
	@echo "Linked object files and compiled into $(result)"

# Compile source files into the required object files
# $@ is the target (.o), $< is its first prerequisite (.c)
%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo "Compiled $<"

# Create output dir if it doesnt exist
output:
	@mkdir -p output
	@echo "Created output directory"

clean:
	@rm -rf $(objects)
	@echo "Deleted all object files"

	@rm -rf $(dependencies)
	@echo "Deleted all dependency files"

	@rm -rf $(outputDir)
	@echo "Deleted output dir"
