# 编译器设置
CC		= gcc
#CC      = arm-linux-gnueabi-gcc
CFLAGS  = -Wall -Wextra -O2 \
          -I./include \
		  -I./core \
		  -I./utils/linked_list \
		  -I./utils/log \
		  -I./utils/queue \
		  -I./third_party/cJSON
LDFLAGS = -lpthread

# 目录设置
BUILD_DIR   = build
TARGET      = dev-service

# 自动收集所有源文件（main.c、core、modules/*、utils、third_party/*）
SRCS = \
    main.c \
    $(wildcard core/*.c) \
    $(wildcard modules/*/*.c) \
    $(wildcard utils/*/*.c) \
    $(wildcard third_party/*/*.c)

# 根据源文件生成对应的目标文件路径 build/xxx.o
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# 默认目标
all: $(BUILD_DIR) $(TARGET)

# 创建 build 目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 链接应用程序
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# .c → build/xxx.o 规则
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean