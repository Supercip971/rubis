MKCWD=mkdir -p $(@D)

CC = gcc

SANITIZERS = 			\
	-fsanitize=address 	\
	-fsanitize=undefined


CFLAGS_WARNS ?= 	\
		-Werror 	\
		-Wextra 	\
		-Wall 		\
		-Wundef 	\
		-Wshadow 	\
		-Wvla

CFLAGS = 			\
		-Og 		\
		-g 		 	\
		-std=gnu2x  \
		--analyzer  \
		-Isrc/      \
		$(CFLAGS_WARNS)

LDFLAGS=$(SANITIZERS) -lglfw -lvulkan -ldl -lpthread

# some people likes to use sources/source instead of src
PROJECT_NAME = test
BUILD_DIR = build
SRC_DIR = src

CFILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*/*/*.c)
DFILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.d, $(CFILES))
OFILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CFILES))

OUTPUT = build/$(PROJECT_NAME)


$(OUTPUT): $(OFILES)
	@$(MKCWD)
	@echo " LD [ $@ ] $<"
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKCWD)
	@echo " CC [ $@ ] $<"
	@$(CC) $(CFLAGS) -MMD -MP $< -c -o $@ 

run: $(OUTPUT)
	@$(OUTPUT)

all: $(OUTPUT)

clean:
	@rm -rf build/

.PHONY: clean all run

-include $(DFILES)
