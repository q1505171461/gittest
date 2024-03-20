#include <stdio.h>
#include "ppp-b2b.h"
#include <ctype.h>

#include <string.h>
#include <stdlib.h>


int main()
{
    int indexSsrStr = 0;
    
    char lines[MAX_NUM_STA][MAX_LEN_LINE];
    FILE *file = fopen("ssr.txt", "r"); // 打开文件
    if (file == NULL)
    {
        printf("无法打开文件\n");
        return 1;
    }

    while (fgets(lines[indexSsrStr], MAX_LEN_LINE, file))
    {
        indexSsrStr ++;
    }

    fclose(file); // 关闭文件

    Corrections corrs[indexSsrStr];
    inputSsr(lines, corrs, indexSsrStr);

    CRCCode myStruct = {0};

    setBit(&myStruct, 0, 1);
    setBit(&myStruct, 33, 1);
    setBit(&myStruct, 32, 1);
    // 打印每个整数的二进制值
    for (int i = 0; i < STRUCT_SIZE; i++)
    {
        // printf("Integer %d (binary): ", i);
        // printBinary(myStruct.bits[i], 32);
        // printf("\n");
    }
#define NUM_SYSTEMS 4
#define NUM_MODES 16
    const char *tracking_modes[NUM_SYSTEMS][NUM_MODES] = {
        {"1I", "1C", "1W", "Reserved", "NAN", "NAN", "Reserved", "2I", "NAN", "Reserved", "Reserved", "Reserved", "3I", "Reserved", "Reserved", "Reserved"},
        {"NAN", "NAN", "Reserved", "Reserved", "1C", "NAN", "Reserved", "2L", "NAN", "Reserved", "Reserved", "5I", "5Q", "NAN", "Reserved", "Reserved"},
        {"1C", "1P", "2C", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"},
        {"Reserved", "1B", "1C", "Reserved", "5aQ", "5aI", "Reserved", "5bI", "5bQ", "Reserved", "Reserved", "6C", "Reserved", "Reserved", "Reserved", "Reserved"}
    };

    // 打印字符串数组内容
    printf("信号和跟踪模式标识：\n");
    for (int i = 0; i < NUM_SYSTEMS; i++)
    {
        printf("%s:\n", i == 0 ? "BDS" : (i == 1 ? "GPS" : (i == 2 ? "GLONASS" : "Galileo")));
        for (int j = 0; j < NUM_MODES; j++)
        {
            printf("%s ", tracking_modes[i][j]);
        }
        printf("\n");
    }
    return 0;
}
