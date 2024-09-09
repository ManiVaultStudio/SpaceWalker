cmake_minimum_required(VERSION 3.17)

# -----------------------------------------------------------------------------
# SpaceWalker Viewer Plugin
# -----------------------------------------------------------------------------
set(PROJECT "SpaceWalker")

PROJECT(${PROJECT})

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    set(MV_GRV_USE_VCPKG ON)
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
SET(faiss_VERSION "1.7.3" CACHE STRING "Version of faiss Library")
SET_PROPERTY(CACHE faiss_VERSION PROPERTY STRINGS 1.73)

find_package(ManiVault COMPONENTS Core PointData ClusterData CONFIG)

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets OpenGL OpenGLWidgets REQUIRED)

find_package(OpenMP)

if(MV_GRV_USE_VCPKG)
	find_package(faiss CONFIG REQUIRED)
	message(STATUS "Found faiss with version ${faiss_VERSION}")
else ()
	include(InstallArtifactoryPackage)
	set(LIBRARY_INSTALL_DIR ${PROJECT_BINARY_DIR})

	if (NOT MV_GRAD_ARTIFACTORY_LIBS_INSTALLED) 
		message(STATUS "Installing artifactory packages to: ${LIBRARY_INSTALL_DIR}")
		# Both HDILib and flann are available prebuilt in the lkeb-artifactory as combined Debug/Release packages
		# lz4 is also available in the lkb-artifactory in separate Debug and |Release packages
		install_artifactory_package(PACKAGE_NAME faiss PACKAGE_VERSION ${faiss_VERSION} PACKAGE_BUILDER lkeb COMBINED_PACKAGE TRUE) 

		message(STATUS "faiss root path ${faiss_ROOT}")
	endif()

	set(faiss_ROOT ${LIBRARY_INSTALL_DIR}/${package_name})
	set(faiss_USE_STATIC_LIBRARIES TRUE)

	find_package(faiss COMPONENTS CXX C static REQUIRED NO_MODULE)

	message(STATUS "Include for faiss at ${faiss_INCLUDE_DIR} - version ${faiss_VERSION_STRING}")
	set(MV_GRAD_ARTIFACTORY_LIBS_INSTALLED TRUE CACHE BOOL "Use the prebuilt libraries from artifactory" FORCE)
endif()

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
set(PLUGIN
    src/Common.h
    src/SpaceWalkerPlugin.h
    src/SpaceWalkerPlugin.cpp
    src/PluginMouseListener.cpp
    src/Timer.h
    src/Timer.cpp
    src/Types.h
    src/DataMatrix.h
    src/DataMatrix.cpp
    src/DataStore.h
    src/Logging.h
    src/Graph/GraphView.h
    src/Graph/GraphView.cpp
)

set(UI
    src/ScatterplotWidget.h
    src/ScatterplotWidget.cpp
    src/CellRenderer.h
    src/CellRenderer.cpp
    src/ProjectionView.h
    src/ProjectionView.cpp
    src/ExportImageDialog.h
    src/ExportImageDialog.cpp
)

set(Actions
    src/Actions/DensityPlotAction.h
    src/Actions/DensityPlotAction.cpp
    src/Actions/MiscellaneousAction.h
    src/Actions/MiscellaneousAction.cpp
    src/Actions/PlotAction.h
    src/Actions/PlotAction.cpp
    src/Actions/PointPlotAction.h
    src/Actions/PointPlotAction.cpp
    src/Actions/PositionAction.h
    src/Actions/PositionAction.cpp
    src/Actions/RenderModeAction.h
    src/Actions/RenderModeAction.cpp
    src/Actions/ScalarAction.h
    src/Actions/ScalarAction.cpp
    src/Actions/ScalarSourceAction.h
    src/Actions/ScalarSourceAction.cpp
    src/Actions/SettingsAction.h
    src/Actions/SettingsAction.cpp
    src/Actions/ExportImageAction.h
    src/Actions/ExportImageAction.cpp
    src/Actions/FilterAction.h
    src/Actions/FilterAction.cpp
    src/Actions/OverlayAction.h
    src/Actions/OverlayAction.cpp
    src/Actions/ExportAction.h
    src/Actions/ExportAction.cpp
    src/Actions/LoadedDatasetsAction.h
    src/Actions/LoadedDatasetsAction.cpp
)

