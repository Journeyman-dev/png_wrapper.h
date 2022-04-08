set(EXAMPLE_NAME "pngw-Example")
# create the executable for the example
add_executable(${EXAMPLE_NAME}
	${EXAMPLE_NAME}/main.c
)
#libpng
find_package(libpng CONFIG REQUIRED)
target_link_libraries(${EXAMPLE_NAME} PRIVATE
	png
)
target_include_directories(${EXAMPLE_NAME} PRIVATE
	${PNGW_INCLUDE_DIR}
)
set_property(TARGET ${EXAMPLE_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)
# grab the content file paths
file(GLOB asset_files
	assets/*
)
# copy the content files to the executable output folder post build
add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD    
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${asset_files}
		$<TARGET_FILE_DIR:${EXAMPLE_NAME}>
)