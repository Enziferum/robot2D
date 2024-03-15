include(cmake/Diagnostics.cmake)
include(cmake/BaseOptions.cmake)
include(cmake/CompilerOptions.cmake)
include(cmake/Config.cmake)

# - brief processing core
print_diagnostic()
process_logging_base_options()
process_logging_compiler_flags()