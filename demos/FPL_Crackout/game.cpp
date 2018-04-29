#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <Box2D/Box2D.cpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "game.h"

#define DRAW_NORMALS 0
#define DRAW_DEBUG 0

constexpr float Pi32 = glm::pi<float>();
constexpr float Tau32 = Pi32 * 2.0f;

constexpr int InitialLevelSeed = 1;

constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
const glm::vec2 WorldRadius = glm::vec2(WorldWidth, WorldHeight) * 0.5f;

constexpr float FrameRadius = WorldWidth * 0.025f;

constexpr float KillAreaExtent = WorldHeight * 0.5f;
constexpr float KillAreaDepth = WorldHeight * 0.25f;
constexpr float KillAreaOffset = WorldHeight * 0.1f;
constexpr float KillAreaTop = -(WorldHeight * 0.5f + KillAreaOffset);

constexpr float BallRadius = WorldWidth * 0.015f;
constexpr float BallDiameter = BallRadius * 2.0f;
constexpr float BallSpeed = 7.0f;

constexpr float AreaPadding = BallRadius * 2.0f;
const float BottomAreaDepth = WorldRadius.y * 0.25f;
const float AreaHalfWidth = WorldRadius.x - FrameRadius * 2.0f;
const float AreaHalfHeight = WorldRadius.y - FrameRadius * 0.5f - BottomAreaDepth;

constexpr float PaddleSpeed = 100.0f;
const glm::vec2 PaddleRadius = glm::vec2(BallRadius * 3, BallRadius);
const float PaddleLineY = -WorldRadius.y + PaddleRadius.y;
const float PaddleGlueOffsetY = PaddleRadius.y * 2 + BallRadius * 0.25f;

const float PaddleAspect = (PaddleRadius.x + BallRadius) / PaddleRadius.y;

constexpr float BrickSpacing = WorldWidth / 1000.0f;
constexpr int MaxBrickCols = 17;
constexpr int MaxBrickRows = 11;
const float SpaceForBricksX = ((AreaHalfWidth - AreaPadding) * 2.0f) - (MaxBrickCols - 1) * BrickSpacing;
const float SpaceForBricksY = ((AreaHalfHeight - AreaPadding) * 2.0f) - (MaxBrickRows - 1) * BrickSpacing;
const glm::vec2 BrickRadius = glm::vec2(SpaceForBricksX / (float)MaxBrickCols, SpaceForBricksY / (float)MaxBrickRows) * 0.5f;

const glm::vec2 Gravity = glm::vec2(0, -10);

// @NOTE(final): CPP is such garbage!
// It cannot handle array index initializer such as [index] = value :-(
// So we need this nonsense just to initialize a static array -.-
template <typename TEnumType, typename TValueType>
class ArrayInitializer {
protected:
	TValueType a[256];
public:
	ArrayInitializer() {
		fplMemoryClear(a, sizeof(TValueType) * FPL_ARRAYCOUNT(a));
	}
	const TValueType &operator [] (TEnumType eindex) const {
		return a[(int)eindex];
	}
	TValueType &operator [] (TEnumType eindex) {
		return a[(int)eindex];
	}
	void Set(TEnumType e, const TValueType &value) {
		a[(int)e] = value;
	}
};

// Brick UVs
enum class BrickType : int32_t {
	NoBrick = 0,
	Solid
};
const Vec2i BrickTileSize = MakeVec2i(30, 24);
const Vec2i BricksTilesetSize = MakeVec2i(34, 28);
const int BrickTilesetBorder = 2;
class BricksUVsClass : public ArrayInitializer<BrickType, UVRect> {
public:
	BricksUVsClass() {
		Set(BrickType::Solid, UVRectFromTile(BricksTilesetSize, BrickTileSize, BrickTilesetBorder, MakeVec2i(0, 0)));
	}
};
static BricksUVsClass BricksUVs = {};

// Background UVs
const Vec2i BackgroundTileSize = MakeVec2i(16, 16);
const Vec2i BackgroundTextureSize = MakeVec2i(38, 20);
enum class BackgroundType : int32_t {
	NoBackground = 0,
	Default,
};
class BackgroundUVsClass : public ArrayInitializer<BackgroundType, UVRect> {
public:
	BackgroundUVsClass() {
		Set(BackgroundType::Default, UVRectFromPos(BackgroundTextureSize, BackgroundTileSize, MakeVec2i(2, 2)));
	}
};
static BackgroundUVsClass BackgroundUVs = {};

// Frame UVs
enum class FrameType : int32_t {
	NoFrame = 0,

	TopLeftEdge, // Top left edge (16x16)
	TopFill, // Normal tile (8x16)
	TopMarks, // Marked tile (16x16)
	TopBegin, // Left (8x16)
	TopEnd, // Right (8x16)
	TopRightEdge, // Top right edge (16x16)

	LeftFill, // Normal tile (16x8)
	LeftStart, // Top (16x8)
	LeftMarks, // Marked tile (16x16)
	LeftEnd, // Bottom (16x8)
	LeftBottomEdge, // Left bottom edge (16x16)

