set(INCLROOT ${PROJECT_SOURCE_DIR}/include/robot2D/Core)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Core)

set(CORE_INCLUDE_FILES

        ${INCLROOT}/FileInputStream.hpp
        ${INCLROOT}/Event.hpp
        ${INCLROOT}/Window.hpp
        ${INCLROOT}/Keyboard.hpp
        ${INCLROOT}/Joystick.hpp
        ${INCLROOT}/Vector2.hpp
        ${INCLROOT}/MessageBus.hpp
        ${INCLROOT}/Message.hpp
        ${INCLROOT}/Time.hpp
        ${INCLROOT}/Clock.hpp
        ${INCLROOT}/Cursor.hpp
        ${INCLROOT}/memory/IAllocator.hpp
        ${INCLROOT}/memory/StackAllocator.hpp
        ${INCLROOT}/memory/MemoryManager.hpp

        PARENT_SCOPE
)

set(CORE_SOURCE_FILES
        ${SRCROOT}/FileInputStream.cpp
        ${SRCROOT}/Window.cpp
        ${SRCROOT}/Event.cpp
        ${SRCROOT}/WindowImpl.cpp
        ${SRCROOT}/WindowImpl.hpp
        ${SRCROOT}/WindowContext.cpp
        ${SRCROOT}/Keyboard.cpp
        ${SRCROOT}/Mouse.cpp
        ${SRCROOT}/Joystick.cpp
        ${SRCROOT}/Clock.cpp
        ${SRCROOT}/Time.cpp
        ${SRCROOT}/Cursor.cpp
        ${SRCROOT}/CursorImpl.cpp
        ${SRCROOT}/memory/IAllocator.cpp
        ${SRCROOT}/memory/StackAllocator.cpp
        ${SRCROOT}/memory/MemoryManager.cpp

        # Desktop Impl
        ${SRCROOT}/Desktop/DesktopWindowImpl.cpp
        ${SRCROOT}/Desktop/DesktopCursorImpl.cpp

        PARENT_SCOPE
    )