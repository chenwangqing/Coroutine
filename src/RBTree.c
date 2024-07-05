
#include "RBTree.h"

#define RB_RED   0
#define RB_BLACK 1

/**
 * @brief    获取父指针
 * @param    n              
 * @return   CM_RBTree_Link_t*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-11
 */
#define _rbtree_GetParent(n) ((CM_RBTree_Link_t *)((size_t)(n)->Parent & ~((size_t)0x01)))

/**
 * @brief    设置父指针
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-11
 */
#define _rbtree_SetParent(n, p) (n)->Parent = (size_t)(p) | ((n)->Parent & 0x01)

/**
 * @brief    获取颜色
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-11
 */
#define _rbtree_GetColor(n) ((n)->Parent & 0x01)

/**
 * @brief    设置颜色
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-11
 */
#define _rbtree_SetColor(n, color) (n)->Parent = ((n)->Parent & ~((size_t)0x01)) | (color)

/* 性质：
    性质1. 结点是红色或黑色。
    性质2. 根结点是黑色。
    性质3. 所有叶子都是黑色。（叶子是NIL结点）
    性质4. 每个红色结点的两个子结点都是黑色。（从每个叶子到根的所有路径上不能有两个连续的红色结点）
    性质5. 从任一节结点其每个叶子的所有路径都包含相同数目的黑色结点。
*/
/*
 * 对红黑树的节点(y)进行右旋转
 *
 * 右旋示意图(对节点y进行左旋)：
 *            py                               py
 *           /                                /
 *          y                                x
 *         /  \      --(右旋)-->            /  \                     #
 *        x   ry                           lx   y
 *       / \                                   / \                   #
 *      lx  rx                                rx  ry
 *
 */
//函数名：_rbtree_Rigth_Rotate
//参  数：y:节点
//作  用：右旋
//日  期：2018年3月17日
//返  回：void
static void _rbtree_Rigth_Rotate(CM_RBTree_t *k, CM_RBTree_Link_t *y)
{
    CM_RBTree_Link_t *x  = y->L;
    y->L                 = x->R;
    CM_RBTree_Link_t *yp = _rbtree_GetParent(y);
    if (x->R != &k->NIL)
        _rbtree_SetParent(x->R, y);   // x->R->P = y;
    _rbtree_SetParent(x, yp);         // x->P = y->P;
    if (yp == &k->NIL)                // if (y->P == &k->NIL)
        k->root = x;                  //
    else if (y == yp->R)              // else if (y == y->P->R)
        yp->R = x;                    //  y->P->R = x;
    else                              //
        yp->L = x;                    //  y->P->L = x;
    x->R = y;                         //
    _rbtree_SetParent(y, x);          // y->P = x;
    return;
}

/*
 * 对红黑树的节点(x)进行左旋转
 *
 * 左旋示意图(对节点x进行左旋)：
 *      px                              px
 *     /                               /
 *    x                               y
 *   /  \      --(左旋)-->           / \                #
 *  lx   y                          x  ry
 *     /   \                       /  \
 *    ly   ry                     lx  ly
 *
 *
 */
//函数名：_rbtree_Left_Rotate
//参  数：x：支点
//作  用：左旋
//日  期：2018年3月17日
//返  回：void
static void _rbtree_Left_Rotate(CM_RBTree_t *k, CM_RBTree_Link_t *x)
{
    CM_RBTree_Link_t *y  = x->R;
    x->R                 = y->L;
    CM_RBTree_Link_t *xp = _rbtree_GetParent(x);
    if (y->L != &k->NIL)
        _rbtree_SetParent(y->L, x);   // y->L->P = x;
    _rbtree_SetParent(y, xp);         // y->P = x->P;
    if (xp == &k->NIL)                // if (x->P == &k->NIL)
        k->root = y;                  //
    if (x == xp->L)                   // else if (x == x->P->L)
        xp->L = y;                    //  x->P->L = y;
    else                              //
        xp->R = y;                    //   x->P->R = y;
    y->L = x;                         //
    _rbtree_SetParent(x, y);          //  x->P = y;
    return;
}

