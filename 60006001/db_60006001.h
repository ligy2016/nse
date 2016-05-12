#ifndef DB_60006001_H
#define DB_60006001_H
#endif
#include "60006001/t60006001_i.h"
#ifdef __cplusplus
extern "C"
{
#endif

int db_main_60006001( xmlNodePtr  bodynode,t60006001_i_root_t * root);
int db_60006001_memlev(char *mid ,char level[3+1]);
int db_60006001_check_expire(void);
int db_60006001_payment(char * listedno, double *payment, char marketcode[2+1],double * SpValUnit );
int db_insert_feelist( char * ListedNo,char *costcode, char* TrType,double costamt,char* mid );
//int db_60006001_insert_baseinfo( t60006001_i_root_t * root,char dsno[20+1],char auditno[20+1],char * wrno, char * goodsno);
int db_60006001_insert_baseinfo( t60006001_i_root_t * root,char dsno[20+1],char auditno[20+1],char * wrno,char * listno, char * goodsno);
int db_insert_tdbaudit(char* dsno,char* auditno);
int db_60006001_check_right(char * mid ,char* storeno);
//int db_get_wahinfo(char* listedno,char rno[20+1],char rcpno[20+1],char goodsno[20+1],char storeno[12+1]);
int db_get_wahinfo(char* listedno,char wrno[20+1],char rcpno[20+1],char goodsno[20+1],char storeno[12+1],char Invoice[1+1]);
//int db_60006001_memlev(char *mid ,char level[3+1]);
int db_60006001_get_sellerinfo(char* dsno ,char listno[20+1],char seller[12+1]);
int db_60006001_check_status(char * listedno);
#ifdef __cplusplus
}
#endif
