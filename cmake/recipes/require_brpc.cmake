
find_path(BRPC_INCLUDE_DIR NAMES brpc/server.h)
find_library(BRPC_LIBRARIES NAMES libbrpc.a)
if ((NOT BRPC_INCLUDE_DIR) OR (NOT BRPC_LIBRARIES))
    message(FATAL_ERROR "Fail to find brpc")
endif ()
