/**
 * @file     Coroutine.hpp
 * @brief    协程C++接口
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-11
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-11 <td>1.0     <td>CXS     <td>创建
 * </table>
 */

#if !defined(__COROUTINE_HPP__)
#define __COROUTINE_HPP__
#include "Coroutine.h"
#include <functional>
namespace CO {

    template<typename R, class... Args>
    class _Function {
    private:
        using _Tuple = std::tuple<std::decay_t<Args>...>;
        _Tuple pars;
        R(*fn)
        (Args...);

        template<size_t...>
        struct tuple_idx
        {
        };

        template<size_t N, size_t... Is>
        struct tuple_bind : tuple_bind<N - 1, N - 1, Is...>
        {
        };

        template<size_t... Is>
        struct tuple_bind<0, Is...>
        {
            typedef tuple_idx<Is...> type;
        };

        template<size_t... Is>
        inline R extendTupleToProcess(tuple_idx<Is...> &&)
        {
            return this->fn(std::get<Is>(this->pars)...);
        }

    public:
        _Function(R (*fn)(Args...), Args... args)
        {
            this->fn   = fn;
            this->pars = _Tuple(std::forward<Args>(args)...);
        }

        inline R Call(void)
        {
            return extendTupleToProcess(typename tuple_bind<sizeof...(Args)>::type());
        }
    };


    /**
     * @brief    协程任务类
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-11
     */
    class Task {
    protected:
        explicit Task() {}

    public:
        /**
         * @brief    开始协程任务
         * @tparam Fn  void func(xxxx)
         * @tparam Args 形参列表
         * @param    name           任务名称
         * @param    pri            任务优先级
         * @param    stacks         任务栈大小
         * @param    fn             执行函数
         * @param    args           函数参数
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        template<typename Fn, class... Args>
        static bool Start(const char *name,
                          uint8_t     pri,
                          uint32_t    stacks,
                          Fn         &fn,
                          Args... args)
        {
            if ((void *)fn == nullptr)
                return false;
            auto n    = new _Function<void, Args...>(fn, std::forward<Args>(args)...);
            auto func = [](void *object) -> void {
                _Function<void, Args...> *n = static_cast<_Function<void, Args...> *>(object);
                n->Call();
                delete n;
            };
            if (Coroutine.AddTask(func, n, pri, stacks, name, nullptr) == nullptr) {
                delete n;
                return false;
            }
            return true;
        }

        /**
         * @brief    开始协程任务
         * @tparam Fn  void func(xxxx)
         * @tparam Args 形参列表
         * @param    stacks         栈大小
         * @param    fn             执行函数
         * @param    args           函数参数
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        template<typename Fn, class... Args>
        static inline bool Start(uint32_t stacks, Fn &fn, Args... args)
        {
            return Start(nullptr, TASK_PRI_NORMAL, stacks, fn, std::forward<Args>(args)...);
        }

        /**
         * @brief    开始协程任务
         * @tparam Fn  void func(xxxx)
         * @tparam Args 形参列表
         * @param    fn             执行函数
         * @param    args           函数参数
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        template<typename Fn, class... Args>
        static bool Start(const char *name, Fn &fn, Args... args)
        {
            auto n    = new _Function<void, Args...>(fn, std::forward<Args>(args)...);
            auto func = [](void *object) -> void {
                _Function<void, Args...> *n = static_cast<_Function<void, Args...> *>(object);
                n->Call();
                delete n;
            };
            if (Coroutine.AddTask(func, n, TASK_PRI_NORMAL, 0, name, nullptr) == nullptr) {
                delete n;
                return false;
            }
            return true;
        }

        /**
         * @brief    开始协程任务
         * @tparam Fn  void func(xxxx)
         * @tparam Args 形参列表
         * @param    fn             执行函数
         * @param    args           函数参数
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        template<typename Fn, class... Args>
        static inline bool Start(Fn &fn, Args... args)
        {
            return Start(nullptr, nullptr, fn, std::forward<Args>(args)...);
        }

        /**
         * @brief    【内部使用】转交控制权
         * @note     调用该函数将转交控制权给协程调度器，协程调度器会选择下一个可运行的协程并运行，直到所有协程都结束。
         * @return   * void
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        static inline void Yield()
        {
            Coroutine.Yield();
        }

        /**
         * @brief    【内部使用】转交控制权
         * @note     调用该函数将转交控制权给协程调度器，协程调度器会选择下一个可运行的协程并运行，直到所有协程都结束。
         * @return   * void
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        static inline void Yield(uint32_t ms)
        {
            Coroutine.YieldDelay(ms);
        }

        /**
         * @brief    【内部使用】转交控制权
         * @note     调用该函数将转交控制权给协程调度器，协程调度器会选择下一个可运行的协程并运行，直到所有协程都结束。
         * @return   * void
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        static inline void Sleep(uint32_t ms)
        {
            Coroutine.YieldDelay(ms);
        }

        /**
         * @brief    设置默认栈大小
         * @param    size
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        static inline void SetDefaultTaskStackSize(uint32_t size)
        {
            Coroutine.SetDefaultStackSize(size);
        }
    };

    /**
     * @brief    异步任务类
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-11
     */

