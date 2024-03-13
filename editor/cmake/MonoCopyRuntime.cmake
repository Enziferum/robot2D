include(cmake/copy_to_runtime_path.cmake)

function(mono_copy_runtimes DESTINATION_PATH BINARY_PATH)
	find_file(MONO_DLL_PATH NAMES mono-2.0.dll mono-2.0-sgen.dll PATHS ${BINARY_PATH})
	copy_files_to_runtime_path(DESTINATION ${DESTINATION_PATH} FILES ${MONO_DLL_PATH})
endfunction()

mono_copy_runtimes(${DESTINATION_PATH} ${MONO_BINARY_PATH})