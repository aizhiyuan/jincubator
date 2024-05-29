# 定义变量
# SET(LIB_ZLOG_FILES sqlite3.c)

#目录下所有源文件
aux_source_directory(${CMAKE_SOURCE_DIR}/zlog LIB_ZLOG_FILES)

# 输出动态库
#ADD_LIBRARY(sqlite3 SHARED ${LIB_ZLOG_FILES})

# 输出静态库
ADD_LIBRARY(zlog STATIC ${LIB_ZLOG_FILES})

# 修改静态库名称
SET_TARGET_PROPERTIES(zlog PROPERTIES VERSION 1.0 SOVERSION 1)


