# Create a library from the source files in this directory.
add_executable (testclient testclient.cpp)
add_executable (testserver testserver.cpp)
target_link_libraries (testclient MinVR)
target_link_libraries (testserver MinVR)
