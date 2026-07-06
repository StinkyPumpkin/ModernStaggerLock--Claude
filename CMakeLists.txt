cmake_minimum_required(VERSION 3.21)

# standards & flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")

include(GNUInstallDirs)

# info
project(
	ModernStaggerLock
	VERSION 1.1.7
	LANGUAGES CXX
)

# out-of-source builds only
if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# in-place configuration
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

# source files
execute_process(COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/!Update.ps1" "SOURCEGEN" "${PROJECT_VERSION}" "${CMAKE_CURRENT_BINARY_DIR}")
include(${CMAKE_CURRENT_BINARY_DIR}/sourcelist.cmake)
source_group(
	TREE 
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES 
		${SOURCES}
)

source_group(
	TREE ${CMAKE_CURRENT_BINARY_DIR}
	FILES ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# dependencies
# Claude fork: build CommonLibSSE-NG FROM SOURCE with ENABLE_SKYRIM_VR=OFF. The
# prebuilt vcpkg port is multi-target (SE+AE+VR) and its interface forces
# HAS_SKYRIM_MULTI_TARGETING=1, which disables direct RE member access + the per-
# class VTABLE arrays these hooks rely on (VTABLE[3], VTABLE[0AD], ...). A VR-off
# source build keeps them. Same approach the Precision--Claude fork uses; the
# CommonLibSSE-NG source is shared via $ENV{CommonLibSSEPath}.
# SKSE_SUPPORT_XBYAK must be ON before add_subdirectory so CommonLib compiles its
# xbyak Trampoline overload (write_call<>) that these hooks need.
set(SKSE_SUPPORT_XBYAK ON)
set(ENABLE_SKYRIM_SE ON)
set(ENABLE_SKYRIM_AE ON)
set(ENABLE_SKYRIM_VR OFF)
set(BUILD_TESTS OFF)
add_subdirectory("$ENV{CommonLibSSEPath}" CommonLibSSE EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/DKUtil DKUtil)
find_package(spdlog CONFIG REQUIRED)

# cmake target
# runtime
add_library(
	${PROJECT_NAME}
	SHARED
		${SOURCES}
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
		${CMAKE_CURRENT_BINARY_DIR}/version.rc
		.clang-format
		vcpkg.json
)

# Claude fork: DKUtil's compile-time enum reflection defaults to 64 samples, whose
# flag-name cache (enumeration.hpp DKU_BUILD_FLAG_CACHE) ICEs MSVC 19.51 / VS2026
# (C1001). Every enum we reflect (Config::DataType=6, Hook::Module::Section=9) is
# small, so cap the range low enough to dodge the ICE. Target-scoped so CommonLibSSE
# (which doesn't use DKUtil) isn't forced to rebuild.
target_compile_definitions(${PROJECT_NAME} PRIVATE DKU_MAX_REFLECTION_ENUM=16)

# include dir
target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

# linkage
target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
		CommonLibSSE::CommonLibSSE
		DKUtil::DKUtil
		spdlog::spdlog
)

# compiler def
if (MSVC)
	add_compile_definitions(_MBCS)

	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/MP
			/W0
			/WX
			/permissive-
			/Zc:alignedNew
			/Zc:auto
			/Zc:__cplusplus
			/Zc:externC
			/Zc:externConstexpr
			/Zc:forScope
			/Zc:hiddenFriend
			/Zc:implicitNoexcept
			/Zc:lambda
			/Zc:noexceptTypes
			/Zc:preprocessor
			/Zc:referenceBinding
			/Zc:rvalueCast
			/Zc:sizedDealloc
			/Zc:strictStrings
			/Zc:ternary
			/Zc:threadSafeInit
			/Zc:trigraphs
			/Zc:wchar_t
			/wd4200 # nonstandard extension used : zero-sized array in struct/union
	)
endif()

# PCH
target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)

# post build event: Claude fork deploys via build.bat (COPY step below removed —
# the original called !Update.ps1 with an AE layout we don't use here).

# called via !Rebuild.ps1
if(DEFINED GROUP)
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
	FOLDER
	${GROUP}
)
endif()
