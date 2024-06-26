
#include "NodeLink.h"

void CM_NodeLink_Insert(CM_NodeLinkList_t *head, CM_NodeLink_t *node, CM_NodeLink_t *link)
{
    CM_NodeLink_Init(link);
    if (nullptr == *head)
        *head = link;
    else {
        CM_NodeLink_t *t = node;
        link->next       = t->next;
        link->up         = t;
        t->next->up      = link;
        t->next          = link;
    }
    return;
}

CM_NodeLink_t* CM_NodeLink_Remove(CM_NodeLinkList_t *list, CM_NodeLink_t *link)
{
    CM_NodeLink_t *next = link->next;
    if (link == *list)
        (*list) = link->next;
    link->next->up = link->up;
    link->up->next = link->next;
    if (link == *list)
        *list = nullptr;
    CM_NodeLink_Init(link);
    return next;
}
