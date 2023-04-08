/********************************************************/

// [システム名]Commetシミュレータ

// [作成者]原口悠也

// [機能概要]Commetシミュレータで作成されたオブジェクトファイルを読み込みシミュレートする

// [記述形式]$ ./comet [-r] [-s n] [executable] 順不同

// [戻り値] -1:異常終了 0:正常終了

// [特筆事項]なし

//
//	
/********************************************************/


//------------------------------------------------
//  include定義(include definition)
//------------------------------------------------
#include<stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

//------------------------------------------------
//  定数定義(constant definition)
//------------------------------------------------

//戻り値
#define RET_OK (0)
#define RET_ERR (-1)
#define RET_ACV (1) //アクセス違反
#define RET_EXIT (4) 
#define RET_NOP (5) //未定義命令

//システム概要定数
#define AUTO_STEP (500)
#define MAXCAPA (0x10000)
#define TRACE_INTERVAL (20)
#define MAX_STEP (8388607)

//型定数
#define SHORT_MAX (32767)
#define SHORT_MIN (-32768)
#define CHAR_LEN (32)

//オプション
#define ROP ("-r")
#define SOP ("-s")

//ファイル
#define FILE_HEADER_SIZE (32)
#define END_ADR (0x8000 - 0x100)


//命令
#define MASK_1 (0xF000)
#define MASK_2 (0x0F00)
#define INST_1x (0x1000)
#define INST_2x (0x2000)
#define INST_3x (0x3000)
#define INST_4x (0x4000)
#define INST_6x (0x6000)
#define INST_10x (0xa000)
#define LIST_ENABLE_OPR1 (0x0001)
#define LIST_ENABLE_OPR2 (0x0002)
#define LIST_ENABLE_EA (0x0004)


//------------------------------------------------
//  構造体定義(Struct definition)
//------------------------------------------------
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned char UCHAR;

typedef struct
{
    USHORT ObjStart; // オブジェクト開始アドレス
    USHORT ExeStart; // オブジェク実行開始アドレス
    UCHAR Reserved[16];
    UINT NextLink;
    UCHAR Reaserved2[8];
    UCHAR Data[0];


}FILE_HEADER;

typedef struct
{
    USHORT GR2  :3; // XR IR Rm
    USHORT Index :1; // I
    USHORT GR1    :4; // R
    USHORT OpCode   :8;  // OPCD
    
}CMD_CODE;

typedef struct
{
    USHORT ZF :1; // 
    USHORT SF :1; // 
    USHORT OF :1;// 
    USHORT flt :13; //
    
}FLAG_CODE;

typedef struct
{
    USHORT adr; // ADR
    USHORT opcd; // OPCD
    USHORT opr1; // OPR1
    USHORT opr2; // OPR2
    char MNE[CHAR_LEN]; // MNE
    USHORT g; // G
    USHORT i; // I
    USHORT x; // X
    USHORT ea; // EA
    USHORT pr; // PR
    USHORT gr0; // GR0
    USHORT gr1; // GR1
    USHORT gr2; // GR2
    USHORT gr3; // GR3
    USHORT gr4; // GR4
    USHORT mr; // MR
    USHORT fr; // FR
    
}TRACELIST;

typedef union
{
    CMD_CODE CmdCode; // 命令語表記
    USHORT Data; // 16進数表記

}MACHINE_CODE;

typedef union
{
    FLAG_CODE FlagCode;// 命令語表記
    USHORT Data;   // 16進数表記 
    
}FLAG_REGISTER;

typedef struct
{
    //File系
    unsigned char FileName[CHAR_LEN]; // 実行ファイルの情報
    struct stat FileInfo; // 実行ファイルのヘッダ
    FILE_HEADER File_Header; // 実行ファイルのヘッダ
    USHORT *CommetMemory;
    
    //処理系
    int LimitCounter; // ステップ数上限
    int CurCounter; // 現在のステップ数
    USHORT ListOut; // トレースリストの有無
    USHORT ListFlag; //トレースリストの出力の有無
    
    //トレースリスト系
    USHORT ADR; // 命令アドレス
    MACHINE_CODE OPCD; // １ワード
    USHORT OPR1; // 2ワード
    USHORT OPR2; // 3ワード
    char MNE[CHAR_LEN]; //ニーモニック 
    USHORT EA; // 有効アドレス
    USHORT PR; // プログラムレジスタ
    USHORT GR[5]; // 0~4のレジスタ 
    USHORT MR; // マスクレジスタ
    FLAG_REGISTER FR; // フラグレジスタ

}ADMINFO;

