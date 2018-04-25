#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include "game.h"
#include "utils.h"

#include <Box2D\Box2D.cpp>

#include <vector>

#define DRAW_NORMALS 0

constexpr glm::f32 Pi32 = glm::pi<glm::f32>();
constexpr glm::f32 Tau32 = Pi32 * 2.0f;

constexpr glm::f32 GameAspect = 16.0f / 9.0f;
constexpr glm::f32 WorldWidth = 100.0f;
constexpr glm::f32 WorldHeight = WorldWidth / GameAspect;
const glm::vec2 WorldRadius = glm::vec2(WorldWidth, WorldHeight) * 0.5f;

constexpr glm::f32 BallRadius = WorldWidth * 0.01f;
constexpr glm::f32 BallDiameter = BallRadius * 2.0f;
constexpr glm::f32 BallSpeed = 10.0f;

constexpr glm::f32 AreaPadding = BallRadius * 3.0f;
const glm::vec2 AreaRadius = glm::vec2(WorldRadius.x - AreaPadding, WorldRadius.y - AreaPadding);

constexpr glm::f32 PaddleSpeed = 180.0f;
const glm::vec2 PaddleRadius = glm::vec2(BallRadius * 5, BallRadius);
const glm::f32 PaddleLineY = -WorldRadius.y + (PaddleRadius.y + AreaPadding);

constexpr glm::f32 BrickSpacing = BallRadius * 0.75f;
constexpr int MaxBrickCols = 15;
constexpr int MaxBrickRows = 14;
const glm::f32 SpaceForBricksX = (AreaRadius.x * 2.0f) - (MaxBrickCols - 1) * BrickSpacing;
const glm::f32 SpaceForBricksY = (AreaRadius.y * 2.0f - BallDiameter * 6) - (MaxBrickRows - 1) * BrickSpacing;
const glm::vec2 BrickRadius = glm::vec2(SpaceForBricksX / (glm::f32)MaxBrickCols, SpaceForBricksY / (glm::f32)MaxBrickRows) * 0.5f;

struct Field {
	b2Body *body;
};

struct Ball {
	bool isMoving;
	b2Body *body;
	glm::f32 radius;
	glm::f32 speed;
};

struct Paddle {
	b2Body *body;
	glm::f32 capsuleHalfWidth;
	glm::f32 capsuleHalfHeight;
	glm::f32 halfCircleRadius;
	glm::f32 speed;
	Ball *gluedBall;
};

struct Brick {
	b2Body *body;
	glm::vec2 initialPos;
	glm::vec2 radius;
};

struct GameState {
	glm::ivec2 viewSize;
	glm::ivec2 viewOffset;

	b2World *world;

	Field field;
	Ball ball;
	Paddle paddle;
	Brick bricks[1024];
	size_t numBricks;
};

static void DrawCircleVertices(const glm::f32 r, const int segments) {
	glm::f32 alpha = Tau32 / (glm::f32)segments;
	for (int i = 0; i <= segments; ++i) {
		glm::f32 a = i * alpha;
		glm::f32 c = glm::cos(a) * r;
		glm::f32 s = glm::sin(a) * r;
		glVertex2f(c, s);
	}
}
static void DrawCircle(glm::f32 r, bool isFilled, const int segments = 24) {
	glBegin(isFilled ? GL_POLYGON : GL_LINE_STRIP);
	DrawCircleVertices(r, segments);
	glEnd();
}

static void DrawNormal(const glm::vec2 &p, const glm::vec2 &n, glm::f32 d) {
	glBegin(GL_LINES);
	glVertex2f(p.x, p.y);
	glVertex2f(p.x + n.x * d, p.y + n.y * d);
	glEnd();
}

inline glm::vec2 Vec2FromAngle(glm::f32 angle) {
	return glm::vec2(glm::cos(angle), glm::sin(angle));
}

static void GlueBallOnPaddle(GameState &state) {
	state.ball.isMoving = false;
	state.paddle.gluedBall = &state.ball;
}

