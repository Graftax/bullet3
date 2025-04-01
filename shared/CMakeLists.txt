cmake_minimum_required(VERSION 3.10)

project(BulletSharedLib)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src
)

LINK_LIBRARIES(
 BulletDynamics BulletCollision LinearMath
)

# Add the source files to the project
set(SOURCE_FILES
    ShapeAPI.cpp
	RigidBodyAPI.cpp
	DiscreteWorldAPI.cpp
)

# Add the header files to the project
#set(HEADER_FILES
#    Operator.h
#    Manager.h
#    Wrapper.h
#)

# Create the shared library (DLL)
add_library(BulletSharedLib SHARED ${HEADER_FILES} ${SOURCE_FILES})