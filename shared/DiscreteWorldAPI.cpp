
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"

#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

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

	while (world->getNumConstraints() > 0)
	{
		world->removeConstraint(world->getConstraint(0));
	}

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
void DiscreteWorld_addRigidBody(void* worldHandle, void* rigidBodyHandle)
{
	btDiscreteDynamicsWorld* world = static_cast<btDiscreteDynamicsWorld*>(worldHandle);
	btRigidBody* body = static_cast<btRigidBody*>(rigidBodyHandle);
	world->addRigidBody(body);
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