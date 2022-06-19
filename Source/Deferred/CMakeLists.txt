cmake_minimum_required(VERSION 3.10)
project(Deferred)

if(PLATFORM_NAME STREQUAL "Windows")
	add_definitions(-DPLATFORM_WINDOWS)
endif() # Windows

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
