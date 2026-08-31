#include <verilated.h>
#include <verilated_fst_c.h> //注意这里变为fst，可以修改为vcd
#include <Vtop.h>

static Vtop *dut = nullptr;
static VerilatedContext *context_p = nullptr;
static VerilatedFstC *tfp = nullptr;

void init_sim(int argc, char **argv)
{
    context_p = new VerilatedContext;
    context_p->commandArgs(argc, argv);
    context_p->traceEverOn(true);

    dut = new Vtop(context_p);

    tfp = new VerilatedFstC;
    dut->trace(tfp, 99);
    tfp->open("build/sim.fst");
}

void step_sim()
{
    // 没有要注释clk并且留下一个eval与time
    // 模拟时钟低电平半周期
    dut->clk = 0;
    dut->eval();
    tfp->dump(context_p->time());
    context_p->timeInc(1);

    // 模拟时钟高电平半周期
    dut->clk = 1;
    dut->eval();
    tfp->dump(context_p->time());
    context_p->timeInc(1);
}

static void reset(int n)
{
    // 没有rst要注释
    dut->rst = 0;
    while (n-- > 0)
    {
        step_sim();
    }
    dut->rst = 1;
}

void end_sim()
{
    dut->final();
    tfp->close();
    delete tfp;
    delete dut;
    delete context_p;
}

int main(int argc, char **argv)
{
    init_sim(argc, argv);
    reset(10);

    // 运行直到遇到 $finish，或者设置一个最大时钟周期上限（比如 50000 拍）
    while (!context_p->gotFinish() && context_p->time() < 100000)
    {
        step_sim();
    }

    end_sim();
    return 0;
}