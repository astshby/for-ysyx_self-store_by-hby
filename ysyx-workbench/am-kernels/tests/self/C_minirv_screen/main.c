#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <am.h>
#include <klib.h>
#include <klib-macros.h>
// 已知：logisim里面的地址最大为2^24 * 4B，也就是2^26B,而且经过映射
// vga的地址是0x20000000开始，所以内存不经过映射不可能少于0x20000000,所以开下4GB内存
// 其余的屏幕，存储，不需要这么大

#define ROM_SIZE_BYTES (1024 * 1024)               // 1 MB
#define RAM_SIZE_BYTES (1024ULL * 1024 * 1024 * 4) // 4 GB
// uint:模拟硬件是模拟器寄存器和内存，没有正负概念，而且必须允许符号溢出，保证位运算安全性
// 4GB内存炸栈，用堆空间
// 调试 ： 1天，原因：炸我栈
uint32_t Register[32];  // Simulated registers，注意，R[0]始终为0
uint8_t *Memory = NULL; // Simulated ROM (1MB)
uint8_t *ROM = NULL;    // Simulated ROM (1MB)
uint32_t PC = 0;        // Program Counter

// 屏幕写入：由于屏幕是0x20000000 到 0x20040000，一共有256*256个像素点，必须也要完全存储
// 根据已知，RAB的每个像素点是24位，但是为了简化占用32位
// 根据am，只需要设定screen数组,然后设定(高*宽)即可
uint32_t screen[256 * 256]; // VGA Screen Buffer
bool is_running = true;

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
            if (addr >= 0x20000000 && addr < 0x20040000)
            {
                // VGA 显存范围,应该减去基地址0x20000000,并且直接存入32位
                uint32_t vga_offset = addr - 0x20000000;
                screen[vga_offset / 4] = Register[rs2]; // 每个像素点占4字节
            }
            else
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
        // uint32_t rd = (inst >> 7) & 0x1F;
        // uint32_t rs1 = (inst >> 15) & 0x1F;
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
                // exit(0);
                is_running = false;
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

/*
// 打印寄存器状态 ,辅助验证
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
}
*/

int main(int argc, char *argv[])
{
    Memory = (uint8_t *)malloc(RAM_SIZE_BYTES);
    if (Memory == NULL)
    {
        perror("Failed to allocate 4GB memory");
        return 1;
    }
    ROM = (uint8_t *)malloc(ROM_SIZE_BYTES);
    if (ROM == NULL)
    {
        perror("Failed to allocate 1MB ROM");
        free(Memory);
        return 1;
    }
    const char *filename = "./vga.bin";

    // 1. 打开文件
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Failed to open binary file");
        free(Memory);
        free(ROM);
        return 1;
    }
    // 2. 获取文件大小
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    printf("File size: %d bytes\n", file_size);
    // 3. 检查文件是否超过内存(RAM)总大小
    if ((unsigned long long)file_size > RAM_SIZE_BYTES)
    {
        fprintf(stderr, "Error: Binary file is too large for RAM (%d bytes)\n", file_size);
        fclose(file);
        return 1;
    }
    // 4. 读取文件内容到模拟内存 (Memory)
    // 注意：这里只读取实际文件的大小，而不是整个 MEM_SIZE
    size_t bytesRead = fread(Memory, 1, (size_t)file_size, file);
    fclose(file);
    if (bytesRead != (size_t)file_size)
    {
        fprintf(stderr, "Warning: Expected %d bytes but read %zu bytes\n", file_size, bytesRead);
    }
    printf("Loaded %zu bytes into Memory (RAM).\n", bytesRead);
    // 5. 将内容复制到 ROM
    // 关键点：必须检查文件大小是否超过 ROM 大小，防止溢出
    size_t rom_copy_size = bytesRead;
    if (rom_copy_size > ROM_SIZE_BYTES)
    {
        printf("Warning: File size (%zu) is larger than ROM size (%d).\n", rom_copy_size, ROM_SIZE_BYTES);
        printf("         Truncating data copied to ROM.\n");
        rom_copy_size = ROM_SIZE_BYTES; // 只拷贝 ROM 能装下的部分
    }
    // 安全复制
    memcpy(ROM, Memory, rom_copy_size);
    printf("Copied %zu bytes from Memory to ROM.\n", rom_copy_size);

    // 6.添加结束指令ebreak
    // am本质不需要，可以把这个写在上一个循环里面，此处用这个
    // 能修改的原因：默认执行halt就一直在halt，所以更改里面的一条指令即可
    uint32_t ebreak_inst = 0x00100073; // ebreak指令编码
    uint32_t halt_addr = 0xdb0;        // 具体位置具体分析
    *(uint32_t *)&ROM[halt_addr] = ebreak_inst;

    /*
        // 调试
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
            if (PC >= RAM_SIZE_BYTES)
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
    while (is_running)
    {
        inst_cycle();
        if (PC > RAM_SIZE_BYTES)
        {
            printf("Program terminated: PC out of bounds or cycle limit reached.\n");
            break;
        }
    }

    // VGA屏幕显示部分
    //  初始化 AM
    ioe_init();

    // 获取 AM 窗口大小 (宽度和高度)
    // AM_GPU_CONFIG_T config = io_read(AM_GPU_CONFIG);
    int w = 256;
    int h = 256;

    // 将模拟器里填好的 screen 数组，一次性画到屏幕上
    // 注意：这里假设 screen 的数据量足够填充 w*h，且 vga 程序就是按这个分辨率画的
    io_write(AM_GPU_FBDRAW, 0, 0, screen, w, h, true);

    // 4. 事件监听循环
    while (1)
    {
        // 读取按键信息
        AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);

        // 只有在按下 ESC 且是按下动作(keydown)时才退出
        if (ev.keycode == AM_KEY_ESCAPE && ev.keydown)
        {
            halt(0); // AM 的退出函数
        }

        // 其他按键不做处理，保持画面显示
    }

    return 0;
}
