 set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generated/${PROJECT_NAME}-config.cmake")
 set(CONFIG_DEST "lib${LIB_SUFFIX}/cmake/${PROJECT_NAME}")
 set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")

 include(CMakePackageConfigHelpers)
 configure_package_config_file(
         "${CMAKE_CURRENT_SOURCE_DIR}/cmake/robot2D-config.cmake.in"
         ${CONFIG_FILE}
         INSTALL_DESTINATION ${CONFIG_DEST}
 )

 # Install
 install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/engine/include/robot2D DESTINATION include)

 install(TARGETS ${PROJECT_NAME} EXPORT ${TARGETS_EXPORT_NAME}
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)

 install(FILES ${CONFIG_FILE} DESTINATION ${CONFIG_DEST})

 install( EXPORT ${TARGETS_EXPORT_NAME}
         DESTINATION "${CONFIG_DEST}")