inline glm::f32 Random01() {
	glm::f32 result = rand() / (float)RAND_MAX;
	return(result);
}

static void ShootBall(GameState &state) {
	const glm::f32 spreadAngle = 30.0f;
	const glm::f32 startAngle = 90;
	Ball *ball = state.paddle.gluedBall;
	ball->isMoving = true;
	glm::f32 newAngle = startAngle + (Random01() > 0.5f ? -1 : 1) * Random01() * spreadAngle;
	glm::f32 a = glm::radians<float>(newAngle);
	b2Vec2 direction = b2Vec2(glm::cos(a), glm::sin(a));
	ball->body->ApplyLinearImpulse(ball->speed * direction, ball->body->GetPosition(), true);
	state.paddle.gluedBall = nullptr;
}

static void SetRandomLevel(GameState &state) {
	state.numBricks = 0;

	glm::f32 halfWidth = (MaxBrickCols * BrickRadius.x) - ((MaxBrickCols - 1) * BrickSpacing * 0.5f);
	glm::f32 y = AreaRadius.y;
	for (int row = 0; row < MaxBrickRows; ++row) {
		glm::f32 x = -AreaRadius.x;
		for (int col = 0; col < MaxBrickCols; ++col) {
			Brick &brick = state.bricks[state.numBricks++];
			brick.initialPos = glm::vec2(x + BrickRadius.x, y);
			brick.radius = BrickRadius;
			x += brick.radius.x * 2.0f + BrickSpacing;
		}
		y -= (BrickRadius.y * 2.0f + BrickSpacing);
	}
}

