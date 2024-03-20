#include "ppp-b2b.h"

void printBinary(unsigned int n, int numBits)
{
    for (int i = numBits - 1; i >= 0; i--)
    {
        printf("%d", (n >> i) & 1);
    }
}

void setBit(CRCCode *crcCode, int bitIndex, int value)
{
    if (value)
    {
        *(((unsigned int *)crcCode) + bitIndex / 32) |= (1 << (bitIndex % 32));
    }
    else
    {
        *(((unsigned int *)crcCode) + bitIndex / 32) &= ~(1 << (bitIndex % 32));
    }
}

// 函数：将时间转换为天内秒
double timeToSeconds(int hour, int minute, int second)
{
    // 假设每天24小时，每小时60分钟，每分钟60秒
    return hour * 3600 + minute * 60 + second;
}

void inputSsr(char (*ssrStr)[MAX_LEN_LINE], Corrections corrs[], int len)
{
    orbitalCorrectionParameters a = {0};
    // corrs[1].orbCorr =  &a;
    for (int i = 0; i < len; i++)
    {
        char *line = ssrStr[i];
        int year, month, day, hour, minute, second;
        char *satellite;
        int satellitenum;
        double orb1, orb2, orb3, clk;
        char cbiasType[16][10]; // 假设cbias后最多有20个数据
        double cbiasValue[16];  // 假设cbias后最多有20个数据
        char *token;

        // printf("%s", line);
        if (sscanf(line, "> SSR %d %d %d %d %d %d", &year, &month, &day, &hour, &minute, &second) == 6)
        {
            double seconds = timeToSeconds(hour, minute, second);
            printf("时间（天内秒）：%lf\n", seconds);
            continue;
        }

        // 提取每行的"G\d\d",orb: 后的前三组数据
        token = strtok(line, " ");
        if ((strncmp(token, "G", 1) == 0 || strncmp(token, "E", 1) == 0 || strncmp(token, "C", 1) == 0 || strncmp(token, "R", 1) == 0) && isdigit(token[strlen(token) - 1]))
        {
            printf("卫星号：%s \n", token);
        }
        else
        {
            continue;
        }
        token = strtok(NULL, " ");
        while (token != NULL)
        {
            if (strcmp(token, "orb:") == 0)
            {
                token = strtok(NULL, " ");
                if (token != NULL && sscanf(token, "%lf", &orb1) == 1)
                {
                    token = strtok(NULL, " ");
                    if (token != NULL && sscanf(token, "%lf", &orb2) == 1)
                    {
                        token = strtok(NULL, " ");
                        if (token != NULL && sscanf(token, "%lf", &orb3) == 1)
                        {
                            printf("轨道数据：%.3lf %.3lf %.3lf\n", orb1, orb2, orb3);
                            break;
                        }
                    }
                }
            }
            token = strtok(NULL, " ");
        }

        // 提取clk后的第一个数据
        while (token != NULL)
        {
            if (strstr(token, "clk:") != NULL)
            {
                token = strtok(NULL, " ");
                if (sscanf(token, "%lf", &clk) == 1)
                {
                    printf("clk：%lf\n", clk);
                }
                break;
            }
            token = strtok(NULL, " ");
        }

        // 提取cbias：后的所有数据
        while (token != NULL)
        {
            // break;
            if (strstr(token, "cbias:") != NULL)
            {
                token = strtok(NULL, " ");
                token = strtok(NULL, " ");
                int i = 0;
                while (token != NULL)
                {
                    if (sscanf(token, "%s", cbiasType[i]) == 1)
                    {
                        printf("cbias：%s: ", cbiasType[i]);
                    }
                    token = strtok(NULL, " ");
                    if (sscanf(token, "%lf", &cbiasValue[i]) == 1)
                    {
                        printf("%lf\n", cbiasValue[i]);
                    }
                    i++;
                    token = strtok(NULL, " ");
                }

                break;
            }
            token = strtok(NULL, " ");
        }
    }
}

