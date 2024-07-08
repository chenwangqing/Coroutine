
#include "InitList.h"

static CM_Init_Node_t *__cm_init_node_list = nullptr;

static void _fix_order(bool isMin)
{
    if (__cm_init_node_list == nullptr)
        return;
    CM_Init_Node_t *p         = __cm_init_node_list->next;
    __cm_init_node_list->next = nullptr;
    while (p != nullptr) {
        CM_Init_Node_t *r  = __cm_init_node_list;
        CM_Init_Node_t *up = r;
        while (r != nullptr && ((isMin && r->order < p->order) || (!isMin && r->order > p->order))) {
            up = r;
            r  = r->next;
        }
        CM_Init_Node_t *t = p;
        p                 = p->next;
        t->next           = nullptr;
        if (r == __cm_init_node_list) {
            t->next             = __cm_init_node_list;
            __cm_init_node_list = t;
        } else {
            t->next  = r;
            up->next = t;
        }
    }
    return;
}

void CM_Init_AddNode(CM_Init_Node_t *n)
{
    n->next             = __cm_init_node_list;
    __cm_init_node_list = n;
    return;
}

void CM_Init(void)
{
    _fix_order(true);
    CM_Init_Node_t *p = __cm_init_node_list;
    while (p != nullptr) {
        if (p->init != nullptr)
            p->init(p->init_val);
        p = p->next;
    }
    return;
}

void CM_DeInit(void)
{
    _fix_order(false);
    CM_Init_Node_t *p = __cm_init_node_list;
    while (p != nullptr) {
        if (p->deinit != nullptr)
            p->deinit(p->deinit_val);
        p = p->next;
    }
    return;
}
