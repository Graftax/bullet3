#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"

#include "LinearMath/btDefaultMotionState.h"

extern "C" __declspec(dllexport) 
void* RigidBody_create(float mass, void* shapeHandle)
{
	btCollisionShape* collShape = static_cast<btCollisionShape*>(shapeHandle);

	btTransform startTransform;
	startTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);
	collShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo bodyBlueprint(mass, motionState,
		collShape, localInertia);

	return new btRigidBody(bodyBlueprint);
}

extern "C" __declspec(dllexport) 
void RigidBody_destroy(void* rigidBodyPtr)
{
	btRigidBody* rigidbody = static_cast<btRigidBody*>(rigidBodyPtr);
	if (rigidbody != nullptr && rigidbody->getMotionState() != nullptr)
		delete rigidbody->getMotionState();

	delete rigidbody;
}

// https://learn.microsoft.com/en-us/dotnet/framework/interop/passing-structures?redirectedfrom=MSDN

extern "C" __declspec(dllexport)
void RigidBody_getWorldPosition(void* bodyHandle, float* outPosition)
{
	btRigidBody* body = static_cast<btRigidBody*>(bodyHandle);
	btTransform& transform = body->getWorldTransform();
	btVector3& origin = transform.getOrigin();

	outPosition[0] = origin.m_floats[0];
	outPosition[1] = origin.m_floats[1];
	outPosition[2] = origin.m_floats[2];
}

extern "C" __declspec(dllexport)
btQuaternion RigidBody_getWorldRotation(void* bodyHandle)
{
	btRigidBody* body = static_cast<btRigidBody*>(bodyHandle);

	btQuaternion rotation;
	body->getWorldTransform().getBasis().getRotation(rotation);

	return rotation;
}

extern "C" __declspec(dllexport)
void RigidBody_setLinearVelocity(void* bodyHandle, btVector3 vel)
{
	btRigidBody* body = static_cast<btRigidBody*>(bodyHandle);
	body->setLinearVelocity(vel);
}

extern "C" __declspec(dllexport)
void RigidBody_setAngularVelocity(void* bodyHandle, btVector3 angVel)
{
	btRigidBody* body = static_cast<btRigidBody*>(bodyHandle);
	body->setAngularVelocity(angVel);
}