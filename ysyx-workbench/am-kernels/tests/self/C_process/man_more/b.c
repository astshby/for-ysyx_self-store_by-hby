#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define 主函数 main
#define 返回 return

char *字符串拼接(char *串1, char *串2)
{
    char *新串 = malloc(strlen(串1) + strlen(串2) + 1);
    assert(新串);
    strcpy(新串, 串1);
    strcat(新串, 串2);
    返回 新串;
}

int 主函数()
{
    char *信息 = 字符串拼接("一生一芯", "很简单");
    printf("%s\n", 信息);
    free(信息);
    返回 0;
}