    template<typename R>
    class Async {
    private:
        Coroutine_ASync async = nullptr;
        R              *ret   = nullptr;
        bool            isOk  = false;

    public:
        /**
         * @brief    异步任务
         * @tparam Fn  R func(xxxx)
         * @tparam Args 形参列表
         * @param    stacks         函数栈大小 0：默认
         * @param    fn             执行函数
         * @param    args           函数参数
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        template<typename Fn, class... Args>
        Async(uint32_t stacks, Fn &fn, Args... args)
        {
            auto n    = new _Function<R, Args...>(fn, std::forward<Args>(args)...);
            auto func = [](void *object) -> void * {
                _Function<R, Args...> *n   = static_cast<_Function<R, Args...> *>(object);
                R                     *ret = new R(n->Call());
                delete n;
                return ret;
            };
            this->async = Coroutine.Async(func, n, stacks);
            if (this->async == nullptr)
                delete n;
        }
        /**
         * @brief    等待异步任务完成
         * @param    timeout
         * @return   true
         * @return   false
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        bool Wait(uint32_t timeout = UINT32_MAX)
        {
            if (this->isOk) return true;
            if (this->async == nullptr)
                return false;
            if (Coroutine.AsyncWait(this->async, timeout)) {
                this->ret   = static_cast<R *>(Coroutine.AsyncGetResultAndDelete(this->async));
                this->isOk  = true;
                this->async = nullptr;
            }
            return this->isOk;
        }

        /**
         * @brief    获取结果
         * @return   * void*
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        inline R &GetResult(void)
        {
            return *this->ret;
        }

        virtual ~Async()
        {
            Wait();   // 等待异步任务完成
            if (this->ret) delete this->ret;
        }
    };

    /**
     * @brief    信号量
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-11
     */
    class Semaphore {
    private:
        Semaphore() {}
        Coroutine_Semaphore sem = nullptr;

    public:
        /**
         * @brief    创建信号量
         * @param    count          数量
         * @param    name           名称 31 字节
         * @return   Semaphore*
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        static Semaphore *Make(uint32_t count, const char *name = nullptr)
        {
            Semaphore *s = new Semaphore();
            s->sem       = Coroutine.CreateSemaphore(name, count);
            if (s->sem == nullptr) {
                delete s;
                s = nullptr;
            }
            return s;
        }

        /**
         * @brief    给予信号
         * @param    count          数量
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        inline void Give(uint32_t count = 1)
        {
            if (this->sem) Coroutine.GiveSemaphore(this->sem, count);
            return;
        }

        /**
         * @brief    等待信号
         * @param    count          等待数量
         * @param    timeout        等待时间 ms
         * @return   true           等待成功
         * @return   false          等待超时
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        inline bool Wait(uint32_t count, uint32_t timeout = UINT32_MAX)
        {
            if (this->sem) return Coroutine.WaitSemaphore(this->sem, count, timeout);
            return false;
        }

        virtual ~Semaphore()
        {
            if (this->sem) Coroutine.DeleteSemaphore(this->sem);
            this->sem = nullptr;
        }
    };

    /**
     * @brief    互斥锁
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-11
     */
    class Mutex {
    private:
        Coroutine_Mutex mutex = nullptr;


    public:
        /**
         * @brief    创建互斥锁
         * @param    name           名称 31 字节
         * @return   * Mutex*
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        Mutex(const char *name = nullptr)
        {
            mutex = Coroutine.CreateMutex(name);
        };

        /**
         * @brief    加锁
         * @param    timeout        等待时间
         * @return   true           加锁成功
         * @return   false          加锁超时
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        inline bool Lock(uint32_t timeout = UINT32_MAX)
        {
            if (this->mutex) return Coroutine.LockMutex(this->mutex, timeout);
            return false;
        }

        /**
         * @brief    解锁
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        inline void UnLock()
        {
            if (this->mutex) Coroutine.UnlockMutex(this->mutex);
            return;
        }

        virtual ~Mutex()
        {
            if (this->mutex) Coroutine.DeleteMutex(this->mutex);
            this->mutex = nullptr;
        }

        void operator=(const Mutex &) = delete;
        void operator=(const Mutex *) = delete;
    };

    /**
     * @brief    邮箱通信（发送不会阻塞）
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-11
     */
    template<typename T>
    class Mailbox {
    private:
        Coroutine_Mailbox mailbox = nullptr;

