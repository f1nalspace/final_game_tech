#include "entity.h"

#include "physics.h"

extern bool EntityInit(Entity *entity, const uint32_t id, const Vec2f radius, const Vec2f position, const Vec4f color) {
	if (entity == fpl_null) {
		return false; // Invalid arguments
	}

	fplClearStruct(entity);
	entity->id = id;
	entity->radius =  V2fHadamard(TileSize, V2fInit(0.3f, 0.7f));
	entity->velocity = radius;
	entity->posCorrect = V2fZero();
	entity->color = color;
	entity->position[0] = entity->position[1] = position;

	entity->applyGroundFriction = true;
	entity->groundFriction = EntityDefaultGroundFriction;

	entity->applyAirFriction = true;
	entity->airFriction = EntityDefaultAirFriction;

	entity->isJumpRequested = false;

	return true;
}

extern void EntityInput(Entity *entity, const Input *input) {
	const Controller *controller = (input->defaultControllerIndex == -1) ? &input->controllers[0] : &input->controllers[input->defaultControllerIndex];

	// Horizontal Movement
	float moveSpeed = EntityIsGrounded(entity) ? EntityDefaultWalkSpeed : EntityDefaultAirSpeed;
	if (IsDown(controller->moveLeft)) {
		entity->velocity.x -= moveSpeed;
	} else if (IsDown(controller->moveRight)) {
		entity->velocity.x += moveSpeed;
	}

	// Jump can always be requested, regardless if in air or not
	if (IsDown(controller->actionDown)) {
		if (!entity->isJumpRequested) {
			entity->isJumpRequested = true;
		}
	} else {
		entity->isJumpRequested = false;
	}

	// Handle requested jump only when grounded
	if (EntityIsGrounded(entity) && entity->isJumpRequested) {
		entity->velocity.y = EntityDefaultJumpVelocity;
		entity->isJumpRequested = false;
	}
}

static void EntityCollisionResponse(Entity *entitiy, const Contact *contact, const float dt) {
	// Get the separation and penetration separately
	const float seperation = F32Max(contact->distance, 0.0f);
	const float penetration = F32Min(contact->distance, 0.0f);

	// Compute relative velocity along normal
	Vec2f n = contact->normal;
	float nv = V2fDot(entitiy->velocity, n) + seperation / dt;

	// Accumulate the penetration correction
	entitiy->posCorrect = V2fSub(entitiy->posCorrect, V2fMultScalar(n, penetration / dt));

	if (nv < 0.0f) {
		// Remove normal velocity
		entitiy->velocity = V2fSub(entitiy->velocity, V2fMultScalar(n, nv));

		// TODO(final): Use a dot product threshold instead of just normal.y with a ground axis (0, 1)
		if (n.y > 0.0f) {
			// We are touching ground
			entitiy->groundState.current = true;

			// Apply friction
			if (entitiy->applyGroundFriction) {
				// Get the tangent from the normal (perp vector)
				Vec2f t = V2fPerp(n);

				// Compute the tangential velocity, scale by friction
				float friction = F32Clamp(entitiy->groundFriction, 0.0f, 1.0f);
				float tv = V2fDot(entitiy->velocity, t) * friction;

				// Subtract that from the main velocity
				entitiy->velocity = V2fSub(entitiy->velocity, V2fMultScalar(t, tv));
			}

			if (!entitiy->groundState.last) {
				// TODO(final): Landing transition
			}
		}
	}
}

static void EntityMapCollisions(Physics *physics, Entity *entity, const Map *map, const Vec2i tileMin, const Vec2i tileMax, const float dt) {
	if (physics == fpl_null || entity == fpl_null || map->width == 0 || map->height == 0) {
		return;
	}

	int mapMaxWidthMinusOne = map->width - 1;
	int mapMaxHeightMinusOne = map->height - 1;

	Contact contacts[2] = fplZeroInit;

	for (int x = tileMin.x; x <= tileMax.x; ++x) {
		for (int y = tileMin.y; y <= tileMax.y; ++y) {
			if ((x < 0 || x > mapMaxWidthMinusOne) || (y < 0 || y > mapMaxHeightMinusOne)) {
				continue;
			}

			Vec2i tilePos = V2iInit(x, y);

			Tile tile = MapGetTile(map, tilePos);
			if (!MapTileTypeIsObstacle(map, tile.type)) {
				continue;
			}

			uint32_t index = y * map->width + x;

			uint32_t tileID = MapTileIDStart + index;

			Vec2f tileWorld = MapTileCoordsToWorld(map, tilePos);
			AABB2f tileBounds = AABB2fInit(tileWorld, V2fAdd(tileWorld, TileSize));
			AABB2f playerBounds = AABB2fInitFromCenter(entity->position[0], entity->radius);

			uint32_t contactCount = CreateContactsAABBvsAABB(map, entity->id, &playerBounds, tileID, &tileBounds, tilePos, true, contacts);
			if (contactCount > 0) {
				for (uint32_t i = 0; i < contactCount; ++i) {
					const Contact *contact = &contacts[i];
					if (PhysicsPushContact(physics, contact)) {
						EntityCollisionResponse(entity, contact, dt);
					}
				}
			}
		}
	}
}

static void EntityHandleCollisions(Physics *physics, Entity *entity, const Map *map, const float dt) {
	Vec2f pos = entity->position[0];

	// Predict position for next frame
	Vec2f predictedPos = V2fAddMultScalar(pos, entity->velocity, dt);

	// Create motion bounds AABB and expand it
	Vec2f min = V2fSub(V2fMin(predictedPos, pos), entity->radius);
	Vec2f max = V2fAdd(V2fMax(predictedPos, pos), entity->radius);
	AABB2f playerMotionBounds = AABB2fInit(min, max);
	AABB2fExpandScalar(&playerMotionBounds, PhysicsAABBExpansion);

	// Get tiles area in min/max tile coordinates
	Vec2i minTile = MapWorldCoordsToTile(map, playerMotionBounds.min);
	Vec2i maxTile = MapWorldCoordsToTile(map, V2fAdd(playerMotionBounds.max, V2fInit(0.5f, 0.5f)));

	EntityMapCollisions(physics, entity, map, minTile, maxTile, dt);
}

extern void EntityUpdate(Physics *physics, Entity *entity, const Map *map, const float dt) {
	// Gravity
	entity->velocity.y += -EntityGravity;

	// Air friction
	if (entity->applyAirFriction && EntityIsAir(entity) && F32Abs(entity->velocity.x) > 0) {
		entity->velocity.x *= (1.0f - entity->airFriction);
	}

	// Clamp speeds
	entity->velocity.x = F32Clamp(entity->velocity.x, -EntityMaxHorizontalSpeed, EntityMaxHorizontalSpeed);
	entity->velocity.y = F32Clamp(entity->velocity.y, -EntityMaxVerticalSpeed, EntityMaxVerticalSpeed);

	// Reset ground state
	entity->groundState.current = false;

	// Handle collisions
	EntityHandleCollisions(physics, entity, map, dt);

	// Integrate with position correction
	entity->position[0] = V2fAddMultScalar(entity->position[0], V2fAdd(entity->velocity, entity->posCorrect), dt);

	// Reset position correction
	entity->posCorrect = V2fZero();

	// Grounding
	entity->groundState.last = entity->groundState.current;
}