#include "entity.h"

extern void InputPlayer(Entity *player, const Input *input) {
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

extern void UpdatePlayer(Entity *player, const Map *map, const float dt) {
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