//函数名：_rbtree_Insert_Fixup
//参  数：z：修正的节点
//作  用：插入修正
//日  期：2018年3月17日
//返  回：void
static void _rbtree_Insert_Fixup(CM_RBTree_t *k, CM_RBTree_Link_t *z)
{
    // 在下面的代码中z结点总是违反性质4的那个结点
    while (_rbtree_GetColor(_rbtree_GetParent(z)) == RB_RED) {   // while (z->P->color == RB_RED) {

        // x是红色,它的父结点也是红色就说明性质4被违反,要持续调整
        // 下面的过程按z->p是其祖父结点的左孩子还是右儿子进行分类讨论
        CM_RBTree_Link_t *zp  = _rbtree_GetParent(z);
        CM_RBTree_Link_t *zpp = _rbtree_GetParent(zp);
        if (zp == zpp->L) {   // if (z->P == z->P->P->L) {

            /// 父结点是其祖父结点的左孩子
            CM_RBTree_Link_t *y = zpp->R;   //  z->P->P->R;   // 表示z的叔结点

            // 下面按y的颜色进行分类讨论
            if (_rbtree_GetColor(y) == RB_RED) {   //  if (y->color == RB_RED) {

                // 如果y是红色并z的祖父结点一定是黑色的,这时我们通过下面的染色过程
                // 在保证黑高不变的情况下(性质5),将z在树中上移两层,z=z->p->p

                _rbtree_SetColor(zp, RB_BLACK);   // z->P->color    = RB_BLACK;
                _rbtree_SetColor(zpp, RB_RED);    // z->P->P->color = RB_RED;
                _rbtree_SetColor(y, RB_BLACK);    // y->color       = RB_BLACK;

                z = zpp;   // 如果上移到根节点或某个父结点不为红的结点就可以结束循环了
            } else {
                // 叔结点为黑色
                // 此时要根据z是其父结点的左孩子还是右孩子进行分类讨论
                // 如果z是左孩子则可以直接可以通过染色和右旋来恢复性质
                // 如果z是右孩子则可以先左旋来转成右孩子的情况

                if (z == zp->R) {                // if (z == z->P->R) {
                    z = zp;                      // z = z->P;
                    _rbtree_Left_Rotate(k, z);   // 直接左旋
                }
                // 重新染色,再右旋就可以恢复性质
                CM_RBTree_Link_t *zp  = _rbtree_GetParent(z);
                CM_RBTree_Link_t *zpp = _rbtree_GetParent(zp);
                _rbtree_SetColor(zp, RB_BLACK);   // z->P->color    = RB_BLACK;
                _rbtree_SetColor(zpp, RB_RED);    // z->P->P->color = RB_RED;
                _rbtree_Rigth_Rotate(k, zpp);     // _rbtree_Rigth_Rotate(k, z->P->P);
            }
        } else {
            // 父结点是祖父结点的右孩子
            CM_RBTree_Link_t *y = zpp->L;          // z->P->P->L;   // 表示z的叔结点
            if (_rbtree_GetColor(y) == RB_RED) {   // if (y->color == RB_RED) {
                _rbtree_SetColor(zp, RB_BLACK);    // z->P->color    = RB_BLACK;
                _rbtree_SetColor(y, RB_BLACK);     // y->color       = RB_BLACK;
                _rbtree_SetColor(zpp, RB_RED);     // z->P->P->color = RB_RED;
                z = zpp;                           // z->P->P;
            } else {
                // 右儿子的时候可以直接左旋,重新调色恢复性质
                // 左儿子可以先右旋成右儿子再处理

                if (z == zp->L) {   // if (z == z->P->L) {
                    z = zp;
                    _rbtree_Rigth_Rotate(k, z);
                }
                CM_RBTree_Link_t *zp  = _rbtree_GetParent(z);
                CM_RBTree_Link_t *zpp = _rbtree_GetParent(zp);
                _rbtree_SetColor(zp, RB_BLACK);   // z->P->color    = RB_BLACK;
                _rbtree_SetColor(zpp, RB_RED);    // z->P->P->color = RB_RED;
                _rbtree_Left_Rotate(k, zpp);      // _rbtree_Left_Rotate(k, z->P->P);
            }
        }
    }
    // 将根节点染成黑色，是必要的步骤；处理两种情况
    // 1.第一次插入根结点被染成红色的情况
    // 2.和在上面的循环中根节点可能被染成红色的情况

    _rbtree_SetColor(k->root, RB_BLACK);   // k->root->color = RB_BLACK;
    return;
}

