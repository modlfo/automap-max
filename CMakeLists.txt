
cmake_minimum_required(VERSION 2.8)

# If you are going to debug change the configuration here and 
# remember that you need to use the Runtime version of Max since 
# the copy protection will not let you debug.
set(CMAKE_BUILD_TYPE Release)

#  Detect if it's a 64 or 32 bit system
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(W64 1)
endif()	

# Source code 
set(AUTOMAP_MAX_SRC AutomapMax.c)

# Max needs these two definitions
add_definitions(-DWIN_VERSION -DEXT_WIN_VERSION)

if(CMAKE_BUILD_TYPE EQUAL Debug)
	add_definitions(-DMAXAPI_USE_MSCRT)
endif()


# Folders for Max and Automap includes. 
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../AutomapSDK)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../MaxSDK-6.1.1/c74support/max-includes)

# If the system is a 64 bits, the includes and libraries are different
if(W64)
	link_directories(${CMAKE_CURRENT_LIST_DIR}/../../AutomapSDK/lib/x64)
	link_directories(${CMAKE_CURRENT_LIST_DIR}/../../MaxSDK-6.1.1/c74support/max-includes/x64)
	set(AUTOMAP_LIB AutomapClient64)
else(W64)
	link_directories(${CMAKE_CURRENT_LIST_DIR}/../../AutomapSDK/lib/x86)
	link_directories(${CMAKE_CURRENT_LIST_DIR}/../../MaxSDK-6.1.1/c74support/max-includes)
	set(AUTOMAP_LIB AutomapClient)
endif(W64)	

# Create the shared library
add_library(automap_max SHARED ${AUTOMAP_MAX_SRC})

# Specify to link Max and Automap libraries
target_link_libraries(automap_max MaxAPI ${AUTOMAP_LIB})

if(MSVC)
	 SET_TARGET_PROPERTIES(automap_max PROPERTIES LINK_FLAGS_RELEASE "/NODEFAULTLIB:libcmt.lib")
	 SET_TARGET_PROPERTIES(automap_max PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:libcmt.lib")
endif(MSVC)

# Change the extension of the external
if(W64)
	set_target_properties(automap_max PROPERTIES SUFFIX .mxe64)
else()
	set_target_properties(automap_max PROPERTIES SUFFIX .mxe)
endif()