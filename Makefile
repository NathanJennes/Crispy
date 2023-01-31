# ==============================================================================
#	Progress bar
# ==============================================================================
ifndef ECHO
HIT_TOTAL != ${MAKE} ${MAKECMDGOALS} --dry-run ECHO="HIT_MARK" | grep -c "HIT_MARK"
HIT_COUNT = $(eval HIT_N != expr ${HIT_N} + 1)${HIT_N}
ECHO = echo -e "[`expr ${HIT_COUNT} '*' 100 / ${HIT_TOTAL}`%]\t"
endif

# ==============================================================================
#	Makefile setup and global variables
# ==============================================================================
MAKEFLAGS		+=		--no-print-directory -r -R
THIS_MAKEFILE	:=		$(lastword $(MAKEFILE_LIST))
ROOT_DIR		:=		$(PWD)

# ==============================================================================
#	Build mode management
# ==============================================================================
RELEASE_MODE_FILE		:=	.release_mode
DEBUG_MODE_FILE			:=	.debug_mode
SANITIZE_MODE_FILE		:=	.sanitize_mode

# ==============================================================================
#	Project name
# ==============================================================================
RELEASE_NAME	:=	vulkan
DEBUG_NAME		:=	debug_vulkan
SANITIZE_NAME	:=	sanitize_vulkan

# ==============================================================================
#	Project environment
# ==============================================================================
BIN_DIR			:=		bin
OBJ_DIR			:=		obj
RELEASE_OBJDIR	:=		$(OBJ_DIR)/release
DEBUG_OBJDIR	:=		$(OBJ_DIR)/debug
SANITIZE_OBJDIR	:=		$(OBJ_DIR)/sanitize
SRC_DIR			:=		src
DEP_DIR			:=		dependencies
SHADER_DIR		:=		shaders

# ==============================================================================
#	Project sources
# ==============================================================================
SRCS				:=		$(shell find $(SRC_DIR) -type f -name *.cpp)
OBJS				:=		$(SRCS:.cpp=.o)
RELEASE_OBJS		:=		$(addprefix $(RELEASE_OBJDIR)/, $(OBJS))
DEBUG_OBJS			:=		$(addprefix $(DEBUG_OBJDIR)/, $(OBJS))
SANITIZE_OBJS		:=		$(addprefix $(SANITIZE_OBJDIR)/, $(OBJS))

SHADERS				:=		$(shell find $(SHADER_DIR) -type f -name *.glsl)
COMPILED_SHADERS	:=		$(addprefix $(OBJ_DIR)/, $(SHADERS:.glsl=.spv))

# ==============================================================================
#	Compilers
# ==============================================================================
CXX				:=	g++
SPIRV_COMPILER	:=	$(VULKAN_SDK)/bin/glslc

# ==============================================================================
#	Compilation and linking flags
# ==============================================================================
CXX_FLAGS	:=		-Wall -Wextra -Werror -std=c++17
CXX_FLAGS	+=		-MD -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE
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

# ==============================================================================
#	Build mode-specific flags
# ==============================================================================
RELEASE_CXX_FLAGS	:=	-O3
RELEASE_LD_FLAGS	:=

DEBUG_CXX_FLAGS		:=	-g3 -DDEBUG
DEBUG_LD_FLAGS		:=

SANITIZE_CXX_FLAGS	:=	$(DEBUG_CXX_FLAGS) -fsanitize=address
SANITIZE_LD_FLAGS	:=	-fsanitize=address

# ==============================================================================
#	Libs
# ==============================================================================
GLFW_LIB	:=		$(DEP_DIR)/glfw/build/src/libglfw3.a

# ==============================================================================
#	Main commands
# ==============================================================================
.PHONY: default
default:
	@if [ -f "$(RELEASE_MODE_FILE)" ]; then $(MAKE) -f $(THIS_MAKEFILE) $(BIN_DIR)/$(RELEASE_NAME); \
	elif [ -f "$(DEBUG_MODE_FILE)" ]; then $(MAKE) -f $(THIS_MAKEFILE) $(BIN_DIR)/$(DEBUG_NAME); \
	elif [ -f "$(SANITIZE_MODE_FILE)" ]; then $(MAKE) -f $(THIS_MAKEFILE) $(BIN_DIR)/$(SANITIZE_NAME); \
	else $(MAKE) -f $(THIS_MAKEFILE) release; fi

.PHONY: all
all: $(RELEASE_MODE_FILE) $(BIN_DIR)/$(RELEASE_NAME) $(BIN_DIR)/$(DEBUG_NAME) $(BIN_DIR)/$(SANITIZE_NAME)
	@echo "[Make all]: make, make run and make re will now target $(_GREEN)release$(_END) mode"

.PHONY: run
run: default
	@if [ -f "$(RELEASE_MODE_FILE)" ]; then ./$(BIN_DIR)/$(RELEASE_NAME); fi
	@if [ -f "$(DEBUG_MODE_FILE)" ]; then ./$(BIN_DIR)/$(DEBUG_NAME); fi
	@if [ -f "$(SANITIZE_MODE_FILE)" ]; then ./$(BIN_DIR)/$(SANITIZE_NAME); fi

.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR)

.PHONY: fclean
fclean: clean
	@rm -rf $(BIN_DIR)
	@rm -rf $(DEP_DIR)/glfw/build

.PHONY: re
re: fclean
	@$(MAKE) -f $(THIS_MAKEFILE) default

# ==============================================================================
#	Build mode commands
# ==============================================================================
.PHONY: release
release: $(RELEASE_MODE_FILE) $(BIN_DIR)/$(RELEASE_NAME)
	@echo "[Make release]: make, make run and make re will now target $(_GREEN)release$(_END) mode"

