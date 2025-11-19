#include "entity.h"

#include "physics.h"

extern bool PlayerInit(Entity *player, const Map *map) {
	if (player == fpl_null || map == fpl_null) {
		return false; // Invalid arguments
	}

	player->radius =  V2fHadamard(TileSize, V2fInit(0.3f, 0.7f));
	player->velocity = V2fInit(0.0f, 0.0f);
	player->posCorrect = V2fZero();
	player->color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	player->position[0] = player->position[1] = V2fInit(0.0f, 0.0f);

	Vec2i playerTilePos;
	if (MapFindPositionByTile(map, TileType_PlayerStart, &playerTilePos)) {
		Vec2f tilePos = MapTileCoordsToWorld(map, playerTilePos);
		Vec2f tileBottomCenter = V2fAdd(tilePos, V2fInit(TileWidth * 0.5f, 0));

		// Move the player above the tile, but to the center
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(0, player->radius.h));

		// Move the player above the tile, but to the right
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(TileSize.w * 0.5f - player->radius.w, player->radius.h));

		// Move the player above the tile, but to the left
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(-TileSize.w * 0.5f + player->radius.w, player->radius.h));
	}

	player->applyGroundFriction = true;
	player->groundFriction = PlayerGroundFriction;

	player->applyAirFriction = true;
	player->airFriction = PlayerAirFriction;

	player->isJumpRequested = false;

	return true;
}

extern void PlayerInput(Entity *player, const Input *input) {
	const Controller *controller = (input->defaultControllerIndex == -1) ? &input->controllers[0] : &input->controllers[input->defaultControllerIndex];

	// Horizontal Movement
	float moveSpeed = EntityIsGrounded(player) ? PlayerWalkSpeed : PlayerAirSpeed;
	if (IsDown(controller->moveLeft)) {
		player->velocity.x -= moveSpeed;
	} else if (IsDown(controller->moveRight)) {
		player->velocity.x += moveSpeed;
	}

	// Jump can always be requested, regardless if in air or not
	if (IsDown(controller->actionDown)) {
		if (!player->isJumpRequested) {
			player->isJumpRequested = true;
		}
	} else {
		player->isJumpRequested = false;
	}

	// Handle requested jump only when grounded
	if (EntityIsGrounded(player) && player->isJumpRequested) {
		player->velocity.y = PlayerJumpVelocity;
		player->isJumpRequested = false;
	}
}

static void PlayerCollisionResponse(Entity *player, const Contact *contact, const float dt) {
	// Get the separation and penetration separately
	const float seperation = F32Max(contact->distance, 0.0f);
	const float penetration = F32Min(contact->distance, 0.0f);

	// Compute relative velocity along normal
	Vec2f n = contact->normal;
	float nv = V2fDot(player->velocity, n) + seperation / dt;

	// Accumulate the penetration correction
	player->posCorrect = V2fSub(player->posCorrect, V2fMultScalar(n, penetration / dt));

	if (nv < 0.0f) {
		// Remove normal velocity
		player->velocity = V2fSub(player->velocity, V2fMultScalar(n, nv));

		// TODO(final): Use a dot product threshold instead of just normal.y with a ground axis (0, 1)
		if (n.y > 0.0f) {
			// We are touching ground
			player->groundState.current = true;

			// Apply friction
			if (player->applyGroundFriction) {
				// Get the tangent from the normal (perp vector)
				Vec2f t = V2fPerp(n);

				// Compute the tangential velocity, scale by friction
				float friction = F32Clamp(player->groundFriction, 0.0f, 1.0f);
				float tv = V2fDot(player->velocity, t) * friction;

				// Subtract that from the main velocity
				player->velocity = V2fSub(player->velocity, V2fMultScalar(t, tv));
			}

			if (!player->groundState.last) {
				// TODO(final): Landing transition
			}
		}
	}
}

static void PlayerMapCollisions(Physics *physics, Entity *player, const Map *map, const Vec2i tileMin, const Vec2i tileMax, const float dt) {
	if (map->width == 0 || map->height == 0) {
		return;
	}

	int mapMaxWidthMinusOne = map->width - 1;
	int mapMaxHeightMinusOne = map->height - 1;

	Contact contacts[2] = fplZeroInit;

	// TODO(final): Use correct entity id
	const uint32_t playerId = EntityIDStart + 0;

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
			AABB2f playerBounds = AABB2fInitFromCenter(player->position[0], player->radius);

			uint32_t contactCount = CreateContactsAABBvsAABB(map, playerId, &playerBounds, tileID, &tileBounds, tilePos, true, contacts);
			if (contactCount > 0) {
				for (uint32_t i = 0; i < contactCount; ++i) {
					const Contact *contact = &contacts[i];
					if (PhysicsPushContact(physics, contact)) {
						PlayerCollisionResponse(player, contact, dt);
					}
				}
			}
		}
	}
}

static void PlayerHandleCollisions(Physics *physics, Entity *player, const Map *map, const float dt) {
	Vec2f pos = player->position[0];

	// Predict position for next frame
	Vec2f predictedPos = V2fAddMultScalar(pos, player->velocity, dt);

	// Create motion bounds AABB and expand it
	Vec2f min = V2fSub(V2fMin(predictedPos, pos), player->radius);
	Vec2f max = V2fAdd(V2fMax(predictedPos, pos), player->radius);
	AABB2f playerMotionBounds = AABB2fInit(min, max);
	AABB2fExpandScalar(&playerMotionBounds, PhysicsAABBExpansion);

	// Get tiles area in min/max tile coordinates
	Vec2i minTile = MapWorldCoordsToTile(map, playerMotionBounds.min);
	Vec2i maxTile = MapWorldCoordsToTile(map, V2fAdd(playerMotionBounds.max, V2fInit(0.5f, 0.5f)));

	PlayerMapCollisions(physics, player, map, minTile, maxTile, dt);
}

extern void PlayerUpdate(Physics *physics, Entity *player, const Map *map, const float dt) {
	// Gravity
	player->velocity.y += -PlayerGravity;

	// Air friction
	if (player->applyAirFriction && EntityIsAir(player) && F32Abs(player->velocity.x) > 0) {
		player->velocity.x *= (1.0f - player->airFriction);
	}

	// Clamp speeds
	player->velocity.x = F32Clamp(player->velocity.x, -PlayerMaxHorizontalSpeed, PlayerMaxHorizontalSpeed);
	player->velocity.y = F32Clamp(player->velocity.y, -PlayerMaxVerticalSpeed, PlayerMaxVerticalSpeed);

	// Reset ground state
	player->groundState.current = false;

	// Handle collisions
	PlayerHandleCollisions(physics, player, map, dt);

	// Integrate with position correction
	player->position[0] = V2fAddMultScalar(player->position[0], V2fAdd(player->velocity, player->posCorrect), dt);

	// Reset position correction
	player->posCorrect = V2fZero();

	// Grounding
	player->groundState.last = player->groundState.current;
}