//函数名：_rbtree_Insert
//参  数：p:节点
//作  用：插入
//日  期：2018年3月16日
//返  回：void
static void _rbtree_Insert(CM_RBTree_t *k, CM_RBTree_Link_t *z)
{
    if (z == NULL || z == &k->NIL)
        return;
    CM_RBTree_Link_t *y = &k->NIL;
    CM_RBTree_Link_t *x = k->root;
    while (x != &k->NIL) {
        y = x;
        if (k->CompareFunc(k, z, x) < 0)   // if (z->key < x->key)
            x = x->L;
        else
            x = x->R;
    }
    _rbtree_SetParent(z, y);   // z->P = y;
    if (y == &k->NIL)
        k->root = z;
    else if (k->CompareFunc(k, z, y) < 0)   // (z->key < y->key)
        y->L = z;
    else
        y->R = z;
    z->L = &k->NIL;
    z->R = &k->NIL;
    _rbtree_SetColor(z, RB_RED);   // z->color = RB_RED;
    _rbtree_Insert_Fixup(k, z);
    return;
}

//函数名：_rbtree_Transplant
//参  数：u:删除者；v:后补
//作  用：替换
//日  期：2018年3月16日
//返  回：void
static void _rbtree_Transplant(CM_RBTree_t *k, CM_RBTree_Link_t *u, CM_RBTree_Link_t *v)
{
    CM_RBTree_Link_t *up = _rbtree_GetParent(u);
    if (up == &k->NIL)          // if (u->P == &k->NIL)
        k->root = v;            //
    else if (u == up->R)        // else if (u == u->P->R)
        up->R = v;              //     u->P->R = v;
    else                        //
        up->L = v;              //     u->P->L = v;
    _rbtree_SetParent(v, up);   // v->P = u->P;
    return;
}

//函数名：_rbtree_FindMix
//参  数：x：节点
//作  用：查找最小节点
//日  期：2018年3月16日
//返  回：节点
static CM_RBTree_Link_t *_rbtree_FindMix(CM_RBTree_t *k, CM_RBTree_Link_t *x)
{
    while (x->L != &k->NIL)
        x = x->L;
    return x;
}

