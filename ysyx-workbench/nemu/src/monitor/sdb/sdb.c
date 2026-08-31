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
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <ctype.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}
static char *skip_spaces(char *s) {
  // 跳过空格与制表符，返回第一个非空格字符的指针,单独列出处理下放字符
  while (s != NULL && isspace((unsigned char)*s)) {
    s ++;
  }
  return s;
}

static bool parse_uint64(char *text, int base, uint64_t *value, char **end) {
  // 用于解析用户输入的数字，保持64位，拒绝负数、0、非数字和溢出
  char *start = skip_spaces(text);
  if (start == NULL || *start == '\0' || *start == '-') {
    return false;
  }

  errno = 0;
  char *tail = NULL;
  unsigned long long result = strtoull(start, &tail, base);
  if (start == tail || errno == ERANGE) {
    return false;
  }

  *value = result;
  *end = tail;
  return true;
}

static int cmd_si(char *args) {
  // 对应PA1：cpu_exec()负责单步循环、指令跟踪以及NEMU状态转换。
  uint64_t n = 1;
  if (args != NULL) {
    char *end = NULL;
    if (!parse_uint64(args, 10, &n, &end) || n == 0 || *skip_spaces(end) != '\0') {
      printf("Usage: si [N] (N must be a positive integer)\n");
      return 0;
    }
  }

  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  // 对应PA1：基础设施-简易调试器：只分发命令，寄存器的ISA相关布局由isa_reg_display()处理。
  char *subcmd = skip_spaces(args);
  if (subcmd != NULL && subcmd[0] == 'r' && *skip_spaces(subcmd + 1) == '\0') {
    isa_reg_display();
    return 0;
  }

  printf("Usage: info r\n");
  return 0;
}

static int cmd_x(char *args) {
  // 对应PA1：基础设施-简易调试器：通过客户虚拟地址接口读取内存，当前先把数字地址作为最简表达式。
  uint64_t n = 0;
  char *addr_str = NULL;
  if (!parse_uint64(args, 10, &n, &addr_str) || n == 0) {
    printf("Usage: x N ADDR (N must be a positive integer)\n");
    return 0;
  }

  uint64_t addr_value = 0;
  char *end = NULL;
  if (!parse_uint64(addr_str, 0, &addr_value, &end) || *skip_spaces(end) != '\0' ||
      addr_value > (uint64_t)(vaddr_t)-1) {
    printf("Usage: x N ADDR (ADDR must be a valid numeric address)\n");
    return 0;
  }

  vaddr_t start = (vaddr_t)addr_value;
  uint64_t max_words = ((uint64_t)(vaddr_t)-1 - start) / 4 + 1;
  if (n > max_words) {
    printf("Memory range wraps around the virtual address space\n");
    return 0;
  }

  for (uint64_t i = 0; i < n; i ++) {
    vaddr_t addr = start + i * 4;
    printf(FMT_WORD ": " FMT_WORD "\n", addr, vaddr_read(addr, 4));
  }
  return 0;
}



static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;   //对应pa1，RTFSC的问题：优雅的退出
  printf("Bye.\n");
  return -1;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute N instructions (default: 1)", cmd_si },
  { "info", "Display register information (usage: info r)", cmd_info },
  { "x", "Examine N words of memory starting at ADDR", cmd_x },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
