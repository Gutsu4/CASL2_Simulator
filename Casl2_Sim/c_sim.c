#include "c_sim.h"

int main(int argc, char *argv[])
{
    int ret_val = 0;
    ADMINFO Admininfo;

    /*初期化*/
    ret_val = Init(&Admininfo);

    if(ret_val == RET_ERR){
        return RET_ERR;
    }

    /*パラメータチェック*/
    ret_val = ParameterCheck(argc,argv,&Admininfo);

    if(ret_val == RET_ERR){
        return RET_ERR;
    }

    /*ファイル展開*/
    ret_val = FileLoad(&Admininfo);

    if(ret_val == RET_ERR){
        return RET_ERR;
    }

    /*プログラム実行開始アドレス*/
    Admininfo.PR =  Admininfo.File_Header.ExeStart;

    /*ダンプ表示*/
    Dmp(&Admininfo);

    for(Admininfo.CurCounter = 0;  Admininfo.CurCounter< Admininfo.LimitCounter; Admininfo.CurCounter++){
        
        /*コマンド実行*/
        ret_val = CmdExecute(&Admininfo);

        if(ret_val == RET_OK){
            
            /*トレースリスト実行*/
            PrintTracelist(&Admininfo);            

        }else if(ret_val == RET_EXIT){

            /*トレースリスト実行*/
            PrintTracelist(&Admininfo);
            TRACE_END();
            return RET_OK;

        }else if(ret_val == RET_ACV){

            /*アクセス違反*/
            ERR_ACSV();
            return RET_OK;

        }else if(ret_val == RET_NOP){

            /*未定義命令*/
            ERR_NOP();
            return RET_OK;

        }
    }

    if(ret_val != RET_EXIT){
        ERR_STO();
        return RET_OK;
    }

    /*終了処理*/
    free(Admininfo.CommetMemory);

    return RET_OK;
}


