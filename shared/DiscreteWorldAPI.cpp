
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"

#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

#include "LinearMath/btSerializer.h"

#include "../Extras/Serialize/BulletWorldImporter/btBulletWorldImporter.h"

static btDefaultCollisionConfiguration CollisionConfigDefault = btDefaultCollisionConfiguration();

extern "C" __declspec(dllexport) 
void* DiscreteWorld_create()
{
	btCollisionDispatcher* collDispatcher = new btCollisionDispatcher(&CollisionConfigDefault);

	btDbvtBroadphase* broadPhase = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(collDispatcher,
		broadPhase, solver, &CollisionConfigDefault);

	return static_cast<void*>(world);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_destroy(void* worldHandle)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldHandle);

	// We need to delete the wrold first, so save off stuff to delete.
	btDispatcher* dispatcher = world->getDispatcher();
	btBroadphaseInterface* broadphase = world->getBroadphase();
	btConstraintSolver* solver = world->getConstraintSolver();

	delete world;
	delete dispatcher;
	delete broadphase;
	delete solver;
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_setGravity(void* worldHandle, btVector3 acceleration)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldHandle);
	world->setGravity(acceleration);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_stepSimulation(void* worldHandle, float timeDelta)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldHandle);
	// Setting maxSubSteps to 0 because we want to have direct control over 
	// stepping, and the client will need to understand how to step correctly, via
	// a fixed timestep.
	world->stepSimulation(timeDelta, 0);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_addRigidBody(void* objectPtr, void* rigidBodyHandle)
{
	btDiscreteDynamicsWorld* castObjectPtr = static_cast<btDiscreteDynamicsWorld*>(objectPtr);
	btRigidBody* body = static_cast<btRigidBody*>(rigidBodyHandle);
	castObjectPtr->addRigidBody(body);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_removeCollisionObject(void* worldHandle, void* collisionObjectPtr)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldHandle);
	btCollisionObject* collObj = static_cast<btCollisionObject*>(collisionObjectPtr);
	world->removeCollisionObject(collObj);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_removeRigidBody(void* worldHandle, void* rigidBodyHandle)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldHandle);
	btRigidBody* body = static_cast<btRigidBody*>(rigidBodyHandle);
	world->removeRigidBody(body);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_serialize(void* worldPtr, void* serializerPtr)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldPtr);
	btSerializer* serializer = static_cast<btSerializer*>(serializerPtr);
	world->serialize(serializer);
}

extern "C" __declspec(dllexport) 
void DiscreteWorld_deserialize(void* worldPtr, void* serializerPtr)
{
	btSerializer* serializer = static_cast<btSerializer*>(serializerPtr);
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldPtr);

	btBulletWorldImporter* importer = new btBulletWorldImporter(world);
	importer->setImporterFlags(btWorldImporterFlags::eRESTORE_EXISTING_OBJECTS);
	importer->loadFileFromMemory((char*)serializer->getBufferPointer(), serializer->getCurrentBufferSize());

	delete importer;
}