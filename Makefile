# Compiler
CC = gcc

# Directories
SRC_DIR = src
OBJ_DIR = obj
SOCKET_DIR = $(SRC_DIR)/socket
MIME_DIR = $(SRC_DIR)/mime
FILE_DIR = $(SRC_DIR)/file

# Header files
DEPS = $(SOCKET_DIR)/socket.h

# Target executable
TARGET = server

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)                
SRC_SOCKET = $(wildcard $(SOCKET_DIR)/*.c)     
SRC_MIME = $(wildcard $(MIME_DIR)/*.c)     
SRC_FILE = $(wildcard $(FILE_DIR)/*.c)     

# Object files
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))         
OBJ_SOCKET = $(patsubst $(SOCKET_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_SOCKET)) 
OBJ_MIME = $(patsubst $(MIME_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_MIME)) 
OBJ_FILE = $(patsubst $(FILE_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILE)) 

# Combine all object files
MAIN_OBJ = $(OBJ) $(OBJ_SOCKET) $(OBJ_MIME) $(OBJ_FILE)

# Compiler flags
CFLAGS = -I$(SOCKET_DIR) -I$(MIME_DIR) -I$(FILE_DIR) -DDEBUG

# Build rules
all: $(TARGET)

$(TARGET): $(MAIN_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SOCKET_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(MIME_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(FILE_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all clean

