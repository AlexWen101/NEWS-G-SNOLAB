cmake_minimum_required(VERSION 2.8)
project(ShieldSimulation)

option(SHIELD_VIS_ENABLE "Enable visualization" OFF)

find_package(PkgConfig REQUIRED)
find_package(Geant4 REQUIRED ui_all vis_all)

pkg_search_module(UUID REQUIRED uuid)

include(${Geant4_USE_FILE})

add_executable(
	ShieldSimulation
	src/main.cpp
	src/detector_construction.cpp
	src/action_initialization.cpp
	src/primary_generator_action.cpp
	src/run_action.cpp
	src/event_action.cpp
	src/stepping_action.cpp
	src/run_type.cpp
	src/spectrum.cpp)

# Data files
set(
	DATA_FILES
	src/configure_commands.mac
	src/run_commands.mac
	src/vis_commands.mac
	data/am-be_neutron_spectrum.txt
	data/am-241_gamma_spectrum.txt
	data/background_neutron_spectrum.txt)

foreach(DATA_FILE ${DATA_FILES})
	get_filename_component(DATA_FILE_NAME ${DATA_FILE} NAME)
	add_custom_command(
		OUTPUT "${PROJECT_BINARY_DIR}/${DATA_FILE_NAME}"
		DEPENDS "${PROJECT_SOURCE_DIR}/${DATA_FILE}"
		COMMAND ${CMAKE_COMMAND} -E copy
		"${PROJECT_SOURCE_DIR}/${DATA_FILE}"
		"${PROJECT_BINARY_DIR}/${DATA_FILE_NAME}")
	add_custom_target(
		"Target_${DATA_FILE_NAME}"
		DEPENDS "${PROJECT_BINARY_DIR}/${DATA_FILE_NAME}")
	add_dependencies(
		ShieldSimulation
		"Target_${DATA_FILE_NAME}")
endforeach(DATA_FILE)

target_include_directories(
	ShieldSimulation PRIVATE
	${Geant4_INCLUDE_DIRS}
	"${CMAKE_SOURCE_DIR}/include")

target_link_libraries(
	ShieldSimulation
	${Geant4_LIBRARIES}
	${UUID_LIBRARIES})

if(SHIELD_VIS_ENABLE)
		target_compile_definitions(
				ShieldSimulation PRIVATE
				SHIELD_VIS_ENABLE)
endif()