//------------------------------------------------
//  コマンドライン引数チェック関数
//------------------------------------------------
int ParameterCheck(int argc,char *argv[],ADMINFO *Admininfo)
{
    char *errbuf;   

    //引数の個数でエラー表示
    if(argc < 2 || argc > 5){
        ERR_AGM_INV();
        return RET_ERR;
    }

    /*-------------2----------------*/
    if(argc == 2){

        //FileName格納
        //「-」で始まっていないとき
        if(argv[1][0] != '-'){
            memcpy(Admininfo->FileName,argv[1],CHAR_LEN);
        }else{
            ERR_AGM_INV();
            return RET_ERR;
        }
        Admininfo->LimitCounter = AUTO_STEP;
        return RET_OK;
        

    /*-------------3----------------*/
    }else if(argc == 3){

        //１つめが-rオプションのとき
        if(strcmp(argv[1],ROP) == 0){

            Admininfo->ListOut = 1;

            //「-」で始まっていないとき
            if(argv[1][0] != '-'){
                memcpy(Admininfo->FileName,argv[1],CHAR_LEN);
            }else{
                ERR_AGM_INV();
                return RET_ERR;
            }

            Admininfo->LimitCounter = AUTO_STEP;
            return RET_OK;

        //１つめが実行ファイル名のとき
        }else if(strcmp(argv[2],ROP) == 0){
            
            Admininfo->ListOut = 1;

            //「-」で始まっていないとき
            if(argv[1][0] != '-'){
                memcpy(Admininfo->FileName,argv[1],CHAR_LEN);
            }else{
                ERR_AGM_INV();
                return RET_ERR;
            }
            Admininfo->LimitCounter = AUTO_STEP;
            return RET_OK;
        
        //-rオプションではない
        }else{
            ERR_AGM_INV();
            return RET_ERR;
        }

    /*-------------4----------------*/
    }else if(argc == 4){

        //LOOP開始
        for(int i = 1; i < argc; i++){
            
            //-rのときはエラー
            if(strcmp(argv[i],ROP) == 0){

                ERR_AGM_INV();
                return RET_ERR;

            //-sのとき
            }else if(strcmp(argv[i],SOP) == 0){

                //-sが末尾ではない
                if(i != argc - 1){

                    //-sだが数字ではない
                    if(strtol(argv[i + 1],&errbuf,10) == 0){
                    
                        ERR_AGM_INV();
                        return RET_ERR;
                    
                    }else{

                        //数値変換後が最大ステップ数を超えている場合
                        if(strtol(argv[i + 1],&errbuf,10) <= MAX_STEP){
                            
                            //step数がマイナスの場合
                            if(strtol(argv[i + 1],&errbuf,10)< 0){
                                ERR_AGM_INV();
                                return RET_ERR;
                            }
                            Admininfo->LimitCounter = strtol(argv[i + 1],&errbuf,10);
                            i++;

                        }else{
                            ERR_STEP_LIMIT();
                            return RET_ERR;
                        }
                    }
                
                //-sが末尾に記述されている
                }else{
                    ERR_AGM_INV();
                    return RET_ERR;
                }

            }else{
                
                //「-」で始まっていないとき
                if(argv[i][0] != '-'){
                    memcpy(Admininfo->FileName,argv[i],CHAR_LEN);
                }else{
                    ERR_OP_NF();
                    return RET_ERR;
                }
            }
        }

    /*-------------5---------------*/ 
    }else if(argc == 5){

        //LOOP開始
        for(int i = 1; i < argc; i++){

            //-rのときはエラー
            if(strcmp(argv[i],ROP) == 0){

                //リスト表示
                Admininfo->ListOut = 1;

            //-sのとき
            }else if(strcmp(argv[i],SOP) == 0){


                //-sが末尾ではない
                if(i != argc - 1){

                    //-sだが数字ではない
                    if(strtol(argv[i + 1],&errbuf,10) == 0){
                    
                        ERR_AGM_INV();
                        return RET_ERR;
                    
                    }else{

                        //数値変換後が最大ステップ数を超えている場合
                        if(strtol(argv[i + 1],&errbuf,10) <= MAX_STEP){

                            //step数がマイナスの場合
                            if(strtol(argv[i + 1],&errbuf,10)< 0){
                                ERR_AGM_INV();
                                return RET_ERR;
                            }                    
                            Admininfo->LimitCounter = strtol(argv[i + 1],&errbuf,10);
                            i++;

                        }else{
                            ERR_STEP_LIMIT();
                            return RET_ERR;
                        }
                    }
                
                //-sが末尾に記述されている
                }else{
                    ERR_AGM_INV();
                    return RET_ERR;
                }

            }else{

                //「-」で始まっていないとき
                if(argv[i][0] != '-'){
                    memcpy(Admininfo->FileName,argv[i],CHAR_LEN);
                }else{
                    ERR_OP_NF();
                    return RET_ERR;
                }
            }
        }

        //FIleNameが空白のとき
        if(strcmp((char *)Admininfo->FileName,"") == 0){
            ERR_AGM_INV();
            return RET_ERR;
        }      
    }
    return RET_OK;
}

//------------------------------------------------
//  初期化関数
//------------------------------------------------
int Init(ADMINFO *Admininfo)
{
    //初期化実行
    memset(Admininfo, 0, sizeof(*Admininfo));

    //メモリ確保
    Admininfo->CommetMemory = (USHORT*) malloc(MAXCAPA);
    
    //確保失敗の場合終了
    if(Admininfo->CommetMemory == NULL){
        ERR_ALLOC_FAL();  
        return RET_ERR;
    }
    
    //スタックレジスタの初期化
    Admininfo->GR[4] = 0x7fff;
    Admininfo->MR = 0xFF;

    return RET_OK;
}

