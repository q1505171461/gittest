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
    // printf("\n");
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

uint64_t getBit(CRCCode *crcCode, int bitIndex)
{
    return (*(((unsigned int *)crcCode) + bitIndex / 32) & (1 << (bitIndex % 32))) == 0 ? 0 : 1;
}

uint64_t getBits(CRCCode *crcCode, int bitIndexBegin, int bitIndexEnd)
{
    uint32_t ret = 0;
    for (int i = 0; i < bitIndexEnd - bitIndexBegin; i++)
    {
        int a = getBit(crcCode, i + bitIndexBegin);
        ret |= (getBit(crcCode, i + bitIndexBegin) << i);
    }
    return ret;
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

void inputSsr(char (*ssrStr)[MAX_LEN_LINE], en_decodeContext *context, Corrections *corrs, int len)
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
            context->BDT = seconds;
            if (DEBUG)
            {
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
            corrs[index_corrs].bdt = context->BDT;
            corrs[index_corrs].IODCorr = context->IODCorr;
            corrs[index_corrs].IODN = context->IODN;
            corrs[index_corrs].IODP = context->IODP;
            corrs[index_corrs].IODSSR = context->IODSSR;
            if (DEBUG)
            {
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
                            corrs[index_corrs].IODCorr = context->IODCorr;
                            corrs[index_corrs].IODN = context->IODN;
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
                            corrs[index_corrs].radialCorr = (int)round(orb1 / 0.0016);
                            corrs[index_corrs].tangentialCorr = (int)round(orb2 / 0.0064);
                            corrs[index_corrs].normalCorr = (int)round(orb3 / 0.0064);
                            if (DEBUG)
                            {
                                printBinary(corrs[index_corrs].radialCorr, 16);
                                printBinary(corrs[index_corrs].tangentialCorr, 16);
                                printBinary(corrs[index_corrs].normalCorr, 16);
                                printf("轨道数据：%.3lf %.3lf %.3lf\n", orb1, orb2, orb3);
                                printf("轨道数据：%d %d %d\n", corrs[index_corrs].radialCorr, corrs[index_corrs].tangentialCorr, corrs[index_corrs].normalCorr);
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
                    if (DEBUG)
                    {
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
                            if (DEBUG)
                            {
                                printf("------------------ok;;cbias：%s: %d\n", cbiasType[index_cbias], j);
                            }
                            cbiasNum = j;
                            index_valid_cbias++;
                            break;
                        };
                    }
                    if (DEBUG)
                    {
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
                    if (DEBUG)
                    {
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
    qsort(corrs, index_corrs + 1, sizeof(Corrections), compare);
}

/******************************************************************************
 * Name:    crc24_pppB2b  x24+x23+x18+x17+x14+x11+x10+x7+x6+x5+x4+x3+x1+1
 * Poly:    0x864CFB
 * Init:    0x000000
 * Refin:   False
 * Refout:  False
 * Xorout:  0x0000000
 * Note:
 *****************************************************************************/
uint32_t crc24_pppB2b(uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint32_t crc = 0; // Initial value
    while (length--)
    {
        crc ^= (uint8_t)(*data++); // crc ^=(uint8_t)(*data); data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x80000000)
                crc = (crc << 1) ^ 0x864CFB00;
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint32_t crcEncoding462(CRCCode crcdata)
{
    int len = 4 * STRUCT_SIZE - 3 - 3;
    uint8_t uint8data[len];
    uint8data[0] = crcdata.bits[STRUCT_SIZE - 1] >> 0;
    for (int i = 1; i < STRUCT_SIZE - 1; i++)
    {
        uint8data[(i - 1) * 4 + 1] = crcdata.bits[STRUCT_SIZE - i - 1] >> 24;
        uint8data[(i - 1) * 4 + 2] = crcdata.bits[STRUCT_SIZE - i - 1] >> 16;
        uint8data[(i - 1) * 4 + 3] = crcdata.bits[STRUCT_SIZE - i - 1] >> 8;
        uint8data[(i - 1) * 4 + 4] = crcdata.bits[STRUCT_SIZE - i - 1] >> 0;
    }
    uint8data[len - 1] = crcdata.bits[0] >> 24;
    return crc24_pppB2b(uint8data, len);
}

uint32_t crcEncoding462_check(CRCCode crcdata)
{
    return !(crcEncoding462(crcdata) ^ (crcdata.bits[0] << 8));
}

void encoding1(Corrections *corrs, int len, CRCCode *encoded_data)
{
    // MesTypeID
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 6, MAX_LEN_CRCMESSAGE, 1);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 6 - 17, MAX_LEN_CRCMESSAGE - 6, corrs[0].bdt);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 27 - 2, MAX_LEN_CRCMESSAGE - 27, corrs[0].IODSSR);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 29 - 4, MAX_LEN_CRCMESSAGE - 29, corrs[0].IODP);

    for (int i = 0; i < len; i++)
    {
        setBits(encoded_data, MAX_LEN_CRCMESSAGE - 33 - corrs[i].SatSlot, MAX_LEN_CRCMESSAGE - 33 - corrs[i].SatSlot + 1, 1);
    }
    // CRC
    uint32_t crc = crcEncoding462(*(encoded_data));
    setBits(encoded_data, 0, 24, crc >> 8);
}

int decoding1(int *sta_list, CRCCode *encoded_data, en_decodeContext *context)
{
    // MesTypeID
    if (!crcEncoding462_check(*encoded_data))
    {
        printf("error：CRC检验失败。");
        return 0;
    }
    uint32_t bdt = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 6 - 17, MAX_LEN_CRCMESSAGE - 6);
    uint8_t iodssr = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 27 - 2, MAX_LEN_CRCMESSAGE - 27);
    uint8_t iodp = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 29 - 4, MAX_LEN_CRCMESSAGE - 29);
    context->BDT = bdt;
    context->IODSSR = iodssr;
    context->IODP = iodp;

    int ret = 0;
    for (int i = 0; i < 255; i++)
    {
        uint64_t mask = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 33 - i - 1, MAX_LEN_CRCMESSAGE - 33 - i);
        if (mask)
        {
            sta_list[ret] = i + 1;
            ret++;
        }
    }
    return ret;
}

