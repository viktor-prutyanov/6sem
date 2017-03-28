NAME := factorial

#compiler
OMPI_CC := mpicc

# directories
INC_DIRS := ./
BIN_DIR := ./bin

# flags
OMPI_CFLAGS := -O2 -g -Wall
OMPI_LDFLAGS := -lm

INCLUDE := $(addprefix -I, $(INC_DIRS))

C_FILES := $(wildcard *.c)
OBJ_C_FILES := ${C_FILES:%.c=%.o}

$(NAME): $(OBJ_C_FILES)
	mkdir -p $(BIN_DIR)
	$(OMPI_CC) $^ $(OMPI_LDFLAGS) -o $(BIN_DIR)/$@

%.o: %.c
	$(OMPI_CC) $^ $(OMPI_CFLAGS) $(INCLUDE) -c -o $@

.PHONY: all clean 

clean:
	rm -rf *.o $(BIN_DIR)