//------------------------------------------------
//  ファイルロード関数
//------------------------------------------------
int FileLoad(ADMINFO *Admininfo)
{

    //stat実行
    int file_exist = stat((char *)Admininfo->FileName,&Admininfo->FileInfo);
    
    //fileポインター
    FILE *fp;

    //File展開
    if ((fp = fopen((char *)Admininfo->FileName, "r")) == NULL)
    {
        ERR_FILE_NF();
        return RET_ERR;
    }

    //Fileが見つからない
    if(file_exist == -1)
    {
        ERR_FILE_NF();
        fclose(fp);
        return RET_ERR;
    }

    /*ファイルの展開*/
    //ヘッダー読み込み
    fread(&(Admininfo->File_Header),sizeof(Admininfo->File_Header),1,fp);

    /*エラー処理*/
    //ファイルサイズエラー
    if((Admininfo->FileInfo.st_size - FILE_HEADER_SIZE) < 2){
        ERR_LOW_CAP();
        fclose(fp);
        return RET_ERR;

    //展開開始アドレスエラー
    }else if((Admininfo->File_Header.ObjStart + (Admininfo->FileInfo.st_size  - FILE_HEADER_SIZE ) / 2) > END_ADR){
        ERR_DEF_INV();
        fclose(fp);
        return RET_ERR;
    }
    
    //オブジェクト展開
    fread(Admininfo->File_Header.ObjStart + Admininfo->CommetMemory,(Admininfo->FileInfo.st_size  - FILE_HEADER_SIZE ),1,fp);

    fclose(fp);

    return RET_OK;
}

//------------------------------------------------
//  命令振り分け関数
//------------------------------------------------
int CmdExecute(ADMINFO *Admininfo)
{
    USHORT and_val;
    USHORT val;
    int ret_flg = 0;

    //JUMP ACV確認
    if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
        return RET_ACV;
    }

    //OPCodeを代入
    val = GetPR();
    Admininfo->OPCD.Data = val;

    //マスクし8ビットの値を取得
    and_val = val & MASK_1;

    //マスクし9ビットから12ビット目を所得    
    val &= MASK_2;

    //命令分岐
    //1x
    if(and_val == INST_1x)
    {
        //LD
        if(val == 0x0000 || val == 0x0400){
        
            ret_flg  = CmdLD(Admininfo);
        
        //ST
        }else if(val == 0x0100){

            ret_flg  = CmdST(Admininfo);

        //LAD
        }else if(val == 0x0200){

            ret_flg  = CmdLAD(Admininfo);
        
        }else{
            return RET_NOP;
        }

    
    //2x
    }else if(and_val == INST_2x)
    {
        //ADDA
        if(val == 0x0000){

            ret_flg  = CmdADDA(Admininfo);

        //SUBA
        }else if(val == 0x0100){

            ret_flg  = CmdSUBA(Admininfo);

        //ADDL
        }else if(val == 0x0200){
        
            ret_flg  = CmdADDL(Admininfo);

        //SUBL
        }else if(val == 0x0300){

            ret_flg  = CmdSUBL(Admininfo);

        }else{
            return RET_NOP;
        }

    //3x
    }else if(and_val == INST_3x)
    {
        //AND
        if(val == 0x0000){

            ret_flg  = CmdAND(Admininfo);

        //OR
        }else if(val == 0x0100){

            ret_flg  = CmdOR(Admininfo);

        //XOR
        }else if(val == 0x0200){

            ret_flg  = CmdXOR(Admininfo);

        //OTHER
        }else{
            return RET_NOP;
        }

    //4x
    }else if(and_val == INST_4x)
    {
        //CPA
        if(val == 0x0000 || val == 0x0400){

            ret_flg  = CmdCPA(Admininfo);

        //CPL
        }else if(val == 0x0100 || 0x0500){

            ret_flg  = CmdCPL(Admininfo);

        }else{
            return RET_NOP;
        }


    //6x
    }else if(and_val == INST_6x)
    {
        //JMI
        if(val == 0x0100){

            ret_flg  = CmdJMI(Admininfo);

        //JNZ
        }else if(val == 0x0200){

            ret_flg  = CmdJNZ(Admininfo);

        //JZE
        }else if(val == 0x0300){

            ret_flg  = CmdJZE(Admininfo);

        //JUMP
        }else if(val == 0x0400){

            ret_flg  = CmdJUMP(Admininfo);

        //JPL
        }else if(val == 0x0500){

            ret_flg  = CmdJPL(Admininfo);

        //JOV
        }else if(val == 0x0600){

            ret_flg  = CmdJOV(Admininfo);

        //OTHER
        }else{
            return RET_NOP;
        }

    //10x
    }else if(and_val == INST_10x)
    {
        //EXITs
        if(val == 0x00000){
            ret_flg = CmdEXIT(Admininfo);
        }

    //other
    }else
    {
        return RET_NOP;
    }

    //returnの値で分岐
    if(ret_flg == RET_EXIT){
        return RET_EXIT;

    }else if(ret_flg == RET_ACV){
        return RET_ACV;

    }else if(ret_flg == RET_ERR){
        return RET_ERR;
    }

   return RET_OK;

}

