# Usage:
# make        					# builds nano-init for Release
# debugEnable=true make        	# builds nano-init for Debug
# make clean  					# remove ALL binaries and objects

# application binary name
APP_NAME := nanoinit
# source folder for sources
SOURCE_DIR := .
# build object folder
BUILD_DIR := ../build
# application deploy folder
APP_DEPLOY := ..

# base flags
CC = gcc
PHONY = 

# flags
CCFLAGS = -fdiagnostics-color=always -Wall -Wextra -Werror -pedantic -MMD -MP

CCINC = -I.
CCLIB = 


# config selector
ifeq ($(debugEnable),true)
	CCFLAGS += -D_DEBUG=1 -O0 -g3
	BUILD_MODE = Debug
else
	CCFLAGS += -O2
	BUILD_MODE = Release
endif

SRC_FILES := $(shell find $(SOURCE_DIR)/ -type f -name '*.c')
OBJ_FILES := $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/$(BUILD_MODE)/%.o, $(SRC_FILES))
DEP_FILES := $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/$(BUILD_MODE)/%.d, $(SRC_FILES))

all: build

# object files
$(BUILD_DIR)/$(BUILD_MODE)/%.o: $(SOURCE_DIR)/%.c
	@ mkdir -p $(@D)
	$(CC) $(CCFLAGS) $(CCINC) -c $< -o $@

PHONY += pre_build
pre_build: Makefile
	@echo "\e[1;34mnano-init Build\e[0m - \e[1;32mstarted\e[0m... (\e[1;37m$(BUILD_MODE)\e[0m)"
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

PHONY += obj_build
obj_build: pre_build $(OBJ_FILES)

PHONY += post_build
post_build: obj_build
	@echo "\e[1;34mnano-init Build\e[0m - \e[1;32mlinking\e[0m..."
	$(CC) $(CCFLAGS) $(OBJ_FILES) $(CCLIB) -o $(APP_DEPLOY)/$(APP_NAME)

PHONY += build
build: post_build
	@echo "\e[1;34mnano-init Build\e[0m - \e[1;32mfinished\e[0m..."


PHONY += clean
clean:
	@echo "\e[1;34mNTS Build\e[0m - \e[1;32mCleaning up...\e[0m"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(APP_DEPLOY)/$(APP_NAME)

.PHONY: $(PHONY)

# include source dependencies
-include $(DEP_FILES)
