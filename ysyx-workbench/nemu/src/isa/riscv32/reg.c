/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  // 对应PA1：基础设施-简易调试器：打印客户机PC和全部有效通用寄存器，RV32E只包含x0-x15。
  int nr_gpr = MUXDEF(CONFIG_RVE, 16, 32);
  printf("%-7s " FMT_WORD "\n", "pc", cpu.pc);
  for (int i = 0; i < nr_gpr; i ++) {
    printf("x%-2d %-3s " FMT_WORD "%s", i, reg_name(i), gpr(i),
        i % 2 == 1 ? "\n" : "    ");
  }
  if (nr_gpr % 2 != 0) {
    putchar('\n');
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  return 0;
}