//------------------------------------------------
//  トレースリスト表示関数
//------------------------------------------------
int PrintTracelist(ADMINFO *Admininfo)
{
    //リスト表示フラグが0のとき
    if(Admininfo->ListOut != 1){
        
        //インターバルごとの命令でラベル表示
        if(Admininfo->CurCounter % TRACE_INTERVAL == 0){

            //ラベル表示
            printf("ADR  OPCD OPR1 OPR2 MNE  G I X EA   PR   GR0  GR1  GR2  GR3  GR4  MR FR\n");
        }

        //リスト表示        
        //ADR
        printf("%04X ",Admininfo->ADR);

        //OPCD
        printf("%04X ",Admininfo->OPCD.Data);
            
        //OPR1
        if(IsTrcOPR1()){
            printf("     ");
        }else{
            printf("%04X ",Admininfo->OPR1);
        }

        //OPR2
        if(IsTrcOPR2()){
           printf("     ");  
        }else{
           printf("%04X ",Admininfo->OPR2);
        }

        //MNE
        printf("%s ",Admininfo->MNE);

        //G
        printf("%d ",GetGR1());

        //I
        printf("%d ",Admininfo->OPCD.CmdCode.Index);

        //X
        printf("%d ",GetGR2());

        //EA
        if(IsTrcEA()){
           printf("     ");
        }else{
           printf("%04X ",Admininfo->EA);
        }

        //PR
        printf("%04X ",Admininfo->PR);

        //GR
        printf("%04X ",Admininfo->GR[0]);
        printf("%04X ",Admininfo->GR[1]);
        printf("%04X ",Admininfo->GR[2]);
        printf("%04X ",Admininfo->GR[3]);
        printf("%04X ",Admininfo->GR[4]);

        //MR
        printf("%02X ",Admininfo->MR);

        //FR
        printf("%02X ",Admininfo->FR.Data);

        //\n
        printf("\n");

        //Flgの初期化
        TrcFlgSet(0x0000);

    }
    return RET_OK;
}

//------------------------------------------------
//  ダンプ関数
//------------------------------------------------
void Dmp(ADMINFO *Admininfo)
{
    /*
    for(int i = 0; i <= Admininfo->FileInfo.st_size - FILE_HEADER_SIZE; i++){
        if(i % 8 == 0){
           printf("\n");
        }
            printf("%04X ",*(Admininfo->CommetMemory + Admininfo->File_Header.ObjStart + i ));
    }

    printf("\n");
    printf("\n");
    */
    printf("\nMEM  :  +0   +1   +2   +3   +4   +5   +6   +7");
    for(int i = 0; i <= Admininfo->FileInfo.st_size - FILE_HEADER_SIZE; i++){
        if(i % 8 == 0){
            printf("\n");
            printf("%04X :",i);
        }
        printf(" %04X",*(Admininfo->CommetMemory + Admininfo->File_Header.ObjStart + i));
    }
    printf("\n");
    printf("\n");

}


//------------------------------------------------
//  命令関数類
//------------------------------------------------
int CmdLD(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(Admininfo->MNE,"LD  ");
    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    //0x10のとき
    if(Admininfo->OPCD.CmdCode.OpCode != 0x14){

        //Indexが0のとき
        if(Admininfo->OPCD.CmdCode.Index == 0){

            //2語目をセット
            Admininfo->OPR1 = GetPR();
            
            //有効アドレスに2語目を格納
            Admininfo->EA = Admininfo->OPR1;
            Admininfo->PR++;

            //XRに値があるか
            if(GetGR2() != 0){
                Admininfo->EA += Admininfo->GR[GetGR2()];
            }

            //ACV確認
            if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
                return RET_ACV;
            }

            Admininfo->GR[GetGR1()] = GetEA();
            
        //Indexが0ではない
        }else{
            Admininfo->EA = Admininfo->GR[GetGR2()]; 
            
            //ACV確認
            if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
                return RET_ACV;
            }
            
            Admininfo->GR[GetGR1()] = GetEA();
            Admininfo->PR++;
        }

    //0x14
    }else{
        TrcOffOPR2();
        TrcOffEA();
        Admininfo->GR[GetGR1()] = Admininfo->GR[GetGR2()];

    }

    //FRの設定
    SetFR(0);
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if((short)Admininfo->GR[GetGR1()] < 0){
        SetSF(1);
    }

    return RET_OK;
}

