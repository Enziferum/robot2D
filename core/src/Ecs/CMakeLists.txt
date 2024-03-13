set(INCLROOT ${PROJECT_SOURCE_DIR}/include/robot2D/Ecs)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Ecs)

set(ECS_INCLUDE_FILES
    ${INCLROOT}/Bitmask.hpp
    ${INCLROOT}/Component.hpp
    ${INCLROOT}/Defines.hpp
    ${INCLROOT}/Entity.hpp
    ${INCLROOT}/EntityManager.hpp
    ${INCLROOT}/Scene.hpp
    ${INCLROOT}/System.hpp
    ${INCLROOT}/SystemManager.hpp
    PARENT_SCOPE
)

set(ECS_SOURCE_FILES
    ${SRCROOT}/Bitmask.cpp
    ${SRCROOT}/Component.cpp
    ${SRCROOT}/Entity.cpp
    ${SRCROOT}/EntityManager.cpp
    ${SRCROOT}/Scene.cpp
    ${SRCROOT}/System.cpp
    ${SRCROOT}/SystemManager.cpp
    PARENT_SCOPE
)