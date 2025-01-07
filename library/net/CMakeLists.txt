add_library(net SHARED
        src/Channel.cpp
        include/Channel.h
        src/EventLoop.cpp
        include/EventLoop.h
        src/Poller.cpp
        include/Poller.h
        src/DefaultPoller.cpp
        src/EPollPoller.cpp
        include/EPollPoller.h
        src/EventLoopThread.cpp
        include/EventLoopThread.h
        src/EventLoopThreadPool.cpp
        include/EventLoopThreadPool.h
        src/Acceptor.cpp
        include/Acceptor.h
        src/InetAddress.cpp
        include/InetAddress.h
        src/Socket.cpp
        include/Socket.h
        include/Callbacks.h
        src/Buffer.cpp
        include/Buffer.h
        src/TcpConnection.cpp
        include/TcpConnection.h
        src/TcpServer.cpp
        include/TcpServer.h)

target_link_libraries(net PUBLIC Thread_Pool base)