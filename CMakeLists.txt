cmake_minimum_required(VERSION 3.10) # Specifies the minimum required CMake version

project(ChatSocketNB LANGUAGES CXX) # Defines the project name and language(s)

set(CMAKE_CXX_STANDARD 17) # Sets the C++ standard to C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensures the specified C++ standard is strictly enforced

# Add an executable target
add_executable(client client.cpp) 
add_executable(server server.cpp) 

# If you have multiple source files, you can list them:
# add_executable(my_executable main.cpp another_file.cpp)

# If your project uses header files in a separate directory, you can add it:
# target_include_directories(my_executable PUBLIC include) 

# If your project links against external libraries:
# target_link_libraries(my_executable PRIVATE MyLibrary)