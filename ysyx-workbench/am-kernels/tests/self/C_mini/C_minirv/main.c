#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MEM_SIZE (1024 * 1024) // 1MB

// uint:模拟硬件是模拟器寄存器和内存，没有正负概念，而且必须允许符号溢出，保证位运算安全性
uint32_t Register[32];    // Simulated registers，注意，R[0]始终为0
uint8_t Memory[MEM_SIZE]; // Simulated RAM (1MB)
uint8_t ROM[MEM_SIZE];    // Simulated ROM (1MB)
uint32_t PC = 0;          // Program Counter

void inst_cycle()
{
    // Fetch instruction
    uint32_t inst = *(uint32_t *)&ROM[PC];
    PC += 4;
    Register[0] = 0; // 保证R[0]始终为0

    // Decode and execute instruction
    // 1f: 00011111,7: 00000111,7f: 01111111
    uint32_t opcode = inst & 0x7F;
    switch (opcode)
    {
    case 0x33:
    { // OP
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        uint32_t rs2 = (inst >> 20) & 0x1F;
        uint32_t funct7 = (inst >> 25) & 0x7F;

        if (funct3 == 0x0 && funct7 == 0x00)
        { // ADD
            // 就算是加法溢出也无所谓，模拟器寄存器是uint32_t类型，溢出后高位丢弃，符合硬件行为
            // 而且硬件也是用这个处理算数的（不然要补码干啥）
            Register[rd] = Register[rs1] + Register[rs2];
        }
        break;
    }

    case 0x13:
    { // OP-IMM
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        // 左移：后面补0，右移：有算术（补符号位）扩展与位扩展（补0）
        // 符号扩展：uint默认是0扩展，左移后前面为0,之后转换为int32，左移后右移补充符号位而不是0,最后再转换uint（保证统一）
        int32_t imm_12 = inst >> 20;           // sign-extend if necessary
        int32_t imm_32 = (imm_12 << 20) >> 20; // sign-extend to 32 bits
        uint32_t imm = (uint32_t)imm_32;       // sign-extend to 32 bits

        if (funct3 == 0x0)
        { // ADDI
            Register[rd] = Register[rs1] + imm;
        }
        break;
    }

    case 0x37:
    { // LUI
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t imm = inst & 0xFFFFF000;
        Register[rd] = imm;
        break;
    }

    case 0x67:
    { // JALR
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        int32_t imm_12 = inst >> 20;           // sign-extend if necessary
        int32_t imm_32 = (imm_12 << 20) >> 20; // sign-extend to 32 bits
        uint32_t imm = (uint32_t)imm_32;       // sign-extend to 32 bits

        if (funct3 == 0x0)
        {
            int temp = PC;
            PC = (Register[rs1] + imm) & ~1;
            Register[rd] = temp;
        }
        break;
    }

    case 0x03:
    { // LOAD
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        int32_t imm_12 = inst >> 20;           // sign-extend if necessary
        int32_t imm_32 = (imm_12 << 20) >> 20; // sign-extend to 32 bits
        uint32_t imm = (uint32_t)imm_32;       // sign-extend to 32 bits
        if (funct3 == 0x2)
        { // LW
            uint32_t addr = Register[rs1] + imm;
            Register[rd] = *(uint32_t *)&Memory[addr];
        }
        else if (funct3 == 0x4)
        { // LBU
            uint32_t addr = Register[rs1] + imm;
            Register[rd] = Memory[addr]; // zero-extend,左值转换的特性
        }
        break;
    }

    case 0x23:
    { // STORE
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        uint32_t rs2 = (inst >> 20) & 0x1F;
        int32_t imm_12 = ((inst >> 7) & 0x1F) | ((inst >> 25) << 5); // sign-extend if necessary
        int32_t imm_32 = (imm_12 << 20) >> 20;
        uint32_t imm = (uint32_t)imm_32; // sign-extend to 32 bits
        if (funct3 == 0x2)
        { // SW
            uint32_t addr = Register[rs1] + imm;
            *(uint32_t *)&Memory[addr] = Register[rs2];
        }
        else if (funct3 == 0x0)
        { // SB
            uint32_t addr = Register[rs1] + imm;
            Memory[addr] = Register[rs2] & 0xFF;
        }
        break;
    }

    case 0x73:
    { // SYSTEM指令（包含ebreak，ecall等）
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        uint32_t imm_12 = inst >> 20;

        if (funct3 == 0x0)
        {
            if (imm_12 == 0x1)
            {
                // ebreak
                // 在ysyx中，判断a0寄存器的值决定是打印还是退出
                if (Register[10] == 0)
                {
                    printf("\033[1;32mHIT GOOD TRAP\033[0m\n"); // 绿色高亮
                }
                else
                {
                    printf("\033[1;31mHIT BAD TRAP\033[0m\n"); // 红色高亮
                }
                exit(0);
            }
            else if (imm_12 == 0x0)
            {
                // ecall
                printf("ECALL called. Exiting.\n");
                exit(0);
            }
        }
        break;
    }
    default:
    {
        printf("Unknown instruction with opcode: 0x%02X at PC: 0x%08X\n", opcode, PC - 4);
        exit(1);
        break;
    }
    }
}

