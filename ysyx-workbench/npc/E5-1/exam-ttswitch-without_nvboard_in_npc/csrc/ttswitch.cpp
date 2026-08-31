// 指令：verilator --cc --exe --build --trace -j 0 -Wall ./csrc/ttswitch.cpp ./vsrc/ttswitch.v
//  这个根据编写.v模块名字
#include <Vttswitch.h>
// 核心库,用于模拟与波形生成
#include <verilated.h>
#include <verilated_vcd_c.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

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