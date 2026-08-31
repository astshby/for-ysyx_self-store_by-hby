#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

uint8_t PC = 0;
uint8_t R[4];
uint8_t M[16];
uint8_t masks[6] = {0xC0, 0x30, 0x0C, 0x03, 0x0F, 0x3C}; // 掩码数组

void inst_cycle()
{
    uint8_t inst = M[PC++];
    uint8_t opcode = (inst & masks[0]) >> 6;
    switch (opcode)
    {
    case 0:
    { // R-type
        uint8_t rd = (inst & masks[1]) >> 4;
        uint8_t rs1 = (inst & masks[2]) >> 2;
        uint8_t rs2 = inst & masks[3];
        R[rd] = R[rs1] + R[rs2];
        break;
    }
    case 1:
    { // S-type
        uint8_t rd = (inst & masks[1]) >> 4;
        uint8_t op = inst & masks[4];
        switch (op)
        {
        case 0x0:
        {
            printf("R[%d] = %d\n", rd, R[rd]);
        }
        }
        break;
    }
    case 2:
    { // I-type
        uint8_t rd = (inst & masks[1]) >> 4;
        uint8_t imm = inst & masks[4];
        R[rd] = imm;
        break;
    }
    case 3:
    { // B-type
        uint8_t rs2 = inst & masks[3];
        if (R[rs2] != R[0])
        {
            uint8_t addr = (inst & masks[5]) >> 2;
            PC = addr;
        }
        break;
    }
    }
}

int main(int argc, char *argv[])
{
    // 示例初始化,数列循环
    M[0] = 0b10001010;
    M[1] = 0b10010000;
    M[2] = 0b10100000;
    M[3] = 0b10110001;
    M[4] = 0b00010111;
    M[5] = 0b00101001;
    M[6] = 0b11010001;
    M[7] = 0b01100000;
    M[8] = 0b11100011;

    // 执行指令周期
    if (argc > 1)
    {
        int n = atoi(argv[1]);
        assert(n > 0 && n <= 15 && "Error: n must be > 0 and <= 15");

        printf("n=%d,cal:1+2+...+%d\n", n, n);
        M[0] = (M[0] & 0xF0) | (n & 0x0F); // 设置n值
        while (PC < 16)
        {
            inst_cycle();
        }
    }
    else
    {
        printf("n=10,cal:1+2+...+10\n");
        while (PC < 16)
        {
            inst_cycle();
        }
    }

    return 0;
}
