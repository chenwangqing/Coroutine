/**
 * @file     InitList.h
 * @brief    初始化列表
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-08
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-08 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#if !defined(__INITLIST_H__)
#define __INITLIST_H__
#include "Define.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief    初始化回调
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-17
 */
typedef void (*CM_Init_func_t)(size_t init_val);

/**
 * @brief    结束回调
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-17
 */
typedef void (*CM_DeInit_func_t)(size_t deinit_val);

/**
 * @brief    初始化节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-17
 */
typedef struct _CM_Init_Node_t
{
    const char             *name;
    size_t                  order;
    CM_Init_func_t          init;
    size_t                  init_val;
    CM_DeInit_func_t        deinit;
    size_t                  deinit_val;
    struct _CM_Init_Node_t *next;
} CM_Init_Node_t;

extern void CM_Init_AddNode(CM_Init_Node_t *n);

#define CM_INIT_ORDER_PERIPHERALS 0x0000   // 外设初始化顺序起始 0x0000 - 0x0FFF
#define CM_INIT_ORDER_INTERFACE   0x1000   // 接口初始化顺序起始 0x1000 - 0x1FFF (包括日志)
#define CM_INIT_ORDER_APPDATA     0x2000   // 应用数据初始化顺序起始 0x2000 - 0x2FFF
#define CM_INIT_ORDER_APP         0x3000   // 应用初始化顺序起始 0x3000

/**
 * @brief    注册初始化列表(CM_Init/CM_DeInit执行)
 * @param    _name          名称
 * @param    order          初始化顺序 0：最先初始化 最后去初始化
 * @param    init_func      初始化函数
 * @param    deinit_func    结束函数
 * @param    init_val       初始化参数
 * @param    deinit_val     结束参数
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-17
 */
#define CM_INIT_LIST_REGISTER(_name, _order, init_func, deinit_func, _init_val, _deinit_val) \
    CM_INIT_FUNCTION                                                                         \
    {                                                                                        \
        static CM_Init_Node_t n;                                                             \
        n.name       = _name;                                                                \
        n.order      = _order;                                                               \
        n.init       = init_func;                                                            \
        n.init_val   = _init_val;                                                            \
        n.deinit     = deinit_func;                                                          \
        n.deinit_val = _deinit_val;                                                          \
        CM_Init_AddNode(&n);                                                                 \
    }

/**
 * @brief    初始化 CM_INIT_LIST_REGISTER
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-11
 */
extern void CM_Init(void);

/**
 * @brief    去初始化 CM_INIT_LIST_REGISTER
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-11
 */
extern void CM_DeInit(void);
#ifdef __cplusplus
}
#endif
#endif   // __INITLIST_H__
