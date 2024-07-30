/**
 * @file     Property.hpp
 * @brief    变量属性
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-24
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-24 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#ifndef __PROPERTY_HPP__
#define __PROPERTY_HPP__

/**
 * @brief    类成员属性
 * @tparam T 成员类型
 * @tparam C 类
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-24
 */
template<typename T, typename C = void>
class Property {
public:
    /**
     * @brief    读写
     * @param get    读取函数
     * @param set    写入函数
     * @note     Property<int, Node>::RW<&Node::_get, &Node::_set> rw;  Node() : rw(this)
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-24
     */
    template<T (C::*get)(void), void (C::*set)(const T &)>
    class RW {
    private:
        C *_class;

    public:
        RW(C *_class) :
            _class(_class) {}

        inline operator T() const
        {
            return (_class->*get)();
        }

        inline RW &operator=(const T &value)
        {
            (_class->*set)(value);
            return *this;
        }
    };

    /**
     * @brief    只读
     * @param get    读取函数
     * @note     Property<int, Node>::RO<&Node::_get>              r;  Node() : r(this)
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-24
     */
    template<T (C::*get)(void)>
    class RO {
    private:
        C *_class;

    public:
        RO(C *_class) :
            _class(_class) {}
        inline operator T() const
        {
            return (_class->*get)();
        }
    };

    /**
     * @brief    只写
     * @param set    写入函数
     * @note     Property<int, Node>::WO<&Node::_set>              w;  Node() : w(this)
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-24
     */
    template<void (C::*set)(const T &)>
    class WO {
    private:
        C *_class;

    public:
        WO(C *_class) :
            _class(_class) {}
        inline void operator=(const T &value)
        {
            (_class->*set)(value);
        }
    };
};

/**
 * @brief    变量属性
 * @tparam T 变量类型
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-24
 */
template<typename T>
class Property<T, void> {
public:
    /**
     * @brief    读写
     * @param get    读取函数
     * @param set    写入函数
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-24
     */
    template<T (*get)(void), void (*set)(const T &)>
    class RW {
    public:
        inline operator T() const
        {
            return (*get)();
        }

        inline RW &operator=(const T &value)
        {
            (*set)(value);
            return *this;
        }
    };

    /**
     * @brief    只读
     * @param get    读取函数
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-24
     */
    template<T (*get)(void)>
    class RO {
    public:
        inline operator T() const
        {
            return (*get)();
        }
    };

    /**
     * @brief    只写
     * @param set    写入函数
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-24
     */
    template<void (*set)(const T &)>
    class WO {
    public:
        inline void operator=(const T &value)
        {
            (*set)(value);
        }
    };
};
#endif
