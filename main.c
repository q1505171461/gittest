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

    // Encoding1 卫星掩码
    CRCCode encoded_data1 = {0};
    encoding1(corrs, indexSsrStr - 1, &encoded_data1);
    print_encoded_data(encoded_data1);

    // Encoding3 码间偏差改正数
    int n3_used = 0;
    while (n3_used < indexSsrStr - 1)
    {
        CRCCode encoded_data = {0};
        n3_used += encoding3(corrs + n3_used, indexSsrStr - 1 - n3_used, &encoded_data);
        print_encoded_data(encoded_data);
        decoding3(corrs, indexSsrStr - 1, &encoded_data);
    }

    // Encoding6 钟差改正数与轨道改正数-组合 1
    int n_used = 0;
    while (n_used < indexSsrStr - 1)
    {
        CRCCode encoded_data = {0};
        n_used += encoding6(corrs + n_used, indexSsrStr - 1 - n_used, &encoded_data);
        // print_encoded_data(encoded_data);
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
