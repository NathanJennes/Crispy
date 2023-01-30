ifndef ECHO
HIT_TOTAL != ${MAKE} ${MAKECMDGOALS} --dry-run ECHO="HIT_MARK" | grep -c "HIT_MARK"
HIT_COUNT = $(eval HIT_N != expr ${HIT_N} + 1)${HIT_N}
ECHO = echo "[`expr ${HIT_COUNT} '*' 100 / ${HIT_TOTAL}`%]"
endif

MAKEFLAGS		+=		--no-print-directory -r -R
THIS_MAKEFILE	:=		$(lastword $(MAKEFILE_LIST))

NAME		:=		vulkan

ROOT_DIR	:=		$(PWD)
BIN_DIR		:=		bin
OBJ_DIR		:=		obj
SRC_DIR		:=		src
DEP_DIR		:=		dependencies

CXX			:=		g++
CXX_FLAGS	:=		-Wall -Wextra -Werror -std=c++17
CXX_FLAGS	+=		-MD -DGLM_FORCE_RADIANS
CXX_FLAGS	+=		-I$(SRC_DIR) -I$(VULKAN_SDK)/include -I$(DEP_DIR)/glfw/include/GLFW -I$(DEP_DIR)

ifeq ($(shell uname), Linux)
	CXX_FLAGS	+=	-DPLATFORM_LINUX
	LD_FLAGS	:=	-L$(VULKAN_SDK)/lib -L/usr/lib64
  	LD_FLAGS	+=	-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon
else ifeq ($(shell uname), Darwin)
	CXX_FLAGS	+=	-DPLATFORM_MACOS
	LD_FLAGS 	:=	-L$(VULKAN_SDK)/lib -L$(DEP_DIR)/glfw/build/src -lglfw3 -framework Cocoa -framework IOKit
	LD_FLAGS	+=	-lvulkan
else
	$(error "Unsupported OS")
endif

#----
# Build mode file
#----
RELEASE_MODE_FILE	:=	.release_mode
DEBUG_MODE_FILE		:=	.debug_mode
SANITIZE_MODE_FILE	:=	.sanitize_mode
BUILD_OPTIONS_FILE	:=	.build_infos

#----
# Build mode specific flags
#----
RELEASE_CXX_FLAGS	:=	-O3
RELEASE_LD_FLAGS	:=

DEBUG_CXX_FLAGS		:=	-g3 -DDEBUG
DEBUG_LD_FLAGS		:=

SANITIZE_CXX_FLAGS	:=	$(DEBUG_CXX_FLAGS) -fsanitize=address
SANITIZE_LD_FLAGS	:=	-fsanitize=address

-include $(BUILD_OPTIONS_FILE)

SRCS		:=		$(shell find $(SRC_DIR) -type f -name *.cpp)
OBJS		:=		$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRCS:.cpp=)))

GLFW_LIB	:=		$(DEP_DIR)/glfw/build/src/libglfw3.a

SHADER_DIR			:=		shaders
SHADERS				:=		$(shell find $(SHADER_DIR) -type f -name *.glsl)
COMPILED_SHADERS	:=		$(addprefix $(OBJ_DIR)/, $(SHADERS:.glsl=.spv))

SPIRV_COMPILER		:=		$(VULKAN_SDK)/bin/glslc

DIRECTORIES			:=		$(shell find $(SRC_DIR) -type d) $(shell find $(SHADER_DIR) -type d)

#----
# Main commands
#----
.PHONY: all
all: before_build $(BIN_DIR)/$(NAME)
	@if [ -f "$(RELEASE_MODE_FILE)" ]; then echo "[Build mode]: Release"; fi
	@if [ -f "$(DEBUG_MODE_FILE)" ]; then echo "[Build mode]: Debug"; fi
	@if [ -f "$(SANITIZE_MODE_FILE)" ]; then echo "[Build mode]: Sanitize" ; fi

.PHONY: run
run: all
	./$(BIN_DIR)/$(NAME)

.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR)

.PHONY: fclean
fclean: clean
	@rm -rf $(BIN_DIR)
	@rm -rf $(DEP_DIR)/glfw/build

.PHONY: re
re: fclean
	@$(MAKE) -f $(THIS_MAKEFILE) all

