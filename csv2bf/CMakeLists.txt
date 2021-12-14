# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Maps to a solution file (Tutorial.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (csv2bf)

# Set c standard
set(CMAKE_C_STANDARD 11)

# Set
if(MSVC)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall")
else()
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -m32")
endif()

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("c Flags: " ${CMAKE_C_FLAGS})

# Set c++ standard
set(CMAKE_Cxx_STANDARD 20)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
        csv2bf.c
#        ${PROJECT_SOURCE_DIR}/libcsv/libcsv.c
        )

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES})

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
# Properties->C/C++->General->Additional Include Directories
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/
        )

# Set compile definitions
# target_compile_definitions(${PROJECT_NAME} 
#         PRIVATE
#         "HAVE_CONFIG_H"
#         "LargestIntegralType=unsigned long"
#         LargestIntegralTypePrintfFormat="%llx"
#         )

# Properties->Linker->Input->Additional Dependencies
# target_link_libraries (app  math)

if(MSVC)
# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
source_group("app" FILES ${SOURCES})

# Add compile definitions to a target.
target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        WIN32
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_WARNINGS
)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Creates a folder "executables" and adds target 
# project (app.vcproj) under it
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "executables")

# Properties->General->Output Directory
# set_target_properties(${PROJECT_NAME} PROPERTIES
#                       RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
# install (TARGETS ${PROJECT_NAME}
#          RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/_install)

# Sub-directories where more CMakeLists.txt exist
# add_subdirectory(app)


# else()

endif()
# Turn on CMake testing capabilities
# enable_testing()

# Add test cases
# add_test(AppTest1 ${PROJECT_BINARY_DIR}/bin/app 100)
# add_test(AppTest2 ${PROJECT_BINARY_DIR}/bin/app 200)
# add_test(AppTest3 ${PROJECT_BINARY_DIR}/bin/app 300)
