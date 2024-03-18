#include <stdint.h>

/** 
 * 定义每颗卫星四类改正数:
 * 1.卫星轨道改正数及用户测距精度指数
 * 2.码间偏差改正数
 * 3.卫星钟差改正数
**/

typedef struct {
    uint16_t SatSlot; //掩码位置号
    float clockCorrectionParameter; // 钟差改正参数
} Corrections; //一颗卫星包含的改正参数

typedef struct {
    uint32_t bdt; //北斗时天内秒（单位：秒）
    uint16_t SatSlot; //掩码位置号
    uint16_t IODN; //匹配导航电文版本号
    uint8_t IODCorr; //匹配钟差改正数版本号
    int16_t radialCorr; //径向改正数
    int16_t tangentialCorr; //切向改正数
    int16_t normalCorr; //法向改正数
} orbitalCorrectionParameters; //轨道改正数

typedef struct {
    uint8_t URAClass; //(3bit)
    uint8_t URAValue; //(3bit)
} URAL; //用户距离精度指数

typedef struct {
    uint8_t codeCount; //码间偏差数量(4bit)
    uint8_t URAValue;  //信号与跟踪模式(4bit) 每种卫星导航系统16种
    int16_t codebiasValue; //码间偏差值（12*bit*0.017）
} codebias; //码间偏差改正数

typedef struct {
    int16_t clockCorrection; //钟差改正数（15*bit*0.0016）
} clockCorrection; //钟差改正数




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
// 
typedef struct {
    uint32_t BDtime: 17; // 历元时刻（天内秒）->4bit预留
    uint32_t CRC: 24; // 校验位(24bit)
    uint8_t mesTypeID : 6; // 消息类型
    uint8_t IODSSR: 2; // IODSSR SSR 版本号
    uint8_t IODP: 4; // IODP 掩码版本号
    uint64_t BDSMask: 63; //BDS掩码
    uint64_t GPSMask: 37; //GPS掩码
    uint64_t GalileoMask: 37; //Galileo掩码
    uint64_t GlonassMask: 37; //Glonass掩码 ->81+174bit预留
}type1satelliteMask; // 卫星掩码

typedef struct {
    uint32_t SatSlot: 9; // 掩码位置号
    uint32_t IODN: 10; // 基本导航电文版本号
    uint8_t IODCorr: 3; // 改正数版本号（匹配钟差的IODCorr）
    uint16_t radialCorr: 15; //径向改正数(15*)*0.0016米
    int16_t tangentialCorr: 13; //切向改正数(13*)*0.0016米
    int16_t normalCorr: 13; //法向改正数(13*)*0.0016米
    uint8_t URAClass: 3; // 用户距离精度指数
} type2sub1orbitCorrection;// 消息类型2子类型1轨道改正数

typedef struct {
    uint32_t BDtime: 17; // 历元时刻（天内秒）->4bit预留
    uint32_t CRC: 24; // 校验位(24bit)
    uint8_t mesTypeID : 6; // 消息类型
    uint8_t IODSSR: 2; // IODSSR SSR 版本号
    uint64_t BDSMask: 63; //BDS掩码
    uint64_t GPSMask: 37; //GPS掩码
    uint64_t GalileoMask: 37; //Galileo掩码
    uint64_t GlonassMask: 37; //Glonass掩码 ->81+174bit预留
} type2orbitCorrectionAndUserRangeAccuracyIndex;// 轨道改正数及用户测距精度指数



typedef struct {
    uint8_t NumC; // 钟差改正数的卫星数量
    uint8_t NumO; // 轨道改正数的卫星数量
    uint32_t OrbitCorrectionEpoch; // 轨道改正数历元时刻
    int16_t OrbitCorrection; // 轨道改正数内容部分
    uint8_t Reserved; // 预留位
    uint8_t CRC; // 校验位
} InformationType7;

typedef struct {
    float userAccuracyIndex; // 用户测距精度指数
} UserAccuracy;

typedef struct {
    float clockCorrectionParameter; // 钟差改正参数
} ClockCorrection;