set(Models
    src/Models/ColorSourceModel.h
    src/Models/ColorSourceModel.cpp
    src/Models/ScalarSourceModel.h
    src/Models/ScalarSourceModel.cpp
)

set(Compute
    src/Compute/LocalDimensionality.h
    src/Compute/LocalDimensionality.cpp
    src/Compute/RandomWalks.h
    src/Compute/RandomWalks.cpp
    src/Compute/FloodFill.h
    src/Compute/FloodFill.cpp
    src/Compute/KnnIndex.h
    src/Compute/KnnIndex.cpp
    src/Compute/KnnGraph.h
    src/Compute/KnnGraph.cpp
    src/Compute/Filters.h
    src/Compute/Filters.cpp
    src/Compute/DataTransformations.h
    src/Compute/DataTransformations.cpp
    src/Compute/SecondaryDistanceMeasures.h
    src/Compute/SecondaryDistanceMeasures.cpp
    src/Compute/Directions.h
    src/Compute/Directions.cpp
)

set(IO
    src/IO/KnnGraphIO.h
    src/IO/KnnGraphIO.cpp
    src/IO/RankingExport.h
    src/IO/RankingExport.cpp
    src/IO/FloodNodeExport.h
    src/IO/FloodNodeExport.cpp
)

set(SHADERS
    res/shaders/SelectionTool.frag
    res/shaders/SelectionTool.vert
    res/shaders/TrianglePlot.frag
    res/shaders/TrianglePlot.vert
)

set(AUX
    res/Resources.qrc
    src/SpaceWalkerPlugin.json
)

set(SOURCES ${PLUGIN} ${UI} ${Actions} ${Models} ${Compute} ${IO})

source_group(Plugin FILES ${PLUGIN})
source_group(UI FILES ${UI})
source_group(Actions FILES ${Actions})
source_group(Models FILES ${Models})
source_group(Compute FILES ${Compute})
source_group(IO FILES ${IO})
source_group(Shaders FILES ${SHADERS})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${PROJECT} SHARED ${SOURCES} ${SHADERS} ${AUX})

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Disable Windows GDI to avoid FloodFill name conflict
target_compile_definitions(${PROJECT} PRIVATE NOGDI)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include directories, SYSTEM included to suppress warnings from external libraries
target_include_directories(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT} PRIVATE "${ManiVault_INCLUDE_DIR}")

if(NOT MV_GRV_USE_VCPKG)
    target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/faiss/include)
    target_include_directories(${PROJECT} SYSTEM PRIVATE ${CUDA_INCLUDE_DIRS})
endif()

target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/Eigen/include)
target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/jcv/include)
target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/annoy/include)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------

target_link_libraries(${PROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGLWidgets)

if (MV_GRV_USE_VCPKG OR USE_ARTIFACTORY_LIBS)
    message(STATUS "Link to faiss")
    target_link_libraries(${PROJECT} PRIVATE faiss)
else()
    message(STATUS "Link to faiss and MKL")
    message(STATUS "${MKL_LIBRARIES}")
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/faiss/lib/$<CONFIGURATION>/faiss.lib)
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/MKL/lib/mkl_core_dll.lib)
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/MKL/lib/mkl_intel_lp64_dll.lib)
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/MKL/lib/mkl_intel_thread_dll.lib)
endif()

target_link_libraries(${PROJECT} PRIVATE ManiVault::Core)
target_link_libraries(${PROJECT} PRIVATE ManiVault::PointData)
target_link_libraries(${PROJECT} PRIVATE ManiVault::ClusterData)

# find_package(MKL CONFIG REQUIRED)
# if(MKL_FOUND)
  # message(STATUS "${MKL_LIBRARIES}")
  # target_link_libraries(${PROJECT} PRIVATE MKL::MKL)
# endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT} PRIVATE OpenMP::OpenMP_CXX)
endif()

install(TARGETS ${PROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

if(NOT MV_GRV_USE_VCPKG)
    # Copy faiss DLLs
    add_custom_command(TARGET ${PROJECT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/thirdparty/faiss/bin/"
            ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>)
            
    add_custom_command(TARGET ${PROJECT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/thirdparty/MKL/bin"
            ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>)
endif()

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/debug/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
