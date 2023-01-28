MAKEFLAGS	+=		--no-print-directory -r -R

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
	LD_FLAGS	:=	-L$(VULKAN_SDK)/lib -L/usr/lib64
  	LD_FLAGS	+=	-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon
else ifeq ($(shell uname), Darwin)
	LD_FLAGS 	:=	-L$(VULKAN_SDK)/lib -L$(DEP_DIR)/glfw/build/src -lglfw3 -framework Cocoa -framework IOKit
	LD_FLAGS	+=	-lvulkan
else
	$(error "Unsupported OS")
endif

# Debug modes
ifeq ($(MAKECMDGOALS), debug)
	CXX_FLAGS	+=	-g3 -DDEBUG
else ifeq ($(MAKECMDGOALS), sanitize)
	CXXFLAGS	+=	-g3 -DDEBUG -fsanitize=address
	LD_FLAGS	+=	-fsanitize=address
else
	CXXFLAGS	+=	-O3
endif

SRCS		:=		$(shell find $(SRC_DIR) -type f -name *.cpp)
OBJS		:=		$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRCS:.cpp=)))

GLFW_LIB	:=		$(DEP_DIR)/glfw/build/src/libglfw3.a

SHADER_DIR			:=		shaders
SHADERS				:=		$(shell find $(SHADER_DIR) -type f -name *.glsl)
COMPILED_SHADERS	:=		$(addprefix $(OBJ_DIR)/, $(SHADERS:.glsl=.spv))

ifeq ($(shell uname), Linux)
SPIRV_COMPILER		:=	$(VULKAN_SDK)/bin/glslc
else ifeq ($(shell uname), Darwin)
SPIRV_COMPILER		:=	$(VULKAN_SDK)/bin/glslc
else
	$(error "Unsupported OS")
endif

DIRECTORIES	:=		$(shell find $(SRC_DIR) -type d) $(shell find $(SHADER_DIR) -type d)

.PHONY: all
all: before_build $(BIN_DIR)/$(NAME)

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
re: fclean all

.PHONY: debug
debug: all

.PHONY: sanitize
sanitize: all

.PHONY: before_build
before_build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(addprefix $(OBJ_DIR)/, $(DIRECTORIES))

$(BIN_DIR)/$(NAME): $(GLFW_LIB) $(COMPILED_SHADERS) $(OBJS) Makefile
	@echo "creating executable $(NAME)..."
	@$(CXX) $(OBJS) $(GLFW_LIB) -o $(BIN_DIR)/$(NAME) $(LD_FLAGS)

$(OBJ_DIR)/%.o: %.cpp Makefile
	@echo   $<...
	@$(CXX) $< $(CXX_FLAGS) -c -o $@

$(OBJ_DIR)/%.vert.spv: %.vert.glsl Makefile
	@echo   $<...
	@$(SPIRV_COMPILER) -fshader-stage=vertex -o $@ $<

$(OBJ_DIR)/%.frag.spv: %.frag.glsl Makefile
	@echo   $<...
	@$(SPIRV_COMPILER) -fshader-stage=fragment -o $@ $<

$(GLFW_LIB):
	@cd $(DEP_DIR)/glfw && \
	cmake -S . -B build \
	-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && \
	cd build && make
	@cd $(ROOT_DIR)

-include $(OBJS:.o=.d)