//函数名：_rbtree_Delete_Fixup
//参  数：z：修正的节点
//作  用：修正删除后的树
//日  期：2018年3月16日
//返  回：void
static void _rbtree_Delete_Fixup(CM_RBTree_t *k, CM_RBTree_Link_t *z)
{
    if (z == NULL)
        return;
    CM_RBTree_Link_t *brothers = NULL;
    while (z != k->root && _rbtree_GetColor(z) == RB_BLACK) {   //  while (z != k->root && z->color == RB_BLACK) {
        //情况1：参照结点z的兄弟B是红色的
        CM_RBTree_Link_t *zp = _rbtree_GetParent(z);
        if (zp->L == z) {       //  if (z->P->L == z) {
            brothers = zp->R;   // z->P->R;   //右兄
            if (_rbtree_GetColor(brothers) == RB_RED) {
                _rbtree_SetColor(zp, RB_RED);           // z->P->color     = RB_RED;
                _rbtree_SetColor(brothers, RB_BLACK);   // brothers->color = RB_BLACK;
                _rbtree_Left_Rotate(k, zp);             //
                brothers = zp->R;                       // z->P->R;
            }
            //情况2：参照结点N的兄弟B是黑色的，且B的两个孩子都是黑色的
            if (_rbtree_GetColor(brothers->L) == RB_BLACK && _rbtree_GetColor(brothers->R) == RB_BLACK) {
                _rbtree_SetColor(brothers, RB_RED);   // brothers->color = RB_RED;
                z = zp;                               // z->P;
            } else {
                //情况3：参照结点N的兄弟B是黑色的，且B的左孩子是红色的，右孩子是黑色的
                if (_rbtree_GetColor(brothers->R) == RB_BLACK) {
                    _rbtree_SetColor(brothers, RB_RED);        // brothers->color    = RB_RED;
                    _rbtree_SetColor(brothers->L, RB_BLACK);   // brothers->L->color = RB_BLACK;
                    _rbtree_Rigth_Rotate(k, brothers);
                    brothers = zp->R;   // z->P->R;
                }
                //情况4：参照结点N的兄弟B是黑色的，且B的左孩子是黑色的，右孩子是红色的
                //  brothers->color = z->P->color;
                _rbtree_SetColor(brothers, _rbtree_GetColor(zp));
                //  z->P->color = brothers->R->color = RB_BLACK;
                _rbtree_SetColor(zp, RB_BLACK);
                _rbtree_SetColor(brothers->R, RB_BLACK);
                _rbtree_Left_Rotate(k, zp);
                z = k->root;
            }
        } else {
            //情况1：参照结点z的兄弟B是红色的
            brothers = zp->L;   // z->P->L;
            if (_rbtree_GetColor(brothers) == RB_RED) {
                _rbtree_SetColor(zp, RB_RED);           // z->P->color     = RB_RED;
                _rbtree_SetColor(brothers, RB_BLACK);   // brothers->color = RB_BLACK;
                _rbtree_Rigth_Rotate(k, zp);
                brothers = zp->L;
            }
            //情况2：参照结点N的兄弟B是黑色的，且B的两个孩子都是黑色的
            else if (_rbtree_GetColor(brothers->L) == RB_BLACK && _rbtree_GetColor(brothers->R) == RB_BLACK) {
                _rbtree_SetColor(brothers, RB_RED);   // brothers->color = RB_RED;
                z = zp;
            } else {
                //情况3：参照结点N的兄弟B是黑色的，且B的右孩子是红色的，左孩子是黑色的
                if (_rbtree_GetColor(brothers->L) == RB_BLACK) {
                    _rbtree_SetColor(brothers, RB_RED);        // brothers->color    = RB_RED;
                    _rbtree_SetColor(brothers->R, RB_BLACK);   // brothers->R->color = RB_BLACK;
                    _rbtree_Left_Rotate(k, brothers);
                    brothers = zp->L;
                }
                //情况4：参照结点N的兄弟B是黑色的，且B的右孩子是黑色的，左孩子是红色的
                // brothers->color = z->P->color;
                _rbtree_SetColor(brothers, _rbtree_GetColor(zp));
                // z->P->color = brothers->L->color = RB_BLACK;
                _rbtree_SetColor(zp, RB_BLACK);
                _rbtree_SetColor(brothers->L, RB_BLACK);
                _rbtree_Rigth_Rotate(k, zp);
                z = k->root;
            }
        }
    }
    // z->color = RB_BLACK;   // z是根
    _rbtree_SetColor(z, RB_BLACK);
    return;
}

//函数名：_rbtree_Delete
//参  数：z:要删除的节点
//作  用：红黑树删除一个节点
//日  期：2018年3月16日
//返  回：void
static void _rbtree_Delete(CM_RBTree_t *k, CM_RBTree_Link_t *z)
{
    if (z == NULL || z == &k->NIL)
        return;
    char              color = _rbtree_GetColor(z);   // z->color;   //记录删除节点的颜色
    CM_RBTree_Link_t *x     = NULL;
    if (z->L == &k->NIL) {
        x = z->R;
        _rbtree_Transplant(k, z, z->R);
    } else if (z->R == &k->NIL) {
        x = z->L;
        _rbtree_Transplant(k, z, z->L);
    } else {
        CM_RBTree_Link_t *y = _rbtree_FindMix(k, z->R);   //查找z右子树最小的节点
        color               = _rbtree_GetColor(y);        // y->color;
        x                   = y->R;
        if (_rbtree_GetParent(y) == z)   //  if (y->P == z)
            _rbtree_SetParent(x, y);     // x->P = y;
        else {
            _rbtree_Transplant(k, y, y->R);   //把mix（最小）取出来，原来的用&k->NIL补上
                                              //把删除节点的原右边树给接上
            y->R = z->R;
            _rbtree_SetParent(y->R, y);   // y->R->P = y;
        }
        _rbtree_Transplant(k, z, y);   //最小的节点替换删除的节点
                                       //把删除节点的原左边树给接上
        y->L = z->L;
        _rbtree_SetParent(y->L, y);                 //  y->L->P  = y;
        _rbtree_SetColor(y, _rbtree_GetColor(z));   // y->color = z->color;
    }
    if (color == RB_BLACK) {
        //修正
        _rbtree_Delete_Fixup(k, x);
    }
    return;
}

