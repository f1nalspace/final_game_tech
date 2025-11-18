#include "entity.h"

extern bool PlayerInit(Entity *player, const Map *map) {
	if (player == fpl_null || map == fpl_null) {
		return false; // Invalid arguments
	}

	player->radius = V2fInit(TileWidth * 0.4f, TileHeight * 0.8f);
	player->velocity = V2fInit(0.0f, 0.0f);
	player->color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	player->position[0] = player->position[1] = V2fInit(0.0f, 0.0f);

	Vec2i playerTilePos;
	if (MapFindPositionByTile(map, TileType_PlayerPosition, &playerTilePos)) {
		Vec2f tilePos = MapTileCoordsToWorld(map, playerTilePos);
		Vec2f tileBottomCenter = V2fAdd(tilePos, V2fInit(TileWidth * 0.5f, 0));

		// Move the player above the tile, but to the center
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(0, player->radius.h));

		// Move the player above the tile, but to the right
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(TileSize.w * 0.5f - player->radius.w, player->radius.h));
	}

	player->applyFriction = true;
	player->groundFriction = PlayerGroundFriction;

	player->applyAirFriction = true;
	player->airFriction = PlayerAirFriction;

	player->jumpRequested = false;

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
		if (!player->jumpRequested) {
			player->jumpRequested = true;
		}
	} else {
		player->jumpRequested = false;
	}

	// Handle requested jump only when grounded
	if (EntityIsGrounded(player) && player->jumpRequested) {
		player->velocity.y = PlayerJumpVelocity;
		player->jumpRequested = false;
	}
}

extern void PlayerUpdate(Entity *player, const Map *map, const float dt) {
#if 0
	// Gravity
	player->velocity += Gravity;
#endif

	// Air friction
	if (player->applyAirFriction && EntityIsAir(player) && F32Abs(player->velocity.x) > 0) {
		player->velocity.x *= (1.0f - player->airFriction);
	}

	// Clamp speed
	player->velocity.x = F32Clamp(player->velocity.x, -PlayerMaxSpeed, PlayerMaxSpeed);

	// Grounding
	player->groundState.last = player->groundState.current;
	player->groundState.current = false;

	// Integrate
	player->position[0] = V2fAddMultScalar(player->position[0], player->velocity, dt);
}