int CmdST(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"ST  ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    //Iが１のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->OPR1 = GetPR();
        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        Admininfo->PR++;

        //実行
        GetEA() = Admininfo->GR[GetGR1()];

    //Iが0のとき
    }else{

        Admininfo->OPR1 = GetPR();
        Admininfo->EA = GetPR();
        Admininfo->PR++;
        
        //XRを加算
        if(GetGR2() != 0){
            
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        GetEA() = Admininfo->GR[GetGR1()];
    }
    return RET_OK;   
}

//Calc
int CmdADDL(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"ADDL");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    int ADD_val = 0;

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        ADD_val = (int)Admininfo->GR[GetGR1()] + (int)GetEA();        
        Admininfo->GR[GetGR1()] += GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRを加算
        if(GetGR2() != 0){          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        ADD_val = (int)Admininfo->GR[GetGR1()] + (int)GetEA();
        Admininfo->GR[GetGR1()] += GetEA();
        Admininfo->PR++;

    }

    //FR設定
    SetFR(0);  

    //!=だったときOFを立てる
    if(ADD_val != Admininfo->GR[GetGR1()]){
        SetOF(1);
    }
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;   
    
}

int CmdSUBL(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"SUBL");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    int SUB_val = 0;

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        SUB_val = (int)Admininfo->GR[GetGR1()] - (int)GetEA();        
        Admininfo->GR[GetGR1()] -= GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRを加算
        if(GetGR2() != 0){          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //実行
        SUB_val = (int)Admininfo->GR[GetGR1()] - (int)GetEA();
        Admininfo->GR[GetGR1()] -= GetEA();

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        Admininfo->PR++;

    }

    //FR設定
    SetFR(0);  

    //!=だったときOFを立てる
    if(SUB_val != Admininfo->GR[GetGR1()]){
        SetOF(1);
    }
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;   
}

//OR,AND,XOR
int CmdAND(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"AND ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        Admininfo->GR[GetGR1()] &= GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRがあるか
        if(GetGR2() != 0){

            //XRを加算          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        Admininfo->GR[GetGR1()] &= GetEA();
        Admininfo->PR++;
    }

    //FR設定
    SetFR(0);
    
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;

    
}

int CmdOR(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"OR  ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];
        
        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        Admininfo->GR[GetGR1()] |= GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRを加算
        if(GetGR2() != 0){          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        Admininfo->GR[GetGR1()] |= GetEA();
        Admininfo->PR++;
    }

    //FR設定
    SetFR(0);
    
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;

}

int CmdXOR(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"XOR ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        Admininfo->GR[GetGR1()] ^= GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRを加算
        if(GetGR2() != 0){          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        Admininfo->GR[GetGR1()] ^= GetEA();
        Admininfo->PR++;
    }

    //FR設定
    SetFR(0);
    
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;

}