int encoding3(Corrections *corrs, int len, CRCCode *encoded_data)
{
    Corrections(*aa)[len] = (Corrections(*)[len])corrs;
    int n_used = 0;
    int n_used_len = 34;
    const int max_len = 462;
    // MesTypeID
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 6, MAX_LEN_CRCMESSAGE, 3);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 6 - 17, MAX_LEN_CRCMESSAGE - 6, corrs[0].bdt);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 27 - 2, MAX_LEN_CRCMESSAGE - 27, corrs[0].IODSSR);
    while (n_used_len + corrs[n_used].len_codebias * 16 + 13 < max_len)
    {
        setBits(encoded_data, MAX_LEN_CRCMESSAGE - n_used_len - 9, MAX_LEN_CRCMESSAGE - n_used_len, corrs[n_used].SatSlot);
        setBits(encoded_data, MAX_LEN_CRCMESSAGE - n_used_len - 9 - 4, MAX_LEN_CRCMESSAGE - n_used_len - 9, corrs[n_used].len_codebias - 1);
        for (int j = 0; j < corrs[n_used].len_codebias; j++)
        {
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - n_used_len - 13 - 4 - j * 16, MAX_LEN_CRCMESSAGE - n_used_len - 13 - j * 16, corrs[n_used].cbias[j].codebiasType);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - n_used_len - 17 - 12 - j * 16, MAX_LEN_CRCMESSAGE - n_used_len - 17 - j * 16, corrs[n_used].cbias[j].codebiasValue);
        }
        n_used_len += corrs[n_used].len_codebias * 16 + 13;
        n_used += 1;
    }
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 29 - 5, MAX_LEN_CRCMESSAGE - 29, n_used);
    // CRC
    uint32_t crc = crcEncoding462(*(encoded_data));
    setBits(encoded_data, 0, 24, crc >> 8);
    return n_used;
}

void decoding3(Corrections *corrs, int len, CRCCode *encoded_data, en_decodeContext* context)
{
    if (!crcEncoding462_check(*encoded_data))
    {
        printf("error：CRC检验失败。");
        return;
    }
    uint32_t bdt = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 6 - 17, MAX_LEN_CRCMESSAGE - 6);
    uint8_t iodssr = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 27 - 2, MAX_LEN_CRCMESSAGE - 27);
    uint8_t num_sta = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 29 - 5, MAX_LEN_CRCMESSAGE - 29);
    if (bdt != context->BDT)
    {
        printf("error：BDT。%d", context->BDT);
        return;
    }
    if (iodssr != context->IODSSR)
    {
        printf("error：IODSSR。");
        return;
    }
    int index_code = 34;
    for (int i = 0; i < num_sta; i++)
    {
        uint16_t satslot = getBits(encoded_data, MAX_LEN_CRCMESSAGE - index_code - 9, MAX_LEN_CRCMESSAGE - index_code);
        int index_corrs;
        index_corrs = get_index(corrs, len, satslot);
        if (index_corrs == -1)
        {
            continue;
        }
        uint8_t num_cbias = getBits(encoded_data, MAX_LEN_CRCMESSAGE - index_code - 9 - 4, MAX_LEN_CRCMESSAGE - index_code - 9) + 1;
        corrs[index_corrs].len_codebias = num_cbias;
        for (int j = 0; j < num_cbias; j++)
        {
            uint8_t codename = getBits(encoded_data, MAX_LEN_CRCMESSAGE - index_code - 13 - 4 - j * 16, MAX_LEN_CRCMESSAGE - index_code - 13 - j * 16);
            uint16_t codebias_v = getBits(encoded_data, MAX_LEN_CRCMESSAGE - index_code - 17 - 12 - j * 16, MAX_LEN_CRCMESSAGE - index_code - 17 - j * 16);
            corrs[index_corrs].cbias[j].codebiasType = codename;
            corrs[index_corrs].cbias[j].codebiasValue = fillUpwards(codebias_v, 12);
        }
        index_code += 16 * num_cbias + 13;
    }
}

