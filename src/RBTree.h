/**
 * @file     RBTree.h
 * @brief    红黑树
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-05
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-05 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#if !defined(__RBTREE_H__)
#define __RBTREE_H__
#include "Define.h"
#ifdef __cplusplus
extern "C" {
#endif

#define TREE_LEFT  -1   // 左边
#define TREE_EQUAL 0   // 等于
#define TREE_RIGHT 1   // 右边

typedef struct _CM_RBTree_Node CM_RBTree_Link_t;
typedef struct _CM_RBTree      CM_RBTree_t;

/**
 * @brief    遍历事件【返回 false 停止遍历】
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
typedef bool (*CM_RBTree_Callback_Traverse_t)(const CM_RBTree_t *tree, const CM_RBTree_Link_t *node, void *object);

/**
 * @brief    比较函数  RBTREE_LEFT 左边（<0） RBTREE_EQUAL 等于(=0)  RBTREE_RIGHT 右边(>0)
 * @param    new_val        新值
 * @param    val            比较值
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
typedef int (*CM_RBTree_Callback_Compare_t)(const CM_RBTree_t *     tree,
                                            const CM_RBTree_Link_t *new_val,
                                            const CM_RBTree_Link_t *val);

/**
 * @brief    垃圾回收
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
typedef void (*CM_RBTree_Callback_free_t)(CM_RBTree_t *tree, CM_RBTree_Link_t *n);

/**
 * @brief    RBTree 节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
struct _CM_RBTree_Node
{
    size_t                  Parent;
    struct _CM_RBTree_Node* L, * R;
};

/**
 * @brief    RBTree
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
struct _CM_RBTree
{
    CM_RBTree_Link_t* root;          // 根节点
    CM_RBTree_Link_t             NIL;           // 空节点
    uint32_t                     count;         // 数量
    CM_RBTree_Callback_Compare_t CompareFunc;   // 比较函数
};

/**
 * @brief    初始化RBTree
 * @param    tree
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
extern void CM_RBTree_Init(CM_RBTree_t *tree, CM_RBTree_Callback_Compare_t func);

/**
 * @brief    插入节点
 * @param    tree           树
 * @param    n              节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
extern void CM_RBTree_Insert(CM_RBTree_t *tree, CM_RBTree_Link_t *n);

/**
 * @brief    移除
 * @param    tree           树
 * @param    n              节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
extern void CM_RBTree_Remove(CM_RBTree_t *tree, CM_RBTree_Link_t *n);

/**
 * @brief    判断树是否为空
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
#define CM_RBTree_IsEmpty(tree) ((tree)->root == &(tree)->NIL)

/**
 * @brief    左结束点
 * @param    tree
 * @return   CM_RBTree_Link_t*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-08
 */
extern CM_RBTree_Link_t *CM_RBTree_LeftEnd(CM_RBTree_t *tree);

/**
 * @brief    右结束点
 * @param    tree
 * @return   CM_RBTree_Link_t*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-08
 */
extern CM_RBTree_Link_t *CM_RBTree_RightEnd(CM_RBTree_t *tree);

/**
 * @brief    遍历
 * @param    tree           树
 * @param    mode           遍历模式：-1：前序（方便查找）0：中序(按顺序) 1：后序（用于释放资源）
 * @param    func           遍历函数
 * @param    object         用户对象
 * @return   CM_RBTree_Link_t*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
extern const CM_RBTree_Link_t *CM_RBTree_Traverse(const CM_RBTree_t *           tree,
                                                  int                           mode,
                                                  CM_RBTree_Callback_Traverse_t func,
                                                  void *                        object);

/**
 * @brief    开始遍历
 * @param    Type           类型
 * @param    link_field     连接字段
 * @param    tree           树
 * @param    var            变量名称
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
#define CM_RBTree_Search_Begin(Type, link_field, tree, var) \
    do {                                                    \
        const CM_RBTree_t *     __tree = tree;              \
        const CM_RBTree_Link_t *__p    = __tree->root;      \
        const CM_RBTree_Link_t *__NIL  = &__tree->NIL;      \
        while (__p != __NIL) {                              \
            Type *var = CM_Field_ToType(Type, link_field, __p);

/**
 * @brief    遍历下一个
 * @param    isless         true: 右边 false：左边
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
#define CM_RBTree_Search_Next(isless) __p = (isless) ? __p->R : __p->L

/**
 * @brief    遍历完成
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
#define CM_RBTree_Search_Complete() break

/**
 * @brief    遍历结束
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
#define CM_RBTree_Search_End() \
    }                          \
    }                          \
    while (false)

/*
    CM_RBTree_Search_Begin(KeyValue, link, &k->tree, p);
    if (p->key == key &&
        p->link.blank == key_len &&
        (key_len <= 8 || memcmp(p->key_data, key_data, key_len) == 0))
        {
            ret = p;
            CM_RBTree_Search_Complete();
        }
    CM_RBTree_Search_Next(p->key < key);
    CM_RBTree_Search_End();
*/

/**
 * @brief    获取数量
 * @param    tree           RBTree
 * @return   uint32_t
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
extern uint32_t CM_RBTree_GetCount(CM_RBTree_t *tree);

/**
 * @brief    清除树
 * @param    tree           树
 * @param    func           回收函数
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-15
 */
extern void CM_RBTree_Clear(CM_RBTree_t *tree, CM_RBTree_Callback_free_t func);
#ifdef __cplusplus
}
#endif
#endif   // __RBTREE_H__