//CP
int CmdCPA(ADMINFO *Admininfo)
{
    USHORT gr1_sign;
    USHORT gr2_sign;

    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"CPA ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();

    //0x40
    if(Admininfo->OPCD.CmdCode.OpCode != 0x44){

        //indexが1
        if(Admininfo->OPCD.CmdCode.Index == 1){

            Admininfo->EA = Admininfo->GR[GetGR2()];

            //ACV確認
            if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
                return RET_ACV;
            }

            Admininfo->PR++; 


        //indexが0
        }else{
            
            //XRを加算
            if(GetGR2() != 0){          
                Admininfo->EA += Admininfo->GR[GetGR2()];
            }

            //ACV確認
            if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
                return RET_ACV;
            }

            Admininfo->PR++;

        }

        gr1_sign = Admininfo->GR[GetGR1()] & 0x8000;
        gr2_sign = GetEA() & 0x8000;

        //FR分岐
        SetFR(0);

        //gr1は1(負)gr2は0(正)のとき
        if(gr1_sign > gr2_sign){
            SetSF(1);

        //符号が0で同値
        }else if(gr1_sign == 0 && gr2_sign == 0){

            if(Admininfo->GR[GetGR1()] == GetEA()){
                SetZF(1);
            }
            if(Admininfo->GR[GetGR1()] < GetEA()){
                SetSF(1);
            }

        //符号が1で同値
        }else if(gr1_sign == 1 && gr2_sign == 1){

            if(Admininfo->GR[GetGR1()] == GetEA()){
                SetZF(1);
            }
            if(Admininfo->GR[GetGR1()] > GetEA()){
                SetSF(1);
            }

        }


    //0x44のとき    
    }else{

        //一語命令
        TrcOffEA();
        TrcOffOPR1();
    
        gr1_sign = Admininfo->GR[GetGR1()] & 0x8000;
        gr2_sign = Admininfo->GR[GetGR2()] & 0x8000;   

        //FR分岐
        SetFR(0);
        
        //gr1は1(負)gr2は0(正)のとき
        if(gr1_sign > gr2_sign){
            SetSF(1);

        //符号が0で同値
        }else if(gr1_sign == 0 && gr2_sign == 0){

            if(Admininfo->GR[GetGR1()] == Admininfo->GR[GetGR2()]){
                SetZF(1);
            }
            if(Admininfo->GR[GetGR1()] < Admininfo->GR[GetGR2()]){
                SetSF(1);
            }

        //符号が1で同値
        }else if(gr1_sign == 1 && gr2_sign == 1){

            if(Admininfo->GR[GetGR1()] == Admininfo->GR[GetGR2()]){
                SetZF(1);
            }
            if(Admininfo->GR[GetGR1()] > Admininfo->GR[GetGR2()]){
                SetSF(1);
            }

        }

    } 
    return RET_OK;   
    
}

int CmdCPL(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"CPL ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();  

    //0x41
    if(Admininfo->OPCD.CmdCode.OpCode != 0x45){

        //indexが1
        if(Admininfo->OPCD.CmdCode.Index == 1){

            Admininfo->EA = Admininfo->GR[GetGR2()];

            //ACV確認
            if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
                return RET_ACV;
            }

            Admininfo->PR++; 


        //indexが0
        }else{
            
            //XRを加算
            if(GetGR2() != 0){          
                Admininfo->EA += Admininfo->GR[GetGR2()];
            }

            //ACV確認
            if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
                return RET_ACV;
            }

            Admininfo->PR++;

        }

    //FR分岐
    SetFR(0);
    if(Admininfo->GR[GetGR1()] == GetEA()){
        SetZF(1);
    }
    if(Admininfo->GR[GetGR1()] < GetEA()){
        SetSF(1);
    }

    //0x45のとき    
    }else{

        //一語命令
        TrcOffEA();
    
        if(*(Admininfo->CommetMemory + Admininfo->GR[GetGR1()]) == *(Admininfo->CommetMemory + Admininfo->GR[GetGR2()])){
            SetZF(1);
        }
        if(*(Admininfo->CommetMemory + Admininfo->GR[GetGR1()]) < *(Admininfo->CommetMemory + Admininfo->GR[GetGR2()])){
            SetSF(1);
        }

    } 

    return RET_OK;   
    
}


