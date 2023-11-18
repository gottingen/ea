
find_path(ARROW_INCLUDE_DIR NAMES arrow/api.h)
find_library(ARROW_LIB NAMES arrow)
if ((NOT ARROW_INCLUDE_DIR) OR (NOT ARROW_LIB))
    message(FATAL_ERROR "Fail to find arrow")
endif ()