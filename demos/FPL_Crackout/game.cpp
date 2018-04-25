#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include "game.h"
#include "utils.h"

#include <Box2D\Box2D.cpp>

#include <vector>

constexpr glm::f32 GameAspect = 16.0f / 9.0f;
constexpr glm::f32 WorldWidth = 200.0f;
constexpr glm::f32 WorldHeight = WorldWidth / GameAspect;
constexpr glm::f32 Pi32 = glm::pi<glm::f32>();
constexpr glm::f32 Tau32 = Pi32 * 2.0f;

struct Field {
	b2Body *body;
};

struct Ball {
	b2Body *body;
	glm::f32 radius;
};

struct Paddle {
	b2Body *body;
	glm::f32 capsuleHalfWidth;
	glm::f32 capsuleHalfHeight;
	glm::f32 halfCircleRadius;
};

struct GameState {
	glm::vec2 halfWorldSize;
	glm::ivec2 viewSize;
	glm::ivec2 viewOffset;

	glm::vec2 halfFieldSize;

	b2World *world;

	Field field;
	Ball ball;
	Paddle paddle;
};

static void DrawCircleVertices(const glm::f32 r, const int segments) {
	glm::f32 alpha = Tau32 / (glm::f32)segments;
	for(int i = 0; i <= segments; ++i) {
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

static bool GameInit(GameState &state) {
	if(!fglLoadOpenGL(true)) {
		return false;
	}

	float dt = 1.0f / 60.0f;

	glLineWidth(0.5f);
	glClearColor(0.0f, 0.1f, 0.2f, 1.0f);

	state.halfWorldSize = glm::vec2(WorldWidth, WorldHeight) * 0.5f;
	glm::f32 w = state.halfWorldSize.x;
	glm::f32 h = state.halfWorldSize.y;

	state.ball.radius = w * 0.025f;
	state.paddle.halfCircleRadius = w * 0.03f;
	state.paddle.capsuleHalfWidth = w * 0.1f;
	state.paddle.capsuleHalfHeight = w * 0.03f;

	state.halfFieldSize = glm::vec2(state.halfWorldSize.x - state.ball.radius * 2, state.halfWorldSize.y - state.ball.radius * 2);

	b2World *world;
	b2Body *body;

	state.world = world = new b2World(b2Vec2(0, 0));

	//
	// Field
	//
	b2BodyDef fieldDef = b2BodyDef();
	fieldDef.type = b2BodyType::b2_staticBody;
	fieldDef.position = b2Vec2(0, 0);
	fieldDef.angle = 0.0f;
	fieldDef.fixedRotation = true;
	fieldDef.linearDamping = 0;
	fieldDef.angularDamping = 0;
	state.field.body = body = world->CreateBody(&fieldDef);

	b2Vec2 fieldVertices[4] = {
		b2Vec2(w, h),
		b2Vec2(-w, h),
		b2Vec2(-w, -h),
		b2Vec2(w, -h),
	};
	b2ChainShape fieldShape = b2ChainShape();
	fieldShape.CreateLoop(fieldVertices, 4);

	b2FixtureDef fieldFixtureDef = b2FixtureDef();
	fieldFixtureDef.shape = &fieldShape;
	fieldFixtureDef.restitution = 1.0f;
	fieldFixtureDef.friction = 0.0f;
	body->CreateFixture(&fieldFixtureDef);

	//
	// Paddle
	// 
	b2BodyDef paddleDef = b2BodyDef();
	paddleDef.type = b2BodyType::b2_kinematicBody;
	paddleDef.allowSleep = false;
	paddleDef.bullet = true;
	paddleDef.position = b2Vec2(0, -h + state.paddle.capsuleHalfHeight + state.paddle.capsuleHalfHeight * 2.0f);
	paddleDef.angle = 0;
	paddleDef.fixedRotation = true;
	paddleDef.linearDamping = 0;
	paddleDef.angularDamping = 0;
	state.paddle.body = body = world->CreateBody(&paddleDef);

	b2PolygonShape capsuleShape = b2PolygonShape();
	capsuleShape.SetAsBox(state.paddle.capsuleHalfWidth, state.paddle.capsuleHalfHeight);

	b2FixtureDef paddleFixtureDef = b2FixtureDef();
	paddleFixtureDef.shape = &capsuleShape;
	paddleFixtureDef.restitution = 1.0f;
	paddleFixtureDef.friction = 0.0f;
	paddleFixtureDef.density = 1.0f;
	body->CreateFixture(&paddleFixtureDef);

	//
	// Ball
	//
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

	b2CircleShape ballShape = b2CircleShape();
	ballShape.m_radius = state.ball.radius;

	b2FixtureDef ballFixtureDef = b2FixtureDef();
	ballFixtureDef.shape = &ballShape;
	ballFixtureDef.restitution = 1.0f;
	ballFixtureDef.friction = 0.0f;
	ballFixtureDef.density = 1.0f;
	body->CreateFixture(&ballFixtureDef);

	float ballSpeed = 10.0f;
	body->SetLinearVelocity(ballSpeed * b2Vec2(1, -1));

	return true;
}

static void GameRelease(GameState &state) {
	std::vector<b2Body *> bodies;
	for(b2Body *body = state.world->GetBodyList(); body != nullptr; body = body->GetNext()) {
		bodies.push_back(body);
	}
	if(bodies.size() > 0) {
		for(size_t i = (int)bodies.size() - 1; i > 0; i--) {
			state.world->DestroyBody(bodies[i]);
		}
	}
	if(state.world != nullptr) {
		delete state.world;
		state.world = nullptr;
	}
	fglUnloadOpenGL();
}

extern void GameDestroy(GameState *state) {
	if(state != nullptr) {
		GameRelease(*state);
		state->~GameState();
		fplMemoryFree(state);
	}
}

extern GameState *GameCreate() {
	void *stateMem = fplMemoryAllocate(sizeof(GameState));
	if(stateMem == nullptr) {
		return nullptr;
	}
	GameState *state = new (stateMem) GameState;
	if(!GameInit(*state)) {
		GameDestroy(state);
	}
	return(state);
}

extern void GameUpdate(GameState &state, const Input &input) {
	glm::ivec2 viewSize = glm::ivec2(input.windowSize.x, (int)(input.windowSize.x / GameAspect));
	if(viewSize.y > input.windowSize.y) {
		viewSize.y = input.windowSize.y;
		viewSize.x = (int)(input.windowSize.y * GameAspect);
	}
	glm::ivec2 viewOffset = glm::ivec2((input.windowSize.x - viewSize.x) / 2, (input.windowSize.y - viewSize.y) / 2);
	state.viewSize = viewSize;
	state.viewOffset = viewOffset;

	state.world->ClearForces();

	float speed = 1.0f;
	const Controller &controller = input.controllers[0];
	if(controller.isConnected) {
		if(controller.moveLeft.isDown) {
			b2Vec2 pos = state.paddle.body->GetPosition();
			pos += speed * b2Vec2(-1, 0);
			state.paddle.body->SetTransform(pos, 0);
		} else if(controller.moveRight.isDown) {
			b2Vec2 pos = state.paddle.body->GetPosition();
			pos += speed * b2Vec2(1, 0);
			state.paddle.body->SetTransform(pos, 0);
		}
	}

	assert(state.ball.body->GetLinearVelocity().Length() >= b2_velocityThreshold);
	state.world->Step(input.deltaTime, 6, 2);
}

extern void GameDraw(GameState &state) {
	glm::f32 w = state.halfWorldSize.x;
	glm::f32 h = state.halfWorldSize.y;

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
	const float normalLen = w * 0.1f;
	b2Body *body = state.world->GetBodyList();
	while(body != nullptr) {
		b2Fixture *fixture = body->GetFixtureList();
		b2Vec2 bodyPos = body->GetPosition();
		glm::f32 bodyRot = body->GetAngle();
		glPushMatrix();
		glTranslatef(bodyPos.x, bodyPos.y, 0);
		glRotatef(glm::degrees<float>(bodyRot), 0, 0, 1);
		while(fixture != nullptr) {
			switch(fixture->GetType()) {
				case b2Shape::Type::e_circle:
				{
					b2CircleShape *circle = (b2CircleShape *)fixture->GetShape();
					glColor3f(0.0f, 0.0f, 1.0f);
					DrawCircle(circle->m_radius, false);
				} break;

				case b2Shape::Type::e_polygon:
				{
					b2PolygonShape *poly = (b2PolygonShape *)fixture->GetShape();

					for(int i = 0; i < poly->m_count; ++i) {
						b2Vec2 a = poly->m_vertices[i];
						b2Vec2 b = poly->m_vertices[(i + 1) % poly->m_count];
						b2Vec2 n = poly->m_normals[i];

						glColor3f(0.0f, 0.0f, 1.0f);
						glBegin(GL_LINES);
						glVertex2f(a.x, a.y);
						glVertex2f(b.x, b.y);
						glEnd();

						b2Vec2 na = a + 0.5f * (b - a);
						b2Vec2 nb = na + normalLen * n;
						glColor3f(0.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(na.x, na.y);
						glVertex2f(nb.x, nb.y);
						glEnd();
					}
				} break;

				case b2Shape::Type::e_chain:
				{
					b2ChainShape *chain = (b2ChainShape *)fixture->GetShape();
					for(int i = 0, c = chain->GetChildCount(); i < c; ++i) {
						b2EdgeShape edge;
						chain->GetChildEdge(&edge, i);
						b2Vec2 a = edge.m_vertex1;
						b2Vec2 b = edge.m_vertex2;
						b2Vec2 n = b2Cross(b - a, 1.0f);
						n.Normalize();

						glColor3f(1.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(a.x, a.y);
						glVertex2f(b.x, b.y);
						glEnd();

						b2Vec2 na = a + 0.5f * (b - a);
						b2Vec2 nb = na + normalLen * n;
						glColor3f(0.0f, 1.0f, 0.0f);
						glBegin(GL_LINES);
						glVertex2f(na.x, na.y);
						glVertex2f(nb.x, nb.y);
						glEnd();
					}
				} break;

			}
			fixture = fixture->GetNext();
		}
		glPopMatrix();
		body = body->GetNext();
	}
}