	RightFill, // Normal tile (16x8)
	RightStart, // Top (16x8)
	RightMarks, // Marked tile (16x16)
	RightEnd, // Bottom (16x8)
	RightBottomEdge, // Right bottom edge (16x16)
};
const Vec2i FrameTopFillSize = MakeVec2i(8, 16);
const Vec2i FrameTopTileSize = MakeVec2i(16, 16);
const Vec2i FrameSideFillSize = MakeVec2i(16, 8);
const Vec2i FrameSideTileSize = MakeVec2i(16, 16);
const Vec2i FrameTextureSize = MakeVec2i(86, 86);
class FrameUVsClass : public ArrayInitializer<FrameType, UVRect> {
public:
	FrameUVsClass() {
		Set(FrameType::TopLeftEdge, UVRectFromPos(FrameTextureSize, FrameTopTileSize, MakeVec2i(2, 2)));
		Set(FrameType::TopFill, UVRectFromPos(FrameTextureSize, FrameTopFillSize, MakeVec2i(20, 2)));
		Set(FrameType::TopMarks, UVRectFromPos(FrameTextureSize, FrameTopFillSize, MakeVec2i(40, 2)));
		Set(FrameType::TopBegin, UVRectFromPos(FrameTextureSize, FrameTopFillSize, MakeVec2i(30, 2)));
		Set(FrameType::TopEnd, UVRectFromPos(FrameTextureSize, FrameTopFillSize, MakeVec2i(58, 2)));
		Set(FrameType::TopRightEdge, UVRectFromPos(FrameTextureSize, FrameTopTileSize, MakeVec2i(68, 2)));

		Set(FrameType::LeftFill, UVRectFromPos(FrameTextureSize, FrameSideFillSize, MakeVec2i(2, 20)));
		Set(FrameType::LeftStart, UVRectFromPos(FrameTextureSize, FrameSideFillSize, MakeVec2i(2, 30)));
		Set(FrameType::LeftMarks, UVRectFromPos(FrameTextureSize, FrameSideTileSize, MakeVec2i(2, 40)));
		Set(FrameType::LeftEnd, UVRectFromPos(FrameTextureSize, FrameSideFillSize, MakeVec2i(2, 58)));
		Set(FrameType::LeftBottomEdge, UVRectFromPos(FrameTextureSize, FrameSideTileSize, MakeVec2i(2, 68)));

		Set(FrameType::RightFill, UVRectFromPos(FrameTextureSize, FrameSideFillSize, MakeVec2i(68, 20)));
		Set(FrameType::RightStart, UVRectFromPos(FrameTextureSize, FrameSideFillSize, MakeVec2i(68, 30)));
		Set(FrameType::RightMarks, UVRectFromPos(FrameTextureSize, FrameSideTileSize, MakeVec2i(68, 40)));
		Set(FrameType::RightEnd, UVRectFromPos(FrameTextureSize, FrameSideFillSize, MakeVec2i(68, 58)));
		Set(FrameType::RightBottomEdge, UVRectFromPos(FrameTextureSize, FrameSideTileSize, MakeVec2i(68, 68)));
	}
};
static FrameUVsClass FrameUVs = {};

enum class EntityType : int32_t {
	NoEntity = 0,
	Ball,
	Paddle,
	Brick,
	Frame,
	KillArea,
};

struct Ball {
	b2Body *body;
	float speed;
	bool isMoving;
	bool isDead;
};

struct Paddle {
	b2Body *body;
	float speed;
	Ball *gluedBall;
};

struct Brick {
	b2Body *body;
	BrickType type;
	bool requestHit;
	bool isHit;
	Vec2f hitPoint;
	Vec2f hitNormal;
	bool isDead;
};

struct Frame {
	b2Body *top;
	b2Body *left;
	b2Body *right;
};

struct KillArea {
	b2Body *body;
};

struct Entity {
	EntityType type;
	union {
		Ball ball;
		Paddle paddle;
		Brick brick;
		Frame frame;
		KillArea killArea;
	};
};

// @NOTE(final): Such a bad design decision from Box2D to force classes on us
class GameContactListener : public b2ContactListener {
private:
	GameState * gameState;
public:
	GameContactListener(GameState *gameState) {
		this->gameState = gameState;
	}
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};

struct Assets {
	GLuint ballTexture;
	GLuint bricksTexture;
	GLuint paddleTexture;
	GLuint frameTexture;
	ArrayInitializer<BackgroundType, GLuint> bgTextures;
};

struct GameState {
	char dataPath[1024];
	Assets assets;

	Vec2i viewSize;
	Vec2i viewOffset;

	b2World *world;

	Entity frame;
	Entity ball;
	Entity paddle;
	Entity killArea;
	BrickType bricksMap[1024];
	Entity activeBricks[1024];
	size_t numActiveBricks;

	GameContactListener *contactListener;

	int levelSeed;

	bool isExiting;
};

static void SetRandomLevel(GameState &state, int seed);
static void GlueBallOnPaddle(GameState &state, Ball *ball);

inline float Random01() {
	float result = rand() / (float)RAND_MAX;
	return(result);
}

inline int RandomInt(int size) {
	int result = rand() % size;
	return(result);
}

static void DrawCircleVertices(const float r, const int segments) {
	float alpha = Tau32 / (float)segments;
	for (int i = 0; i <= segments; ++i) {
		float a = i * alpha;
		float c = glm::cos(a) * r;
		float s = glm::sin(a) * r;
		glVertex2f(c, s);
	}
}
static void DrawCircle(float r, bool isFilled, const int segments = 24) {
	glBegin(isFilled ? GL_POLYGON : GL_LINE_STRIP);
	DrawCircleVertices(r, segments);
	glEnd();
}

static void DrawNormal(const glm::vec2 &p, const glm::vec2 &n, float d) {
	glBegin(GL_LINES);
	glVertex2f(p.x, p.y);
	glVertex2f(p.x + n.x * d, p.y + n.y * d);
	glEnd();
}

static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const bool repeatable) {
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);

	return(handle);
}

static void ClearWorld(b2World *world) {
	b2Body *body = world->GetBodyList();
	while (body != nullptr) {
		b2Body *next = body->GetNext();
		world->DestroyBody(body);
		body = next;
	}
}

