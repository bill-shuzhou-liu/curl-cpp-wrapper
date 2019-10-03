cmake_minimum_required(VERSION 2.6)
project(ASIO-multi-CURL)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


include(CheckIncludeFileCXX)

function(add_linker_options)
	foreach(arg ${ARGV})
		set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${arg}")
		set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES}" PARENT_SCOPE)
	endforeach(arg)
endfunction()

#	Where our project's headers live
include_directories(include)

#	We need libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

#	General command line arguments to the compiler that should be present
#	on all platforms
#add_compile_options(-std=c++1z)
#add_compile_options(-g -D__DEBUG__)
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O0 -g -fno-inline -fno-omit-frame-pointer -Wall -Wpedantic -Wextra -Werror)
	#	Address Sanitizer (non-Windows only)
	if(NOT WIN32)
		if(NOT DEFINED USE_ADDRESS_SANITIZER OR USE_ADDRESS_SANITIZER)
			add_compile_options(-fsanitize=address)
			add_linker_options(-fsanitize=address)
		endif()
	endif()
	#	Enable all warnings except a choice few on Clang
	if (DEFINED CMAKE_CXX_COMPILER_ID AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-Weverything)
		#	We don't care about C++98 compatibility: This is a C++14
		#	library
		add_compile_options(-Wno-c++98-compat)
		#	These warnings are just noise
		add_compile_options(-Wno-weak-vtables -Wno-padded)
		#	curl_easy_setopt is apparently implemented in terms of
		#	a recursive macro which sets this warning off
		add_compile_options(-Wno-disabled-macro-expansion)
	endif()
else()
	add_compile_options(-O2)
endif()

if(NOT DEFINED USE_BOOST_FUTURE)
	set(USE_BOOST_FUTURE 1)
endif()

if(NOT DEFINED USE_BOOST_ASIO)
	set(USE_BOOST_ASIO 1)
endif()


#add_library(asiocurl SHARED
#	src/AsioEventEngine.cpp
#	src/CurlTask.cpp
#	src/FileDownloadJob.cpp
#	src/MultiCurl.cpp
#)
#target_link_libraries(asiocurl ${CURL_LIBRARIES})
#if (WIN32)
#	target_link_libraries(asiocurl ws2_32)
#endif()

file(GLOB lib_SRC "src/*.cpp")
foreach(f ${lib_SRC})
     if(f MATCHES "src/main.cpp")
       list(REMOVE_ITEM lib_SRC ${f})
     endif()
endforeach()

add_library(multicurl SHARED
          ${lib_SRC}
)

#list(FILTER lib_SRC EXCLUDE REGEX ".*src/main.cpp$")
#message( ${lib_SRC} )

add_executable(downloader
       src/main.cpp
)
target_link_libraries(downloader multicurl boost_thread boost_system pthread curl)

#if(USE_BOOST_FUTURE)
#	target_link_libraries(asiocurl boost_thread)
#endif()
#if(USE_BOOST_ASIO OR USE_BOOST_FUTURE)
#	target_link_libraries(asiocurl boost_system)
#endif()

#if((DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug") OR (DEFINED BUILD_TESTS AND BUILD_TESTS))
file(GLOB test_SRC
    "test/*.cpp"
)
	add_executable(tests
		${test_SRC}
	)
target_link_libraries(tests  multicurl boost_thread boost_system pthread curl)

	add_custom_target(tests_run ALL
		COMMAND tests
		DEPENDS tests
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		COMMENT "Run test suite"
	)
#endif()