#----
# Build Modes
#----
.PHONY: release
release: $(RELEASE_MODE_FILE)
	@$(MAKE) -f $(THIS_MAKEFILE) all

.PHONY: debug
debug: $(DEBUG_MODE_FILE)
	@$(MAKE) -f $(THIS_MAKEFILE) all

.PHONY: sanitize
sanitize: $(SANITIZE_MODE_FILE)
	@$(MAKE) -f $(THIS_MAKEFILE) all

$(BUILD_OPTIONS_FILE):
	@touch $(BUILD_OPTIONS_FILE)
	@echo "BUILD_MODE_CXX_FLAGS := $(RELEASE_CXX_FLAGS)" > $(BUILD_OPTIONS_FILE)
	@echo "BUILD_MODE_LD_FLAGS := $(RELEASE_LD_FLAGS)" >> $(BUILD_OPTIONS_FILE)

$(RELEASE_MODE_FILE): $(BUILD_OPTIONS_FILE)
	@rm -f $(DEBUG_MODE_FILE) $(SANITIZE_MODE_FILE)
	@$(MAKE) -f $(THIS_MAKEFILE) fclean
	@touch $(RELEASE_MODE_FILE)
	@echo "BUILD_MODE_CXX_FLAGS := $(RELEASE_CXX_FLAGS)" > $(BUILD_OPTIONS_FILE)
	@echo "BUILD_MODE_LD_FLAGS := $(RELEASE_LD_FLAGS)" >> $(BUILD_OPTIONS_FILE)

$(DEBUG_MODE_FILE): $(BUILD_OPTIONS_FILE)
	@rm -f $(RELEASE_MODE_FILE) $(SANITIZE_MODE_FILE)
	@$(MAKE) -f $(THIS_MAKEFILE) fclean
	@touch $(DEBUG_MODE_FILE)
	@echo "BUILD_MODE_CXX_FLAGS := $(DEBUG_CXX_FLAGS)" > $(BUILD_OPTIONS_FILE)
	@echo "BUILD_MODE_LD_FLAGS := $(DEBUG_LD_FLAGS)" >> $(BUILD_OPTIONS_FILE)

$(SANITIZE_MODE_FILE): $(BUILD_OPTIONS_FILE)
	@rm -f $(DEBUG_MODE_FILE) $(RELEASE_MODE_FILE)
	@$(MAKE) -f $(THIS_MAKEFILE) fclean
	@touch $(SANITIZE_MODE_FILE)
	@echo "BUILD_MODE_CXX_FLAGS := $(SANITIZE_CXX_FLAGS)" > $(BUILD_OPTIONS_FILE)
	@echo "BUILD_MODE_LD_FLAGS := $(SANITIZE_LD_FLAGS)" >> $(BUILD_OPTIONS_FILE)

#----
# Build workspace setup
#----
.PHONY: before_build
before_build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(addprefix $(OBJ_DIR)/, $(DIRECTORIES))


#----
# Compilation
#----
$(BIN_DIR)/$(NAME): $(GLFW_LIB) $(COMPILED_SHADERS) $(OBJS) Makefile
	@$(ECHO) $@
	@$(CXX) $(OBJS) $(GLFW_LIB) -o $(BIN_DIR)/$(NAME) $(LD_FLAGS) $(BUILD_MODE_LD_FLAGS)

$(OBJ_DIR)/%.o: %.cpp $(GLFW_LIB) Makefile
	@$(ECHO) $<
	@$(CXX) $< $(CXX_FLAGS) $(BUILD_MODE_CXX_FLAGS) -c -o $@

$(OBJ_DIR)/%.vert.spv: %.vert.glsl $(GLFW_LIB) Makefile
	@$(ECHO) $<
	@$(SPIRV_COMPILER) -fshader-stage=vertex -o $@ $<

$(OBJ_DIR)/%.frag.spv: %.frag.glsl $(GLFW_LIB) Makefile
	@$(ECHO) $<
	@$(SPIRV_COMPILER) -fshader-stage=fragment -o $@ $<

$(GLFW_LIB):
	@cd $(DEP_DIR)/glfw && \
	cmake -S . -B build \
	-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && \
	cd build && make
	@cd $(ROOT_DIR)

-include $(OBJS:.o=.d)