static void LoadLevel(GameState &state, int levelSeed) {
	SetRandomLevel(state, levelSeed);

	b2World *world = state.world;
	const float hw = WorldRadius.x;
	const float hh = WorldRadius.y;

	//
	// Clear world
	//
	ClearWorld(world);

	//
	// Field
	//
	{
		b2BodyDef bodyDef = b2BodyDef();
		bodyDef.type = b2BodyType::b2_staticBody;
		bodyDef.angle = 0.0f;
		bodyDef.fixedRotation = true;

		b2FixtureDef fixtureDef = b2FixtureDef();
		fixtureDef.restitution = 1.0f;
		fixtureDef.friction = 0.0f;
		fixtureDef.density = 1.0f;

		b2Body *body;

		b2PolygonShape sideShape = b2PolygonShape();
		sideShape.SetAsBox(FrameRadius, hh + KillAreaExtent);
		b2PolygonShape topShape = b2PolygonShape();
		topShape.SetAsBox(hw, FrameRadius);

		Entity *frameEntity = &state.frame;
		*frameEntity = {};
		frameEntity->type = EntityType::Frame;
		Frame &frame = frameEntity->frame;

		// Right
		bodyDef.position = b2Vec2(hw - FrameRadius, -KillAreaExtent);
		frame.right = body = world->CreateBody(&bodyDef);
		body->SetUserData(frameEntity);
		fixtureDef.shape = &sideShape;
		body->CreateFixture(&fixtureDef);

		// Top
		bodyDef.position = b2Vec2(0, hh - FrameRadius);
		frame.top = body = world->CreateBody(&bodyDef);
		body->SetUserData(frameEntity);
		fixtureDef.shape = &topShape;
		body->CreateFixture(&fixtureDef);

		// Left
		bodyDef.position = b2Vec2(-hw + FrameRadius, -KillAreaExtent);
		frame.left = body = world->CreateBody(&bodyDef);
		body->SetUserData(frameEntity);
		fixtureDef.shape = &sideShape;
		body->CreateFixture(&fixtureDef);
	}

	//
	// Kill area
	//
	{
		Entity *killAreaEntity = &state.killArea;
		*killAreaEntity = {};
		killAreaEntity->type = EntityType::KillArea;
		KillArea &killArea = killAreaEntity->killArea;

		b2Body *body;
		b2BodyDef bodyDef = b2BodyDef();
		bodyDef.type = b2BodyType::b2_staticBody;
		bodyDef.position = b2Vec2(0, KillAreaTop - KillAreaDepth * 0.5f);
		bodyDef.angle = 0.0f;
		bodyDef.fixedRotation = true;
		bodyDef.linearDamping = 0;
		bodyDef.angularDamping = 0;
		killArea.body = body = world->CreateBody(&bodyDef);
		body->SetUserData(killAreaEntity);

		b2PolygonShape killShape = b2PolygonShape();
		killShape.SetAsBox(hw, KillAreaDepth * 0.5f);

		b2FixtureDef fixtureDef = b2FixtureDef();
		fixtureDef.shape = &killShape;
		fixtureDef.density = 0;
		fixtureDef.restitution = 0;
		fixtureDef.friction = 1;
		fixtureDef.filter.maskBits = 0xFFFF;
		body->CreateFixture(&fixtureDef);
	}

	//
	// Bricks
	//
	{
		b2Body *body;
		float halfWidth = (MaxBrickCols * BrickRadius.x) - ((MaxBrickCols - 1) * BrickSpacing * 0.5f);
		float brickY = WorldRadius.y - FrameRadius * 2.0f - AreaPadding - BrickRadius.y;
		state.numActiveBricks = 0;
		for (int row = 0; row < MaxBrickRows; ++row) {
			//float brickX = -WorldRadius.x + SideBorderRadius * 2.0f - AreaPadding + BrickRadius.x;
			float brickX = -WorldRadius.x + FrameRadius * 2.0f + AreaPadding + BrickRadius.x;
			for (int col = 0; col < MaxBrickCols; ++col) {
				BrickType brickType = state.bricksMap[row * MaxBrickCols + col];
				if (brickType == BrickType::Solid) {
					Entity *brickEntity = &state.activeBricks[state.numActiveBricks++];
					*brickEntity = {};
					brickEntity->type = EntityType::Brick;

					Brick &brick = brickEntity->brick;
					brick.type = brickType;

					b2Vec2 brickPos = b2Vec2(brickX, brickY);
					b2BodyDef bodyDef = b2BodyDef();
					bodyDef.type = b2BodyType::b2_staticBody;
					bodyDef.position = brickPos;
					bodyDef.angle = 0.0f;
					bodyDef.linearDamping = 0;
					bodyDef.angularDamping = 0;
                    bodyDef.bullet = true;
					brick.body = body = world->CreateBody(&bodyDef);
					body->SetUserData(brickEntity);

					b2PolygonShape brickShape = b2PolygonShape();
					brickShape.SetAsBox(BrickRadius.x, BrickRadius.y);

					b2FixtureDef fixtureDef = b2FixtureDef();
					fixtureDef.shape = &brickShape;
					fixtureDef.restitution = 0.5f;
					fixtureDef.friction = 0.1f;
					fixtureDef.density = 1.0f;
					fixtureDef.filter.maskBits = 0xFFFF;
					body->CreateFixture(&fixtureDef);
				}
				brickX += BrickRadius.x * 2.0f + BrickSpacing;
			}
			brickY -= (BrickRadius.y * 2.0f + BrickSpacing);
		}
	}

	//
	// Paddle
	// 
	{
		b2Body *body;
		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;

		// Limiter
		bodyDef = b2BodyDef();
		bodyDef.type = b2BodyType::b2_staticBody;
		bodyDef.position = b2Vec2(0, PaddleLineY);
		b2Body *paddleLimiterBody = body = world->CreateBody(&bodyDef);
		b2PolygonShape limiterShape = b2PolygonShape();
		limiterShape.SetAsBox(PaddleRadius.x, PaddleRadius.y);
		fixtureDef = b2FixtureDef();
		fixtureDef.shape = &limiterShape;
		fixtureDef.restitution = 0.0f;
		fixtureDef.friction = 1.0f;
		fixtureDef.density = 1.0f;
		fixtureDef.filter.maskBits = 0x0000;
		body->CreateFixture(&fixtureDef);
		body->SetUserData(nullptr);

		// Paddle
		Entity *paddleEntity = &state.paddle;
		*paddleEntity = {};
		paddleEntity->type = EntityType::Paddle;

		Paddle &paddle = paddleEntity->paddle;
		paddle.speed = PaddleSpeed;

		bodyDef = b2BodyDef();
		bodyDef.type = b2BodyType::b2_dynamicBody;
		bodyDef.allowSleep = false;
		bodyDef.bullet = true;
		bodyDef.position = b2Vec2(0, PaddleLineY);
		bodyDef.angle = 0;
		bodyDef.fixedRotation = true;
		bodyDef.linearDamping = 14.0f;
		bodyDef.angularDamping = 0;
		bodyDef.gravityScale = 0;
		paddle.body = body = world->CreateBody(&bodyDef);
		body->SetUserData(paddleEntity);

		fixtureDef = b2FixtureDef();
		fixtureDef.restitution = 0.0f;
		fixtureDef.friction = 0.0f;
		fixtureDef.density = 20.0f;
		fixtureDef.filter.maskBits = 0xFFFF;

		b2PolygonShape capsuleShape = b2PolygonShape();
		capsuleShape.SetAsBox(PaddleRadius.x, PaddleRadius.y);
		fixtureDef.shape = &capsuleShape;
		body->CreateFixture(&fixtureDef);

		b2CircleShape leftShape = b2CircleShape();
		leftShape.m_radius = PaddleRadius.y;
		leftShape.m_p = b2Vec2(-PaddleRadius.x, 0);
		fixtureDef.shape = &leftShape;
		body->CreateFixture(&fixtureDef);

		b2CircleShape rightShape = b2CircleShape();
		rightShape.m_radius = PaddleRadius.y;
		rightShape.m_p = b2Vec2(PaddleRadius.x, 0);
		fixtureDef.shape = &rightShape;
		body->CreateFixture(&fixtureDef);

		// Paddle join (Restrict to X-Axis)
		b2PrismaticJointDef jointDef = b2PrismaticJointDef();
		b2Vec2 limiterAxis = b2Vec2(1, 0);
		jointDef.collideConnected = true;
		jointDef.Initialize(body, paddleLimiterBody, paddle.body->GetWorldCenter(), limiterAxis);
		world->CreateJoint(&jointDef);
	}

	//
	// Ball
	//
	{
		b2Body *body;
		b2BodyDef ballDef = b2BodyDef();

		Entity *ballEntity = &state.ball;
		*ballEntity = {};
		ballEntity->type = EntityType::Ball;

		Ball &ball = ballEntity->ball;
		ball.speed = BallSpeed;

		ballDef.type = b2BodyType::b2_dynamicBody;
		ballDef.allowSleep = false;
		ballDef.bullet = true;
		ballDef.position = b2Vec2(0, 0);
		ballDef.angle = 0;
		ballDef.fixedRotation = true;
		ballDef.linearDamping = 0;
		ballDef.angularDamping = 0;
		ballDef.gravityScale = 0;
		ball.body = body = world->CreateBody(&ballDef);
		body->SetUserData(ballEntity);

		b2CircleShape ballShape = b2CircleShape();
		ballShape.m_radius = BallRadius;

		b2FixtureDef ballFixtureDef = b2FixtureDef();
		ballFixtureDef.shape = &ballShape;
		ballFixtureDef.restitution = 1.0f;
		ballFixtureDef.friction = 0.0f;
		ballFixtureDef.density = 1.0f;
		ballFixtureDef.filter.maskBits = 0xFFFF;
		body->CreateFixture(&ballFixtureDef);
	}

	GlueBallOnPaddle(state, &state.ball.ball);
}

