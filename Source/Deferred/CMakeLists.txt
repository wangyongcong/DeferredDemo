cmake_minimum_required(VERSION 3.10)
project(Deferred)

set(GAME_SOURCE
    Source/Main.cpp
)

add_executable(${PROJECT_NAME} WIN32
	${GAME_SOURCE}
	# ${RESOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME}
    D3DGame
)
