cmake_minimum_required(VERSION 3.10)

project(BulletSharedLib)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src)

LINK_LIBRARIES(
 BulletCollision BulletDynamics BulletFileLoader BulletWorldImporter LinearMath)

# add_custom_target(CodegamiDirectory ALL
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Codegami)
make_directory(${PROJECT_SOURCE_DIR}/Codegami)

add_custom_command(
	COMMAND Codegami 
		${BULLET_PHYSICS_SOURCE_DIR}/src/ BulletDynamics/Dynamics/btRigidBody.h 
		./Codegami/btRigidBodyCDecl.cpp

	OUTPUT ./Codegami/btRigidBodyCDecl.cpp
	DEPENDS Codegami)

add_custom_command(
	COMMAND Codegami 
		${BULLET_PHYSICS_SOURCE_DIR}/src/ BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h 
		./Codegami/btDiscreteDynamicsWorldCDecl.cpp

	OUTPUT ./Codegami/btDiscreteDynamicsWorldCDecl.cpp
	DEPENDS Codegami)

add_custom_command(
	COMMAND Codegami 
		${BULLET_PHYSICS_SOURCE_DIR}/src/ ../Extras/Serialize/BulletWorldImporter/btMultiBodyWorldImporter.h 
		./Codegami/btMultiBodyWorldImporterCDecl.cpp

	OUTPUT ./Codegami/btMultiBodyWorldImporterCDecl.cpp
	DEPENDS Codegami)

# Add the source files to the project
set(SOURCE_FILES
    ShapeAPI.cpp
	Codegami/btRigidBodyCDecl.cpp
	Codegami/btDiscreteDynamicsWorldCDecl.cpp
	Codegami/btMultiBodyWorldImporterCDecl.cpp)

# Add the header files to the project
#set(HEADER_FILES
#    Operator.h
#    Manager.h
#    Wrapper.h
#)

# Create the shared library (DLL)
add_library(BulletSharedLib SHARED ${HEADER_FILES} ${SOURCE_FILES})