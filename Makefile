MAKEFLAGS	+=	--no-print-directory -r -R --warn-undefined-variables

NAME		:=		vulkan

VULKAN_SDK	:=		$(HOME)/vulkan/1.3.216.0/x86_64

CURRENT_DIR	:=		$(PWD)
BIN_DIR		:=		bin
OBJ_DIR		:=		obj
SRC_DIR		:=		src

CXX			:=		g++
CXX_FLAGS	:=		-Wall -Wextra
CXX_FLAGS	+=		-DDEBUG -DVK_USE_PLATFORM_XCB_KHR -g3 -MD
CXX_FLAGS	+=		-I$(SRC_DIR) -I$(VULKAN_SDK)/include

LD_FLAGS	:=		-L$(VULKAN_SDK)/lib -L/usr/lib64
LD_FLAGS	+=		-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon

SRCS		:=		$(shell find $(SRC_DIR) -type f -name *.cpp)
OBJS		:=		$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRCS:.cpp=)))
DIRECTORIES	:=		$(shell find $(SRC_DIR) -type d)

.PHONY: all
all: before_build $(BIN_DIR)/$(NAME)

.PHONY: run
run: $(BIN_DIR)/$(NAME)
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

$(BIN_DIR)/$(NAME): $(OBJS) Makefile
	@echo "creating executable $(NAME)..."
	@$(CXX) $(OBJS) -o $(BIN_DIR)/$(NAME) $(LD_FLAGS)

$(OBJ_DIR)/%.o: %.cpp Makefile
	@echo   $<...
	@$(CXX) $< $(CXX_FLAGS) -c -o $@

-include $(OBJS:.o=.d)