void CM_RBTree_Init(CM_RBTree_t *tree, CM_RBTree_Callback_Compare_t func)
{
    if (tree == nullptr)
        return;
    CM_ZERO(tree);
    _rbtree_SetColor(&tree->NIL, RB_BLACK);   // tree->NIL.color   = RB_BLACK;
    tree->count       = 0;
    tree->root        = &tree->NIL;
    tree->CompareFunc = func;
    tree->NIL.L       = &tree->NIL;
    tree->NIL.R       = &tree->NIL;
    _rbtree_SetParent(&tree->NIL, &tree->NIL);
    return;
}

void CM_RBTree_Insert(CM_RBTree_t *tree, CM_RBTree_Link_t *n)
{
    if (tree == nullptr || n == nullptr)
        return;
    n->L = n->R = &tree->NIL;
    n->Parent   = 0;
    _rbtree_SetParent(n, &tree->NIL);   // n->P = &tree->NIL;
    _rbtree_Insert(tree, n);
    tree->count++;
    return;
}

void CM_RBTree_Remove(CM_RBTree_t *tree, CM_RBTree_Link_t *n)
{
    if (tree == nullptr || n == nullptr || n->Parent == 0)
        return;
    _rbtree_Delete(tree, n);
    CM_ZERO(n);
    tree->count--;
    return;
}

CM_RBTree_Link_t *CM_RBTree_LeftEnd(CM_RBTree_t *tree)
{
    if (tree == nullptr)
        return nullptr;
    CM_RBTree_Link_t *p = tree->root;
    while (p->L != &tree->NIL)
        p = p->L;
    return p == &tree->NIL ? nullptr : p;
}

CM_RBTree_Link_t *CM_RBTree_RightEnd(CM_RBTree_t *tree)
{
    if (tree == nullptr)
        return nullptr;
    CM_RBTree_Link_t *p = tree->root;
    while (p->L != &tree->NIL)
        p = p->L;
    return p == &tree->NIL ? nullptr : p;
}

static const CM_RBTree_Link_t *_rbtree_Traverse(const CM_RBTree_t *           k,
                                                const CM_RBTree_Link_t *      n,
                                                int                           mode,
                                                CM_RBTree_Callback_Traverse_t func,
                                                void *                        obj)
{
    const CM_RBTree_Link_t *re = nullptr;
    if (n == &k->NIL)
        return NULL;
    if (mode < 0 && !func(k, n, obj))   // 前序
        return n;
    re = _rbtree_Traverse(k, n->L, mode, func, obj);
    if (re != NULL)
        return re;
    if (mode == 0 && !func(k, n, obj))   // 中序
        return n;
    re = _rbtree_Traverse(k, n->R, mode, func, obj);
    if (re != NULL)
        return re;
    if (mode > 0 && !func(k, n, obj))   // 后序
        return n;
    return re;
}

const CM_RBTree_Link_t *CM_RBTree_Traverse(const CM_RBTree_t *           tree,
                                           int                           mode,
                                           CM_RBTree_Callback_Traverse_t func,
                                           void *                        object)
{
    if (tree == nullptr || func == nullptr)
        return nullptr;
    return _rbtree_Traverse(tree, tree->root, mode, func, object);
}

uint32_t CM_RBTree_GetCount(CM_RBTree_t *tree)
{
    return tree == nullptr ? 0 : tree->count;
}

static bool _CM_RBTree_Clear_TraverseFunc(const CM_RBTree_t *tree, const CM_RBTree_Link_t *node, void *object)
{
    CM_RBTree_Callback_free_t func = (CM_RBTree_Callback_free_t)object;
    func((CM_RBTree_t *)tree, (CM_RBTree_Link_t *)node);
    return true;
}

void CM_RBTree_Clear(CM_RBTree_t *tree, CM_RBTree_Callback_free_t func)
{
    if (tree == nullptr || func == nullptr)
        return;
    CM_RBTree_Traverse(tree, 1, _CM_RBTree_Clear_TraverseFunc, func);
    CM_RBTree_Init(tree, tree->CompareFunc);
    return;
}
