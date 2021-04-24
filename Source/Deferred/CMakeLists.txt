cmake_minimum_required(VERSION 3.10)
project(Deferred)

set(GAME_SOURCE
	Source/Main.cpp
)

add_executable(${PROJECT_NAME} WIN32
	${GAME_SOURCE}
)

target_link_libraries(${PROJECT_NAME}
	GameFramework
)

set_property(TARGET ${PROJECT_NAME} 
	PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
