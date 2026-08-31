#创建目录
TOPNAME = ttswitch
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj
BIN = $(BUILD_DIR)/$(TOPNAME)
NXDC_FILES = ./constr/top.nxdc
INC_PATH ?=
$(shell mkdir -p $(BUILD_DIR))

#定义verilator参数，将一些下面参数提到上面
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -j 0 \
				-O3 --x-assign fast --x-initial fast --noassert \
				--trace \
                --top-module $(TOPNAME) \
                --Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

#自动化引脚生成
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

#寻找文件，配置编译
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
CSRCS += $(SRC_AUTO_BIND)

#编译环境包含(nvboard+ysyx)
include $(NVBOARD_HOME)/scripts/nvboard.mk
include ../../Makefile

# 传递 TOP_NAME 宏给 C++ ,其实不必要，但还是加上
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""


#-----构建-----#

default : $(BIN)

$(BIN):$(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	@echo "Building Project: $(TOPNAME)"	
	@echo "Verilog Source: $(VSRCS)"
	@echo "C++ Source:     $(CSRCS)"
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		$(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS))

#写好了目录，没必要了加在最后了 
#@mv -f *.vcd $(BUILD_DIR) 2>/dev/null || true
run:$(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo "Simulation $(TOPNAME)"
	@$(BIN) # 运行生成的可执行文件
	@echo "Simulation done. Waveform generated."

clean:
	@echo "rm $(BUILD_DIR)"	
	@rm -rf $(BUILD_DIR)

.PHONY:default run clean