static bool GameInit(GameState &state) {
	if (!fglLoadOpenGL(true)) {
		return false;
	}

	srand((int)fplGetTimeInMillisecondsLP());

	float dt = 1.0f / 60.0f;

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	glClearColor(0.0f, 0.1f, 0.2f, 1.0f);

	state.ball.radius = BallRadius;
	state.ball.speed = BallSpeed;
	state.paddle.halfCircleRadius = PaddleRadius.y;
	state.paddle.capsuleHalfWidth = PaddleRadius.x;
	state.paddle.capsuleHalfHeight = PaddleRadius.y;
	state.paddle.speed = PaddleSpeed;

	const glm::f32 hw = WorldRadius.x;
	const glm::f32 hh = WorldRadius.y;

	SetRandomLevel(state);

	b2World *world;

	state.world = world = new b2World(b2Vec2(0, 0));
	world->SetContinuousPhysics(true);

	//
	// Field
	//
	{
		b2Body *body;
		b2BodyDef fieldDef = b2BodyDef();
		fieldDef.type = b2BodyType::b2_staticBody;
		fieldDef.position = b2Vec2(0, 0);
		fieldDef.angle = 0.0f;
		fieldDef.fixedRotation = true;
		fieldDef.linearDamping = 0;
		fieldDef.angularDamping = 0;
		state.field.body = body = world->CreateBody(&fieldDef);
		body->SetUserData(&state.field);

		b2Vec2 fieldVertices[4] = {
			b2Vec2(hw, hh),
			b2Vec2(-hw, hh),
			b2Vec2(-hw, -hh),
			b2Vec2(hw, -hh),
		};
		b2ChainShape fieldShape = b2ChainShape();
		fieldShape.CreateLoop(fieldVertices, 4);

		b2FixtureDef fieldFixtureDef = b2FixtureDef();
		fieldFixtureDef.shape = &fieldShape;
		fieldFixtureDef.restitution = 1.0f;
		fieldFixtureDef.friction = 0.0f;
		fieldFixtureDef.density = 1.0f;
		body->CreateFixture(&fieldFixtureDef);
	}

	//
	// Bricks
	//
	{
		b2Body *body;
		for (size_t i = 0; i < state.numBricks; ++i) {
			Brick &brick = state.bricks[i];
			b2BodyDef brickDef = b2BodyDef();
			brickDef.type = b2BodyType::b2_staticBody;
			brickDef.position = b2Vec2(brick.initialPos.x, brick.initialPos.y);
			brickDef.angle = 0.0f;
			brickDef.fixedRotation = true;
			brickDef.linearDamping = 0;
			brickDef.angularDamping = 0;
			brick.body = body = world->CreateBody(&brickDef);
			body->SetUserData(&brick);

			b2PolygonShape brickShape = b2PolygonShape();
			brickShape.SetAsBox(brick.radius.x, brick.radius.y);

			b2FixtureDef brickFixtureDef = b2FixtureDef();
			brickFixtureDef.shape = &brickShape;
			brickFixtureDef.restitution = 1.0f;
			brickFixtureDef.friction = 0.0f;
			brickFixtureDef.density = 1.0f;
			body->CreateFixture(&brickFixtureDef);
		}
	}

	//
	// Paddle
	// 
	{
		b2Body *body;
		b2BodyDef paddleDef;
		b2FixtureDef paddleFixtureDef;

		// Limiter
		paddleDef = b2BodyDef();
		paddleDef.type = b2BodyType::b2_staticBody;
		paddleDef.position = b2Vec2(0, PaddleLineY);
		b2Body *paddleLimiterBody = body = world->CreateBody(&paddleDef);
		b2PolygonShape limiterShape = b2PolygonShape();
		limiterShape.SetAsBox(state.ball.radius, state.ball.radius);
		paddleFixtureDef = b2FixtureDef();
		paddleFixtureDef.shape = &limiterShape;
		paddleFixtureDef.restitution = 0.0f;
		paddleFixtureDef.friction = 1.0f;
		paddleFixtureDef.density = 1.0f;
		paddleFixtureDef.filter.maskBits = 0x0000;
		body->CreateFixture(&paddleFixtureDef);
		body->SetUserData("PaddleLimiter");

		// Paddle
		paddleDef = b2BodyDef();
		paddleDef.type = b2BodyType::b2_dynamicBody;
		paddleDef.allowSleep = false;
		paddleDef.bullet = true;
		paddleDef.position = b2Vec2(0, PaddleLineY);
		paddleDef.angle = 0;
		paddleDef.fixedRotation = true;
		paddleDef.linearDamping = 2.5f;
		paddleDef.angularDamping = 0;
		state.paddle.body = body = world->CreateBody(&paddleDef);
		body->SetUserData(&state.paddle);

		b2PolygonShape capsuleShape = b2PolygonShape();
		capsuleShape.SetAsBox(state.paddle.capsuleHalfWidth, state.paddle.capsuleHalfHeight);

		paddleFixtureDef = b2FixtureDef();
		paddleFixtureDef.shape = &capsuleShape;
		paddleFixtureDef.restitution = 0.0f;
		paddleFixtureDef.friction = 0.0f;
		paddleFixtureDef.density = 20.0f;
		paddleFixtureDef.filter.maskBits = 0xFFFF;
		body->CreateFixture(&paddleFixtureDef);

		// Paddle join (Restrict to X-Axis)
		b2PrismaticJointDef jointDef = b2PrismaticJointDef();
		b2Vec2 limiterAxis = b2Vec2(1, 0);
		jointDef.collideConnected = true;
		jointDef.Initialize(body, paddleLimiterBody, state.paddle.body->GetWorldCenter(), limiterAxis);
		world->CreateJoint(&jointDef);
	}

	//
	// Ball
	//
	{
		b2Body *body;
		b2BodyDef ballDef = b2BodyDef();
		ballDef.type = b2BodyType::b2_dynamicBody;
		ballDef.allowSleep = false;
		ballDef.bullet = true;
		ballDef.position = b2Vec2(0, 0);
		ballDef.angle = 0;
		ballDef.fixedRotation = true;
		ballDef.linearDamping = 0;
		ballDef.angularDamping = 0;
		state.ball.body = body = world->CreateBody(&ballDef);
		body->SetUserData(&state.ball);

		b2CircleShape ballShape = b2CircleShape();
		ballShape.m_radius = state.ball.radius;

		b2FixtureDef ballFixtureDef = b2FixtureDef();
		ballFixtureDef.shape = &ballShape;
		ballFixtureDef.restitution = 1.0f;
		ballFixtureDef.friction = 0.0f;
		ballFixtureDef.density = 1.0f;
		ballFixtureDef.filter.maskBits = 0xFFFF;
		body->CreateFixture(&ballFixtureDef);
	}

	GlueBallOnPaddle(state);

	return true;
}

