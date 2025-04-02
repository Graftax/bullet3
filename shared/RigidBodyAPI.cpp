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
void RigidBody_setWorldPosition(void* bodyHandle, float* position)
{
	btRigidBody* body = static_cast<btRigidBody*>(bodyHandle);
	btTransform& transform = body->getWorldTransform();

	btVector3 origin = btVector3(position[0], position[1], position[2]);
	transform.setOrigin(origin);
}

extern "C" __declspec(dllexport)
void RigidBody_getWorldRotation(void* rigidBodyPtr, float* outRotation)
{
	btRigidBody* body = static_cast<btRigidBody*>(rigidBodyPtr);
	btTransform& transform = body->getWorldTransform();
	btMatrix3x3& basis = transform.getBasis();

	btQuaternion rotation;
	basis.getRotation(rotation);

	outRotation[0] = rotation.getX();
	outRotation[1] = rotation.getY();
	outRotation[2] = rotation.getZ();
	outRotation[3] = rotation.getW();
}

extern "C" __declspec(dllexport) 
void RigidBody_setWorldRotation(void* rigidBodyPtr, float* rotation)
{
	btRigidBody* body = static_cast<btRigidBody*>(rigidBodyPtr);
	btTransform& transform = body->getWorldTransform();
	btMatrix3x3& basis = transform.getBasis();

	btQuaternion worldRot = btQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);
	basis.setRotation(worldRot);
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