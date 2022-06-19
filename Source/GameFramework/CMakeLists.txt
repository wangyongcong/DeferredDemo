project(GameFramework)

set(PLATFORM_NAME "Windows" CACHE STRING "Platform name")
set_property(CACHE PLATFORM_NAME PROPERTY STRINGS Windows)

set(RENDERER_NAME "Direct3D12" CACHE STRING "Renderer name")
set_property(CACHE RENDERER_NAME PROPERTY STRINGS Direct3D12)

# -----------
# 设置模块源码
# -----------

set(GAME_FRAMEWORK_PUBLIC 
	Game/Public
)

set(GAME_FRAMEWORK_PRIVATE 
	Game/Private
)

set(PUBLIC_SOURCE
	# Game module
	Game/Public/GameFramework.h
	Game/Public/GameApplication.h
	Game/Public/IGameInstance.h
	Game/Public/IRenderer.h
	Game/Public/IGameWindow.h
	Game/Public/AssertionMacros.h
	Game/Public/LogMacros.h
	Game/Public/Utility.h
	Game/Public/IMemory.h
	Game/Public/Platform.h
)

set(PRIVATE_SOURCE
	# Game module
	Game/Private/GameFrameworkPCH.h
	Game/Private/GameApplication.cpp
	Game/Private/Utility.cpp
	Game/Private/MemoryTracking.cpp
)

set(MODULE_SOURCE
	${PUBLIC_SOURCE}
	${PRIVATE_SOURCE}
)

# ---------------
# 选择不同的操作系统
# ---------------
set(PLATFORM_SOURCE
	Platform/${PLATFORM_NAME}/${PLATFORM_NAME}Application.h
	Platform/${PLATFORM_NAME}/${PLATFORM_NAME}Application.cpp
	Platform/${PLATFORM_NAME}/${PLATFORM_NAME}Window.h
	Platform/${PLATFORM_NAME}/${PLATFORM_NAME}Window.cpp
	Platform/${PLATFORM_NAME}/${PLATFORM_NAME}Platform.h
	Platform/${PLATFORM_NAME}/${PLATFORM_NAME}Platform.cpp
)
set(PLATFORM_PRIVATE
	Platform/${PLATFORM_NAME}
)

if(PLATFORM_NAME STREQUAL "Windows")
	add_definitions(-DPLATFORM_WINDOWS)
endif() # Windows

# ---------------
# 选择不同的渲染器
# ---------------

if(RENDERER_NAME STREQUAL "Direct3D12")
add_definitions(-DRENDERER_DIRECT3D12)
set(RENDERER_SOURCE
	Renderer/${RENDERER_NAME}/D3DHelper.h
	Renderer/${RENDERER_NAME}/D3DHelper.cpp
	Renderer/${RENDERER_NAME}/RendererD3D12.h
	Renderer/${RENDERER_NAME}/RendererD3D12.cpp
)
set(RENDERER_PRIVATE
	Renderer/${RENDERER_NAME}
)
endif() # Direct3D 12

set(STB_SOURCE
	stb/StbModule.cpp
	stb/stb_log.h
)

# ---------------
# 设置第三方库依赖
# ---------------

set(MEMORY_MANAGER_PUBLIC ThirdParty/MemoryManager)
set(MEMORY_MANAGER_SOURCE
	ThirdParty/MemoryManager/mmgr.cpp
	ThirdParty/MemoryManager/mmgr.h
	ThirdParty/MemoryManager/nommgr.h
)

# real time math lib
set(RTM_SOURCE
	ThirdParty/rtm/macros.h
	ThirdParty/rtm/mask4d.h
	ThirdParty/rtm/mask4f.h
	ThirdParty/rtm/mask4i.h
	ThirdParty/rtm/mask4q.h
	ThirdParty/rtm/math.h
	ThirdParty/rtm/matrix3x3d.h
	ThirdParty/rtm/matrix3x3f.h
	ThirdParty/rtm/matrix3x4d.h
	ThirdParty/rtm/matrix3x4f.h
	ThirdParty/rtm/matrix4x4d.h
	ThirdParty/rtm/matrix4x4f.h
	ThirdParty/rtm/quatd.h
	ThirdParty/rtm/quatf.h
	ThirdParty/rtm/qvvd.h
	ThirdParty/rtm/qvvf.h
	ThirdParty/rtm/scalard.h
	ThirdParty/rtm/scalarf.h
	ThirdParty/rtm/type_traits.h
	ThirdParty/rtm/types.h
	ThirdParty/rtm/vector4d.h
	ThirdParty/rtm/vector4f.h
)

