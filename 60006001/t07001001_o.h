/**
 * WARNING: 禁止手工修改本文件
 * generated by doc2cpp at the Fri Apr 29 11:09:33 2016
 */
#include "xml2st/xml2st.h"

#ifndef	__t07001001_o_TBL_HEADER__
#define	__t07001001_o_TBL_HEADER__

typedef struct t07001001_o_feelist_s
{
    double * costamt; // M 费用金额
    char * costcode; // M [0-4] 费用代码
} t07001001_o_feelist_t;

typedef struct t07001001_o_memlist_s
{
    t07001001_o_feelist_t ** feelist; // O multiple 费用列表
    char * mid; // M [0-12] 会员编号
} t07001001_o_memlist_t;

typedef struct t07001001_o_body_s
{
    t07001001_o_memlist_t ** memlist; // O multiple 费用列表
} t07001001_o_body_t;

typedef struct t07001001_o_head_s
{
    long * succflag; // M 成功标志
} t07001001_o_head_t;

typedef struct t07001001_o_root_s
{
    t07001001_o_body_t ** body; // O Single 体节点
    t07001001_o_head_t ** head; // O Single 头节点
} t07001001_o_root_t;

#ifdef	__cplusplus
extern	"C" {
#endif

const struct xml2st_table * ref_t07001001_o_root_tbl(void);

#ifdef	__cplusplus
}
#endif
#endif	/*__t07001001_o_TBL_HEADER__*/