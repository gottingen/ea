

find_path(ICONV_INCLUDE_DIR NAMES iconv.h)
find_library(ICONV_LIBRARIES NAMES iconv)
if ((NOT ICONV_INCLUDE_DIR) OR (NOT ICONV_LIBRARIES))
    message(FATAL_ERROR "Fail to find iconv")
endif ()