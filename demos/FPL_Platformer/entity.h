#ifndef ENTITY_H
#define ENTITY_H

#include <final_math.h>

//
// Entity / Player values
//
#define PlayerGravity V2fInit(0, -10.0f)
#define PlayerMaxSpeed 100.0f
#define PlayerWalkSpeed 30.0f
#define PlayerAirSpeed 40.0f
#define PlayerJumpVelocity (200.0f * 1.2f)
#define PlayerGroundFriction 0.2f
#define PlayerAirFriction 0.2f

#endif // ENTITY_H
