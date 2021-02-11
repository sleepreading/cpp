/**
 * @file      z_locker.h
 * @date      2014-7-30 16:48:45
 * @author    leizi
 * @copyright zhanglei. email:847088355@qq.com
 * @remark    supply a auto locker and a mutex
 */
#ifndef __X_LOCKER_H__
#define __X_LOCKER_H__

#ifdef  _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <pthread.h>
#endif

class Mutex
{
#if defined(_WIN32)
#ifndef pthread_mutex_t
#define pthread_mutex_t         CRITICAL_SECTION
#endif
#ifndef pthread_mutex_init
#define pthread_mutex_init(x,y) InitializeCriticalSection(x)
#endif
#ifndef pthread_mutex_destroy
#define pthread_mutex_destroy   DeleteCriticalSection
#endif
#ifndef pthread_mutex_lock
#define pthread_mutex_lock      EnterCriticalSection
#endif
#ifndef pthread_mutex_unlock
#define pthread_mutex_unlock    LeaveCriticalSection
#endif
#endif  // WIN32
    pthread_mutex_t m_mutex;
public:
    Mutex()
    {
#if defined(__WIN32__) || defined(WIN32)
        pthread_mutex_init(&m_mutex, 0);
#elif defined(__linux__)
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&m_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
#else  //maybe is msdos etc...
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
#endif
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
};

class Locker
{
    Mutex &m_mutex;
public:
    Locker(Mutex &mutex) : m_mutex(mutex)
    {
        m_mutex.lock();
    }
    ~Locker()
    {
        m_mutex.unlock();
    }
};

#endif