int get_index(Corrections *corrs, int len, uint16_t satslot)
{
    for (int i = 0; i < len; i++)
    {
        if (satslot == (corrs + i)->SatSlot)
        {
            return i;
        }
    }
    return -1;
}

uint64_t fillUpwards(uint64_t value, uint8_t originalLen)
{
    uint64_t a = 1;
    if ((a <<= --originalLen) & value)
    { // 负数
        return (0xffffffffffffffff << originalLen) | value;
    }
    else
    {
        return value;
    };
}

int encoding6(Corrections *corrs, int len, CRCCode *encoded_data)
{
    int ret = 3;
    // MesTypeID
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 6, MAX_LEN_CRCMESSAGE, 6);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 14 - 17, MAX_LEN_CRCMESSAGE - 14, corrs[0].bdt);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 35 - 2, MAX_LEN_CRCMESSAGE - 35, corrs[0].IODSSR);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 37 - 4, MAX_LEN_CRCMESSAGE - 37, corrs[0].IODP);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - 41 - 9, MAX_LEN_CRCMESSAGE - 41, corrs[0].SatSlot);

    int index_orb_begin = -1;
    for (int i = 0; i < 3; i++)
    { // 每条消息3组轨道钟差改正数
        if (i < len)
        {
            index_orb_begin = 50 + 18 * (i + 1);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - 6 - 5, MAX_LEN_CRCMESSAGE - 6, i + 1);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - 11 - 3, MAX_LEN_CRCMESSAGE - 11, i + 1);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - 50 - 3 - 18 * i, MAX_LEN_CRCMESSAGE - 50 - 18 * i, corrs[i].IODCorr);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - 53 - 15 - 18 * i, MAX_LEN_CRCMESSAGE - 53 - 18 * i, corrs[i].cloCorr);
        }
    }

    // orb
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 17, MAX_LEN_CRCMESSAGE - index_orb_begin, corrs[0].bdt);
    setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 21 - 2, MAX_LEN_CRCMESSAGE - index_orb_begin - 21, corrs[0].IODSSR);
    for (int i = 0; i < 3; i++)
    { // 每条消息3组轨道钟差改正数
        if (i < len)
        {
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 23 - 9, MAX_LEN_CRCMESSAGE - index_orb_begin - 23, corrs[i].SatSlot);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 32 - 10, MAX_LEN_CRCMESSAGE - index_orb_begin - 32, corrs[i].IODN);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 42 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 42, corrs[i].IODCorr);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 45 - 15, MAX_LEN_CRCMESSAGE - index_orb_begin - 45, corrs[i].radialCorr);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 60 - 13, MAX_LEN_CRCMESSAGE - index_orb_begin - 60, corrs[i].tangentialCorr);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 73 - 13, MAX_LEN_CRCMESSAGE - index_orb_begin - 73, corrs[i].normalCorr);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 86 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 86, corrs[i].URAClass);
            setBits(encoded_data, MAX_LEN_CRCMESSAGE - index_orb_begin - 89 - 3, MAX_LEN_CRCMESSAGE - index_orb_begin - 89, corrs[i].URAValue);
            index_orb_begin += 69;
        }
        else
            ret = i;
    }
    // CRC
    uint32_t crc = crcEncoding462(*encoded_data);
    setBits(encoded_data, 0, 24, crc >> 8);
    return ret;
}

