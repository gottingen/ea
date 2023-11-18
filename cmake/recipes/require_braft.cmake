
find_path(BRAFT_INCLUDE_DIR NAMES braft/raft.h)
find_library(BRAFT_LIB NAMES braft)
if ((NOT BRAFT_INCLUDE_DIR) OR (NOT BRAFT_LIB))
    message(FATAL_ERROR "Fail to find braft")
endif ()