struct TextureImage {
	int width;
	int height;
	int components;
	uint8_t *data;
};

static void FreeTextureImage(TextureImage &img) {
	stbi_image_free(img.data);
	img = {};
}

static TextureImage CreateImageTile(TextureImage &source, int x, int y, int w, int h) {
	FPL_ASSERT(source.components == 4);
	TextureImage result = {};
	result.width = w;
	result.height = h;
	result.components = 4;
	result.data = (uint8_t *)STBI_MALLOC(w * h * 4);
	int sourceScanline = source.width * 4;
	int destScanline = w * 4;
	int dstY = 0;
	for (int srcY = y; srcY < (y + h); ++srcY) {
		int dstX = 0;
		for (int srcX = x; srcX < (x + w); ++srcX) {
			uint8_t *src = source.data + (srcY * sourceScanline + srcX * 4);
			uint8_t *dst = result.data + (dstY * destScanline + dstX * 4);
			uint32_t *srcPixel = (uint32_t *)src;
			uint32_t *dstPixel = (uint32_t *)dst;
			*dstPixel = *srcPixel;
			++dstX;
		}
		++dstY;
	}
	return(result);
}

static TextureImage LoadTextureImage(const char *dataPath, const char *filename) {
	TextureImage result = {};

	char filePath[1024];
	fplCopyAnsiString(dataPath, filePath, FPL_ARRAYCOUNT(filePath));
	fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);

	fplFileHandle file;
	if (fplOpenAnsiBinaryFile(filePath, &file)) {
		uint32_t fileLen = fplGetFileSizeFromHandle32(&file);
		uint8_t *fileBuffer = (uint8_t *)fplMemoryAllocate(fileLen);
		if (fileBuffer != nullptr) {
			if (fplReadFileBlock32(&file, fileLen, fileBuffer, fileLen) == fileLen) {
				int imageWidth = 0;
				int imageHeight = 0;
				int imageComponents = 0;
				stbi_set_flip_vertically_on_load(0);
				stbi_uc *imageData = stbi_load_from_memory(fileBuffer, fileLen, &imageWidth, &imageHeight, &imageComponents, 4);
				if (imageData != nullptr) {
					result.width = imageWidth;
					result.height = imageHeight;
					result.components = imageComponents;
					result.data = imageData;
				} else {
					fplConsoleFormatError("Image file '%s' of size '%lu' is broken!\n", filePath, fileLen);
				}
			} else {
				fplConsoleFormatError("Failed reading file of size '%lu'!\n", fileLen);
			}
			fplMemoryFree(fileBuffer);
		} else {
			fplConsoleFormatError("Failed allocating memory of size '%lu'!\n", fileLen);
		}
		fplCloseFile(&file);
	} else {
		fplConsoleFormatError("Image file '%s' could not be found!\n", filePath);
	}
	return(result);
}