//JUMP系
int CmdJMI(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"JMI ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    Admininfo->PR++;

    //JUMPの条件を満たす
    if(IsSF()){

        //XRがあるか
        if(GetGR2() != 0){
            //XRを加算          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        Admininfo->PR = Admininfo->EA;

    }else{
        return RET_OK;
    }

    return RET_OK;
    
}

int CmdJZE(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"JZE ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    Admininfo->PR++;

    //JUMPの条件を満たす
    if(IsZF()){

        //XRがあるか
        if(GetGR2() != 0){
            //XRを加算          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        Admininfo->PR = Admininfo->EA;

    }else{
        return RET_OK;
    }

    return RET_OK;

    
}

int CmdJNZ(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"JNZ ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    Admininfo->PR++;

    //JUMPの条件を満たす
    if(IsZF() == 0){

        //XRがあるか
        if(GetGR2() != 0){
            //XRを加算          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        Admininfo->PR = Admininfo->EA;

    }else{
        return RET_OK;
    }

    return RET_OK;
   
}

int CmdJUMP(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"JUMP");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();

    //XRがあるか
    if(GetGR2() != 0){
        //XRを加算          
        Admininfo->EA += Admininfo->GR[GetGR2()];
    }

    Admininfo->PR = Admininfo->EA;

    return RET_OK;
    
}

int CmdJOV(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"JOV ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    Admininfo->PR++;

    //JUMPの条件を満たす
    if(IsOF()){

        //XRがあるか
        if(GetGR2() != 0){
            //XRを加算          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        Admininfo->PR = Admininfo->EA;

    }else{
        return RET_OK;
    }

    return RET_OK;
}

int CmdJPL(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"JPL ");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    Admininfo->PR++;

    //JUMPの条件を満たす
    if(IsSF() == 0 || IsZF() == 0){

        //XRがあるか
        if(GetGR2() != 0){
            //XRを加算          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        Admininfo->PR = Admininfo->EA;

    }else{
        return RET_OK;
    }

    return RET_OK;

    
}

int CmdEXIT(ADMINFO *Admininfo)
{
    TrcOffOPR1();
    TrcOffOPR2();
    TrcOffEA();
    strcpy(&Admininfo->MNE[0],"EXIT");
    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;
    return RET_EXIT;   
    
}

int CmdSUBA(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"SUBA");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    int SUB_val = 0;

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        SUB_val = (int)((short)Admininfo->GR[GetGR1()] - (short)GetEA());        
        Admininfo->GR[GetGR1()] -= GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRを加算
        if(GetGR2() != 0){          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //実行
        SUB_val = (int)((short)Admininfo->GR[GetGR1()] - (short)GetEA());        
        Admininfo->GR[GetGR1()] -= GetEA();

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        Admininfo->PR++;

    }

    //FR設定
    SetFR(0);  

    //!=だったときOFを立てる
    if(SHORT_MAX < SUB_val || SHORT_MIN > SUB_val){
        SetOF(1);
    }
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;   

}

int CmdADDA(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(&Admininfo->MNE[0],"ADDA");

    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    Admininfo->OPR1 = GetPR();
    Admininfo->EA = GetPR();
    int ADD_val = 0;

    //Iが1のとき
    if(Admininfo->OPCD.CmdCode.Index == 1){

        Admininfo->EA = Admininfo->GR[GetGR2()];

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        ADD_val = (int)((short)Admininfo->GR[GetGR1()] + (short)GetEA());        
        Admininfo->GR[GetGR1()] += GetEA();
        Admininfo->PR++;

    //Iが0のとき
    }else{

        //XRを加算
        if(GetGR2() != 0){          
            Admininfo->EA += Admininfo->GR[GetGR2()];
        }

        //ACV確認
        if(0 > Admininfo->EA || Admininfo->EA > END_ADR){
            return RET_ACV;
        }

        //実行
        ADD_val = (int)((short)Admininfo->GR[GetGR1()] + (short)GetEA());
        Admininfo->GR[GetGR1()] += GetEA();
        Admininfo->PR++;

    }

    //FR設定
    SetFR(0);  

    //!=だったときOFを立てる
    if(SHORT_MAX < ADD_val || SHORT_MIN > ADD_val){
        SetOF(1);
    }
    if(Admininfo->GR[GetGR1()] == 0){
        SetZF(1);
    }
    if(((short)Admininfo->GR[GetGR1()] & 0x8000) == 0x8000){
        SetSF(1);
    }

    return RET_OK;
 
}

int CmdLAD(ADMINFO *Admininfo)
{
    TrcOffOPR2();
    strcpy(Admininfo->MNE,"LAD ");
    Admininfo->ADR = Admininfo->PR;
    Admininfo->PR++;

    //2語目をセット
    Admininfo->OPR1 = GetPR();

    Admininfo->EA = Admininfo->OPR1;
    

    if(GetGR2() != 0){
        Admininfo->EA += Admininfo->GR[GetGR2()];
    }

    Admininfo->GR[GetGR1()] = Admininfo->EA;
    Admininfo->PR++;

    return RET_OK;

}
