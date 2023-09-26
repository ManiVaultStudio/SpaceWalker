cmake_minimum_required(VERSION 3.17)

set(PROJECT "SpaceWalker")

PROJECT(${PROJECT})

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

SET(faiss_VERSION "1.7.3" CACHE STRING "Version of faiss Library")
SET_PROPERTY(CACHE faiss_VERSION PROPERTY STRINGS 1.73)

file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets OpenGL OpenGLWidgets REQUIRED)

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
    src/DataStore.cpp
    src/Logging.h
    src/Logging.cpp
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

add_library(${PROJECT} SHARED ${SOURCES} ${SHADERS} ${AUX})

# Disable Windows GDI to avoid FloodFill name conflict
target_compile_definitions(${PROJECT} PRIVATE NOGDI)

# Include directories, SYSTEM included to suppress warnings from external libraries
target_include_directories(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/include/")

include(InstallArtifactoryPackage)
set(LIBRARY_INSTALL_DIR ${PROJECT_BINARY_DIR})
if (USE_ARTIFACTORY_LIBS)
	if (NOT ARTIFACTORY_LIBS_INSTALLED) 
		message(STATUS "Installing artifactory packages to: ${LIBRARY_INSTALL_DIR}")
		# Both HDILib and flann are available prebuilt in the lkeb-artifactory as combined Debug/Release packages
		# lz4 is also available in the lkb-artifactory in separate Debug and |Release packages
		install_artifactory_package(PACKAGE_NAME faiss PACKAGE_VERSION ${faiss_VERSION} PACKAGE_BUILDER lkeb COMBINED_PACKAGE TRUE) 

		message(STATUS "faiss root path ${faiss_ROOT}")
	endif()
	set(faiss_ROOT ${LIBRARY_INSTALL_DIR}/${package_name})
	set(faiss_USE_STATIC_LIBRARIES TRUE)
	find_package(faiss COMPONENTS CXX C static REQUIRED NO_MODULE)
	#get_cmake_property(_variableNames VARIABLES)
	#list (SORT _variableNames)
	#foreach (_variableName ${_variableNames})
	#		message(STATUS "${_variableName}=${${_variableName}}")
	#endforeach()
	message(STATUS "Include for faiss at ${faiss_INCLUDE_DIR} - version ${faiss_VERSION_STRING}")
	set(ARTIFACTORY_LIBS_INSTALLED TRUE CACHE BOOL "Use the prebuilt libraries from artifactory" FORCE)
	target_include_directories(${PROJECT} PRIVATE ${HDF5_INCLUDE_DIR})
	message(status "faiss include ${faiss_INCLUDE_DIR}")
else()    
    target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/faiss/include)
    target_include_directories(${PROJECT} SYSTEM PRIVATE ${CUDA_INCLUDE_DIRS})

endif()

target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/Eigen/include)
target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/jcv/include)
target_include_directories(${PROJECT} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/annoy/include)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

set(HDPS_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(HDPS_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(HDPS_LINK_LIBRARY "${HDPS_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${HDPS_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${HDPS_LINK_SUFFIX}") 
set(CLUSTERDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ClusterData${HDPS_LINK_SUFFIX}") 

message(STATUS "${MKL_LIBRARIES}")
target_link_libraries(${PROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGLWidgets)
if (USE_ARTIFACTORY_LIBS)
    target_link_libraries(${PROJECT} PRIVATE faiss)
else()
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/faiss/lib/$<CONFIGURATION>/faiss.lib)
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/MKL/lib/mkl_core_dll.lib)
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/MKL/lib/mkl_intel_lp64_dll.lib)
    target_link_libraries(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/MKL/lib/mkl_intel_thread_dll.lib)
endif()

target_link_libraries(${PROJECT} PRIVATE "${HDPS_LINK_LIBRARY}")
target_link_libraries(${PROJECT} PRIVATE "${POINTDATA_LINK_LIBRARY}")
target_link_libraries(${PROJECT} PRIVATE "${CLUSTERDATA_LINK_LIBRARY}")

# find_package(MKL CONFIG REQUIRED)
# if(MKL_FOUND)
  # message(STATUS "${MKL_LIBRARIES}")
  # target_link_libraries(${PROJECT} PRIVATE MKL::MKL)
# endif()

find_package(OpenMP)
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
        --prefix ${INSTALL_DIR}/$<CONFIGURATION>
)

# Copy faiss DLLs
add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/thirdparty/faiss/bin/"
        ${INSTALL_DIR}/$<CONFIGURATION>)
        
add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/thirdparty/MKL/bin"
        ${INSTALL_DIR}/$<CONFIGURATION>)

# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug,${INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug/HDPS.exe,${INSTALL_DIR}/release/HDPS.exe>)
endif()
