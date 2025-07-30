# logging模块

set(LOGGING_SRC
    AsyncLogging.cpp
    FileUtil.cpp
    LogFile.cpp
    Logging.cpp
    LogStream.cc
)

target_sources(mynet PRIVATE ${LOGGING_SRC})
target_include_directories(mynet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
