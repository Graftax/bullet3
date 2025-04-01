#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"

extern "C" __declspec(dllexport) 
void* BoxShape_create(btVector3 halfExtents)
{
	return new btBoxShape(halfExtents);
}

extern "C" __declspec(dllexport) 
void* SphereShape_create(float radius)
{
	return new btSphereShape(radius);
}

extern "C" __declspec(dllexport) 
void* CapsuleShape_create(float radius, float height)
{
	return new btCapsuleShape(radius, height);
}

extern "C" __declspec(dllexport) 
void* CylinderShape_create(btVector3 halfExtents)
{
	return new btCylinderShape(halfExtents);
}

extern "C" __declspec(dllexport) 
void ConvexInternalShape_destroy(void* shapePtr)
{
	btConvexInternalShape* shape = static_cast<btConvexInternalShape*>(shapePtr);
	delete shape;
}