//------------------------------------------------
//  マクロ定義(Macro definition)
//------------------------------------------------
#define ERR_ACSV()  (printf("TRACE END (ACCESS VIOLATION)\n")); //メモリアクセス違反
#define ERR_STO()  (printf("TRACE END (STEP OVER)\n")); //ステップオーバー
#define ERR_NOP()  (printf("TRACE END (NO OPERATION)\n")); //未定義命令
#define ERR_FILE_NF()  (printf("TRACE END (FILE NOT FOUND)\n")); //ファイルが存在しない
#define ERR_OP_NF()  (printf("TRACE END (OPTION NOT FOUND)\n")); //オプションが存在しない
#define ERR_LOW_CAP()  (printf("TRACE END (LOW CAPACITY)\n"));  //仮想メモリ容量不足
#define ERR_DEF_INV()   (printf("TRACE END (DESTINATION INVALID)\n")); //仮想メモリ展開宛先違反
#define ERR_ALLOC_FAL()   (printf("TRACE END (ALLOCATION FAILURE)\n")); //仮想メモリ確保失敗
#define ERR_AGM_INV()   (printf("TRACE END (ARGUMENT INVALID)\n")); //引数違反
#define ERR_STEP_LIMIT()   (printf("TRACE END (STEP_LIMIT)\n")); //トレース終了
#define TRACE_END() (printf("TRACE END\n"))

//FR系
#define SetFR(x)    (Admininfo-> FR.Data = x)
#define SetZF(x)    (Admininfo-> FR.FlagCode.ZF = x)
#define SetSF(x)    (Admininfo-> FR.FlagCode.SF = x)
#define SetOF(x)    (Admininfo-> FR.FlagCode.OF = x)

//FR判定
#define IsZF()  (Admininfo-> FR.FlagCode.ZF != 0 ? 1 : 0)
#define IsSF()  (Admininfo-> FR.FlagCode.SF != 0 ? 1 : 0)
#define IsOF()  (Admininfo-> FR.FlagCode.OF != 0 ? 1 : 0)

//取得系
#define GetPR() (*(Admininfo->CommetMemory + Admininfo->PR))
#define GetEA() (*(Admininfo->CommetMemory + Admininfo->EA))
#define GetGR1() (Admininfo->OPCD.CmdCode.GR1)
#define GetGR2() (Admininfo->OPCD.CmdCode.GR2)

//TraceList系
#define TrcOffOPR1()    (Admininfo->ListFlag |= LIST_ENABLE_OPR1)
#define TrcOffOPR2()    (Admininfo->ListFlag |= LIST_ENABLE_OPR2)
#define TrcOffEA()    (Admininfo->ListFlag |= LIST_ENABLE_EA)
#define TrcFlgSet(x)    (Admininfo->ListFlag = x)

#define IsTrcOPR1() ((Admininfo->ListFlag & LIST_ENABLE_OPR1) ? 1 : 0)
#define IsTrcOPR2() ((Admininfo->ListFlag & LIST_ENABLE_OPR2) ? 1 : 0)
#define IsTrcEA()   ((Admininfo->ListFlag & LIST_ENABLE_EA) ? 1 : 0)


//------------------------------------------------
//  プロトタイプ定義(Prototype definition)
//------------------------------------------------

//起動処理
int ParameterCheck(int argc,char *argv[],ADMINFO *Admininfo);
int Init(ADMINFO *Admininfo);
int FileLoad(ADMINFO *Admininfo);

//機能系
int PrintTracelist(ADMINFO *Admininfo);
int CmdExecute(ADMINFO *Admininfo);
void Dmp(ADMINFO *Admininfo);

//命令系
int CmdLD(ADMINFO *Admininfo);
int CmdST(ADMINFO *Admininfo);
int CmdLAD(ADMINFO *Admininfo);
int CmdADDA(ADMINFO *Admininfo);
int CmdSUBA(ADMINFO *Admininfo);
int CmdADDL(ADMINFO *Admininfo);
int CmdSUBL(ADMINFO *Admininfo);
int CmdAND(ADMINFO *Admininfo);
int CmdOR(ADMINFO *Admininfo);
int CmdXOR(ADMINFO *Admininfo);
int CmdCPA(ADMINFO *Admininfo);
int CmdCPL(ADMINFO *Admininfo);
int CmdSRL(ADMINFO *Admininfo);
//製造対象外
int CmdSLL(ADMINFO *Admininfo);
//製造対象外
int CmdSRA(ADMINFO *Admininfo);
//製造対象外
int CmdSLA(ADMINFO *Admininfo);
//製造対象外
int CmdJMI(ADMINFO *Admininfo);
int CmdJNZ(ADMINFO *Admininfo);
int CmdJZE(ADMINFO *Admininfo);
int CmdJUMP(ADMINFO *Admininfo);
int CmdJPL(ADMINFO *Admininfo);
int CmdJOV(ADMINFO *Admininfo);
int CmdEXIT(ADMINFO *Admininfo);
