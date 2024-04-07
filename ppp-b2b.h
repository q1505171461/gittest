#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#include <string.h>
#include <stdlib.h>

#ifndef pppb2b
#define pppb2b
#define NUM_SYSTEMS 4
#define NUM_MODES 16
#define MAX_NUM_STA 255
#define MAX_LEN_LINE 600
#define MAX_LEN_CRCMESSAGE 486
#define DEBUG 0

#define BDS_MASK_BEGIN 0
#define GPS_MASK_BEGIN 63
#define Galileo_MASK_BEGIN 100
#define GLONASS_MASK_BEGIN 137

/**
 * 定义每颗卫星四类改正数:
 * 1.卫星轨道改正数及用户测距精度指数
 * 2.码间偏差改正数
 * 3.卫星钟差改正数
 **/

typedef struct
{
    uint8_t codebiasType;  // 信号与跟踪模式(4bit) 每种卫星导航系统16种
    int16_t codebiasValue; // 码间偏差值（12*bit*0.017）
} codebias;                // 码间偏差改正数

typedef struct
{
    int16_t clockCorrection; // 钟差改正数（15*bit*0.0016）
} clockCorrection;           // 钟差改正数

typedef struct
{
    uint16_t SatSlot;                     // 掩码位置号
    uint32_t bdt;                         // 北斗时天内秒（单位：秒）
    uint8_t IODSSR;                 // SSR 版本号 2bit
    uint8_t IODP;                   // 掩码版本号 4bit
    int16_t IODN;                  // 基本导航电文版本号 10bit
    int8_t IODCorr;                // 改正数版本号 3bit
    // orbitalCorrectionParameters *orbCorr; // 钟差改正参数
    int16_t radialCorr;        // 径向改正数
    int16_t tangentialCorr;    // 切向改正数
    int16_t normalCorr;        // 法向改正数
    int16_t cloCorr;
    uint8_t URAClass; //(3bit)
    uint8_t URAValue; //(3bit)
    codebias cbias[16];
    int len_codebias;
} Corrections; // 一颗卫星包含的改正参数
/**
 * 定义编码后数据帧结构,共有7种:
 * 1.卫星掩码
 * 2.卫星轨道改正数及用户测距精度指数
 * 3.码间偏差改正数
 * 4.卫星钟差改正数
 * 5.用户测距精度指数
 * 6.钟差改正数与轨道改正数-组合 1
 * 7.钟差改正数与轨道改正数-组合 2
 * 8-62.预留
 * 63.空信息
 **/

/**
 * 编解码
 */
// 函数：将整数转换为二进制字符串
#define NUM_SYSTEMS 4
#define NUM_MODES 16
extern const char *tracking_modes[NUM_SYSTEMS][NUM_MODES];
void printBinary(unsigned int n, int numBits);

#define STRUCT_SIZE 16 // 486位除以32位（一个整数的位数）得到15.18，向上取整为16个整数

// 定义一个包含486位的结构体
typedef struct
{
    uint32_t bits[STRUCT_SIZE]; // 使用数组来存储位
} CRCCode;

typedef struct
{
    uint32_t BDT;                   // 北斗时天内秒
    uint8_t IODSSR;                 // SSR 版本号 2bit
    uint8_t IODP;                   // 掩码版本号 4bit
    uint16_t IODN;                  // 基本导航电文版本号 10bit
    uint8_t IODCorr;                // 改正数版本号 3bit
} en_decodeContext;

void setBit(CRCCode *, int, int);
void setBits(CRCCode *crcCode, int bitIndexBegin, int bitIndexEnd, uint64_t value);
uint32_t crc24_pppB2b(uint8_t *data, uint16_t length);
uint32_t crcEncoding462(CRCCode crcdata);

void inputSsr(char (*ssrStr)[MAX_LEN_LINE], en_decodeContext *context, Corrections *corrs, int len);

void encoding1(Corrections *corrs, int len, CRCCode *);
int decoding1(int*, CRCCode*, en_decodeContext*);
int encoding3(Corrections *corrs, int len, CRCCode *);
void decoding3(Corrections *corrs, int len, CRCCode *, en_decodeContext*);
int encoding6(Corrections *corrs, int len, CRCCode *, int);
void decoding6(Corrections *corrs, int len, CRCCode *, en_decodeContext*);
void print_encoded_data(CRCCode);
int get_index(Corrections * corrs,int len, uint16_t satslot);
uint64_t fillUpwards(uint64_t, uint8_t);
void send_encoded_data_one_sec(CRCCode);
#endif