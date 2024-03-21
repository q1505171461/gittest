#include "ppp-b2b.h"
const char *tracking_modes[NUM_SYSTEMS][NUM_MODES] = {
    {"1I", "1C", "1W", "Reserved", "NAN", "NAN", "Reserved", "2I", "NAN", "Reserved", "Reserved", "Reserved", "3I", "Reserved", "Reserved", "Reserved"},
    {"NAN", "NAN", "Reserved", "Reserved", "1C", "NAN", "Reserved", "2L", "NAN", "Reserved", "Reserved", "5I", "5Q", "NAN", "Reserved", "Reserved"},
    {"1C", "1P", "2C", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"},
    {"Reserved", "1B", "1C", "Reserved", "5aQ", "5aI", "Reserved", "5bI", "5bQ", "Reserved", "Reserved", "6C", "Reserved", "Reserved", "Reserved", "Reserved"}};

void printBinary(unsigned int n, int numBits)
{
    for (int i = numBits - 1; i >= 0; i--)
    {
        printf("%d", (n >> i) & 1);
    }
    printf("\n");
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
uint32_t timeToSeconds(int hour, int minute, int second)
{
    // 假设每天24小时，每小时60分钟，每分钟60秒
    return hour * 3600 + minute * 60 + second;
}

void inputSsr(char (*ssrStr)[MAX_LEN_LINE], en_decodeContext context, Corrections *corrs, int len)
{
    int index_corrs = -1;
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
        int index_tracking_modes = -1;

        if (sscanf(line, "> SSR %d %d %d %d %d %d", &year, &month, &day, &hour, &minute, &second) == 6)
        {
            uint32_t seconds = timeToSeconds(hour, minute, second);
            context.BDT = seconds;
            printf("时间（天内秒）：%d\n", seconds);
            continue;
        }


        // 提取每行的卫星号
        token = strtok(line, " ");
        if (isdigit(token[strlen(token) - 1]))
        {
            index_corrs++;
            memset(corrs+index_corrs, 0, sizeof(corrs[index_corrs]));
            int stanum = 0;
            sscanf(token+1, "%d", &stanum);
            switch (token[0])
            {
            case 'C':
                corrs[index_corrs].SatSlot = stanum + BDS_MASK_BEGIN;
                index_tracking_modes = 0;
                break;
            case 'G':
                corrs[index_corrs].SatSlot = stanum + GPS_MASK_BEGIN;
                index_tracking_modes = 1;
                break;
            case 'E':
                corrs[index_corrs].SatSlot = stanum + Galileo_MASK_BEGIN;
                index_tracking_modes = 2;
                break;
            case 'R':
                corrs[index_corrs].SatSlot = stanum + GLONASS_MASK_BEGIN;
                index_tracking_modes = 3;
                break;
            default:
                break;
            }
            
            printf("卫星号：%s : %d\n", token, corrs[index_corrs].SatSlot);
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
                            orbitalCorrectionParameters *orb = (orbitalCorrectionParameters *)malloc(sizeof(orbitalCorrectionParameters));
                            orb->bdt = context.BDT;
                            orb->IODCorr = context.IODCorr;
                            orb->IODN = context.IODN;
                            orb->SatSlot = corrs[index_corrs].SatSlot;
                            if (orb1 > 26.2128)
                            {
                                orb1 = 26.2128;
                            }
                            if (orb1 < -26.2128)
                            {
                                orb1 = -26.2128;
                            }
                            if (orb2 > 26.208)
                            {
                                orb2 = 26.208;
                            }
                            if (orb2 < -26.208)
                            {
                                orb2 = -26.208;
                            }
                            if (orb3 > 26.208)
                            {
                                orb3 = 26.208;
                            }
                            if (orb3 < -26.208)
                            {
                                orb3 = -26.208;
                            }
                            orb->radialCorr = (int)round(orb1 / 0.0016);
                            orb->tangentialCorr = (int)round(orb2 / 0.0064);
                            orb->normalCorr = (int)round(orb3 / 0.0064);
                            corrs[index_corrs].orbCorr = orb;
                            printBinary(orb->radialCorr, 16);
                            printBinary(orb->tangentialCorr, 16);
                            printBinary(orb->normalCorr, 16);
                            printf("轨道数据：%.3lf %.3lf %.3lf\n", orb1, orb2, orb3);
                            printf("轨道数据：%d %d %d\n", orb->radialCorr, orb->tangentialCorr, orb->normalCorr);
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
                    if (clk > 26.2128)
                    {
                        clk = 26.2128;
                    }
                    if (clk < -26.2128)
                    {
                        clk = -26.2128;
                    }
                    corrs[index_corrs].cloCorr = (int)round(clk / 0.0016);
                    printf("clk：%lf\n", clk);
                    printf("clk：%d\n", corrs[index_corrs].cloCorr);
                }
                break;
            }
            token = strtok(NULL, " ");
        }

        // 提取cbias：后的所有数据
        while (token != NULL)
        {
            if (strstr(token, "cbias:") == NULL)
            {
                token = strtok(NULL, " ");
                continue;
            }
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
            int index_cbias = 0;
            int index_valid_cbias = -1;
            while (token != NULL)
            {
                int cbiasNum = -1;
                if (sscanf(token, "%s", cbiasType[index_cbias]) == 1)
                {
                    for (int j = 0; j < NUM_MODES; j++)
                    {
                        if (strcmp(tracking_modes[index_tracking_modes][j], cbiasType[index_cbias]) == 0)
                        {
                            printf("------------------ok;;cbias：%s: %d\n", cbiasType[index_cbias], j);
                            cbiasNum = j;
                            index_valid_cbias++;
                            break;
                        };
                    }
                    printf("cbias：%s: ", cbiasType[index_cbias]);
                }
                token = strtok(NULL, " ");
                if (sscanf(token, "%lf", &cbiasValue[index_cbias]) == 1)
                {
                    if (cbiasValue[index_cbias] > 35.746)
                    {
                        cbiasValue[index_cbias] = 35.746;
                    }
                    if (cbiasValue[index_cbias] < -35.746)
                    {
                        cbiasValue[index_cbias] = -35.746;
                    }
                    printf("%lf\n", cbiasValue[index_cbias]);
                }
                if (cbiasNum != -1)
                {
                    corrs[index_corrs].cbias[index_valid_cbias].codebiasType = cbiasNum;
                    corrs[index_corrs].cbias[index_valid_cbias].codebiasValue = (int)round(cbiasValue[index_cbias] / 0.017);
                }
                index_cbias++;
                token = strtok(NULL, " ");
            }
            corrs[index_corrs].len_codebias = index_valid_cbias + 1;
            break;
        }
    }
}


void encoding6(Corrections *corrs, int len, CRCCode *crcCode){
    int index_corrs = 0;
    for (int i=0; i<3; i++){
        
    } 
}

void encodingOrb(Corrections *corrs, int len, CRCCode crcCode){
    
}