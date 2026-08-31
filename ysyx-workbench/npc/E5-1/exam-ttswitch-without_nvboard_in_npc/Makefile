#@：静默执行：不打印命令，直接执行

#构建文件结构,top尽量就是top module，保证模块的合理性
TOPNAME = ttswitch
BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

#寻找文件并编译
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
VERILATOR_FLAGS = --trace --cc --exe --build -j 0 -Wall \
                  --top-module $(TOPNAME) \
                  --Mdir $(OBJ_DIR) -o $(abspath $(BIN))

default:sim

#最后这行把生成的 vcd 移动到 build 目录 (如果 C++ 里写死路径可能不需要这步)
sim:$(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	@echo "Simulation $(TOPNAME)"
	@$(BIN) # 运行生成的可执行文件
	@echo "Simulation done. Waveform generated."
	@mv -f *.vcd $(BUILD_DIR) 2>/dev/null || true

#真正执行命令
$(BIN): $(VSRCS) $(CSRCS)
	@mkdir -p $(BUILD_DIR)
	@echo "Building Verilator..."
	@echo "Verilog Source: $(VSRCS)"
	@echo "C++ Source:     $(CSRCS)"
	verilator $(VERILATOR_FLAGS) $(VSRCS) $(CSRCS)

clean:
	@rm -rf $(BUILD_DIR)
	@echo "rm $(BUILD_DIR)"	

#包含ysyx通用环境,此处应为../..
include ../../Makefile

.PHONY:default sim clean
