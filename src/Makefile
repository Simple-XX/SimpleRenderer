
# This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
#
# Makefile for SimpleXX/SimpleRenderer.

# 设置 make 环境

CURR_DIR := $(shell pwd)
ROOT_DIR := $(CURR_DIR)

include $(ROOT_DIR)/Makefile.env

.PHONY: default
default:
	@$(MAKE) all

.PHONY: all
all: $(SUB_DIR)
	@echo Entry $(CURR_DIR)
	@for subdir in $(SUB_DIR); \
    	do $(MAKE) -C $$subdir all || exit 1; \
    done
	@$(MAKE) renderer
	@time ./renderer
	@echo Leave $(CURR_DIR)

.PHONY: remake
remake:
	$(MAKE) clean
	@$(MAKE) all

# 删除源码外的所有文件
.PHONY: clean
clean:
	@echo Entry $(CURR_DIR)
	@echo Deleting... $<
	@find . -name "*.o"  | xargs rm -f
	@find . -name "*.gch"  | xargs rm -f
	@rm -f *.map
	@rm -f *.nm
	@rm -f *.bin
	@rm -f *.tga
	@rm -f ./renderer
	@for subdir in $(SUB_DIR); \
		do $(MAKE) -C $$subdir clean || exit 1; \
	done
	@echo Delete Done
	@echo Leave $(CURR_DIR)

C_SOURCES := $(shell find . -name "*.c")
CXX_SOURCES := $(shell find . -name "*.cpp")
S_SOURCES := $(shell find . -name "*.s")
OBJ := $(patsubst %.c, %.o, $(C_SOURCES)) \
     		  $(patsubst %.cpp, %.o, $(CXX_SOURCES)) \
 	    	  $(patsubst %.s, %.o, $(S_SOURCES))

.PHONY: renderer
renderer: $(OBJ)
	@$(LD) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	@echo Compile C++ file $< ...
	@$(CXX) $(CXXFLAGS) $< -o $@
	@echo Compile Done

.PHONY: code_line_count
code_line_count:
	find . -type f -name "*.[c|cpp|s|S|h|hpp]" -exec cat {} \; | wc -l

.PHONY: info
info:
	@echo Current dir: $(CURR_DIR)
	@echo CC: $(CC)
	@echo CFLAGS: $(CFLAGS)
	@echo CXX: $(CXX)
	@echo CXXFLAGS: $(CXXFLAGS)
	@echo LD: $(LD)
	@echo LDFLAGS: $(LDFLAGS)
	@echo AS: $(AS)
	@echo ASFLAGS: $(ASFLAGS)
	@echo exclude_dirs: $(exclude_dirs)
	@echo SUBDIRS: $(SUB_DIR)
	@echo Include dir: $(INCLUDE_DIR)
