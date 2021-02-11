/**
 * @file      zthread.h
 * @date      2013-7-29 10:47:46
 * @author    leizi
 * @copyright zhanglei. email:847088355@qq.com
 * @remark    thread start/wait functions
 */
#ifndef __Z_THREAD_H__
#define __Z_THREAD_H__

#if defined(__linux__) || defined(__MSDOS__) || (defined(__GNUC__) && __GNUC__>=4)
    #include <pthread.h>
    typedef void *(PFThreadRoutine)(void *);
#else  // WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <process.h>
    #include <windows.h> //win api & types
    typedef unsigned int (__stdcall PFThreadRoutine)(void *);
#endif // __linux__...

/**
 * long spawn
 * @brief   start a thread
 * @return  Error: (unsigned long)-1
            OK: thread ID
 */
uintptr_t zspawn(PFThreadRoutine func, void *arg, unsigned int stack_size = 0)
{
    uintptr_t pid = -1L; //0xffffffff
#if defined(_WIN32)
    pid = _beginthreadex(NULL, stack_size, func, arg, 0, NULL);
#else  // __linux__
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stack_size);
    if (pthread_create(&pid, 0, func, arg)) pid = -1L;
    pthread_attr_destroy(&attr);
#endif
    return pid;
}

void zjoin(HANDLE pid)
{
#if defined(_WIN32)
    WaitForSingleObject((HANDLE)pid, INFINITE);
    CloseHandle((HANDLE)pid);
#else
    pthread_join((pthread_t)pid, 0);
#endif
}

int  zkill(unsigned long tid)
{
#if defined(_WIN32)
    DWORD dwRes = 0;
    if (GetExitCodeThread((HANDLE)tid, &dwRes) && dwRes == (DWORD)0x00000103L)
        return 1;
#else
    int ret = pthread_kill(tid, 0);
    if (3 != ret && 22 != ret) {
        return 1; //alive
    }
#endif
    return 0; //doen't exists or has already exited;
}


#endif  // __ZTHREAD_H__

