//
// Created by guo on 25-1-5.
//

#ifndef WEBSERVER_CURRENTTHREAD_H
#define WEBSERVER_CURRENTTHREAD_H


namespace CurrentThread {
    extern thread_local int t_cachedTid;

    void cacheTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }

}


#endif //WEBSERVER_CURRENTTHREAD_H
