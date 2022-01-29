cmake_minimum_required(VERSION 3.0)
project(webServer)

set(CMAKE_CXX_STANDARD 14)
link_libraries(pthread)



add_executable(main main.cpp
        httpConnection/parse/parse.h httpConnection/parse/parse.cpp
        utils/utils.h utils/utils.cpp
        webServer.h webServer.cpp
        config/config.h
        threadPool/threadPool.h
        httpConnection/httpConnection.h httpConnection/httpConnection.cpp
        timerManage/minHeap/minHeap.h
        timerManage/timer/timer.h
        timerManage/timerManage.h timerManage/timerManage.cpp)
