MKCWD=mkdir -p $(@D)

CC = gcc

SANITIZERS =
#	-fsanitize=address 	\
	-fsanitize=undefined \
	-fsanitize-address-use-after-scope 

CFLAGS_WARNS ?= 	\
		-Werror 	\
		-Wextra 	\
		-Wall 		\
		-Wundef 	\
		-Wshadow 	\
		-Wvla

CFLAGS = 			\
		-Ofast 		\
		-g 		 	\
		-std=gnu2x  \
		-Isrc/      \
		$(CFLAGS_WARNS)

LDFLAGS=$(SANITIZERS) -lm -lglfw -lvulkan -ldl -lpthread

# some people likes to use sources/source instead of src
PROJECT_NAME = test
BUILD_DIR = build
SRC_DIR = src

VSFILES = $(wildcard $(SRC_DIR)/*.vs) $(wildcard $(SRC_DIR)/*/*.vs) $(wildcard $(SRC_DIR)/*/*/*.vs)
FSFILES = $(wildcard $(SRC_DIR)/*.fs) $(wildcard $(SRC_DIR)/*/*.fs) $(wildcard $(SRC_DIR)/*/*/*.fs)
COMPSFILES = $(wildcard $(SRC_DIR)/*.comp) $(wildcard $(SRC_DIR)/*/*.comp) $(wildcard $(SRC_DIR)/*/*/*.comp)
COMPDFILES = $(patsubst $(SRC_DIR)/%.comp, $(BUILD_DIR)/%.d, $(COMPSFILES))

OSFILES = $(patsubst $(SRC_DIR)/%.vs, $(BUILD_DIR)/%.spv, $(VSFILES)) \
		$(patsubst $(SRC_DIR)/%.fs, $(BUILD_DIR)/%.spv, $(FSFILES)) \
		$(patsubst $(SRC_DIR)/%.comp, $(BUILD_DIR)/%.spv, $(COMPSFILES))


CFILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*/*/*.c)
DFILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.d, $(CFILES))
OFILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CFILES))

OUTPUT = build/$(PROJECT_NAME)

$(BUILD_DIR)/%.spv: $(SRC_DIR)/%.fs
	@$(MKCWD)
	@echo " FS [ $@ ] $<"
	@glslc -O -fshader-stage=frag $< -o $@

$(BUILD_DIR)/%.spv: $(SRC_DIR)/%.vs
	@$(MKCWD)
	@echo " VS [ $@ ] $<"
	@glslc -O -fshader-stage=vert $< -o $@


$(BUILD_DIR)/%.spv: $(SRC_DIR)/shaders/comp.comp
	@$(MKCWD)
	@echo " CS [ $@ ] $<"
	@glslc -Isrc/shaders/ -O -MD -fshader-stage=comp $< -o $@


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKCWD)
	@echo " CC [ $@ ] $<"
	@clang $(CFLAGS) $(SANITIZERS) -MMD -MP $< -c -o $@


$(OUTPUT): $(OFILES)
	@$(MKCWD)
	@echo " LD [ $@ ] $^"
	@clang -o $@ $^ $(LDFLAGS) $(SANITIZERS)

run: $(OUTPUT) $(OSFILES)
	@$(OUTPUT)

all: $(OUTPUT) $(OSFILES)

clean:
	@rm -rf build/

.PHONY: clean all run

-include $(DFILES)
-include $(COMPDFILES)

