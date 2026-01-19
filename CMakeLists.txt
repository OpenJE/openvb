cmake_minimum_required(VERSION 3.24)

project(openvb LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/obj")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/obj")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

option(OPENVB_TRACING "Enable tracing output" OFF)

add_library(openje STATIC
	openje/src/JE.cpp
)

add_library(tracing STATIC
	tracing/src/tracing.cpp
)

add_executable(F3 WIN32
	src/main.cpp
	src/F3.cpp
)

set_target_properties(F3 PROPERTIES
	OUTPUT_NAME "F3"
	DEBUG_POSTFIX "-dbg"
)

if (OPENVB_TRACING)
	set_target_properties(openje PROPERTIES OUTPUT_NAME "openje-trace")
	set_target_properties(tracing PROPERTIES OUTPUT_NAME "tracing-trace")
	set_target_properties(F3 PROPERTIES OUTPUT_NAME "F3-trace")
endif()

foreach(target_name openje tracing F3)
	target_include_directories(${target_name} PRIVATE
		"${CMAKE_SOURCE_DIR}/src"
		"${CMAKE_SOURCE_DIR}/openje/src"
		"${CMAKE_SOURCE_DIR}/tracing/src"
	)
endforeach()

if(MSVC)
	foreach(target_name openje tracing F3)
		target_compile_options(${target_name} PRIVATE /EHsc)
	endforeach()

	foreach(target_name openje tracing F3)
		target_compile_options(${target_name} PRIVATE
			$<$<CONFIG:Debug>:/Zi /Od /GS>
			$<$<CONFIG:Release>:/O2>
		)
	endforeach()
else()
	foreach(target_name openje tracing F3)
		target_compile_options(${target_name} PRIVATE -std=c++98)
	endforeach()
endif()

foreach(target_name openje tracing F3)
	target_compile_definitions(${target_name} PRIVATE
		$<$<OR:$<CONFIG:Debug>,$<BOOL:${OPENVB_TRACING}>>:TRACING TRACE_TERMINAL TRACE_FILE>
	)
endforeach()

if(WIN32)
	target_link_libraries(F3 PRIVATE
		openje
		tracing
		user32
		gdi32
		winmm
		shell32
	)
else()
	target_link_libraries(F3 PRIVATE openje tracing)
endif()
