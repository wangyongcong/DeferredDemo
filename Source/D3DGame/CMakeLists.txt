project(D3DGame)

# -----------
# 设置模块源码
# -----------

set(PUBLIC_DIR 
	Game/Public
)

set(PRIVATE_DIR 
	Game/Private
)

set(PUBLIC_SOURCE
	# Game module
	Game/Public/D3DGame.h
	Game/Public/GameApplication.h
	Game/Public/GameWindow.h
	Game/Public/GameInstance.h
)

set(PRIVATE_SOURCE
	# Game module
	Game/Private/D3DGamePCH.h
	Game/Private/GameApplication.cpp
	Game/Private/GameWindow.cpp
	Game/Private/GameInstance.cpp
)

set(MODULE_SOURCE
	${PUBLIC_SOURCE}
	${PRIVATE_SOURCE}
)

# ---------------
# 设置第三方库依赖
# ---------------

# d3d12 helper library
set(D3D12LIB_DIR D3DLib)
set(D3D12LIB_SOURCE
	${D3D12LIB_DIR}/d3dx12.h
)

set(THIRD_PARTH_HEADER
	${D3D12LIB_DIR}
)

set(THIRD_PARTY_SOURCE
	${D3D12LIB_SOURCE}
)

source_group("ThirdParty/D3D12Lib" FILES ${D3D12LIB_SOURCE})
source_group("Game/Public" FILES ${PUBLIC_SOURCE})
source_group("Game/Private" FILES ${PRIVATE_SOURCE})

# --------
# 资源文件
# --------

set(RESOURCE_FILES 
	D3DGame.rc
	Resources/Icon/app_icon.ico
)

source_group("Resources" FILES D3DGame.rc)
source_group("Resources/Icon" FILES Resources/Icon/app_icon.ico)

# --------
# 设置工程
# --------

add_library(${PROJECT_NAME} SHARED
	${THIRD_PARTY_SOURCE}
	${MODULE_SOURCE}
	${RESOURCE_FILES}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE Game/Private/D3DGamePCH.h
)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${PUBLIC_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${THIRD_PARTH_HEADER}
	${PRIVATE_DIR}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC d3d12.lib
	PUBLIC dxgi.lib
	PUBLIC dxguid.lib
	# 要将资源文件链接到库里面
	PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/D3DGame.dir/${CMAKE_CFG_INTDIR}/D3DGame.res
)