.PHONY: debug
debug: $(DEBUG_MODE_FILE) $(BIN_DIR)/$(DEBUG_NAME)
	@echo "[Make debug]: make, make run and make re will now target $(_BLUE)debug$(_END) mode"

.PHONY: sanitize
sanitize: $(SANITIZE_MODE_FILE) $(BIN_DIR)/$(SANITIZE_NAME)
	@echo "[Make sanitize]: make, make run and make re will now target $(_ORANGE)sanitize$(_END) mode"

# ==============================================================================
#	Build mode file creation
# ==============================================================================

$(RELEASE_MODE_FILE):
	@rm -f $(DEBUG_MODE_FILE) $(SANITIZE_MODE_FILE)
	@touch $(RELEASE_MODE_FILE)

$(DEBUG_MODE_FILE):
	@rm -f $(RELEASE_MODE_FILE) $(SANITIZE_MODE_FILE)
	@touch $(DEBUG_MODE_FILE)

$(SANITIZE_MODE_FILE):
	@rm -f $(DEBUG_MODE_FILE) $(RELEASE_MODE_FILE)
	@touch $(SANITIZE_MODE_FILE)

# ==============================================================================
#	Project workspace setup
# ==============================================================================
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR)/$(SHADER_DIR):
	@mkdir -p $(OBJ_DIR)/$(SHADER_DIR)

# ==============================================================================
#	Compilation
# ==============================================================================

#====Release build====#
$(BIN_DIR)/$(RELEASE_NAME): $(GLFW_LIB) $(COMPILED_SHADERS) $(RELEASE_OBJS) Makefile | $(BIN_DIR)
	@$(ECHO) "$(_GREEN)$@$(_END)"
	@$(CXX) $(RELEASE_OBJS) $(GLFW_LIB) -o $(BIN_DIR)/$(RELEASE_NAME) $(LD_FLAGS) $(RELEASE_LD_FLAGS)
	@echo "$(_GREEN)[Build mode]: Release$(_END)"

$(RELEASE_OBJDIR)/%.o: %.cpp $(GLFW_LIB) Makefile
	@$(ECHO) "$(_GREEN)$<$(_END)"
	@mkdir -p $(dir $@)
	@$(CXX) $< $(CXX_FLAGS) $(RELEASE_CXX_FLAGS) -c -o $@

#====Debug build====#
$(BIN_DIR)/$(DEBUG_NAME): $(GLFW_LIB) $(COMPILED_SHADERS) $(DEBUG_OBJS) Makefile | $(BIN_DIR)
	@$(ECHO) "$(_BLUE)$@$(_END)"
	@$(CXX) $(DEBUG_OBJS) $(GLFW_LIB) -o $(BIN_DIR)/$(DEBUG_NAME) $(LD_FLAGS) $(DEBUG_LD_FLAGS)
	@echo "$(_BLUE)[Build mode]: Debug$(_END)"

$(DEBUG_OBJDIR)/%.o: %.cpp $(GLFW_LIB) Makefile
	@$(ECHO) "$(_BLUE)$<$(_END)"
	@mkdir -p $(dir $@)
	@$(CXX) $< $(CXX_FLAGS) $(DEBUG_CXX_FLAGS) -c -o $@

#====Sanitize build====#
$(BIN_DIR)/$(SANITIZE_NAME): $(GLFW_LIB) $(COMPILED_SHADERS) $(SANITIZE_OBJS) Makefile | $(BIN_DIR)
	@$(ECHO) "$(_ORANGE)$@$(_END)"
	@$(CXX) $(SANITIZE_OBJS) $(GLFW_LIB) -o $(BIN_DIR)/$(SANITIZE_NAME) $(LD_FLAGS) $(SANITIZE_LD_FLAGS)
	@echo "$(_ORANGE)[Build mode]: Sanitize$(_END)"

$(SANITIZE_OBJDIR)/%.o: %.cpp $(GLFW_LIB) Makefile
	@$(ECHO) "$(_ORANGE)$<$(_END)"
	@mkdir -p $(dir $@)
	@$(CXX) $< $(CXX_FLAGS) $(SANITIZE_CXX_FLAGS) -c -o $@

#====Shaders====#
$(OBJ_DIR)/%.vert.spv: %.vert.glsl $(GLFW_LIB) Makefile | $(OBJ_DIR)/$(SHADER_DIR)
	@$(ECHO) "$(_PURPLE)$<$(_END)"
	@$(SPIRV_COMPILER) -fshader-stage=vertex -o $@ $<

$(OBJ_DIR)/%.frag.spv: %.frag.glsl $(GLFW_LIB) Makefile | $(OBJ_DIR)/$(SHADER_DIR)
	@$(ECHO) "$(_PURPLE)$<$(_END)"
	@$(SPIRV_COMPILER) -fshader-stage=fragment -o $@ $<

$(GLFW_LIB):
	@cd $(DEP_DIR)/glfw && \
	cmake -S . -B build \
	-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && \
	cd build && make
	@cd $(ROOT_DIR)

-include $(OBJS:.o=.d)

# ==============================================================================
#	Extra
# ==============================================================================
_GREY	= \033[30m
_RED	= \033[31m
_ORANGE	= \033[38;5;209m
_GREEN	= \033[32m
_YELLOW	= \033[33m
_BLUE	= \033[34m
_PURPLE	= \033[35m
_CYAN	= \033[36m
_WHITE	= \033[37m
_END	= \033[0m
