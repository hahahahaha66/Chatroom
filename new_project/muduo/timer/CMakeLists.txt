# timer模块

set(TIMER_SRC
    Timer.cpp
    TimerQueue.cpp
)

target_sources(mynet PRIVATE ${TIMER_SRC})
target_include_directories(mynet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
