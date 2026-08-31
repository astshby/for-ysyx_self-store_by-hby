#编辑器已经加入 -WALL --lint-only

#创建目录
TOPNAME = top
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj
BIN = $(BUILD_DIR)/$(TOPNAME)
NXDC_FILES = constr/top.nxdc
INC_PATH ?=
$(shell mkdir -p $(BUILD_DIR))

#定义verilator参数，将一些下面参数提到上面
#要忽略仿真时间刻度有些文件没有的缺陷,以及反斜杠
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -j 0 \
                -O2 --x-assign fast --x-initial fast --noassert \
                --top-module $(TOPNAME) \
                --Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) \
				-Wno-TIMESCALEMOD
VERILATOR_SIM_CFLAGS += --cc --exe --build -j 0 \
                --timing --trace-fst \
                --top-module $(TOPNAME) \
				-Wno-TIMESCALEMOD

#自动化引脚生成
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

#寻找文件，配置编译
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
CSRCS += $(SRC_AUTO_BIND)

#寻找仿真专用文件 (避免与 NVBoard 冲突) 
SIM_VSRCS = $(shell find $(abspath ./simsrc) -name "*.v" 2>/dev/null)
SIM_CSRCS = $(shell find $(abspath ./simsrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp" 2>/dev/null)
SIM_BIN = $(BUILD_DIR)/sim_$(TOPNAME)
SIM_OBJ_DIR = $(BUILD_DIR)/sim_obj

#编译环境包含(nvboard+ysyx)
include $(NVBOARD_HOME)/scripts/nvboard.mk
include ../../Makefile

# 传递 TOP_NAME 宏给 C++,之后写都写为top所以没必要加
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

#-----构建-----#

default : $(BIN)

$(BIN):$(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	@echo "Building Project: $(TOPNAME) (NVBoard Version)"  
	@echo "Verilog Source: $(VSRCS)"
	@echo "C++ Source:     $(CSRCS)"
	$(VERILATOR) $(VERILATOR_CFLAGS) \
        $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE) \
        $(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS))

###切记每次最后提交都要git
#  $(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!
run:$(BIN)
	$(call git_commit, "完成实验7,键盘与状态机")
	@echo "Running NVBoard Simulation: $(TOPNAME)"
	@$(BIN) 
	@echo "Simulation done. "

#纯逻辑仿真与波形生成目标,在build中动态拼接
sim: $(VSRCS) $(SIM_VSRCS) $(SIM_CSRCS)
	@rm -rf $(SIM_OBJ_DIR)
	@echo "Building Simulation: $(TOPNAME) (FST Waveform Version)"
	@$(VERILATOR) $(VERILATOR_SIM_CFLAGS) \
		--Mdir $(SIM_OBJ_DIR) -o $(abspath $(SIM_BIN)) \
		$(VSRCS) $(SIM_VSRCS) $(SIM_CSRCS)  
	@echo "Running Simulation..."
	@$(SIM_BIN)
	@echo "Simulation done. Opening Waveform with GTKWave..."
	@gtkwave $(BUILD_DIR)/sim.fst &

clean:
	@echo "rm $(BUILD_DIR)"  
	@rm -rf $(BUILD_DIR)

.PHONY: default run sim clean