#pragma once

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <iostream>
#include "../GlobalData/GlobalData.h"
#include "../Components/PhysicsMeshComponent.h"

// Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
JPH_SUPPRESS_WARNINGS

// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

namespace Prisma {
	namespace Layers
	{
		static constexpr ObjectLayer NON_MOVING = 0;
		static constexpr ObjectLayer MOVING = 1;
		static constexpr ObjectLayer NUM_LAYERS = 2;
	};

	/// Class that determines if two object layers can collide
	class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
	{
	public:
		virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
			case Layers::NON_MOVING:
				return inObject2 == Layers::MOVING; // Non moving only collides with moving
			case Layers::MOVING:
				return true; // Moving collides with everything
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	namespace BroadPhaseLayers
	{
		static constexpr BroadPhaseLayer NON_MOVING(0);
		static constexpr BroadPhaseLayer MOVING(1);
		static constexpr uint NUM_LAYERS(2);
	};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual uint					GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
		{
			switch ((BroadPhaseLayer::Type)inLayer)
			{
			case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	/// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
			case Layers::NON_MOVING:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return true;
			default:
				JPH_ASSERT(false);
				return false;
			}
		}
	};

	// An example contact listener
	class MyContactListener : public ContactListener
	{
	public:
		// See: ContactListener
		virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
		{

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
		{
			auto physicsComponent = dynamic_cast<Prisma::PhysicsMeshComponent*>(sceneComponents[inBody1.GetUserData()]);
			if (physicsComponent && physicsComponent->onCollisionEnter()) {
				auto addComponent = physicsComponent->onCollisionEnter();
				addComponent(inBody2);
			}
			auto physicsComponent2 = dynamic_cast<Prisma::PhysicsMeshComponent*>(sceneComponents[inBody2.GetUserData()]);
			if (physicsComponent2 && physicsComponent2->onCollisionEnter()) {
				auto addComponent = physicsComponent2->onCollisionEnter();
				addComponent(inBody1);
			}
		}

		virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
		{
			auto physicsComponent = dynamic_cast<Prisma::PhysicsMeshComponent*>(sceneComponents[inBody1.GetUserData()]);
			if (physicsComponent && physicsComponent->onCollisionStay()) {
				auto stayComponent = physicsComponent->onCollisionStay();
				stayComponent(inBody2);
			}
			auto physicsComponent2 = dynamic_cast<Prisma::PhysicsMeshComponent*>(sceneComponents[inBody2.GetUserData()]);
			if (physicsComponent2 && physicsComponent2->onCollisionStay()) {
				auto stayComponent = physicsComponent2->onCollisionStay();
				stayComponent(inBody1);
			}
		}

		virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
		{
			auto physicsComponent = dynamic_cast<Prisma::PhysicsMeshComponent*>(sceneComponents[Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().GetUserData(inSubShapePair.GetBody1ID())]);
			if (physicsComponent && physicsComponent->onCollisionExit()) {
				auto removeComponent = physicsComponent->onCollisionExit();
				removeComponent(inSubShapePair.GetBody2ID());
			}

			auto physicsComponent2 = dynamic_cast<Prisma::PhysicsMeshComponent*>(sceneComponents[Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().GetUserData(inSubShapePair.GetBody2ID())]);
			if (physicsComponent2 && physicsComponent2->onCollisionExit()) {
				auto removeComponent2 = physicsComponent2->onCollisionExit();
				removeComponent2(inSubShapePair.GetBody1ID());
			}
		}
	};

	// An example activation listener
	class MyBodyActivationListener : public BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
		{

		}

		virtual void OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
		{

		}
	};
}