static void GameRelease(GameState &state) {
	std::vector<b2Body *> bodies;
	for (b2Body *body = state.world->GetBodyList(); body != nullptr; body = body->GetNext()) {
		bodies.push_back(body);
	}
	if (bodies.size() > 0) {
		for (size_t i = (int)bodies.size() - 1; i > 0; i--) {
			state.world->DestroyBody(bodies[i]);
		}
	}
	if (state.world != nullptr) {
		delete state.world;
		state.world = nullptr;
	}
	fglUnloadOpenGL();
}

extern void GameDestroy(GameState *state) {
	if (state != nullptr) {
		GameRelease(*state);
		state->~GameState();
		fplMemoryFree(state);
	}
}

extern GameState *GameCreate() {
	void *stateMem = fplMemoryAllocate(sizeof(GameState));
	if (stateMem == nullptr) {
		return nullptr;
	}
	GameState *state = new (stateMem) GameState;
	if (!GameInit(*state)) {
		GameDestroy(state);
	}
	return(state);
}

extern void GameUpdate(GameState &state, const Input &input) {
	glm::ivec2 viewSize = glm::ivec2(input.windowSize.x, (int)(input.windowSize.x / GameAspect));
	if (viewSize.y > input.windowSize.y) {
		viewSize.y = input.windowSize.y;
		viewSize.x = (int)(input.windowSize.y * GameAspect);
	}
	glm::ivec2 viewOffset = glm::ivec2((input.windowSize.x - viewSize.x) / 2, (input.windowSize.y - viewSize.y) / 2);
	state.viewSize = viewSize;
	state.viewOffset = viewOffset;

	state.world->ClearForces();

	if (state.paddle.gluedBall != nullptr) {
		Ball *ball = state.paddle.gluedBall;
		b2Vec2 gluePos = state.paddle.body->GetPosition() + b2Vec2(0, state.paddle.capsuleHalfHeight + ball->radius * 4);
		ball->body->SetTransform(gluePos, 0);
	}

	const Controller &controller = input.controllers[0];
	if (controller.isConnected) {
		if (controller.moveLeft.isDown) {
			state.paddle.body->ApplyLinearImpulse(state.paddle.speed * b2Vec2(-1, 0), state.paddle.body->GetPosition(), true);
		} else if (controller.moveRight.isDown) {
			state.paddle.body->ApplyLinearImpulse(state.paddle.speed * b2Vec2(1, 0), state.paddle.body->GetPosition(), true);
		}

		if (controller.actionDown.WasPressed() && state.paddle.gluedBall != nullptr) {
			ShootBall(state);
		}
	}

	const glm::f32 angleTolerance = 2.5f;
	const glm::f32 angleCorrection = 15.0f;
	glm::f32 squaredAngles[] = { 0, 90, 180, 270, 360 };
	if (state.ball.isMoving) {
		Ball *ball = &state.ball;

		b2Vec2 vel = ball->body->GetLinearVelocity();
		assert(vel.Length() >= b2_velocityThreshold);

		b2Vec2 dir = vel;
		dir.Normalize();

		// Get angle from direction and correct it when too squared
		glm::f32 a = glm::atan<float>(dir.y, dir.x);
		glm::f32 deg = glm::degrees<float>(a);
		for (int i = 0; i < ArrayCount(squaredAngles); ++i) {
			if (glm::abs<float>(deg) > (squaredAngles[i] - angleTolerance) && glm::abs<float>(deg) < (squaredAngles[i] + angleTolerance)) {
				deg += (glm::abs(deg) - squaredAngles[i] > 0 ? 1 : -1) * angleCorrection;
				a = glm::radians<float>(deg);
			}
		}
		dir = b2Vec2(glm::cos(a), glm::sin(a));
		dir *= ball->speed;
		ball->body->SetLinearVelocity(dir);
	}

	state.world->Step(input.deltaTime, 6, 2);
}

