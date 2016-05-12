/**
 * WARNING: 禁止手工修改本文件
 * generated by doc2cpp at the Wed Mar 23 10:32:51 2016
 */
#include "./t07001002_o.h"
#include "xml2st/xml2st.h"

static const struct xml2st_column _feelist_columns_[] =
{
    DEF_X2S_MDBL("costamt", t07001002_o_feelist_t, costamt), // M 费用金额
    DEF_X2S_MSTR("costcode", 0,  t07001002_o_feelist_t, costcode), // M [0-4] 费用代码
};

static const struct xml2st_table _feelist_table_ =
    DEF_X2S_MTBL("feelist", t07001002_o_feelist_t, _feelist_columns_);

static const struct xml2st_column _memlist_columns_[] =
{
    DEF_X2S_OPTR("feelist", t07001002_o_memlist_t, feelist, _feelist_table_), // O multiple 费用列表
    DEF_X2S_MSTR("mid", 0,  t07001002_o_memlist_t, mid), // M [0-12] 会员编号
};

static const struct xml2st_table _memlist_table_ =
    DEF_X2S_MTBL("memlist", t07001002_o_memlist_t, _memlist_columns_);

static const struct xml2st_column _body_columns_[] =
{
    DEF_X2S_OPTR("memlist", t07001002_o_body_t, memlist, _memlist_table_), // O multiple 费用列表
};

static const struct xml2st_table _body_table_ =
    DEF_X2S_MTBL("body", t07001002_o_body_t, _body_columns_);

static const struct xml2st_column _head_columns_[] =
{
    DEF_X2S_MINT("succflag", t07001002_o_head_t, succflag), // M 成功标志
};

static const struct xml2st_table _head_table_ =
    DEF_X2S_MTBL("head", t07001002_o_head_t, _head_columns_);

static const struct xml2st_column _root_columns_[] =
{
    DEF_X2S_OPTR("body", t07001002_o_root_t, body, _body_table_), // O Single 体节点
    DEF_X2S_OPTR("head", t07001002_o_root_t, head, _head_table_), // O Single 头节点
};

static const struct xml2st_table _root_table_ =
    DEF_X2S_MTBL("root", t07001002_o_root_t, _root_columns_);

const struct xml2st_table * ref_t07001002_o_root_tbl(void)
{
    return &_root_table_;
}
