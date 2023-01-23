MAKEFLAGS	+=	--no-print-directory -r -R --warn-undefined-variables

NAME		:=		vulkan

VULKAN_SDK	:=		$(HOME)/vulkan/1.3.216.0/x86_64

CURRENT_DIR	:=		$(PWD)
BIN_DIR		:=		bin
OBJ_DIR		:=		obj
SRC_DIR		:=		src
DEP_DIR		:=		dependencies

CXX			:=		g++
CXX_FLAGS	:=		-Wall -Wextra
CXX_FLAGS	+=		-DDEBUG -DVK_USE_PLATFORM_XCB_KHR -g3 -MD
CXX_FLAGS	+=		-I$(SRC_DIR) -I$(VULKAN_SDK)/include -I$(DEP_DIR)
CXX_FLAGS	+=		-DGLM_FORCE_RADIANS
#CXX_FLAGS	+=		-fsanitize=address

LD_FLAGS	:=		-L$(VULKAN_SDK)/lib -L/usr/lib64
LD_FLAGS	+=		-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon
#LD_FLAGS	+=		-fsanitize=address

SRCS		:=		$(shell find $(SRC_DIR) -type f -name *.cpp)
OBJS		:=		$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRCS:.cpp=)))

SHADER_DIR			:=		shaders
SHADERS				:=		$(shell find $(SHADER_DIR) -type f -name *.glsl)
COMPILED_SHADERS	:=		$(addprefix $(OBJ_DIR)/, $(SHADERS:.glsl=.spv))

SPIRV_COMPILER		:=	$(VULKAN_SDK)/bin/glslc

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

.PHONY: re
re: fclean all

.PHONY: before_build
before_build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(addprefix $(OBJ_DIR)/, $(DIRECTORIES))

$(BIN_DIR)/$(NAME): $(COMPILED_SHADERS) $(OBJS) Makefile
	@echo "creating executable $(NAME)..."
	@$(CXX) $(OBJS) -o $(BIN_DIR)/$(NAME) $(LD_FLAGS)

$(OBJ_DIR)/%.o: %.cpp Makefile
	@echo   $<...
	@$(CXX) $< $(CXX_FLAGS) -c -o $@

$(OBJ_DIR)/%.vert.spv: %.vert.glsl Makefile
	@echo   $<...
	@$(SPIRV_COMPILER) -fshader-stage=vertex -o $@ $<

$(OBJ_DIR)/%.frag.spv: %.frag.glsl Makefile
	@echo   $<...
	@$(SPIRV_COMPILER) -fshader-stage=fragment -o $@ $<

-include $(OBJS:.o=.d)