static bool LoadTexture(const TextureImage &image, const bool repeatable, GLuint &outTexture) {
	bool result = false;
	GLuint texId = AllocateTexture(image.width, image.height, image.data, repeatable);
	if (texId > 0) {
		outTexture = texId;
		result = true;
	}
	return(result);
}

static bool LoadTexture(const char *dataPath, const char *filename, const bool repeatable, GLuint &outTexture) {
	TextureImage image = LoadTextureImage(dataPath, filename);
	if (image.data == nullptr) {
		return false;
	}
	bool result = LoadTexture(image, repeatable, outTexture);
	FreeTextureImage(image);
	return(result);
}

static bool LoadAssets(GameState &state) {
	LoadTexture(state.dataPath, "ball.bmp", false, state.assets.ballTexture);
	LoadTexture(state.dataPath, "bricks.bmp", false, state.assets.bricksTexture);
	LoadTexture(state.dataPath, "paddle.bmp", false, state.assets.paddleTexture);
	LoadTexture(state.dataPath, "frame.bmp", false, state.assets.frameTexture);

	TextureImage bgImage = LoadTextureImage(state.dataPath, "bg.bmp");
	if (bgImage.data != nullptr) {
        TextureImage bgTileImage0 = CreateImageTile(bgImage, 2, 2, 16, 16);
        LoadTexture(bgTileImage0, true, state.assets.bgTextures[BackgroundType::Default]);
        FreeTextureImage(bgTileImage0);
        FreeTextureImage(bgImage);
    }

	return true;
}

static bool GameInit(GameState &state) {
	if (!fglLoadOpenGL(true)) {
		return false;
	}

	fplConsoleFormatOut("%f\n", PaddleAspect);

	fplGetExecutableFilePath(state.dataPath, FPL_ARRAYCOUNT(state.dataPath));
	fplExtractFilePath(state.dataPath, state.dataPath, FPL_ARRAYCOUNT(state.dataPath));
	fplPathCombine(state.dataPath, FPL_ARRAYCOUNT(state.dataPath), 2, state.dataPath, "data");

	srand((int)fplGetTimeInMillisecondsLP());

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glLineWidth(1.0f);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	LoadAssets(state);

	b2World *world;

	state.world = world = new b2World(b2Vec2(Gravity.x, Gravity.y));
	world->SetContinuousPhysics(true);

	state.contactListener = new GameContactListener(&state);
	world->SetContactListener(state.contactListener);

	LoadLevel(state, InitialLevelSeed);

	return true;
}

static void GameRelease(GameState &state) {
	ClearWorld(state.world);
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
	GameState *state = (GameState *)stateMem;
	if (!GameInit(*state)) {
		GameDestroy(state);
		state = nullptr;
	}
	return(state);
}

static void GlueBallOnPaddle(GameState &state, Ball *ball) {
	Paddle &paddle = state.paddle.paddle;
	FPL_ASSERT(paddle.gluedBall == nullptr);
	ball->isMoving = false;
	paddle.gluedBall = ball;
	ball->body->SetType(b2BodyType::b2_staticBody);
}

static void LaunchBall(GameState &state) {
	Paddle &paddle = state.paddle.paddle;
	const float spreadAngle = 30.0f;
	const float startAngle = 90;
	Ball *ball = paddle.gluedBall;
	ball->isMoving = true;
	ball->isDead = false;
	ball->body->SetType(b2BodyType::b2_dynamicBody);
	float newAngle = startAngle + (Random01() > 0.5f ? -1 : 1) * Random01() * spreadAngle;
	float a = glm::radians<float>(newAngle);
	b2Vec2 direction = b2Vec2(glm::cos(a), glm::sin(a));
	ball->body->ApplyLinearImpulse(ball->speed * direction, ball->body->GetPosition(), true);
	paddle.gluedBall = nullptr;
}

static void SetRandomLevel(GameState &state, int seed) {
#define ALL_BRICKS 0
	FPL_STATICASSERT(MaxBrickCols % 2 != 0);
	FPL_ASSERT((MaxBrickCols * MaxBrickRows) <= FPL_ARRAYCOUNT(state.bricksMap));
	fplMemoryClear(state.bricksMap, sizeof(Brick) * FPL_ARRAYCOUNT(state.bricksMap));
	srand(seed);
	state.levelSeed = seed;
	int halfColCount = (MaxBrickCols - 1) / 2;
	bool reverse = RandomInt(100) > 25;
	for (int row = 0; row < MaxBrickRows; ++row) {
#if ALL_BRICKS
		int randomColCount = halfColCount;
#else
		int randomColCount = RandomInt(halfColCount);
#endif
		for (int col = 0; col < randomColCount; ++col) {
			int c = reverse ? (halfColCount - 1 - col) : col;
			int leftCol = c;
			int rightCol = (MaxBrickCols - 1) - c;
			state.bricksMap[row * MaxBrickCols + leftCol] = BrickType::Solid;
			state.bricksMap[row * MaxBrickCols + rightCol] = BrickType::Solid;
		}
#if ALL_BRICKS
		state.bricksMap[row * MaxBrickCols + halfColCount] = BrickType::Solid;
#else
		if (Random01() > 0.5f) {
			state.bricksMap[row * MaxBrickCols + halfColCount] = BrickType::Solid;
		}
#endif
	}

#undef ALL_BRICKS
}

