#include "log4c/trace.h"
#include <string.h>
#include <assert.h>
#include <iostream>
#include "xml/node.h"
#include "xml/xml.h"
#include "xml2st/xml2st.h"
#include "tuxrpc/tuxrpc.h"
#include "tuxsvc/context.h"
#include "oradb/oradbc_work.h"
#include "60006001.h"
#include "db_60006001.h"
#include "t60006001_i.h"
#include "t07001002_o.h"
#include "t07001001_o.h"
#include "wahbuysvc.h"
#include <vector>
#ifdef	__cplusplus
extern	"C" {
#endif


using namespace std;

int main_60006001(svc_ctx_t *ctx)
{
    int iRet = 0;
    char dsno[20+1]= {0};//摘牌编号
    char auditno[20+1] = {0};//审核编号
    char wrno[20+1]= {0};//注册编号
    char rcpno[20+1]= {0};//签发编号
    char goodsno[20+1]= {0};//货物编号
    char storeno[12+1]= {0};//仓库编号
    char trtype[4+1]= {0};
    char seller[12+1] = {0};
    long status = 0;//状态为待审核
    char Invoice[1+1]={0};
    char ErrorMsg[1024] = {0};

    //xmlNodePtr memlistnode = NULL;

    //向计费中心请求的结构体
    req_calcfee_s req_t;
    memset(&req_t,0,sizeof(req_calcfee_s));

    vector <fee_s> vfee;

    xmlDocPtr reqdoc = ctx->req_doc;
    xmlDocPtr rspdoc = ctx->rsp_doc;

    xmlNodePtr bodynode = NULL;

    bodynode = cnacex_xml_getnode( rspdoc, "/ns:root/body", "ns", NULL );
    //xmlNodePtr feelistnode  = xmlNewTextChild( bodynode, NULL, BAD_CAST("costpay"), NULL);
    //初始化handle
    xml2st_hndl hndl = NULL;
    hndl = xml2st_easy_init ( ref_t60006001_i_root_tbl());
    if(!hndl)
    {
        cnacex_oops( ctx, " 解析60006001请求xml，初始化xml2st句柄失败!");
        return 0;
    }

    //解析xml获得root
    t60006001_i_root_t* root = (t60006001_i_root_t *)xml2st_easy_parse(hndl,reqdoc);
    if(!root)
    {
        cnacex_oops( ctx, " 解析 60006001 请求xml失败!");
        xml2st_easy_free(hndl);
        return 0;
    }


    if(!root->body)
    {
        cnacex_trace_warn( "请求xml中body为空");
        xml2st_easy_free(hndl);
        return 1;
    }

    iRet = db_60006001_check_status(BODY->listedno);

    if(iRet)
    {
         strcpy(ErrorMsg,"挂牌单状态错误禁止交易！");
         goto L_ERROR;
    }


    // 获取该被摘牌的挂牌单相关信息，后面会用到
    iRet = db_get_wahinfo(BODY->listedno,wrno, rcpno, goodsno, storeno,Invoice);
    if(iRet) goto L_ERROR;



    //首先检查买方的权限
    //iRet = check_buyer_right(BODY->mid,BODY->listedno,storeno);
    if(iRet) goto L_ERROR;

    strncpy (req_t.Mid,BODY->mid,sizeof(req_t.Mid)-1);
    //获取会员等级
    iRet = db_60006001_memlev(req_t.Mid, req_t.MemLevel);
    if(iRet) goto L_ERROR;

    //获取货款金额和市场代码特殊计价单元
    iRet = db_60006001_payment(BODY->listedno,&req_t.WRAmt,req_t.MarkCode,&req_t.SpValUnit);
    if(iRet) goto L_ERROR;
    //RPC 调用计费中心 买家费用
    req_t.flag[0]='B';//B代表是买家
    iRet = rpc_07001002(&req_t, (void*)&vfee);
    if(iRet) goto L_ERROR;






    //生成摘牌单号和审核单号，并插入订单表
    iRet = db_60006001_insert_baseinfo(root,dsno,auditno,wrno,BODY->listedno,goodsno);
    if(iRet) goto L_ERROR;

    //插入审核表
    iRet = db_insert_tdbaudit(dsno,auditno);
    if(iRet) goto L_ERROR;

    //插入费用表构造返回报文

    xmlNewTextChild( bodynode, NULL, BAD_CAST("delistno"), (const xmlChar*)dsno);
    xmlNewTextChild( bodynode, NULL, BAD_CAST("auditno"), (const xmlChar*)auditno);
    cnacex_xmlNewINTNode(bodynode, "status", status);

    for ( unsigned int i = 0; i < vfee.size(); ++i )
    {
        if(!strcmp(vfee[i].CostCode,"8102"))
            strcpy(trtype,"6009");

        iRet = db_insert_feelist( dsno,vfee[i].CostCode,  trtype, vfee[i].CostAmt,BODY->mid );
        if(iRet) goto L_ERROR;

        iRet = rsp_60006001(   bodynode ,vfee[i].CostCode,vfee[i].CostAmt);
        if(iRet) goto L_ERROR;
    }

    //插入买家需支付的货款 8001 + 6005 = 购货支出 ...............改为8203+6003

    iRet = db_insert_feelist( dsno,(char*)"8203",  (char*)"6003", req_t.WRAmt,BODY->mid );
    if(iRet) goto L_ERROR;

    //计算卖家.的费用..

    //...计费中心卖家费用RPC


    iRet =  db_60006001_get_sellerinfo( dsno ,wrno,  seller );
    if(iRet) goto L_ERROR;
    iRet =  db_60006001_memlev(seller , req_t.MemLevel);
    if(iRet) goto L_ERROR;
    strncpy(req_t.Mid,seller,sizeof(req_t.Mid)-1);
    req_t.flag[0]='S';

    vfee.clear();

    //卖家手续费
    iRet = rpc_07001002(&req_t, (void*)&vfee);
    if(iRet) goto L_ERROR;

    //如果要监管发票 计算卖家发票保证金
    if( Invoice[0] == 'Y' )
    {
        iRet = rpc_07001001(&req_t, (void*)&vfee);
        if(iRet) goto L_ERROR;
    }

    for (  unsigned int    j = 0; j < vfee.size(); ++j )
    {
        if(!strcmp(vfee[j].CostCode,"8103"))
            strcpy(trtype,"6009");
        else if(!strcmp(vfee[j].CostCode,"8104"))
            strcpy(trtype,"6009");
        else if(!strcmp(vfee[j].CostCode,"8204"))//发票保证金
            strcpy(trtype,"6003");

        iRet = db_insert_feelist( dsno,vfee[j].CostCode,trtype, vfee[j].CostAmt,seller );
        if(iRet) goto L_ERROR;

        iRet = rsp_60006001(  bodynode ,vfee[j].CostCode,vfee[j].CostAmt);
        if(iRet) goto L_ERROR;
    }


    //插入买家需支付的货款 8001 + 9005 = 销售收入
    // iRet = db_insert_feelist( dsno,(char*)"8001",  (char*)"9005", req_t.WRAmt,BODY->mid );
    //if(iRet) goto L_ERROR;

    //货款单独列出
    rsp_60006001(   bodynode ,(char*)"8203",req_t.WRAmt);


    oradbc_commitwork();
    cnacex_done(ctx, "0001", "0");
    xml2st_easy_free(hndl);
    return 0;
L_ERROR:

    oradbc_rollbackwork();
    cnacex_error(ctx, "0001", "",ErrorMsg);
    xml2st_easy_free(hndl);
    return 0;
}


int rpc_req_07001002(xmlDocPtr     reqdoc, void* usrptr, size_t length)
{

    //‘8001’	交易货款
    //‘8102’	买方交易手续费
    //‘8104’	仓单过户费：按交易单元收取
    req_calcfee_s * p = (req_calcfee_s *) usrptr;
    xmlNodePtr bodynode;
    xmlNodePtr  memlistnode;
    xmlNodePtr feelistnode ;
    bodynode = cnacex_xml_getnode( reqdoc, "/ns:root/body", "ns", NULL );

    memlistnode = xmlNewTextChild( bodynode, NULL, BAD_CAST("memlist"), NULL);

    xmlNewTextChild( memlistnode, NULL, BAD_CAST("mid"), BAD_CAST(p->Mid));
    xmlNewTextChild( memlistnode, NULL, BAD_CAST("memlevel"), BAD_CAST(p->MemLevel));
    xmlNewTextChild( memlistnode, NULL, BAD_CAST("markcode"), BAD_CAST(p->MarkCode));
    //wramt改为pog！！二者区别是？
    cnacex_xmlNewDBLNode(memlistnode, "wramt", p->WRAmt);
    cnacex_xmlNewDBLNode(memlistnode, "pog", p->WRAmt);

//    feelistnode = xmlNewTextChild( memlistnode, NULL, BAD_CAST("feelist"), NULL);
//    xmlNewTextChild( feelistnode, NULL, BAD_CAST("costcode"), (const xmlChar*)"8001");


    //计算买家费用
    if(p->flag[0] == 'B')
    {
        feelistnode = xmlNewTextChild( memlistnode, NULL, BAD_CAST("feelist"), NULL);
        xmlNewTextChild( feelistnode, NULL, BAD_CAST("costcode"), (const xmlChar*)"8102");
        //feelistnode = xmlNewTextChild( memlistnode, NULL, BAD_CAST("feelist"), NULL);

        //cnacex_xmlNewDBLNode(feelistnode, "spvalunit", p->SpValUnit);
    }
    //计算卖家费用
    else if(p->flag[0] == 'S')
    {
        feelistnode = xmlNewTextChild( memlistnode, NULL, BAD_CAST("feelist"), NULL);
        xmlNewTextChild( feelistnode, NULL, BAD_CAST("costcode"), (const xmlChar*)"8103");
        //feelistnode = xmlNewTextChild( memlistnode, NULL, BAD_CAST("feelist"), NULL);
        //xmlNewTextChild( feelistnode, NULL, BAD_CAST("costcode"), (const xmlChar*)"8204");

        //cnacex_xmlNewDBLNode(feelistnode, "spvalunit", p->SpValUnit);
    }
    return 0;
}

int rpc_rsp_07001002(xmlDocPtr     rspdoc, void* usrptr, size_t length)
{
    long i = 0;
    long j = 0;
    fee_s fee_t;

    vector <fee_s> *v = (vector <fee_s> *)usrptr;

    //初始化handle
    xml2st_hndl hndl = NULL;
    hndl = xml2st_easy_init ( ref_t07001002_o_root_tbl());
    if(!hndl)
    {
        cnacex_trace_error ( " 解析07001002响应xml，初始化xml2st句柄失败!");
        return 0;
    }

    //解析xml获得root
    t07001002_o_root_t* root = (t07001002_o_root_t *)xml2st_easy_parse(hndl,rspdoc);
    if(!root)
    {
        cnacex_trace_error( " 解析 07001002 响应xml失败!");
        goto L_ERROR;
    }

    if(SUCCFLAG != 1)
    {
        cnacex_trace_error( "RPC获取计费列表失败");
        goto L_ERROR;
    }

    if(!root->body)
    {
        cnacex_trace_error( "返回报文无body");
        goto L_ERROR;
    }
    if(!BODY->memlist)
    {
        cnacex_trace_error( "获取memlist失败");
        goto L_ERROR;
    }
    i = 0;
    while(BODY->memlist[i])
    {
        if(!BODY->memlist[i]->feelist)
        {
            cnacex_trace_error( "获取feelist失败");
            goto L_ERROR;
        }
        j= 0;
        while(BODY->memlist[i]->feelist[j])
        {
            memset(&fee_t,0,sizeof(fee_s));

            strcpy(fee_t.CostCode,BODY->memlist[i]->feelist[j]->costcode);
            fee_t.CostAmt = *BODY->memlist[i]->feelist[j]->costamt;

            (*v).push_back(fee_t);

            j++;
        }
        i++;
    }


    xml2st_easy_free(hndl);

    return 0;

L_ERROR:
    xml2st_easy_free(hndl);
    return 1;

}

int rpc_07001002( req_calcfee_s *pReqData, void *pRspData)
{
    int iRet = 0;

    int(*pfReq)(xmlDocPtr,void*,size_t) = rpc_req_07001002;
    int(*pfRsp)(xmlDocPtr,void*,size_t) = rpc_rsp_07001002;

    iRet = tuxrpc(0,"07001002",0,pReqData,0,pRspData,0,pfReq, pfRsp);

    return iRet;
}


///////////////////////////////////////////

int rpc_rsp_07001001(xmlDocPtr     rspdoc, void* usrptr, size_t length)
{
    long i = 0;
    long j = 0;
    fee_s fee_t;

    vector <fee_s> *v = (vector <fee_s> *)usrptr;

    //初始化handle
    xml2st_hndl hndl = NULL;
    hndl = xml2st_easy_init ( ref_t07001001_o_root_tbl());
    if(!hndl)
    {
        cnacex_trace_error ( " 解析07001001响应xml，初始化xml2st句柄失败!");
        return 0;
    }

    //解析xml获得root
    t07001001_o_root_t* root = (t07001001_o_root_t *)xml2st_easy_parse(hndl,rspdoc);
    if(!root)
    {
        cnacex_trace_error( " 解析 07001001 响应xml失败!");
        goto L_ERROR;
    }

    if(SUCCFLAG != 1)
    {
        cnacex_trace_error( "RPC获取计费列表失败");
        goto L_ERROR;
    }

    if(!root->body)
    {
        cnacex_trace_error( "返回报文无body");
        goto L_ERROR;
    }
    if(!BODY->memlist)
    {
        cnacex_trace_error( "获取memlist失败");
        goto L_ERROR;
    }
    i = 0;
    while(BODY->memlist[i])
    {
        if(!BODY->memlist[i]->feelist)
        {
            cnacex_trace_error( "获取feelist失败");
            goto L_ERROR;
        }
        j= 0;
        while(BODY->memlist[i]->feelist[j])
        {
            memset(&fee_t,0,sizeof(fee_s));

            strcpy(fee_t.CostCode,BODY->memlist[i]->feelist[j]->costcode);
            fee_t.CostAmt = *BODY->memlist[i]->feelist[j]->costamt;

            (*v).push_back(fee_t);

            j++;
        }
        i++;
    }


    xml2st_easy_free(hndl);

    return 0;

L_ERROR:
    xml2st_easy_free(hndl);
    return 1;

}

int rpc_req_07001001(xmlDocPtr     reqdoc, void* usrptr, size_t length)
{

    //‘8204’	发票保证金
    req_calcfee_s * p = (req_calcfee_s *) usrptr;
    xmlNodePtr bodynode;
    xmlNodePtr  memlistnode;
    xmlNodePtr feelistnode ;
    bodynode = cnacex_xml_getnode( reqdoc, "/ns:root/body", "ns", NULL );

    memlistnode = xmlNewTextChild( bodynode, NULL, BAD_CAST("memlist"), NULL);

    xmlNewTextChild( memlistnode, NULL, BAD_CAST("mid"), BAD_CAST(p->Mid));
    xmlNewTextChild( memlistnode, NULL, BAD_CAST("memlevel"), BAD_CAST(p->MemLevel));
    xmlNewTextChild( memlistnode, NULL, BAD_CAST("markcode"), BAD_CAST(p->MarkCode));
    cnacex_xmlNewDBLNode(memlistnode, "pog", p->WRAmt);



    //计算卖家费用
     if(p->flag[0] == 'S')
    {
        feelistnode = xmlNewTextChild( memlistnode, NULL, BAD_CAST("feelist"), NULL);

        xmlNewTextChild( feelistnode, NULL, BAD_CAST("costcode"), (const xmlChar*)"8204");

        //cnacex_xmlNewDBLNode(feelistnode, "spvalunit", p->SpValUnit);
    }
    return 0;
}


int rpc_07001001( req_calcfee_s *pReqData, void *pRspData)
{
    int iRet = 0;

    int(*pfReq)(xmlDocPtr,void*,size_t) = rpc_req_07001001;
    int(*pfRsp)(xmlDocPtr,void*,size_t) = rpc_rsp_07001001;

    iRet = tuxrpc(0,"07001001",0,pReqData,0,pRspData,0,pfReq, pfRsp);

    return iRet;
}


int rsp_60006001( xmlNodePtr bodynode ,char* costcode,double costamt)
{
    if((!strcmp(costcode ,"8103") )||(!strcmp(costcode ,"8204") ))
        return 0;//屏蔽返回时卖家的费用
    xmlNodePtr feelistnode  = xmlNewTextChild( bodynode, NULL, BAD_CAST("costpay"), NULL);
    xmlNewTextChild( feelistnode, NULL, BAD_CAST("costcode"), (const xmlChar*)costcode);
    cnacex_xmlNewDBLNode(feelistnode, "costamt", costamt);
    return 0;
}

int check_buyer_right(char * mid ,char * listedno,char* storeno)
{
    //检查买方权限：1是否是对应仓库会员，2是否在挂牌有效期内
    int iRet = 0;
    iRet = db_60006001_check_right(mid ,storeno);
    if(iRet)    return iRet;

    return 0;
}

#ifdef	__cplusplus
}
#endif
