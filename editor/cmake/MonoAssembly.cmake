include(CMakeParseArguments)

# A file-producing rule lets Ninja/MSBuild track source and reference changes.
function(add_mono_assembly)
    cmake_parse_arguments(ASSEMBLY "" "TARGET;DESTINATION;TYPE" "SOURCES;REFERENCES;ADDITIONAL_ARGS" ${ARGN})
    if(ASSEMBLY_UNPARSED_ARGUMENTS OR NOT ASSEMBLY_TARGET OR NOT ASSEMBLY_SOURCES)
        message(FATAL_ERROR "add_mono_assembly requires TARGET and SOURCES; unknown arguments: ${ASSEMBLY_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT MONO_MCS_EXECUTABLE OR NOT EXISTS "${MONO_MCS_EXECUTABLE}")
        message(FATAL_ERROR "C# compiler not found: '${MONO_MCS_EXECUTABLE}'. Install Mono mcs or set MONO_MCS_EXECUTABLE.")
    endif()
    if(NOT ASSEMBLY_TYPE)
        set(ASSEMBLY_TYPE library)
    endif()
    if(ASSEMBLY_TYPE STREQUAL "library")
        set(extension dll)
    elseif(ASSEMBLY_TYPE STREQUAL "exe")
        set(extension exe)
    else()
        message(FATAL_ERROR "C# assembly TYPE must be library or exe")
    endif()
    if(NOT ASSEMBLY_DESTINATION)
        set(ASSEMBLY_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    get_filename_component(destination "${ASSEMBLY_DESTINATION}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(output "${destination}/${ASSEMBLY_TARGET}.${extension}")
    set(sources)
    foreach(source IN LISTS ASSEMBLY_SOURCES)
        get_filename_component(source "${source}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND sources "${source}")
    endforeach()
    set(reference_args)
    foreach(reference IN LISTS ASSEMBLY_REFERENCES)
        list(APPEND reference_args "-r:${reference}")
    endforeach()
    add_custom_command(OUTPUT "${output}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${destination}"
        COMMAND "${CMAKE_COMMAND}" -E rm -f "${output}"
        COMMAND "${MONO_MCS_EXECUTABLE}" "-target:${ASSEMBLY_TYPE}" "-out:${output}"
            ${ASSEMBLY_ADDITIONAL_ARGS} ${reference_args} ${sources}
        DEPENDS ${sources} ${ASSEMBLY_REFERENCES} "${MONO_MCS_EXECUTABLE}"
        COMMENT "Building C# ${ASSEMBLY_TARGET}"
        VERBATIM)
    add_custom_target(${ASSEMBLY_TARGET} DEPENDS "${output}" SOURCES ${sources})
    set(${ASSEMBLY_TARGET}_OUTPUT "${output}" PARENT_SCOPE)
endfunction()
