#pragma once
#ifndef _60006001_H
#define _60006001_H
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct  _req_calcfee_s
    {
        char Mid[12+1];
        char MemLevel[3+1];
        char MarkCode[2+1];
        double WRAmt;
        double SpValUnit;
        char flag[1+1];  //区分是买家B,还是卖家S
    } req_calcfee_s;
    typedef struct  _req_07001001_s
    {
        char Mid[12+1];
        char MemLevel[3+1];
        char MarkCode[2+1];
        double WRAmt;
        double SpValUnit;
        char flag[1+1];  //区分是买家B,还是卖家S
    } req_07001001_s;

    typedef struct  _fee_s
    {
        char CostCode[4+1];
        char TrType[4+1];
        double  CostAmt;
    } fee_s;


int main_60006001(svc_ctx_t *ctx);
int rpc_07001002( req_calcfee_s *pReqData, void *pRspData);
int rpc_07001001( req_calcfee_s *pReqData, void *pRspData);
int rsp_60006001( xmlNodePtr bodynode ,char* costcode,double costamt);
int check_buyer_right(char * mid ,char * listedno,char* storeno);
#ifdef __cplusplus
}
#endif
