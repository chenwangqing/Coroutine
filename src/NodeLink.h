
#if !defined(__CXSMETHOD_NODELINK_H__)
#define __CXSMETHOD_NODELINK_H__
#include "Define.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    节点连接
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
typedef struct _CM_NodeLink_t
{
    struct _CM_NodeLink_t *next;
    struct _CM_NodeLink_t *up;
} CM_NodeLink_t, *CM_NodeLinkList_t;

/**
 * @brief    在之后插入节点
 * @param    list           节点列表
 * @param    node           之后插入的节点
 * @param    link           需要插入的连接
 * @note     CM_NodeLink_Insert(&C_Static.semaphores, CM_NodeLink_End(C_Static.semaphores), &sem->link);
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
extern void CM_NodeLink_Insert(CM_NodeLinkList_t *list, CM_NodeLink_t *node, CM_NodeLink_t *link);

/**
 * @brief    设置为头节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
#define CM_NodeLink_SetHeadNode(list, link)                     \
    do {                                                        \
        CM_NodeLinkList_t *__list = list;                       \
        CM_NodeLink_t *    node   = CM_NodeLink_End(*(__list)); \
        CM_NodeLink_Insert(__list, node, link);                 \
        (*__list) = link;                                       \
    } while (false)

/**
 * @brief    移除节点
 * @param    list           节点列表
 * @param    link           移除的连接
 * @return   CM_NodeLink_t * 返回下一个节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
extern CM_NodeLink_t *CM_NodeLink_Remove(CM_NodeLinkList_t *list, CM_NodeLink_t *link);

/**
 * @brief    获取节点的结构体
 * @param    Type           结构体类型
 * @param    link_field     连接字段
 * @param    link           连接点
 * @note     Node *p = NodeLink_ToType(Node, link, a);
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_ToType(Type, link_field, link) CM_Field_ToType(Type, link_field, link)

/**
 * @brief    初始化连接
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_Init(link) (link)->next = (link)->up = (link)

/**
 * @brief    第一个
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_First(list) (list)

/**
 * @brief    最后一个
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_End(list) ((list) == nullptr ? nullptr : (list)->up)

#define CM_NodeLink_Next(p)       (p) = (p)->next   // 下一个
#define CM_NodeLink_Up(p)         (p) = (p)->Up   // 上一个
#define CM_NodeLink_IsEmpty(list) (nullptr == (list))   // 是否为空 CM_NodeLink_IsEmpty(sem->list)

/**
 * @brief    正序遍历
 * @param    Type           结构体类型
 * @param    link_field     连接字段
 * @param    list           节点列表
 * @param    iter           迭代器
 * @note     NodeLink_Foreach_Positive(Node, link, head, p)
 * @note     {
 * @note        p->a++;
 * @note     }
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_Foreach_Positive(Type, link_field, list, iter)                                               \
    Type *CM_S_S(__list, __LINE__) = (list) == nullptr ? nullptr : CM_NodeLink_ToType(Type, link_field, list);   \
    for (Type *iter = CM_S_S(__list, __LINE__), *__flag = nullptr, *__for_p_list_ptr = CM_S_S(__list, __LINE__); \
         nullptr != __for_p_list_ptr && (nullptr == __flag || iter != __for_p_list_ptr);                         \
         iter = CM_NodeLink_ToType(Type, link_field, iter->link_field.next), __flag = __for_p_list_ptr)

/**
 * @brief    逆序遍历
 * @param    Type           结构体类型
 * @param    link_field     连接字段
 * @param    list           节点列表
 * @param    iter           迭代器
 * @note     NodeLink_Foreach_Reverse(Node, link, head, p)
 * @note     {
 * @note        p->a++;
 * @note     }
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_Foreach_Reverse(Type, link_field, list, iter)                                                               \
    Type *CM_S_S(__list, __LINE__) = (list) == nullptr ? nullptr : CM_NodeLink_ToType(Type, link_field, CM_NodeLink_End(list)); \
    for (Type *iter = CM_S_S(__list, __LINE__), *__flag = nullptr, *__for_r_list_ptr = CM_S_S(__list, __LINE__);                \
         nullptr != __for_r_list_ptr && (nullptr == __flag || iter != __for_r_list_ptr);                                        \
         iter = CM_NodeLink_ToType(Type, link_field, iter->link_field.up), __flag = __for_r_list_ptr)

#define CM_NODELINK_MODE_POSITIVE CM_NodeLink_Foreach_Positive   // 正序
#define CM_NODELINK_MODE_REVERSE  CM_NodeLink_Foreach_Reverse   // 逆序

/**
 * @brief    查找
 * @param    mode           查找模式 CM_NODELINK_MODE_POSITIVE/CM_NODELINK_MODE_REVERSE
 * @param    Type           结构体类型
 * @param    link_field     连接字段
 * @param    expression     查找表达式(p->a == 3) true结束 变量：p
 * @param    list           节点列表
 * @note     CM_NodeLink_FindPositive(Node, link, (p->a == 30), head);
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define _CM_NodeLink_Find(mode, Type, link_field, expression, list)               \
    ({                                                                            \
        Type *            CM_S_S(___CM_NodeLink_Find_result, __LINE__) = nullptr; \
        CM_NodeLinkList_t __list                                       = list;    \
        mode(Type, link_field, __list, p)                                         \
        {                                                                         \
            if expression {                                                       \
                CM_S_S(___CM_NodeLink_Find_result, __LINE__) = p;                 \
                break;                                                            \
            }                                                                     \
        }                                                                         \
        CM_S_S(___CM_NodeLink_Find_result, __LINE__);                             \
    })

/**
 * @brief    正序查找
 * @param    Type           结构体类型
 * @param    link_field     连接字段
 * @param    expression     查找表达式(p->a == 3) true结束 变量：p
 * @param    list           节点列表
 * @return   Type *         查找结果
 * @note     CM_NodeLink_FindPositive(Node, link, (p->a == 30), head);
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_FindPositive(Type, link_field, expression, list) \
    _CM_NodeLink_Find(CM_NODELINK_MODE_POSITIVE, Type, link_field, expression, list)

/**
 * @brief    逆序查找
 * @param    Type           结构体类型
 * @param    link_field     连接字段
 * @param    expression     查找表达式(p->a == 3) true结束 变量：p
 * @param    list           节点列表
 * @return   Type *         查找结果
 * @note     CM_NodeLink_FindPositive(Node, link, (p->a == 30), head);
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_NodeLink_FindReverse(Type, link_field, expression, list) \
    _CM_NodeLink_Find(CM_NODELINK_MODE_REVERSE, Type, link_field, expression, list)

/**
 * @brief    拼接
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
#define CM_NodeLink_Splice(link1, link2)                      \
    do {                                                      \
        CM_NodeLink_t *__s1 = link1, *__s2 = link2;           \
        if (__s1 != nullptr && __s2 != nullptr) {             \
            CM_NodeLink_t *__e1 = __s1->up, *__e2 = __s2->up; \
            __s2->up   = __e1;                                \
            __e1->next = __s2;                                \
            __s1->up   = __e2;                                \
            __e2->next = __s1;                                \
        }                                                     \
    } while (false)


#ifdef __cplusplus
}
#endif
#endif
