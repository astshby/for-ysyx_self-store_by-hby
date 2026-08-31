#include <nvboard.h>
#include <Vttswitch.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

/*
int main(int argc, char **argv)
{
    // 初始化环境,最后一句开启波形追踪
    VerilatedContext *context_p = new VerilatedContext;
    context_p->commandArgs(argc, argv);
    context_p->traceEverOn(true);

    // 实例化电路
    Vttswitch *top = new Vttswitch{context_p};

    // 生成波形文件
    VerilatedVcdC *trace = new VerilatedVcdC;
    top->trace(trace, 0); // 层级深入，可知模块内部信息，0在本次只看端口已经够用
    trace->open("ttswitch.vcd");

    int count = 0;
    // 看有没有finish，没有的话继续运行
    // while (!context_p->gotFinish())
    // 防止过分运行
    while (count < 20)
    {
        count++;
        // 产生随机输入
        int a = rand() & 1;
        int b = rand() & 1;
        // 驱动模块top
        top->a = a;
        top->b = b;
        // 刷新电路
        top->eval();
        printf("a = %d, b = %d, f = %d\n", a, b, top->f);
        assert(top->f == (a ^ b));

        // 推进波形与时间
        trace->dump(context_p->time());
        context_p->timeInc(1); // 展开波形
    }
    // 关闭波形文件
    trace->close();

    delete trace;
    delete top;
    delete context_p;
    return 0;
}
*/

// nvboard不需要模拟输入，只需要api接口就可以，
// 也可以画波形图，接口nvboard已经提交给verilator，但在没有满足推出条件要强制写入
// 由于没与限定时间，这个是组合逻辑，会不断写入，导致波形文件很大

// 定义测试对象，注意Vttswitch模块
static Vttswitch *dut = nullptr;
static VerilatedContext *context_p = nullptr;
static VerilatedVcdC *trace = nullptr;

void nvboard_bind_all_pins(TOP_NAME *top);

void init_sim(int argc, char **argv)
{
    context_p = new VerilatedContext;
    context_p->commandArgs(argc, argv);
    context_p->traceEverOn(true);

    // 电路实例化，nvinit用来生成图形化界面
    dut = new Vttswitch(context_p);
    nvboard_bind_all_pins(dut);
    nvboard_init();

    trace = new VerilatedVcdC;
    dut->trace(trace, 0);
    trace->open("build/ttswitch.vcd");
}

void step_sim()
{
    /*时序逻辑
    dut->clk = 0;
    dut->eval();
    dut->clk = 1;
    */
    dut->eval();

    trace->dump(context_p->time());
    trace->flush();
    context_p->timeInc(1);
}

void end_sim()
{
    nvboard_quit();
    trace->close();
    delete trace;
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