static void EntersKillArea(GameState &state, Entity &other) {
	if (other.type == EntityType::Ball) {
		Ball *ball = &other.ball;
		Paddle &paddle = state.paddle.paddle;
		ball->isDead = true;
	} else if (other.type == EntityType::Brick) {
		Brick &brick = other.brick;
		brick.isDead = true;
	}
}

static void HandleBallCollision(GameState &state, Ball &ball, Entity &other, b2Contact &contact) {
	if (other.type == EntityType::Brick) {
		Brick &brick = other.brick;
		if (!brick.requestHit && !brick.isDead) {
			// @TODO(final): Score
			// @TODO(final): Save contact info so we can add an impulse to "simulate" hit-by-ball
			brick.requestHit = true;
			b2WorldManifold manifold;
			contact.GetWorldManifold(&manifold);
			brick.hitPoint = MakeVec2f(manifold.points[0].x, manifold.points[0].y);
			brick.hitNormal = MakeVec2f(manifold.normal.x, manifold.normal.y);
		}
	}
}

struct CollisionPair {
	b2Fixture *fixtureA;
	b2Fixture *fixtureB;
	b2Body *bodyA;
	b2Body *bodyB;
	Entity *entityA;
	Entity *entityB;
};

static CollisionPair GetCollisionPair(b2Contact* contact) {
	CollisionPair result = {};
	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	FPL_ASSERT(fixtureA != nullptr && fixtureB != nullptr);
	b2Body *bodyA = fixtureA->GetBody();
	b2Body *bodyB = fixtureB->GetBody();
	FPL_ASSERT(bodyA != nullptr && bodyB != nullptr);
	void *dataA = bodyA->GetUserData();
	void *dataB = bodyB->GetUserData();
	if (dataA != nullptr && dataB != nullptr) {
		Entity *entityA = (Entity *)dataA;
		Entity *entityB = (Entity *)dataB;

		// Sort entity by type
		if (entityA->type > entityB->type) {
			entityB = (Entity *)dataA;
			entityA = (Entity *)dataB;
			fixtureB = contact->GetFixtureA();
			fixtureA = contact->GetFixtureB();
			bodyB = fixtureB->GetBody();
			bodyA = fixtureA->GetBody();
		}
		result.fixtureA = fixtureA;
		result.fixtureB = fixtureB;
		result.bodyA = bodyA;
		result.bodyB = bodyB;
		result.entityA = entityA;
		result.entityB = entityB;
	}
	return(result);
}

static void HandleContactCollision(GameState &state, b2Contact *contact) {
	CollisionPair pair = GetCollisionPair(contact);
	if (pair.entityA->type == EntityType::Ball) {
		HandleBallCollision(state, pair.entityA->ball, *pair.entityB, *contact);
	}
}

static void HandlePreCollision(GameState &state, b2Contact* contact) {
	CollisionPair pair = GetCollisionPair(contact);
	if (pair.entityA->type == EntityType::KillArea) {
		EntersKillArea(state, *pair.entityB);
	} else if (pair.entityB->type == EntityType::KillArea) {
		EntersKillArea(state, *pair.entityA);
	}
}

// @NOTE(final): These are bad design decision from Box2D!
void GameContactListener::BeginContact(b2Contact* contact) {
	HandleContactCollision(*gameState, contact);
}
void GameContactListener::EndContact(b2Contact* contact) {
}
void GameContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
	HandlePreCollision(*gameState, contact);
}
void GameContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
}

extern bool IsGameExiting(GameState &state) {
	return state.isExiting;
}

extern void GameInput(GameState &state, const Input &input, bool isActive) {
	if (!isActive) {
		return;
	}

	// Single player input
	Paddle &paddle = state.paddle.paddle;
	if (input.defaultControllerIndex != -1) {
		FPL_ASSERT(input.defaultControllerIndex < FPL_ARRAYCOUNT(input.controllers));
		const Controller &controller = input.controllers[input.defaultControllerIndex];
		if (controller.isConnected) {
			if (controller.moveLeft.isDown) {
				paddle.body->ApplyLinearImpulse(paddle.speed * b2Vec2(-1, 0), paddle.body->GetPosition(), true);
			} else if (controller.moveRight.isDown) {
				paddle.body->ApplyLinearImpulse(paddle.speed * b2Vec2(1, 0), paddle.body->GetPosition(), true);
			}
			if (WasPressed(controller.actionDown) && paddle.gluedBall != nullptr) {
				LaunchBall(state);
			}
			if (WasPressed(controller.editorToggle)) {
				LoadLevel(state, state.levelSeed + 1);
			}
		}
	}
}

