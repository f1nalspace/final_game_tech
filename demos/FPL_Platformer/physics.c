#include "physics.h"

extern bool PhysicsInit(Physics *physics) {
	if (physics == fpl_null) {
		return false; // Invalid arguments
	}
	fplClearStruct(physics);
	physics->contactList.capacity = fplArrayCount(physics->contactList.data);
	physics->contactList.used = 0;
	return true;
}

extern bool PhysicsClear(Physics *physics) {
	if (physics == fpl_null) {
		return false; // Invalid arguments
	}
	physics->contactList.used = 0;
	return true;
}

extern void PhysicsBegin(Physics *physics) {
	if (physics == fpl_null) {
		return; // Invalid arguments
	}
	physics->contactList.used = 0;
}

extern void PhysicsEnd(Physics *physics) {
	if (physics == fpl_null) {
		return; // Invalid arguments
	}
}

extern bool PhysicsPushContact(Physics *physics, const Contact *contact) {
	if (physics == fpl_null || contact == fpl_null) {
		return false; // Invalid arguments
	}

	if (physics->contactList.used >= physics->contactList.capacity) {
		return false; // Not enough space for contact
	}

	size_t index = physics->contactList.used;
	Contact *target = physics->contactList.data + index;
	physics->contactList.used++;
	*target = *contact;

	return true;
}

static bool IsNextTileInDirectionObstacle(const Map *map, const Vec2i tilePos, const Vec2f normal) {
	int nextTileX = tilePos.x + (int)normal.x;
	int nextTileY = tilePos.y + (int)normal.y;
	Vec2i nextPos = V2iInit(nextTileX, nextTileY);
	if (!MapIsTileInside(map, nextPos)) {
		return false;
	}
	bool currentTile = MapTilePosIsObstacle(map, tilePos);
	bool nextTile = MapTilePosIsObstacle(map, nextPos);
	bool result = currentTile && nextTile;
	return result;
}

fpl_force_inline Vec2f GetContactMajorAxis(const Vec2f v) {
	float x = F32Abs(v.x);
	float y = F32Abs(v.y);

	// NOTE(final): Prefer x axis over the y axis
	const float k = 0.1f * PhysicsLinearSlope;
	if (y > x + k) {
		return V2fInit(0.0f, F32Sign(v.y));
	} else {
		return V2fInit(F32Sign(v.x), 0.0f);
	}
}

static bool ComputeClosestContactsAABBvsAABB(Vec2f delta, Vec2f aabbCenter, Vec2f aabbRadius, Vec2f point, Contact *outContact) {
	// Form the closest point to the plane
	Vec2f majorAxis = GetContactMajorAxis(delta);
	Vec2f planeN = V2fNegate(majorAxis);
	Vec2f planeCenter = V2fAdd(aabbCenter, V2fHadamard(planeN, aabbRadius));

	// Compute edge of the AABB
	Vec2f tangent = V2fPerp(planeN);
	float projTangent = F32Abs(V2fDot(tangent, aabbRadius));
	Vec2f a = V2fAddMultScalar(planeCenter, tangent, -projTangent);
	Vec2f b = V2fAddMultScalar(planeCenter, tangent, projTangent);

	// Get difference between A-B and A-Q
	Vec2f ab = V2fSub(b, a);
	Vec2f aq = V2fSub(point, a);

	// Get magnitude of AB (length squared) and calculate closest point
	float abLenSquared = V2fDot(ab, ab);
	Vec2f g = V2fAdd(a, V2fMultScalar(ab, V2fDot(aq, ab) / abLenSquared));

	// Get squared root AB magnitude and compute baricentric coordinate
	float abLen = F32SquareRoot(abLenSquared);
	float v = V2fDot(V2fSub(g, a), tangent) / abLen;
	float u = V2fDot(V2fSub(b, g), tangent) / abLen;

	// Skip vertex to vertex contacts entirely
	bool isVertexContact = v <= PhysicsLinearSlope || u <= PhysicsLinearSlope;
	if (isVertexContact) {
		return false;
	}

	// Distance point from plane
	Vec2f planeDelta = V2fSub(point, planeCenter);
	float distance = V2fDot(planeDelta, planeN);

	// Fill out contact
	outContact->normal = planeN;
	outContact->distance = distance;
	outContact->posA = point;
	outContact->posB = planeCenter;
	outContact->impulse = 0.0f;

	return true;
}

extern uint32_t CreateContactsAABBvsAABB(const Map *map, const uint32_t idA, const AABB2f *a, const uint32_t idB, const AABB2f *b, const Vec2i tilePos, const bool checkInternal, Contact outContacts[2]) {
    if (map == fpl_null || idA == 0 || a == fpl_null || idB == 0 || b == fpl_null || outContacts == fpl_null) {
		return 0; // Invalid arguments
	}

	Vec2f radiusA, radiusB;
	Vec2f centerA, centerB;
	AABB2fExtract(a, &centerA, &radiusA);
	AABB2fExtract(b, &centerB, &radiusB);

	Vec2f minkowskiSum = V2fAdd(radiusA, radiusB);

	Vec2f delta = V2fSub(centerB, centerA);

	Contact contact = fplZeroInit;

	if (!ComputeClosestContactsAABBvsAABB(delta, centerB, minkowskiSum, centerA, &contact)) {
		return 0; // No contacts found
	}

	if (checkInternal) {
		bool isInternalCollision = IsNextTileInDirectionObstacle(map, tilePos, contact.normal);
		if (isInternalCollision) {
			return false;
		}
	}

	float radADistance = F32Abs(V2fDot(contact.normal, radiusA));
	contact.idPair = fplStructInit(ContactIDPair, idA, idB);
	contact.posB = V2fAddMultScalar(contact.posB, contact.normal, -radADistance);
	contact.posA = V2fAddMultScalar(contact.posB, contact.normal, contact.distance);

	outContacts[0] = contact;

	return 1;
}