set(THIRD_PARTH_PUBLIC
	ThirdParty
)

set(THIRD_PARTY_SOURCE
	${MEMORY_MANAGER_SOURCE}
	${RTM_SOURCE}
)

# ------------------
# 渲染器相关的第三方库
# ------------------

if(RENDERER_NAME STREQUAL "Direct3D12")
# D3D12 helper library
set(D3D12LIB_PUBLIC D3DLib)
set(D3D12LIB_SOURCE
	D3DLib/d3dx12.h
)

# D3D12 memory allocation lib
set(D3D12_MEMORY_ALLOCATOR_PUBLIC ThirdParty/D3D12MemoryAllocator)
set(D3D12_MEMORY_ALLOCATOR_SOURCE
	ThirdParty/D3D12MemoryAllocator/D3D12MemAlloc.h
	ThirdParty/D3D12MemoryAllocator/D3D12MemAlloc.cpp
	ThirdParty/D3D12MemoryAllocator/D3D12MemAlloc.natvis
)

set(THIRD_PARTH_PUBLIC
	${THIRD_PARTH_PUBLIC}
	${D3D12LIB_PUBLIC}
	${D3D12_MEMORY_ALLOCATOR_PUBLIC}
)

set(THIRD_PARTY_SOURCE
	${THIRD_PARTY_SOURCE}
	${D3D12LIB_SOURCE}
	${D3D12_MEMORY_ALLOCATOR_SOURCE}
)
endif() # End Direct3D 12


# game framework group
source_group("Game/Public" FILES ${PUBLIC_SOURCE})
source_group("Game/Private" FILES ${PRIVATE_SOURCE})
# render group
source_group("Renderer" FILES ${RENDERER_SOURCE})
# platform group
source_group("Platform${PLATFORM_NAME}" FILES ${PLATFORM_SOURCE})
# third parth group
source_group("ThirdParty/D3D12Lib" FILES ${D3D12LIB_SOURCE})
source_group("ThirdParty/D3D12MemoryAllocator" FILES ${D3D12_MEMORY_ALLOCATOR_SOURCE})
source_group("ThirdParty/MemoryManager" FILES ${MEMORY_MANAGER_SOURCE})
source_group("ThirdParty/rtm" FILES ${RTM_SOURCE})
source_group("ThirdParty/stb" FILES ${STB_SOURCE})
source_group("Natvis" FILES ${NATVIS})

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

option(USE_MEMORY_TRACKING "Enable memory tracking" "ON")

if (USE_MEMORY_TRACKING)
	add_definitions(-DUSE_MEMORY_TRACKING)
endif()

add_library(${PROJECT_NAME} SHARED
	${THIRD_PARTY_SOURCE}
	${MODULE_SOURCE}
	${PLATFORM_SOURCE}
	${RENDERER_SOURCE}
	${RESOURCE_FILES}
	${STB_SOURCE}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE Game/Private/GameFrameworkPCH.h
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${THIRD_PARTH_PUBLIC}
	PRIVATE ${GAME_FRAMEWORK_PRIVATE}
	PUBLIC ${GAME_FRAMEWORK_PUBLIC}
	PRIVATE ${PLATFORM_PRIVATE}
	PRIVATE ${RENDERER_PRIVATE}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC d3d12.lib
	PUBLIC dxgi.lib
	PUBLIC dxguid.lib
	# 如果是 static lib 要将资源文件链接到库里面
	# PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/GameFramework.dir/${CMAKE_CFG_INTDIR}/GameFramework.res
)