extern void GameUpdate(GameState &state, const Input &input, bool isActive) {
	if (!isActive) {
		return;
	}

	// Compute viewport
	Vec2i viewSize = MakeVec2i(input.windowSize.x, (int)(input.windowSize.x / GameAspect));
	if (viewSize.y > input.windowSize.y) {
		viewSize.y = input.windowSize.y;
		viewSize.x = (int)(input.windowSize.y * GameAspect);
	}
	Vec2i viewOffset = MakeVec2i((input.windowSize.x - viewSize.x) / 2, (input.windowSize.y - viewSize.y) / 2);
	state.viewSize = viewSize;
	state.viewOffset = viewOffset;

	// Move glued ball
	Paddle &paddle = state.paddle.paddle;
	if (paddle.gluedBall == nullptr && state.ball.ball.isDead) {
		GlueBallOnPaddle(state, &state.ball.ball);
	}
	if (paddle.gluedBall != nullptr) {
		Ball *ball = paddle.gluedBall;
		b2Vec2 gluePos = paddle.body->GetPosition() + b2Vec2(0, PaddleGlueOffsetY);
		ball->body->SetTransform(gluePos, 0);
	} else {
	}

	// Correct ball angle when too squared
	{
		const float angleTolerance = 2.5f;
		const float angleCorrection = 15.0f;
		float squaredAngles[] = { 0, 90, 180, 270, 360 };
		Ball &ball = state.ball.ball;
		if (ball.isMoving) {
			b2Vec2 vel = ball.body->GetLinearVelocity();
			b2Vec2 dir = vel;
			dir.Normalize();
			float a = glm::atan<float>(dir.y, dir.x);
			float deg = glm::degrees<float>(a);
			for (int i = 0; i < FPL_ARRAYCOUNT(squaredAngles); ++i) {
				if (glm::abs<float>(deg) > (squaredAngles[i] - angleTolerance) && glm::abs<float>(deg) < (squaredAngles[i] + angleTolerance)) {
					deg += (glm::abs(deg) - squaredAngles[i] > 0 ? 1 : -1) * angleCorrection;
					a = glm::radians<float>(deg);
				}
			}
			dir = b2Vec2(glm::cos(a), glm::sin(a));
			dir *= ball.speed;
			ball.body->SetLinearVelocity(dir);
		}
	}

	// Make all bricks dynamic when hit
	const float hitStrength = 0.5f;
	for (size_t i = 0; i < state.numActiveBricks; ++i) {
		Entity &brickEntity = state.activeBricks[i];
		Brick &brick = brickEntity.brick;
		if (brick.requestHit && !brick.isDead && !brick.isHit) {
			brick.body->SetType(b2BodyType::b2_dynamicBody);
			b2Vec2 impulse = hitStrength * -b2Vec2(brick.hitNormal.x, brick.hitNormal.y);
			b2Vec2 point = b2Vec2(brick.hitPoint.x, brick.hitPoint.y);
			brick.body->ApplyLinearImpulse(impulse, point, true);
			brick.isHit = true;
		}
	}

	// Remove dead bricks
	if (state.numActiveBricks > 0) {
		for (size_t i = 0, c = state.numActiveBricks; i < c; ++i) {
			if (state.activeBricks[i].brick.isDead) {
				Entity temp = state.activeBricks[i];
				if (temp.brick.body != nullptr) {
					state.world->DestroyBody(temp.brick.body);
				}
				temp = {};
				if (i < state.numActiveBricks - 1) {
					state.activeBricks[i] = state.activeBricks[state.numActiveBricks - 1];
					state.activeBricks[state.numActiveBricks - 1] = temp;
					state.activeBricks[i].brick.body->SetUserData(&state.activeBricks[i]);
				}
				--state.numActiveBricks;
			}
		}
		if (state.numActiveBricks == 0) {
			// Level done
			LoadLevel(state, state.levelSeed + 1);
		}
	}

	// Run physics simulation
	state.world->Step(input.deltaTime, 10, 10);
	state.world->ClearForces();
}

static void DrawSprite(const GLuint texId, const float rx, const float ry, const float uMin = 0.0f, const float vMin = 0.0f, const float uMax = 1.0f, const float vMax = 1.0f, const float xoffset = 0, const float yoffset = 0) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glTexCoord2f(uMax, vMax); glVertex2f(xoffset + rx, yoffset + ry);
	glTexCoord2f(uMin, vMax); glVertex2f(xoffset + -rx, yoffset + ry);
	glTexCoord2f(uMin, vMin); glVertex2f(xoffset + -rx, yoffset + -ry);
	glTexCoord2f(uMax, vMin); glVertex2f(xoffset + rx, yoffset + -ry);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

static void DrawSprite(const GLuint texId, float rx, float ry, const UVRect &uv, const float xoffset = 0, const float yoffset = 0) {
	DrawSprite(texId, rx, ry, uv.uMin, uv.vMax, uv.uMax, uv.vMin, xoffset, yoffset);
}

