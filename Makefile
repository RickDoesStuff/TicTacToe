# Define the compiler
CC=gcc

# Compiler flags
CFLAGS=-Wall -g -fsanitize=address

# Debugging flags
DFLAGS=-Wall -g -fsanitize=address -D__DEBUG 

# Define the target executable names
TARGET_SERVER=server
TARGET_CLIENT=client

# List of source files
SOURCES_SERVER=ttts.c network.c
SOURCES_CLIENT=xmit.c network.c

# Object files to generate
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)

# Default rule to build the server and client
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Rule to build the SERVER
$(TARGET_SERVER): $(OBJECTS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^
	make CleanServer

# Rule to build the CLIENT
$(TARGET_CLIENT): $(OBJECTS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^
	make CleanClient

# Debug rule to build the server with debug flags
DebugServer: CFLAGS += $(DFLAGS)
DebugServer: $(TARGET_SERVER)
	make CleanServer

# Debug rule to build the client with debug flags
DebugClient: CFLAGS += $(DFLAGS)
DebugClient: $(TARGET_CLIENT)
	make CleanClient

# Rule for compiling source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Rule for cleaning up server
CleanServer:
	rm -f $(OBJECTS_SERVER)

# Rule for cleaning up client
CleanClient:
	rm -f $(OBJECTS_CLIENT)

# File dependencies
network.o: network.c network.h
ttts.o: ttts.c ttts.h
xmit.o: xmit.c xmit.h