void decoding6(Corrections *corrs, int len, CRCCode *encoded_data, en_decodeContext *context)
{
    // MesTypeID
    if (!crcEncoding462_check(*encoded_data))
    {
        printf("error：CRC检验失败。");
        return;
    }
    int NumC = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 6 - 5, MAX_LEN_CRCMESSAGE - 6);
    int NumO = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 11 - 3, MAX_LEN_CRCMESSAGE - 11);
    if (NumC > 0)
    {
        uint32_t bdt = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 14 - 17, MAX_LEN_CRCMESSAGE - 14);
        uint8_t iodssr = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 35 - 2, MAX_LEN_CRCMESSAGE - 35);
        uint8_t iodp = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 37 - 4, MAX_LEN_CRCMESSAGE - 37);
        uint8_t satslot = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 41 - 9, MAX_LEN_CRCMESSAGE - 41);
        if (bdt != context->BDT)
        {
            printf("error：BDT。%d %d ", context->BDT, bdt);
            return;
        }
        if (iodssr != context->IODSSR)
        {
            printf("error：IODSSR。");
            return;
        }
        if (iodp != context->IODP)
        {
            printf("error：IODP。");
            return;
        }

        int index_corrs = get_index(corrs, len, satslot);
        for (int i = 0; i < NumC; i++)
        {
            uint8_t iodcorr = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 50 - 3 - 18 * i, MAX_LEN_CRCMESSAGE - 50 - 18 * i);
            uint16_t corr = getBits(encoded_data, MAX_LEN_CRCMESSAGE - 53 - 15 - 18 * i, MAX_LEN_CRCMESSAGE - 53 - 18 * i);
            if (0 == corrs[index_corrs].IODCorr || iodcorr == corrs[index_corrs].IODCorr)
            {
                corrs[index_corrs + i].IODCorr = iodcorr;
                corrs[index_corrs + i].cloCorr = fillUpwards(corr, 15);
            }
            else
            {
                printf("error：IODCorr。");
                return;
            }
        }
    }

    if (NumO > 0)
    {
        int index_code = MAX_LEN_CRCMESSAGE - 14 - 36 * (NumC > 0 ? 1 : 0) - 18 * NumC;
        uint32_t bdt = getBits(encoded_data, index_code - 17, index_code);
        uint8_t iodssr = getBits(encoded_data, index_code - 21 - 2, index_code - 21);
        if (bdt != context->BDT)
        {
            printf("error：BDT。%d", context->BDT);
            return;
        }
        if (iodssr != context->IODSSR)
        {
            printf("error：IODSSR。");
            return;
        }

        for (int i = 0; i < NumO; i++)
        {
            uint16_t satslot = getBits(encoded_data, index_code - 69 * i - 23 - 9, index_code - 69 * i - 23);
            printf("%d\n", satslot);
            uint16_t IODN = getBits(encoded_data, index_code - 69 * i - 32 - 10, index_code - 69 * i - 32);
            uint16_t IODCorr = getBits(encoded_data, index_code - 69 * i - 42 - 3, index_code - 69 * i - 42);
            uint16_t radialCorr = getBits(encoded_data, index_code - 69 * i - 45 - 15, index_code - 69 * i - 45);
            uint16_t tangentialCorr = getBits(encoded_data, index_code - 69 * i - 60 - 13, index_code - 69 * i - 60);
            uint16_t normalCorr = getBits(encoded_data, index_code - 69 * i - 73 - 13, index_code - 69 * i - 73);
            uint8_t URAL_CLASS = getBits(encoded_data, index_code - 69 * i - 86 - 3, index_code - 69 * i - 86);
            uint8_t URAL_VALUE = getBits(encoded_data, index_code - 69 * i - 89 - 3, index_code - 69 * i - 89);
            int index_corrs = get_index(corrs, len, satslot);
            if (0 == corrs[index_corrs].IODCorr || IODCorr == corrs[index_corrs].IODCorr)
            {
                corrs[index_corrs].IODCorr = IODCorr;
                corrs[index_corrs].radialCorr = fillUpwards(radialCorr, 15);
                corrs[index_corrs].tangentialCorr = fillUpwards(tangentialCorr, 13);
                corrs[index_corrs].normalCorr = fillUpwards(normalCorr, 13);
                corrs[index_corrs].URAClass = URAL_CLASS;
                corrs[index_corrs].URAValue = URAL_VALUE;
            }
            else
            {
                printf("error：IODCorr。");
                return;
            }
        }
    }
}

void print_encoded_data(CRCCode encoded_data)
{
    printf("uint32_t %3d: ", 15 * 32 + 6);
    printBinary(encoded_data.bits[15], 6);
    printf("\n");
    for (int i = 0; i < STRUCT_SIZE - 1; i++)
    {
        printf("uint32_t %3d: ", (STRUCT_SIZE - 1 - i) * 32);
        printBinary(encoded_data.bits[STRUCT_SIZE - 2 - i], 32);
        printf("\n");
    }
    uint32_t crc = crcEncoding462(encoded_data);
    printf("CRC-24 校验码为: 0x%08X\n", crc >> 0);
    printf("\n");
}