/* 打印寄存器状态 ,辅助验证
void sprint_regs(char *buf)
{
    char temp[32];
    buf[0] = '\0';
    for (int i = 0; i < 32; i++)
    {
        sprintf(temp, "x%02d:%08x ", i, Register[i]);
        strcat(buf, temp);
        if ((i + 1) % 8 == 0)
            strcat(buf, "\n      "); // 每8个换一行，方便查看
    }
}*/

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    // 1. 读取二进制文件到模拟内存
    FILE *file = fopen(argv[1], "rb");
    if (!file)
    {
        perror("Failed to open binary file");
        return 1;
    }
    // 2. 检查文件大小
    fseek(file, 0, SEEK_END);
    long img_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (img_size > sizeof(Memory))
    {
        fprintf(stderr, "Error: Binary file is too large (%ld bytes)\n", img_size);
        fclose(file);
        return 1;
    }
    // 3. 读取文件内容到内存与ROM
    size_t bytesRead = fread(Memory, 1, MEM_SIZE, file);
    fclose(file);
    printf("Loaded %zu bytes into memory.\n", bytesRead);
    memcpy(ROM, Memory, MEM_SIZE);

    // 4.添加结束指令ebreak
    // 能修改的原因：默认执行halt就一直在halt，所以更改里面的一条指令即可
    uint32_t ebreak_inst = 0x00100073; // ebreak指令编码
    uint32_t halt_addr = 0x1218;       // 具体位置具体分析
    switch (argv[1][15])
    {
    case 'm':
        halt_addr = 0x1218;
        break; // mem.bin
    case 's':
        halt_addr = 0x224;
        break; // sum.bin
    case 'v':
        halt_addr = 0xdb0;
        break; // vga.bin
    default:
        break;
    }
    // 此处字节序与平台相关为小端序，也就是内存低位存放指令低位，与人类习惯相反
    // 此处暂时只存在ROM上
    *(uint32_t *)&ROM[halt_addr] = ebreak_inst;

    /*
      // --- 日志文件准备 ---
        FILE *log = fopen("trace.txt", "w");
        if (!log)
        {
            perror("Log file open failed");
            return 1;
        }

        char reg_buf_before[1024];
        char reg_buf_after[1024];

        printf("Starting simulation... Logging to trace.txt\n");

        Register[0] = 0;
        for (int i = 0; i < 10000; i++) // 只跑10000次
        {
            if (PC >= MEM_SIZE)
            {
                fprintf(log, "PC out of bounds at cycle %d\n", i);
                break;
            }

            // 1. 记录开始前的 PC 和 寄存器
            uint32_t start_pc = PC;
            sprint_regs(reg_buf_before);

            // 2. 获取当前指令十六进制
            uint32_t inst = *(uint32_t *)&Memory[PC];

            // 3. 执行指令
            inst_cycle();

            // 4. 记录执行后的 PC 和 寄存器
            uint32_t end_pc = PC;
            sprint_regs(reg_buf_after);

            // 5. 写入文件
            fprintf(log, "Cycle: %d\n", i);
            fprintf(log, "  Exec Inst: [0x%08x] -> 0x%08x\n", start_pc, inst);
            fprintf(log, "  Regs Before: %s\n", reg_buf_before);
            fprintf(log, "  Regs After:  %s\n", reg_buf_after);
            fprintf(log, "  Next PC: 0x%08x\n", end_pc);
            fprintf(log, "----------------------------------------------------------\n");

            // 如果执行了 ebreak 导致 exit，这里就写不到了，
            // 但如果程序是在死循环，你会看到 trace.txt 里的 PC 在重复。
        }

        fclose(log);
        printf("Done. Please check trace.txt\n");
    */

    // 主要执行循环
    Register[0] = 0;
    while (1)
    {
        inst_cycle();
        if (PC > MEM_SIZE)
        {
            printf("Program terminated: PC out of bounds or cycle limit reached.\n");
            break;
        }
    }

    return 0;
}

// 补充：
// 为minirv添加屏幕暂时忽略，D阶段再看看，需要用AM
// vga没有显示屏，所以有问题