extern void GameDraw(GameState &state) {
	const glm::f32 w = WorldRadius.x;
	const glm::f32 h = WorldRadius.y;

	glm::ivec2 viewSize = state.viewSize;
	glm::ivec2 viewOffset = state.viewOffset;
	glViewport(viewOffset.x, viewOffset.y, viewSize.x, viewSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w, w, -h, h, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Viewport
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(w, h);
	glVertex2f(-w, h);
	glVertex2f(-w, -h);
	glVertex2f(w, -h);
	glEnd();

	// Bodies
	const float normalLen = w * 0.025f;
	b2Body *body = state.world->GetBodyList();
	while (body != nullptr) {
		if (body->GetUserData() == "PaddleLimiter") {
			body = body->GetNext();
			continue;
		}
		b2Fixture *fixture = body->GetFixtureList();
		b2Vec2 bodyPos = body->GetPosition();
		glm::f32 bodyRot = body->GetAngle();
		glPushMatrix();
		glTranslatef(bodyPos.x, bodyPos.y, 0);
		glRotatef(glm::degrees<float>(bodyRot), 0, 0, 1);
		while (fixture != nullptr) {
			switch (fixture->GetType()) {
				case b2Shape::Type::e_circle:
				{
					b2CircleShape *circle = (b2CircleShape *)fixture->GetShape();
					glColor3f(0.0f, 0.0f, 1.0f);
					DrawCircle(circle->m_radius, false);
				} break;

				case b2Shape::Type::e_polygon:
				{
					b2PolygonShape *poly = (b2PolygonShape *)fixture->GetShape();
					for (int i = 0; i < poly->m_count; ++i) {
						b2Vec2 a = poly->m_vertices[i];
						b2Vec2 b = poly->m_vertices[(i + 1) % poly->m_count];
						b2Vec2 n = poly->m_normals[i];

						glColor3f(0.0f, 0.0f, 1.0f);
						glBegin(GL_LINES);
						glVertex2f(a.x, a.y);
						glVertex2f(b.x, b.y);
						glEnd();

#if DRAW_NORMALS
						b2Vec2 na = a + 0.5f * (b - a);
						b2Vec2 nb = na + normalLen * n;
						glColor3f(0.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(na.x, na.y);
						glVertex2f(nb.x, nb.y);
						glEnd();
#endif
					}
				} break;

				case b2Shape::Type::e_chain:
				{
					b2ChainShape *chain = (b2ChainShape *)fixture->GetShape();
					for (int i = 0, c = chain->GetChildCount(); i < c; ++i) {
						b2EdgeShape edge;
						chain->GetChildEdge(&edge, i);
						b2Vec2 a = edge.m_vertex1;
						b2Vec2 b = edge.m_vertex2;
						b2Vec2 n = b2Cross(1.0f, b - a);
						n.Normalize();

						glColor3f(1.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(a.x, a.y);
						glVertex2f(b.x, b.y);
						glEnd();

#if DRAW_NORMALS
						b2Vec2 na = a + 0.5f * (b - a);
						b2Vec2 nb = na + normalLen * n;
						glColor3f(0.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(na.x, na.y);
						glVertex2f(nb.x, nb.y);
						glEnd();
#endif
					}
				} break;

			}
			fixture = fixture->GetNext();
		}
		glPopMatrix();
		body = body->GetNext();
	}
}