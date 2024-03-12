include(Diagnostics)
include(Options)
include(CompilerOptions)
include(FindCoreDeps)

# - brief processing core
print_diagnostic()
process_logging_options()
process_logging_compiler_flags()
process_logging_core_deps()

# use spdlog for some time
add_subdirectory(engine)

#for Freetype and Windows TODO(a.raag): check compiler
list(APPEND CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/extlibs/libs-msvc-universal/x64")	


add_library(${PROJECT_NAME} ${ENGINE_INCLUDES} ${ENGINE_SRC})
set(LIBS ${LIBS} glfw spdlog::spdlog)

function(GroupSourcesByFolder target) 
  set(SOURCE_GROUP_DELIMITER "/")
  set(last_dir "")
  set(files "")

  get_target_property(sources ${target} SOURCES)
  foreach(file ${sources})                                            
    file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file}) 
    get_filename_component(dir "${relative_file}" PATH)               
    if(NOT "${dir}" STREQUAL "${last_dir}")
      if(files)
        source_group("${last_dir}" FILES ${files})
        message(${files})
      endif()
      set(files "")
    endif()
    set(files ${files} ${file})
    set(last_dir "${dir}")
  endforeach()

  if(files)
    source_group("${last_dir}" FILES ${files})
  endif()
endfunction()
GroupSourcesByFolder(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/include>"
        INTERFACE
        "$<INSTALL_INTERFACE:include>")
		
list(APPEND CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/extlibs/headers/freetype2")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules/")

find_package(Freetype REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE Freetype::Freetype)

if(RB2D_INSTALL_CORE)
    include(cmake/InstallCore.cmake)
endif ()

