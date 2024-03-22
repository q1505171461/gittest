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
        if ((numBits - 1 - i) % 8 == 0 && (numBits - 1 - i) != 0)
        {
            printf(" ");
        }
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

void setBits(CRCCode *crcCode, int bitIndexBegin, int bitIndexEnd, uint64_t value)
{
    for (int i = 0; i < bitIndexEnd - bitIndexBegin; i++)
    {
        // printf("%d:%d ", i,(value >> i) & 1);
        setBit(crcCode, i + bitIndexBegin, i > 62 ? 0 : ((value >> i) & 1));
    }
}

// 函数：将时间转换为天内秒
uint32_t timeToSeconds(int hour, int minute, int second)
{
    // 假设每天24小时，每小时60分钟，每分钟60秒
    return hour * 3600 + minute * 60 + second;
}

// 比较函数，根据SatSlot进行排序
int compare(const void *a, const void *b)
{
    Corrections *correctionsA = (Corrections *)a;
    Corrections *correctionsB = (Corrections *)b;

    return correctionsA->SatSlot - correctionsB->SatSlot;
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
            if (DEBUG){
                printf("时间（天内秒）：%d\n", seconds);
            }
            
            continue;
        }

        // 提取每行的卫星号
        token = strtok(line, " ");
        if (isdigit(token[strlen(token) - 1]))
        {
            index_corrs++;
            memset(corrs + index_corrs, 0, sizeof(corrs[index_corrs]));
            int stanum = 0;
            sscanf(token + 1, "%d", &stanum);
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
            corrs[index_corrs].bdt = context.BDT;
            corrs[index_corrs].IODCorr = context.IODCorr;
            corrs[index_corrs].IODN = context.IODN;
            corrs[index_corrs].IODP = context.IODP;
            corrs[index_corrs].IODSSR = context.IODSSR;
            if (DEBUG){
                printf("卫星号：%s : %d\n", token, corrs[index_corrs].SatSlot);
            }
            
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
                            if (DEBUG)
                            {
                                printBinary(orb->radialCorr, 16);
                                printBinary(orb->tangentialCorr, 16);
                                printBinary(orb->normalCorr, 16);
                                printf("轨道数据：%.3lf %.3lf %.3lf\n", orb1, orb2, orb3);
                                printf("轨道数据：%d %d %d\n", orb->radialCorr, orb->tangentialCorr, orb->normalCorr);
                            }

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
                    if (DEBUG){
                        printf("clk：%lf\n", clk);
                        printf("clk：%d\n", corrs[index_corrs].cloCorr);
                    }
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
                            if (DEBUG){
                                printf("------------------ok;;cbias：%s: %d\n", cbiasType[index_cbias], j);
                            }
                            cbiasNum = j;
                            index_valid_cbias++;
                            break;
                        };
                    }
                    if (DEBUG){
                        printf("cbias：%s: ", cbiasType[index_cbias]);
                    }
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
                    if(DEBUG){
                        printf("%lf\n", cbiasValue[index_cbias]);
                    }
                    
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
    qsort(corrs, index_corrs+1, sizeof(Corrections), compare);
}

void encoding6(Corrections *corrs, int len, CRCCode *encoded_data, int len_encoded_data)
{
    int index_corrs = 0;
    for (int i = 0; i < len_encoded_data; i++)
    {
        memset(encoded_data + i, 0, sizeof(CRCCode));
        // if (i ==11){
        //     printf("%d", corrs[i * 3].bdt);
        // }
        // MesTypeID
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 6, MAX_LEN_CRCMESSAGE, 6);
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 14 - 17, MAX_LEN_CRCMESSAGE - 14, corrs[i * 3].bdt);
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 35 - 2, MAX_LEN_CRCMESSAGE - 35, corrs[i * 3].IODSSR);
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 37 - 4, MAX_LEN_CRCMESSAGE - 37, corrs[i * 3].IODP);
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 41 - 9, MAX_LEN_CRCMESSAGE - 41, corrs[i * 3].SatSlot);

        int index_orb_begin = -1;
        for (int j = 0; j < 3; j++)
        { // 每条消息3组轨道钟差改正数
            if (i * 3 + j < len)
            {
                index_orb_begin = 50 + 18 * (j + 1);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 6 - 5, MAX_LEN_CRCMESSAGE - 6, j + 1);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 11 - 3, MAX_LEN_CRCMESSAGE - 11, j + 1);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 50 - 3 - 18 * j, MAX_LEN_CRCMESSAGE - 50 - 18 * j, corrs[i * 3 + j].IODCorr);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - 53 - 15 - 18 * j, MAX_LEN_CRCMESSAGE - 53 - 18 * j, corrs[i * 3 + j].cloCorr);
            }
        }

        // orb
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 17, MAX_LEN_CRCMESSAGE - index_orb_begin, corrs[i * 3].bdt);
        setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 21 - 2, MAX_LEN_CRCMESSAGE - index_orb_begin - 21, corrs[i * 3].IODSSR);
        for (int j = 0; j < 3; j++)
        { // 每条消息3组轨道钟差改正数
            if (i * 3 + j < len)
            {
                index_orb_begin += 69 * j;
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 23 - 9, MAX_LEN_CRCMESSAGE - index_orb_begin - 23, corrs[i * 3 + j].SatSlot);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 32 - 10, MAX_LEN_CRCMESSAGE - index_orb_begin - 32, corrs[i * 3 + j].IODN);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 42 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 42, corrs[i * 3 + j].IODCorr);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 45 - 15, MAX_LEN_CRCMESSAGE - index_orb_begin - 45, corrs[i * 3 + j].orbCorr->radialCorr);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 60 - 13, MAX_LEN_CRCMESSAGE - index_orb_begin - 60, corrs[i * 3 + j].orbCorr->tangentialCorr);
                setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 73 - 13, MAX_LEN_CRCMESSAGE - index_orb_begin - 73, corrs[i * 3 + j].orbCorr->normalCorr);
                if (corrs[i * 3 + j].ural){
                    setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 86 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 86, corrs[i * 3 + j].ural->URAClass);
                    setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 89 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 89, corrs[i * 3 + j].ural->URAValue);
                }else{
                    setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 86 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 86, 0);
                    setBits(encoded_data + i, MAX_LEN_CRCMESSAGE - index_orb_begin - 89 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 89, 0);
                }
                
            }
        }
    }
}
