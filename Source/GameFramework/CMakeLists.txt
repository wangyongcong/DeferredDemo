project(GameFramework)

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
	Game/Public/GameFramework.h
	Game/Public/GameApplication.h
	Game/Public/GameInstance.h
	Game/Public/IRenderDevice.h
	Game/Public/IGameWindow.h
	Game/Public/AssertionMacros.h
	Game/Public/LogMacros.h
	Game/Public/Utility.h
)

set(PRIVATE_SOURCE
	# Game module
	Game/Private/GameFrameworkPCH.h
	Game/Private/DllMain.cpp
	Game/Private/GameApplication.cpp
	Game/private/WindowsGameWindow.h
	Game/Private/WindowsGameWindow.cpp
	Game/Private/GameInstance.cpp
	Game/Private/D3DHelper.h
	Game/Private/D3DHelper.cpp
	Game/Private/DeviceD3D12.h
	Game/Private/DeviceD3D12.cpp
	Game/Private/Utility.cpp
)

set(MODULE_SOURCE
	${PUBLIC_SOURCE}
	${PRIVATE_SOURCE}
)

set(STB_SOURCE
	stb/StbModule.cpp
	stb/stb_log.h
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
source_group("Stb" FILES ${STB_SOURCE})

# --------
# 资源文件
# --------

set(RESOURCE_FILES 
	GameFramework.rc
    Resources/Icon/app_icon.ico
)

source_group("Resources" FILES GameFramework.rc)
source_group("Resources/Icon" FILES Resources/Icon/app_icon.ico)

# --------
# 设置工程
# --------

add_library(${PROJECT_NAME} SHARED
	${THIRD_PARTY_SOURCE}
	${MODULE_SOURCE}
	${RESOURCE_FILES}
	${STB_SOURCE}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE Game/Private/GameFrameworkPCH.h
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
	# 如果是 static lib 要将资源文件链接到库里面
	# PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/GameFramework.dir/${CMAKE_CFG_INTDIR}/GameFramework.res
)