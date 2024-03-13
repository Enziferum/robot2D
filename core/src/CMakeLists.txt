set(INCLROOT ${PROJECT_SOURCE_DIR}/include/robot2D)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option(RB_DEBUG "Enable debugging options" ON)
endif()

configure_file(${INCLROOT}/OptionConfig.hpp.in ${INCLROOT}/OptionConfig.hpp @ONLY)

add_subdirectory(Core)
add_subdirectory(Ecs)
add_subdirectory(Graphics)
add_subdirectory(Util)

set(ENGINE_INCLUDES
	${INCLROOT}/robot2D.hpp
        ${INCLROOT}/Engine.hpp
        ${INCLROOT}/Application.hpp
        
	${CORE_INCLUDE_FILES}
        ${ECS_INCLUDE_FILES}
        ${GRAPHICS_INCLUDE_FILES}
        ${UTIL_INCLUDE_FILES}

        PARENT_SCOPE
)

set(ENGINE_SRC
	${SRCROOT}/Engine.cpp
        ${SRCROOT}/Application.cpp

        ${CORE_SOURCE_FILES}
        ${ECS_SOURCE_FILES}
        ${GRAPHICS_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}

    
        PARENT_SCOPE
)