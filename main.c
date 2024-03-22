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
        indexSsrStr++;
    }

    fclose(file); // 关闭文件

    en_decodeContext context = {0};
    context.IODCorr = 1;
    context.IODN = 1;
    context.IODP = 1;
    context.IODSSR = 1;
    Corrections corrs[indexSsrStr - 1];
    inputSsr(lines, context, corrs, indexSsrStr);

    int len_encoded_data = (indexSsrStr - 1) % 3 == 0 ? (indexSsrStr - 1) / 3 : (indexSsrStr - 1) / 3 + 1;
    CRCCode encoded_data[len_encoded_data];
    encoding6(corrs, indexSsrStr - 1, encoded_data, len_encoded_data);

    // 打印每个整数的二进制值
    for (int j = 0; j < len_encoded_data; j++)
    {
        printf("uint32_t %3d: ", 15 * 32 + 6);
        printBinary(encoded_data[j].bits[15], 6);
        for (int i = 0; i < STRUCT_SIZE - 1; i++)
        {
            printf("uint32_t %3d: ", (STRUCT_SIZE - 1 - i)* 32);
            printBinary(encoded_data[j].bits[STRUCT_SIZE - 2 - i], 32);
            // printf("\n");
        }
        printf("\n");
        // break;
    }

    // 打印字符串数组内容
    // printf("信号和跟踪模式标识：\n");
    // for (int i = 0; i < NUM_SYSTEMS; i++)
    // {
    //     printf("%s:\n", i == 0 ? "BDS" : (i == 1 ? "GPS" : (i == 2 ? "GLONASS" : "Galileo")));
    //     for (int j = 0; j < NUM_MODES; j++)
    //     {
    //         printf("%s ", tracking_modes[i][j]);
    //     }
    //     printf("\n");
    // }
    return 0;
}
