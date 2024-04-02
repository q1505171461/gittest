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
    context.IODN = 1;     /* 需要告知版本号 */
    context.IODP = 1;
    context.IODSSR = 1;  /* 需要告诉当前用的是哪个轨道 */
    Corrections corrs[indexSsrStr - 1];
    inputSsr(lines, &context, corrs, indexSsrStr);

    // Encoding1 卫星掩码
    CRCCode encoded_data1 = {0};
    encoding1(corrs, indexSsrStr - 1, &encoded_data1);
    print_encoded_data(encoded_data1);

    int sta_list[MAX_NUM_STA]={0};
    en_decodeContext context_decode = {0};
    int len_sta_list = decoding1(sta_list, &encoded_data1, &context_decode);
    Corrections corrs_decoded[len_sta_list];
    for (int i = 0; i < len_sta_list; i++){
        memset(corrs_decoded + i, 0, sizeof(Corrections));
        corrs_decoded[i].bdt = context_decode.BDT;
        corrs_decoded[i].SatSlot = sta_list[i];
        corrs_decoded[i].IODSSR = context_decode.IODSSR;
        corrs_decoded[i].IODP = context_decode.IODP;
    }

    // Encoding3 码间偏差改正数
    int n3_used = 0;
    while (n3_used < indexSsrStr - 1)
    {
        CRCCode encoded_data = {0};
        n3_used += encoding3(corrs + n3_used, indexSsrStr - 1 - n3_used, &encoded_data);
        print_encoded_data(encoded_data);
        decoding3(corrs_decoded, len_sta_list, &encoded_data, &context_decode);
    }
 
    // Encoding6 钟差改正数与轨道改正数-组合 1
    int n_used = 0;
    while (n_used < indexSsrStr - 1)
    {
        CRCCode encoded_data = {0};
        n_used += encoding6(corrs + n_used, indexSsrStr - 1 - n_used, &encoded_data);
        print_encoded_data(encoded_data);
        decoding6(corrs_decoded, len_sta_list, &encoded_data, &context_decode);
    }
    return 0;
}
