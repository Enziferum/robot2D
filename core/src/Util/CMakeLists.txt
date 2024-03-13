set(INCLROOT ${PROJECT_SOURCE_DIR}/include/robot2D/Util)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Util)


set(UTIL_INCLUDE_FILES
   ${INCLROOT}/ResourceHandler.hpp
   ${INCLROOT}/Logger.hpp
   PARENT_SCOPE)

set(UTIL_SOURCE_FILES
    ${SRCROOT}/Logger.cpp

    PARENT_SCOPE)