extern void GameDraw(GameState &state) {
	const float w = WorldRadius.x;
	const float h = WorldRadius.y;

	Vec2i viewSize = state.viewSize;
	Vec2i viewOffset = state.viewOffset;
	glViewport(viewOffset.x, viewOffset.y, viewSize.x, viewSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float scale = 1.0f;
	glOrtho(-w * scale, w*scale, -h * scale, h*scale, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Increase radius a tiny bit to match collision shape
	const float ROffset = WorldWidth / 1000.0f;

	// Background
	{
		GLuint bgTex = state.assets.bgTextures[BackgroundType::Default];
		float uMax = (float)(int)(WorldRadius.x / FrameRadius);
		float vMax = (float)(int)(WorldRadius.y / FrameRadius);
		UVRect bgUV = BackgroundUVs[BackgroundType::Default];
		DrawSprite(bgTex, WorldRadius.x - FrameRadius * 2, WorldRadius.y - FrameRadius, 0.0f, vMax, uMax, 0.0f, 0, -FrameRadius);
	}

	// Frame
	{
		GLuint frameTex = state.assets.frameTexture;
		UVRect topLeftEdgeUV = FrameUVs[FrameType::TopLeftEdge];
		UVRect topRightEdgeUV = FrameUVs[FrameType::TopRightEdge];
		UVRect topFillUV = FrameUVs[FrameType::TopFill];
		UVRect bottomLeftEdgeUV = FrameUVs[FrameType::LeftBottomEdge];
		UVRect bottomRightEdgeUV = FrameUVs[FrameType::RightBottomEdge];
		UVRect leftFillUV = FrameUVs[FrameType::LeftFill];
		UVRect rightFillUV = FrameUVs[FrameType::RightFill];

		// Top
		DrawSprite(frameTex, FrameRadius, FrameRadius, topLeftEdgeUV, -WorldRadius.x + FrameRadius, WorldRadius.y - FrameRadius);
		DrawSprite(frameTex, WorldRadius.x - FrameRadius * 2, FrameRadius, topFillUV, 0, WorldRadius.y - FrameRadius);
		DrawSprite(frameTex, FrameRadius, FrameRadius, topRightEdgeUV, WorldRadius.x - FrameRadius, WorldRadius.y - FrameRadius);

		// Left
		DrawSprite(frameTex, FrameRadius, FrameRadius, bottomLeftEdgeUV, -WorldRadius.x + FrameRadius, -WorldRadius.y + FrameRadius);
		DrawSprite(frameTex, FrameRadius, WorldRadius.y - FrameRadius * 2, leftFillUV, -WorldRadius.x + FrameRadius, 0);

		// Right
		DrawSprite(frameTex, FrameRadius, FrameRadius, bottomRightEdgeUV, WorldRadius.x - FrameRadius, -WorldRadius.y + FrameRadius);
		DrawSprite(frameTex, FrameRadius, WorldRadius.y - FrameRadius * 2, rightFillUV, WorldRadius.x - FrameRadius, 0);
	}

	// Ball
	{
		const Ball &ball = state.ball.ball;
		b2Vec2 ballPos = ball.body->GetPosition();
		float ballRot = ball.body->GetAngle();
		glPushMatrix();
		glTranslatef(ballPos.x, ballPos.y, 0);
		glRotatef(glm::degrees<float>(ballRot), 0, 0, 1);
		glColor4f(1, 1, 1, 1);
		DrawSprite(state.assets.ballTexture, BallRadius + ROffset, BallRadius + ROffset, 0.0f, 1.0f, 1.0f, 0.0f);
		glPopMatrix();
	}

	// Paddle
	{
		const Paddle &paddle = state.paddle.paddle;
		b2Vec2 paddlePos = paddle.body->GetPosition();
		float paddleRot = paddle.body->GetAngle();
		glPushMatrix();
		glTranslatef(paddlePos.x, paddlePos.y, 0);
		glRotatef(glm::degrees<float>(paddleRot), 0, 0, 1);
		glColor4f(1, 1, 1, 1);
		DrawSprite(state.assets.paddleTexture, PaddleRadius.x + BallRadius + ROffset, PaddleRadius.y + ROffset, 0.0f, 1.0f, 1.0f, 0.0f);
		glPopMatrix();
	}

	// Bricks
	for (size_t i = 0; i < state.numActiveBricks; ++i) {
		const Brick &brick = state.activeBricks[i].brick;
		b2Vec2 brickPos = brick.body->GetPosition();
		float brickRot = brick.body->GetAngle();
		UVRect brickUV = BricksUVs[brick.type];
		glPushMatrix();
		glTranslatef(brickPos.x, brickPos.y, 0);
		glRotatef(glm::degrees<float>(brickRot), 0, 0, 1);
		glColor4f(1, 1, 1, 1);
		DrawSprite(state.assets.bricksTexture, BrickRadius.x, BrickRadius.y, brickUV);
		glPopMatrix();
	}

#if DRAW_DEBUG
	// Bodies
	const float normalLen = w * 0.025f;
	b2Body *body = state.world->GetBodyList();
	while (body != nullptr) {
		if (body->GetUserData() == nullptr) {
			body = body->GetNext();
			continue;
		}
		Entity *entity = (Entity *)body->GetUserData();
		b2Fixture *fixture = body->GetFixtureList();
		b2Vec2 bodyPos = body->GetPosition();
		float bodyRot = body->GetAngle();
		while (fixture != nullptr) {
			switch (fixture->GetType()) {
				case b2Shape::Type::e_circle:
				{
					b2CircleShape *circle = (b2CircleShape *)fixture->GetShape();
					glColor4f(1, 1, 1, 1);
					glPushMatrix();
					glTranslatef(bodyPos.x + circle->m_p.x, bodyPos.y + circle->m_p.y, 0);
					glRotatef(glm::degrees<float>(bodyRot), 0, 0, 1);
					DrawCircle(circle->m_radius, false);
					glPopMatrix();
				} break;

				case b2Shape::Type::e_polygon:
				{
					b2PolygonShape *poly = (b2PolygonShape *)fixture->GetShape();
					glPushMatrix();
					glTranslatef(bodyPos.x, bodyPos.y, 0);
					glRotatef(glm::degrees<float>(bodyRot), 0, 0, 1);

					glColor4f(1, 1, 1, 1);
					glBegin(GL_LINE_LOOP);
					for (int i = 0; i < poly->m_count; ++i) {
						b2Vec2 v = poly->m_vertices[i];
						glVertex2fv(&v.x);
					}
					glEnd();
					for (int i = 0; i < poly->m_count; ++i) {
						b2Vec2 a = poly->m_vertices[i];
						b2Vec2 b = poly->m_vertices[(i + 1) % poly->m_count];
						b2Vec2 n = poly->m_normals[i];
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
					glPopMatrix();
				} break;

#if 1
				case b2Shape::Type::e_chain:
				{
					glPushMatrix();
					glTranslatef(bodyPos.x, bodyPos.y, 0);
					glRotatef(glm::degrees<float>(bodyRot), 0, 0, 1);
					b2ChainShape *chain = (b2ChainShape *)fixture->GetShape();
					for (int i = 0, c = chain->GetChildCount(); i < c; ++i) {
						b2EdgeShape edge;
						chain->GetChildEdge(&edge, i);
						b2Vec2 a = edge.m_vertex1;
						b2Vec2 b = edge.m_vertex2;
						b2Vec2 n = b2Cross(1.0f, b - a);
						n.Normalize();

						glColor3f(1.0f, 1.0f, 1.0f);
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
					glPopMatrix();
				} break;
#endif

	}
			fixture = fixture->GetNext();
}
		body = body->GetNext();
	}
#endif
}