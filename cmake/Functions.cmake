function(ProjectLog message)
    message(STATUS "[${PROJECT_NAME}] ${message}")
endfunction()

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
                #message(${files})
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

function(GroupSourcesByFolderCustomPath target relative_path)
    set(SOURCE_GROUP_DELIMITER "/")
    set(last_dir "")
    set(files "")
	message(WARNING ${relative_path})
    get_target_property(sources ${target} SOURCES)
    foreach(file ${sources})
        file(ABSOLUTE_PATH relative_file "${relative_path}" ${file})
        get_filename_component(dir "${relative_file}" PATH)
        if(NOT "${dir}" STREQUAL "${last_dir}")
            if(files)
                source_group("${last_dir}" FILES ${files})
                #message(${files})
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