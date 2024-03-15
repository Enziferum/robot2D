# Box2D patching
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
add_compile_definitions(B2_USER_SETTINGS)
configure_file(src/physics/b2_user_settings.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/box2d/include/box2d/b2_user_settings.h @ONLY)
add_subdirectory(extlibs/box2d)