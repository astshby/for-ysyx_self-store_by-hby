#include <nvboard.h>
#include <Vtop.h>

static Vtop *dut = nullptr;

void nvboard_bind_all_pins(Vtop *top);

void init_sim()
{
    dut = new Vtop;
    nvboard_bind_all_pins(dut);
    nvboard_init();
}

void step_sim()
{
    // 没有时序逻辑注释掉前三个
    dut->clk = 0;
    dut->eval();
    dut->clk = 1;
    dut->eval();
}

static void reset(int n)
{
    // 没有rst要注释掉rst
    dut->rst = 0;
    while (n-- > 0)
    {
        step_sim();
    }
    dut->rst = 1;
}

void end_sim()
{
    nvboard_quit();
    delete dut;
}

int main()
{
    init_sim();
    reset(10);

    while (1)
    {
        nvboard_update();
        step_sim();
    }

    end_sim(); // 实际上在 while(1) 下永远不会走到这里，但保留规范
    return 0;
}