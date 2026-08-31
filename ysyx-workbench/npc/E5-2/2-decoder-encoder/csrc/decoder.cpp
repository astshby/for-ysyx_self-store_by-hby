#include <nvboard.h>
#include <Vdecoder.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

// nvboard不需要模拟输入，只需要api接口就可以，

// 定义测试对象
static Vdecoder *dut = nullptr;
static VerilatedContext *context_p = nullptr;

void nvboard_bind_all_pins(TOP_NAME *top);

void init_sim(int argc, char **argv)
{
    context_p = new VerilatedContext;
    context_p->commandArgs(argc, argv);
    context_p->traceEverOn(true);

    // 电路实例化，nvinit用来生成图形化界面
    dut = new Vdecoder(context_p);
    nvboard_bind_all_pins(dut);
    nvboard_init();
}

void step_sim()
{
    /*dut->clk = 0;
    dut->eval();
    dut->clk = 1;*/
    dut->eval();
}

void end_sim()
{
    nvboard_quit();
    delete dut;
    delete context_p;
}

static void reset(int n)
{
    // 注意：没有rst要注释掉rst
    // dut->rst = 1;
    while (n-- > 0)
        step_sim();
    // dut->rst = 0;
}

int main(int argc, char **argv)
{

    init_sim(argc, argv);

    reset(10);

    // 交互式，不要写count
    // 由于写入波形要求$FINISH退出,现在的话缓冲区不会有信号，所以直接写入
    while (!context_p->gotFinish())
    {
        nvboard_update();
        step_sim();
    }

    end_sim();
}