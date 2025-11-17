#include "collision.h"

#define COLLISION_VERTEX_EPSILON 0.001f

static bool IsNextTileInDirectionObstacle(const Map *map, const Vec2i tilePos, const Vec2f normal) {
	int nextTileX = tilePos.x + (int)normal.x;
	int nextTileY = tilePos.y + (int)normal.y;
	uint32_t currentTile = MapGetTile(map, tilePos);
	uint32_t nextTile = MapGetTile(map, V2iInit(nextTileX, nextTileY));
	bool result = MapIsObstacle(map, nextTile);
	return result;
}

static uint32_t ComputeClosestContactsAABBvsAABB(Vec2f delta, Vec2f aabbCenter, Vec2f aabbHalfExtents, Vec2f point, Contact *outContacts) {
	// Form the closest plane to the point
	Vec2f majorAxis = V2fMajorAxis(delta);
	Vec2f planeN = V2fNegate(majorAxis);
	Vec2f planeCenter = V2fAdd(aabbCenter, V2fHadamard(planeN, aabbHalfExtents));

	// Compute edge of the AABB
	Vec2f tangent = V2fPerp(planeN);
	float projTangent = F32Abs(V2fDot(tangent, aabbHalfExtents));
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
	bool isVertexContact = v <= COLLISION_VERTEX_EPSILON || u <= COLLISION_VERTEX_EPSILON;
	if (isVertexContact) {
		return 0;
	}

	// Distance point from plane
	Vec2f planeDelta = V2fSub(point, planeCenter);
	float distance = V2fDot(planeDelta, planeN);

	// Fill out contact
	outContacts[0].normal = planeN;
	outContacts[0].distance = distance;
	outContacts[0].posA = point;
	outContacts[0].posB = planeCenter;
	outContacts[0].impulse = 0.0f;

	return 1;
}

extern uint32_t CreateContactsAABBvsAABB(const Map *map, const uint32_t idA, const AABB2f *a, const uint32_t idB, const AABB2f *b, const Vec2i tilePos, const bool checkInternal, Contact outContacts[2]) {
    if (map == fpl_null || idA == 0 || a == fpl_null || idB == 0 || b == fpl_null || outContacts == fpl_null) {
		return 0;
	}

	Vec2f radiusA, radiusB;
	Vec2f centerA, centerB;
	AABB2fExtract(a, &centerA, &radiusA);
	AABB2fExtract(b, &centerB, &radiusB);

	Vec2f minkowskiSum = V2fAdd(radiusA, radiusB);

	Vec2f delta = V2fSub(centerB, centerA);

	uint32_t numContacts = ComputeClosestContactsAABBvsAABB(delta, centerB, minkowskiSum, centerA, outContacts);
	if (numContacts == 0) {
		return 0;
	}

	uint32_t result = 0;

	for (uint32_t i = 0; i < numContacts; ++i) {
		Contact *contact = outContacts + i;
		bool isInternalCollision = IsNextTileInDirectionObstacle(map, tilePos, contact->normal);
		if (!isInternalCollision) {
			float radADistance = F32Abs(V2fDot(contact->normal, radiusA));
			contact->idPair = fplStructInit(ContactIDPair, fplMin(idA, idB), fplMax(idA, idB));
			contact->posB = V2fAddMultScalar(contact->posB, contact->normal, -radADistance);
			contact->posA = V2fAddMultScalar(contact->posB, contact->normal, contact->distance);
			++result;
		}
	}

	return result;
}