    public:
        Mailbox(const char *name = nullptr, uint32_t max_msg = 10)
        {
            this->mailbox = Coroutine.CreateMailbox(name, max_msg);
        }

        /**
         * @brief    发送数据
         * @param    id             邮件id
         * @param    msg            数据
         * @param    time           有效时间
         * @return   true
         * @return   false
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        bool Send(uint64_t id, T &&msg, uint32_t time)
        {
            if (this->mailbox == nullptr) return false;
            uint64_t data = 0;
            if (sizeof(T) <= 8)
                memcpy(&data, &msg, sizeof(T));
            else {
                data = (uint64_t) new T(std::forward<T>(msg));
                if (data == 0) return false;
            }
            bool re = Coroutine.SendMail(this->mailbox, id, data, time);
            if (!re && sizeof(T) > 8)
                delete (T *)data;
            return re;
        }

        /**
         * @brief    接收数据（阻塞接收）
         * @param    timeout        接收超时
         * @param    id_mask        id掩码
         * @return   std::tuple<bool, uint64_t, T>
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        std::tuple<bool, uint64_t, T> Receive(uint32_t timeout = UINT32_MAX, uint64_t id_mask = UINT64_MAX)
        {
            if (this->mailbox == nullptr) return std::make_tuple(false, 0, T());
            auto re = Coroutine.ReceiveMail(this->mailbox, id_mask, timeout);
            T    data;
            if (re.isOk) {
                if (sizeof(T) <= 8)
                    memcpy(&data, &re.data, sizeof(T));
                else {
                    data = std::move(*(T *)re.data);
                    delete (T *)re.data;
                }
            }
            return {re.isOk, re.id, std::move(data)};
        }

        virtual ~Mailbox()
        {
            if (this->mailbox) Coroutine.DeleteMailbox(this->mailbox);
            this->mailbox = nullptr;
        }

        void operator=(const Mailbox &) = delete;
        void operator=(const Mailbox *) = delete;
    };

    /**
     * @brief    管道通信（发送会阻塞）
     * @tparam T
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-11
     */
    template<typename T>
    class Channel {
    private:
        Coroutine_Channel ch = nullptr;

    public:
        explicit Channel(uint32_t caches = 0, const char *name = nullptr)
        {
            this->ch = Coroutine.CreateChannel(name, caches);
        }

        explicit Channel(uint32_t caches)
        {
            this->ch = Coroutine.CreateChannel(nullptr, caches);
        }

        explicit Channel(const char *name)
        {
            this->ch = Coroutine.CreateChannel(name, 0);
        }

        /**
         * @brief    写通道数据(！！！不能在协程以外的地方使用！！！)
         * @param    msg            数据
         * @param    timeout        等待超时
         * @return   true
         * @return   false
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        bool Write(T &&msg, uint32_t timeout = UINT32_MAX)
        {
            if (this->ch == nullptr) return false;
            uint64_t data = 0;
            if (sizeof(T) <= 8)
                memcpy(&data, &msg, sizeof(T));
            else {
                data = (uint64_t) new T(std::forward<T>(msg));
                if (data == 0) return false;
            }
            bool re = Coroutine.WriteChannel(this->ch, data, timeout);
            if (!re && sizeof(T) > 8)
                delete (T *)data;
            return re;
        }

        /**
         * @brief    读通道数据(！！！不能在协程以外的地方使用！！！)
         * @param    timeout        等待超时
         * @return   std::tuple<bool, T>
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        std::tuple<bool, T> Read(uint32_t timeout)
        {
            if (this->ch == nullptr) return std::make_tuple(false, T());
            T        data;
            uint64_t rs = 0;
            bool     re = Coroutine.ReadChannel(this->ch, &rs, timeout);
            if (re) {
                if (sizeof(T) <= 8)
                    memcpy(&data, &rs, sizeof(T));
                else {
                    data = std::move(*(T *)rs);
                    delete (T *)rs;
                }
            }
            return {re, std::move(data)};
        }


        /**
         * @brief    读通道数据(！！！不能在协程以外的地方使用！！！)
         * @return   std::tuple<bool, T>
         * @author   CXS (chenxiangshu@outlook.com)
         * @date     2024-07-11
         */
        T Read(void)
        {
            auto ret = Read(UINT32_MAX);
            if (std::get<0>(ret))
                return std::get<1>(ret);
            else
                return T();
        }

        virtual ~Channel()
        {
            if (this->ch) Coroutine.DeleteChannel(this->ch);
            this->ch = nullptr;
        }

        void operator=(const Channel &) = delete;
        void operator=(const Channel *) = delete;
    };
}   // namespace CO
#endif   // __COROUTINE_HPP__
