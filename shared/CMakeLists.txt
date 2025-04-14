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

# add_custom_command(
# 	COMMAND Codegami 
# 		${BULLET_PHYSICS_SOURCE_DIR}/src/ *.h 
# 		./Codegami/*.cpp
# 
#	OUTPUT ./Codegami/*.cpp
#	DEPENDS Codegami)

function(RunCodegami relPath outPath)
add_custom_command(
	COMMAND Codegami 
		${BULLET_PHYSICS_SOURCE_DIR}/src/ ${relPath} ${outPath}
	OUTPUT ${outPath}
	DEPENDS Codegami)
endfunction()

RunCodegami(LinearMath/btTransform.h ./Codegami/btTransformCDecl.cpp)

RunCodegami(BulletCollision/BroadphaseCollision/btDbvtBroadphase.h ./Codegami/btDbvtBroadphaseCDecl.cpp)
RunCodegami(BulletCollision/CollisionShapes/btCollisionShape.h ./Codegami/btCollisionShapeCDecl.cpp)
RunCodegami(BulletCollision/CollisionDispatch/btCollisionDispatcher.h ./Codegami/btCollisionDispatcherCDecl.cpp)
RunCodegami(BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h ./Codegami/btDefaultCollisionConfigurationCDecl.cpp)

RunCodegami(BulletDynamics/Dynamics/btRigidBody.h ./Codegami/btRigidBodyCDecl.cpp)
RunCodegami(BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h ./Codegami/btDiscreteDynamicsWorldCDecl.cpp)
RunCodegami(BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h ./Codegami/btMultiBodyConstraintSolverCDecl.cpp)
RunCodegami(BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h ./Codegami/btMultiBodyDynamicsWorldCDecl.cpp)

RunCodegami(../Extras/Serialize/BulletWorldImporter/btWorldImporter.h ./Codegami/btWorldImporterCDecl.cpp)
RunCodegami(../Extras/Serialize/BulletWorldImporter/btMultiBodyWorldImporter.h ./Codegami/btMultiBodyWorldImporterCDecl.cpp)



# Add the source files to the project
set(SOURCE_FILES
	Codegami/btTransformCDecl.cpp

	Codegami/btDbvtBroadphaseCDecl.cpp
	Codegami/btCollisionShapeCDecl.cpp
	Codegami/btCollisionDispatcherCDecl.cpp
	Codegami/btDefaultCollisionConfigurationCDecl.cpp

	Codegami/btRigidBodyCDecl.cpp
	Codegami/btDiscreteDynamicsWorldCDecl.cpp
	Codegami/btMultiBodyConstraintSolverCDecl.cpp
	Codegami/btMultiBodyDynamicsWorldCDecl.cpp

	Codegami/btWorldImporterCDecl.cpp
	Codegami/btMultiBodyWorldImporterCDecl.cpp)

# Add the header files to the project
#set(HEADER_FILES
#    Operator.h
#    Manager.h
#    Wrapper.h
#)

# Create the shared library (DLL)
add_library(BulletSharedLib SHARED ${HEADER_FILES} ${SOURCE_FILES})