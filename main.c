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

    en_decodeContext context = {0};
    Corrections corrs[indexSsrStr-1];
    inputSsr(lines, context, corrs, indexSsrStr);

    int len_encoded_data = (indexSsrStr-1)%3==0?(indexSsrStr-1)/3:(indexSsrStr-1)/3+1;
    CRCCode encoded_data[len_encoded_data];
    encoding6(corrs, indexSsrStr-1,  encoded_data);

    setBit(&encoded_data, 0, 1);
    setBit(&encoded_data, 33, 1);
    setBit(&encoded_data, 32, 1);

    // 打印每个整数的二进制值
    for (int i = 0; i < STRUCT_SIZE; i++)
    {
        // printf("Integer %d (binary): ", i);
        // printBinary(myStruct.bits[i], 32);
        // printf("\n");
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
