/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Crackout

Description:
	A breakout-like game based on FPL written in C++.

Requirements:
	- C++/11 Compiler
	- Box2D
	- Final Memory
	- Final Framework

Author:
	Torsten Spaete

Changelog:
	## 2019-06-23
	- Modify the bounce normal for ball vs paddle, based on projection
	- Made the brick assets 50 % smaller to fit the pixel style
	- Print out level percentage

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-08-09
	- Use IsDown() for launching the ball (More responsive)
	- Use actionDown and actionStart for menu instead "any"
	- Support analog paddle movement

	## 2018-07-05
	- Corrected for api change in final_game.h
	- Corrected for api change in final_render.h
	- Migrated to new render system and removed all opengl calls

	## 2018-06-06
	- Refactored files

	## 2018-06-04:
	- Added Score & Lifes
	- Simple Text HUD
	- Simple Menu

	## 2018-05-05:
	- Updated description & todo

	## 2018-04-26:
	- Game implemented

	## 2018-04-24:
	- Initial creation

Todo:
	- Use OpenGL 3.x and use a proper pixel shader to render the 2D-Pixel-Art sprites:
		https://www.shadertoy.com/view/ltBGWc
		https://www.youtube.com/watch?v=Yu8k7a1hQuU
	- Main menu
	- Pause menu (Detect pause)
	- Music
	- Sound
	- Multiball
	- Brick types (Harder, Metal)
	- Items (Ball speed, Paddle grow, Autoglue, Multiball, Player Up)
	- Re-create sprites in HD

License:
	Copyright (c) 2017-2020 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#define BOX2D_IMPLEMENTATION
#include <Box2D/Box2D.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_OPENGL_RENDER_IMPLEMENTATION
#include <final_opengl_render.h>

#include <final_math.h>

#include <final_utils.h>

#include <final_game.h>

//
// Game constants
//
#define DRAW_NORMALS 0
#define DRAW_DEBUG 0

constexpr int InitialLevelSeed = 1;

constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
const Vec2f WorldRadius = V2fInit(WorldWidth, WorldHeight) * 0.5f;

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
const Vec2f PaddleRadius = V2fInit(BallRadius * 3, BallRadius);
const float PaddleLineY = -WorldRadius.y + PaddleRadius.y;
const float PaddleGlueOffsetY = PaddleRadius.y * 2 + BallRadius * 0.25f;

const float PaddleAspect = (PaddleRadius.x + BallRadius) / PaddleRadius.y;

constexpr float BrickSpacing = WorldWidth / 1000.0f;
constexpr int MaxBrickCols = 17;
constexpr int MaxBrickRows = 11;
const float SpaceForBricksX = ((AreaHalfWidth - AreaPadding) * 2.0f) - (MaxBrickCols - 1) * BrickSpacing;
const float SpaceForBricksY = ((AreaHalfHeight - AreaPadding) * 2.0f) - (MaxBrickRows - 1) * BrickSpacing;
const Vec2f BrickRadius = V2fInit(SpaceForBricksX / (float)MaxBrickCols, SpaceForBricksY / (float)MaxBrickRows) * 0.5f;

const Vec2f Gravity = V2fInit(0, -10);

fplStaticAssert(MaxBrickCols % 2 != 0);

// Brick UVs
enum class BrickType : int32_t {
	NoBrick = 0,
	Solid
};
const Vec2i BrickTileSize = V2iInit(15, 12);
const Vec2i BricksTilesetSize = V2iInit(19, 16);
const int BrickTilesetBorder = 2;
class BricksUVsClass : public ArrayInitializer<BrickType, UVRect, 256> {
public:
	BricksUVsClass() {
		Set(BrickType::Solid, UVRectFromTile(BricksTilesetSize, BrickTileSize, BrickTilesetBorder, V2iInit(0, 0)));
	}
};
static BricksUVsClass BricksUVs = {};

// Background UVs
const Vec2i BackgroundTileSize = V2iInit(16, 16);
const Vec2i BackgroundTextureSize = V2iInit(38, 20);
enum class BackgroundType : int32_t {
	NoBackground = 0,
	Default,
};
class BackgroundUVsClass : public ArrayInitializer<BackgroundType, UVRect, 256> {
public:
	BackgroundUVsClass() {
		Set(BackgroundType::Default, UVRectFromPos(BackgroundTextureSize, BackgroundTileSize, V2iInit(2, 2)));
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
const Vec2i FrameTopFillSize = V2iInit(8, 16);
const Vec2i FrameTopTileSize = V2iInit(16, 16);
const Vec2i FrameSideFillSize = V2iInit(16, 8);
const Vec2i FrameSideTileSize = V2iInit(16, 16);
const Vec2i FrameTextureSize = V2iInit(86, 86);
class FrameUVsClass : public ArrayInitializer<FrameType, UVRect, 256> {
public:
	FrameUVsClass() {
		Set(FrameType::TopLeftEdge, UVRectFromPos(FrameTextureSize, FrameTopTileSize, V2iInit(2, 2)));
		Set(FrameType::TopFill, UVRectFromPos(FrameTextureSize, FrameTopFillSize, V2iInit(20, 2)));
		Set(FrameType::TopMarks, UVRectFromPos(FrameTextureSize, FrameTopFillSize, V2iInit(40, 2)));
		Set(FrameType::TopBegin, UVRectFromPos(FrameTextureSize, FrameTopFillSize, V2iInit(30, 2)));
		Set(FrameType::TopEnd, UVRectFromPos(FrameTextureSize, FrameTopFillSize, V2iInit(58, 2)));
		Set(FrameType::TopRightEdge, UVRectFromPos(FrameTextureSize, FrameTopTileSize, V2iInit(68, 2)));

		Set(FrameType::LeftFill, UVRectFromPos(FrameTextureSize, FrameSideFillSize, V2iInit(2, 20)));
		Set(FrameType::LeftStart, UVRectFromPos(FrameTextureSize, FrameSideFillSize, V2iInit(2, 30)));
		Set(FrameType::LeftMarks, UVRectFromPos(FrameTextureSize, FrameSideTileSize, V2iInit(2, 40)));
		Set(FrameType::LeftEnd, UVRectFromPos(FrameTextureSize, FrameSideFillSize, V2iInit(2, 58)));
		Set(FrameType::LeftBottomEdge, UVRectFromPos(FrameTextureSize, FrameSideTileSize, V2iInit(2, 68)));

		Set(FrameType::RightFill, UVRectFromPos(FrameTextureSize, FrameSideFillSize, V2iInit(68, 20)));
		Set(FrameType::RightStart, UVRectFromPos(FrameTextureSize, FrameSideFillSize, V2iInit(68, 30)));
		Set(FrameType::RightMarks, UVRectFromPos(FrameTextureSize, FrameSideTileSize, V2iInit(68, 40)));
		Set(FrameType::RightEnd, UVRectFromPos(FrameTextureSize, FrameSideFillSize, V2iInit(68, 58)));
		Set(FrameType::RightBottomEdge, UVRectFromPos(FrameTextureSize, FrameSideTileSize, V2iInit(68, 68)));
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
	b2Body* body;
	float speed;
	bool isMoving;
	bool isDead;
};

struct Paddle {
	b2Body* body;
	float speed;
	Ball* gluedBall;
};

struct Brick {
	b2Body* body;
	BrickType type;
	bool requestHit;
	bool isHit;
	Vec2f hitPoint;
	Vec2f hitNormal;
	bool isDead;
};

struct Frame {
	b2Body* top;
	b2Body* left;
	b2Body* right;
};

struct KillArea {
	b2Body* body;
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

struct GameState;

// @BAD(final): Such a bad design decision from Box2D to force classes on us
class GameContactListener : public b2ContactListener {
private:
	GameState* gameState;
public:
	GameContactListener(GameState* gameState) {
		this->gameState = gameState;
	}
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};

struct Assets {
	TextureAsset ballTexture;
	TextureAsset bricksTexture;
	TextureAsset paddleTexture;
	TextureAsset frameTexture;
	ArrayInitializer<BackgroundType, TextureAsset, 256> bgTextures;
	FontAsset fontMenu;
	FontAsset fontHud;
	AudioSource *ballHitSound;
	AudioSource *music;
	uint64_t musicPlayId;
};

enum class GameMode {
	Title,
	Menu,
	Play,
	GameOver,
};

enum class MenuSection : uint32_t {
	Main = 0,
	Options,
	Exit
};

struct MenuRenderState {
	float ypos;
	float fontHeight;
};

typedef const char* MenuID;

struct MenuState {
	int itemIndex;
	int itemCount;
	MenuID hotID;
	MenuID activeID;
	MenuSection section;
	bool itemActivated;
};

struct GameState {
	char dataPath[1024];
	Assets assets;

	AudioSystem *audioSys;

	Viewport viewport;

	b2World* world;

	Entity frame;
	Entity ball;
	Entity paddle;
	Entity killArea;
	BrickType bricksMap[1024];
	Entity activeBricks[1024];
	size_t totalBricks;
	size_t remainingBricks;

	GameContactListener* contactListener;

	int levelSeed;
	int levelsCompleted;

	GameMode mode;
	int lifes;
	int score;
	MenuState menu;

	bool isExiting;
};

static void SetRandomLevel(GameState& state, int seed);
static void GlueBallOnPaddle(GameState& state, Ball* ball);

inline float Random01() {
	float result = rand() / (float)RAND_MAX;
	return(result);
}

inline int RandomInt(int size) {
	int result = rand() % size;
	return(result);
}



static void ClearWorld(b2World* world) {
	b2Body* body = world->GetBodyList();
	while (body != nullptr) {
		b2Body* next = body->GetNext();
		world->DestroyBody(body);
		body = next;
	}
}

static void LoadLevel(GameState& state, int levelSeed) {
	SetRandomLevel(state, levelSeed);

	b2World* world = state.world;
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

		b2Body* body;

		b2PolygonShape sideShape = b2PolygonShape();
		sideShape.SetAsBox(FrameRadius, hh + KillAreaExtent);
		b2PolygonShape topShape = b2PolygonShape();
		topShape.SetAsBox(hw, FrameRadius);

		Entity* frameEntity = &state.frame;
		*frameEntity = {};
		frameEntity->type = EntityType::Frame;
		Frame& frame = frameEntity->frame;

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
		Entity* killAreaEntity = &state.killArea;
		*killAreaEntity = {};
		killAreaEntity->type = EntityType::KillArea;
		KillArea& killArea = killAreaEntity->killArea;

		b2Body* body;
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
		b2Body* body;
		float halfWidth = (MaxBrickCols * BrickRadius.x) - ((MaxBrickCols - 1) * BrickSpacing * 0.5f);
		float brickY = WorldRadius.y - FrameRadius * 2.0f - AreaPadding - BrickRadius.y;
		state.totalBricks = 0;
		for (int row = 0; row < MaxBrickRows; ++row) {
			//float brickX = -WorldRadius.x + SideBorderRadius * 2.0f - AreaPadding + BrickRadius.x;
			float brickX = -WorldRadius.x + FrameRadius * 2.0f + AreaPadding + BrickRadius.x;
			for (int col = 0; col < MaxBrickCols; ++col) {
				BrickType brickType = state.bricksMap[row * MaxBrickCols + col];
				if (brickType == BrickType::Solid) {
					Entity* brickEntity = &state.activeBricks[state.totalBricks++];
					*brickEntity = {};
					brickEntity->type = EntityType::Brick;

					Brick& brick = brickEntity->brick;
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
		state.remainingBricks = state.totalBricks;
	}

	//
	// Paddle
	// 
	{
		b2Body* body;
		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;

		// Limiter
		bodyDef = b2BodyDef();
		bodyDef.type = b2BodyType::b2_staticBody;
		bodyDef.position = b2Vec2(0, PaddleLineY);
		b2Body* paddleLimiterBody = body = world->CreateBody(&bodyDef);
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
		Entity* paddleEntity = &state.paddle;
		*paddleEntity = {};
		paddleEntity->type = EntityType::Paddle;

		Paddle& paddle = paddleEntity->paddle;
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

		// Box
		b2PolygonShape boxShape = b2PolygonShape();
		boxShape.SetAsBox(PaddleRadius.x, PaddleRadius.y);
		fixtureDef.shape = &boxShape;
		fixtureDef.userData = paddleEntity;
		body->CreateFixture(&fixtureDef);

		fixtureDef.userData = nullptr;

		// Left circle
		b2CircleShape leftShape = b2CircleShape();
		leftShape.m_radius = PaddleRadius.y;
		leftShape.m_p = b2Vec2(-PaddleRadius.x, 0);
		fixtureDef.shape = &leftShape;
		body->CreateFixture(&fixtureDef);

		// Right circle
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
		b2Body* body;
		b2BodyDef ballDef = b2BodyDef();

		Entity* ballEntity = &state.ball;
		*ballEntity = {};
		ballEntity->type = EntityType::Ball;

		Ball& ball = ballEntity->ball;
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

	// Play music
	AudioSystemStopSource(state.audioSys, state.assets.musicPlayId);
	state.assets.musicPlayId = AudioSystemPlaySource(state.audioSys, state.assets.music, true, 0.5f);
}


static bool LoadTexture(const TextureData& source, const bool repeatable, TextureAsset& outTexture) {
	GLuint texId = AllocateTexture(source.width, source.height, source.data, repeatable, GL_NEAREST);
	outTexture.texture = ValueToPointer(texId);
	bool result = texId > 0;
	return(result);
}

static bool LoadTexture(const char* dataPath, const char* filename, const bool repeatable, TextureAsset& outTexture) {
	TextureData image = LoadTextureData(dataPath, filename);
	if (image.data == nullptr) {
		return false;
	}
	bool result = LoadTexture(image, repeatable, outTexture);
	FreeTextureData(image);
	return(result);
}

static bool LoadSound(AudioSystem *audio, const char *dataPath, const char *filename, AudioSource *&outSource) {
	char filePath[1024];
	fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);
	AudioSource *source = AudioSystemLoadFileSource(audio, filePath);
	bool result = source != fpl_null;
	if (result) {
		outSource = source;
	}
	return(result);
}

static bool LoadAssets(GameState& state) {
	LoadTexture(state.dataPath, "ball.bmp", false, state.assets.ballTexture);
	LoadTexture(state.dataPath, "bricks.bmp", false, state.assets.bricksTexture);
	LoadTexture(state.dataPath, "paddle.bmp", false, state.assets.paddleTexture);
	LoadTexture(state.dataPath, "frame.bmp", false, state.assets.frameTexture);

	TextureData bgImage = LoadTextureData(state.dataPath, "bg.bmp");
	if (bgImage.data != nullptr) {
		TextureData bgTileImage0 = CreateSubTextureData(bgImage, 2, 2, 16, 16);
		LoadTexture(bgTileImage0, true, state.assets.bgTextures[BackgroundType::Default]);
		FreeTextureData(bgTileImage0);
	}
	FreeTextureData(bgImage);

	if (LoadFontFromFile(state.dataPath, "hemi_head_bd_it.ttf", 0, 36.0f, 32, 127, 512, 512, true, &state.assets.fontMenu.desc)) {
		GLuint texId = AllocateTexture(state.assets.fontMenu.desc.atlasWidth, state.assets.fontMenu.desc.atlasHeight, state.assets.fontMenu.desc.atlasAlphaBitmap, false, GL_NEAREST, true);
		state.assets.fontMenu.texture = ValueToPointer(texId);
	}
	if (LoadFontFromFile(state.dataPath, "hemi_head_bd_it.ttf", 0, 18.0f, 32, 127, 512, 512, true, &state.assets.fontHud.desc)) {
		GLuint texId = AllocateTexture(state.assets.fontHud.desc.atlasWidth, state.assets.fontHud.desc.atlasHeight, state.assets.fontHud.desc.atlasAlphaBitmap, false, GL_NEAREST, true);
		state.assets.fontHud.texture = ValueToPointer(texId);
	}

	LoadSound(state.audioSys, state.dataPath, "bounce_44100hz.wav", state.assets.ballHitSound);

	LoadSound(state.audioSys, state.dataPath, "music_44100hz.mp3", state.assets.music);

	return true;
}

static void StartGame(GameState& state) {
	state.lifes = 5;
	state.score = 0;
	state.mode = GameMode::Play;
	state.levelsCompleted = 0;
	LoadLevel(state, InitialLevelSeed);
}

static bool InitGame(GameState& state) {
	if (!fglLoadOpenGL(true)) {
		return false;
	}

	fplGetExecutableFilePath(state.dataPath, fplArrayCount(state.dataPath));
	fplExtractFilePath(state.dataPath, state.dataPath, fplArrayCount(state.dataPath));
	fplPathCombine(state.dataPath, fplArrayCount(state.dataPath), 2, state.dataPath, "data");

	srand((int)fplGetTimeInMillisecondsLP());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);

	LoadAssets(state);

	b2World* world;

	state.world = world = new b2World(b2Vec2(Gravity.x, Gravity.y));
	world->SetContinuousPhysics(true);

	state.contactListener = new GameContactListener(&state);
	world->SetContactListener(state.contactListener);

#if 1
	state.mode = GameMode::Title;
	state.menu = {};
#else
	StartGame(state);
#endif

	return true;
}

static void ReleaseGame(GameState& state) {
	ClearWorld(state.world);
	if (state.world != nullptr) {
		delete state.world;
		state.world = nullptr;
	}
	fglUnloadOpenGL();
}

extern void GameRelease(GameMemory& gameMemory) {
	GameState* state = gameMemory.game;
	if (state != nullptr) {
		ReleaseGame(*state);
		state->~GameState();
	}
}

extern bool GameInit(GameMemory& gameMemory) {
	GameState* state = (GameState*)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;
	state->audioSys = gameMemory.audio;
	if (!InitGame(*state)) {
		GameRelease(gameMemory);
		return(false);
	}
	return(true);

}

static void GlueBallOnPaddle(GameState& state, Ball* ball) {
	Paddle& paddle = state.paddle.paddle;
	fplAssert(paddle.gluedBall == nullptr);
	ball->isMoving = false;
	paddle.gluedBall = ball;
	ball->body->SetType(b2BodyType::b2_staticBody);
}

static void LaunchBall(GameState& state) {
	Paddle& paddle = state.paddle.paddle;
	const float spreadAngle = 30.0f;
	const float startAngle = 90;
	Ball* ball = paddle.gluedBall;
	ball->isMoving = true;
	ball->isDead = false;
	ball->body->SetType(b2BodyType::b2_dynamicBody);
	float newAngle = startAngle + (Random01() > 0.5f ? -1 : 1) * Random01() * spreadAngle;
	float a = DegreesToRadians(newAngle);
	b2Vec2 direction = b2Vec2(Cosine(a), Sine(a));
	ball->body->ApplyLinearImpulse(ball->speed * direction, ball->body->GetPosition(), true);
	paddle.gluedBall = nullptr;
}

static void SetRandomLevel(GameState& state, int seed) {
#define ALL_BRICKS 0
	fplAssert((MaxBrickCols * MaxBrickRows) <= fplArrayCount(state.bricksMap));
	fplMemoryClear(state.bricksMap, sizeof(Brick) * fplArrayCount(state.bricksMap));
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

static void EntersKillArea(GameState& state, Entity& other) {
	if (other.type == EntityType::Ball) {
		Ball* ball = &other.ball;
		Paddle& paddle = state.paddle.paddle;
		ball->isDead = true;
		state.lifes--;
		if (state.lifes <= 0) {
			state.lifes = 0;
		}
	} else if (other.type == EntityType::Brick) {
		Brick& brick = other.brick;
		if (!brick.isDead) {
			brick.isDead = true;
			++state.score;
		}
	}
}

static void HandleBallCollision(GameState& state, Ball& ball, Entity& other, b2Contact& contact) {
	if (other.type == EntityType::Brick) {
		Brick& brick = other.brick;
		if (!brick.requestHit && !brick.isDead) {
			brick.requestHit = true;
			b2WorldManifold manifold;
			contact.GetWorldManifold(&manifold);
			brick.hitPoint = V2fInit(manifold.points[0].x, manifold.points[0].y);
			brick.hitNormal = V2fInit(manifold.normal.x, manifold.normal.y);
		}
	}

	// Play sound
	AudioSystemPlaySource(state.audioSys, state.assets.ballHitSound, false, 0.5f);
}

struct CollisionPair {
	b2Fixture* fixtureA;
	b2Fixture* fixtureB;
	b2Body* bodyA;
	b2Body* bodyB;
	Entity* entityA;
	Entity* entityB;
};

static CollisionPair GetCollisionPair(b2Contact* contact) {
	CollisionPair result = {};
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	fplAssert(fixtureA != nullptr && fixtureB != nullptr);
	b2Body* bodyA = fixtureA->GetBody();
	b2Body* bodyB = fixtureB->GetBody();
	fplAssert(bodyA != nullptr && bodyB != nullptr);
	void* dataA = bodyA->GetUserData();
	void* dataB = bodyB->GetUserData();
	if (dataA != nullptr && dataB != nullptr) {
		Entity* entityA = (Entity*)dataA;
		Entity* entityB = (Entity*)dataB;

		// Sort entity by type
		if (entityA->type > entityB->type) {
			entityB = (Entity*)dataA;
			entityA = (Entity*)dataB;
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

static void HandleContactCollision(GameState& state, b2Contact* contact) {
	CollisionPair pair = GetCollisionPair(contact);
	if (pair.entityA->type == EntityType::Ball) {
		HandleBallCollision(state, pair.entityA->ball, *pair.entityB, *contact);
	}
}

static void HandlePreCollision(GameState& state, b2Contact* contact) {
	CollisionPair pair = GetCollisionPair(contact);
	if (pair.entityA->type == EntityType::KillArea) {
		EntersKillArea(state, *pair.entityB);
	} else if (pair.entityB->type == EntityType::KillArea) {
		EntersKillArea(state, *pair.entityA);
	} else if (pair.entityA->type == EntityType::Ball && pair.entityB->type == EntityType::Paddle) {
		b2Manifold* manifold = contact->GetManifold();

		b2Vec2 unitUpVec = b2Vec2(0, 1);
		b2Vec2 unitRightVec = b2Vec2(1, 0);

		if (pair.fixtureB->GetUserData() != nullptr) {
			b2Vec2 ballPos = pair.bodyA->GetPosition();
			b2Vec2 paddlePos = pair.bodyB->GetPosition();

			b2Vec2 distanceToPaddle = ballPos - paddlePos;
			float t = b2Clamp(b2Dot(distanceToPaddle, unitRightVec) / PaddleRadius.x, -1.0f, 1.0f);

			Vec2f bounce = V2fInit(0, 1);
			if (t < 0.0f) {
				t = Abs(t);
				bounce = V2fLerp(V2fInit(0, 1), t, V2fInit(-1, 0.25f));
			} else {
				bounce = V2fLerp(V2fInit(0, 1), t, V2fInit(1, 0.25f));
			}
			bounce = V2fNormalize(bounce);

			b2Vec2 n = b2Vec2(bounce.x, bounce.y);

			fplDebugFormatOut("Out direction: %f %f, t: %f\n", n.x, n.y, t);

			manifold->localNormal = n;
		}
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

extern bool IsGameExiting(GameMemory& gameMemory) {
	GameState* state = gameMemory.game;
	fplAssert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory& gameMemory, const Input& input) {
	if (!input.isActive) {
		return;
	}

	GameState* state = gameMemory.game;
	fplAssert(state != nullptr);

	if (input.defaultControllerIndex > -1) {
		fplAssert(input.defaultControllerIndex < fplArrayCount(input.controllers));
		const Controller* controller = &input.controllers[input.defaultControllerIndex];
		if (controller->isConnected) {
			switch (state->mode) {
			case GameMode::Play:
			{
				// Single player input
				Paddle& paddle = state->paddle.paddle;

				if (controller->isAnalog) {
					float x = controller->analogMovement.x;
					if (Abs(x) > 0) {
						paddle.body->ApplyLinearImpulse(paddle.speed * b2Vec2(x, 0), paddle.body->GetPosition(), true);
					}
				} else {
					if (IsDown(controller->moveLeft)) {
						paddle.body->ApplyLinearImpulse(paddle.speed * b2Vec2(-1, 0), paddle.body->GetPosition(), true);
					} else if (IsDown(controller->moveRight)) {
						paddle.body->ApplyLinearImpulse(paddle.speed * b2Vec2(1, 0), paddle.body->GetPosition(), true);
					}
				}

				if (IsDown(controller->actionDown) && paddle.gluedBall != nullptr) {
					LaunchBall(*state);
				}
			} break;

			case GameMode::Title:
			{
				if (WasPressed(controller->actionDown) || WasPressed(controller->actionStart)) {
					state->mode = GameMode::Menu;
					state->menu = {};
					state->menu.section = MenuSection::Main;
				}
			} break;

			case GameMode::GameOver:
			{
				if (WasPressed(controller->actionDown) || WasPressed(controller->actionStart)) {
					state->mode = GameMode::Title;
				}
			};

			case GameMode::Menu:
			{
				if (WasPressed(controller->moveDown)) {
					if (state->menu.itemIndex < (state->menu.itemCount - 1)) {
						++state->menu.itemIndex;
					}
				} else if (WasPressed(controller->moveUp)) {
					if (state->menu.itemIndex > 0) {
						--state->menu.itemIndex;
					}
				}
				if (WasPressed(controller->actionDown) || WasPressed(controller->actionStart)) {
					if (state->menu.hotID != nullptr) {
						state->menu.itemActivated = true;
					}
				}
			} break;
			}
		}
	}
}

static void UpdatePlayMode(GameState& state, const Input& input) {
	// Game over?
	if (state.lifes == 0) {
		state.mode = GameMode::GameOver;
		return;
	}

	// Move glued ball
	Paddle& paddle = state.paddle.paddle;
	if (paddle.gluedBall == nullptr && state.ball.ball.isDead) {
		GlueBallOnPaddle(state, &state.ball.ball);
	}
	if (paddle.gluedBall != nullptr) {
		Ball* ball = paddle.gluedBall;
		b2Vec2 gluePos = paddle.body->GetPosition() + b2Vec2(0, PaddleGlueOffsetY);
		ball->body->SetTransform(gluePos, 0);
	} else {
	}

	// Force ball speed and correct ball angle when needed
	{
		const float angleTolerance = 3.0f;
		const float angleCorrection = 6.0f;
		float squaredAngles[] = { 0, 90, 180, 270, 360 };
		Ball& ball = state.ball.ball;
		if (ball.isMoving) {
			b2Vec2 vel = ball.body->GetLinearVelocity();
			b2Vec2 dir = vel;
			dir.Normalize();

			float a = ArcTan2(dir.y, dir.x);
			float deg = RadiansToDegrees(a);
			for (int i = 0; i < fplArrayCount(squaredAngles); ++i) {
				if (Abs(deg) > (squaredAngles[i] - angleTolerance) && Abs(deg) < (squaredAngles[i] + angleTolerance)) {
					deg += (Abs(deg) - squaredAngles[i] > 0 ? 1 : -1) * angleCorrection;
					a = DegreesToRadians(deg);
				}
			}
			dir = b2Vec2(Cosine(a), Sine(a));

			dir *= ball.speed;
			ball.body->SetLinearVelocity(dir);
		}
	}

	// Make all bricks dynamic when hit
	const float hitStrength = 1.5f;
	for (size_t i = 0; i < state.remainingBricks; ++i) {
		Entity& brickEntity = state.activeBricks[i];
		Brick& brick = brickEntity.brick;
		if (brick.requestHit && !brick.isDead && !brick.isHit) {
			brick.body->SetType(b2BodyType::b2_dynamicBody);
			b2Vec2 impulse = hitStrength * -b2Vec2(brick.hitNormal.x, brick.hitNormal.y);
			b2Vec2 point = b2Vec2(brick.hitPoint.x, brick.hitPoint.y);
			brick.body->ApplyLinearImpulse(impulse, point, true);
			brick.isHit = true;
		}
	}

	// Remove dead bricks
	if (state.remainingBricks > 0) {
		for (size_t i = 0, c = state.remainingBricks; i < c; ++i) {
			if (state.activeBricks[i].brick.isDead) {
				Entity temp = state.activeBricks[i];
				if (temp.brick.body != nullptr) {
					state.world->DestroyBody(temp.brick.body);
				}
				temp = {};
				if (i < state.remainingBricks - 1) {
					state.activeBricks[i] = state.activeBricks[state.remainingBricks - 1];
					state.activeBricks[state.remainingBricks - 1] = temp;
					state.activeBricks[i].brick.body->SetUserData(&state.activeBricks[i]);
				}
				--state.remainingBricks;
			}
		}
		if (state.remainingBricks == 0) {
			// Level done
			++state.levelsCompleted;
			LoadLevel(state, state.levelSeed + 1);
		}
	}

	// Run physics simulation
	state.world->Step(input.deltaTime, 10, 10);
	state.world->ClearForces();
}

extern void GameUpdate(GameMemory& gameMemory, const Input& input) {
	if (!input.isActive) {
		return;
	}

	GameState* state = gameMemory.game;
	fplAssert(state != nullptr);
	state->viewport = ComputeViewportByAspect(input.windowSize, GameAspect);

	if (state->mode == GameMode::Play) {
		UpdatePlayMode(*state, input);
	}
}



static void DrawField(GameState& state) {
	// Background
	{
		GLuint bgTex = PointerToValue<GLuint>(state.assets.bgTextures[BackgroundType::Default].texture);
		float uMax = (float)(int)(WorldRadius.x / FrameRadius);
		float vMax = (float)(int)(WorldRadius.y / FrameRadius);
		UVRect bgUV = BackgroundUVs[BackgroundType::Default];
		glColor4f(1, 1, 1, 1);
		DrawSprite(bgTex, WorldRadius.x - FrameRadius * 2, WorldRadius.y - FrameRadius, 0.0f, vMax, uMax, 0.0f, 0, -FrameRadius);
	}

	// Frame
	{
		GLuint frameTex = PointerToValue<GLuint>(state.assets.frameTexture.texture);
		UVRect topLeftEdgeUV = FrameUVs[FrameType::TopLeftEdge];
		UVRect topRightEdgeUV = FrameUVs[FrameType::TopRightEdge];
		UVRect topFillUV = FrameUVs[FrameType::TopFill];
		UVRect bottomLeftEdgeUV = FrameUVs[FrameType::LeftBottomEdge];
		UVRect bottomRightEdgeUV = FrameUVs[FrameType::RightBottomEdge];
		UVRect leftFillUV = FrameUVs[FrameType::LeftFill];
		UVRect rightFillUV = FrameUVs[FrameType::RightFill];

		glColor4f(1, 1, 1, 1);

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
}

static void DrawPlayMode(GameState& state) {
	// Increase radius a tiny bit to match collision shape
	const float ROffset = WorldWidth / 1000.0f;

	// Field
	DrawField(state);

	// Ball
	{
		const Ball& ball = state.ball.ball;
		b2Vec2 ballPos = ball.body->GetPosition();
		float ballRot = ball.body->GetAngle();
		GLuint texId = PointerToValue<GLuint>(state.assets.ballTexture.texture);
		glPushMatrix();
		glTranslatef(ballPos.x, ballPos.y, 0);
		glRotatef(RadiansToDegrees(ballRot), 0, 0, 1);
		glColor4f(1, 1, 1, 1);
		DrawSprite(texId, BallRadius + ROffset, BallRadius + ROffset, 0.0f, 1.0f, 1.0f, 0.0f);
		glPopMatrix();
	}

	// Paddle
	{
		const Paddle& paddle = state.paddle.paddle;
		b2Vec2 paddlePos = paddle.body->GetPosition();
		float paddleRot = paddle.body->GetAngle();
		GLuint texId = PointerToValue<GLuint>(state.assets.paddleTexture.texture);
		glPushMatrix();
		glTranslatef(paddlePos.x, paddlePos.y, 0);
		glRotatef(RadiansToDegrees(paddleRot), 0, 0, 1);
		glColor4f(1, 1, 1, 1);
		DrawSprite(texId, PaddleRadius.x + BallRadius + ROffset, PaddleRadius.y + ROffset, 0.0f, 1.0f, 1.0f, 0.0f);
		glPopMatrix();
	}

	// Bricks
	for (size_t i = 0; i < state.remainingBricks; ++i) {
		const Brick& brick = state.activeBricks[i].brick;
		b2Vec2 brickPos = brick.body->GetPosition();
		float brickRot = brick.body->GetAngle();
		GLuint texId = PointerToValue<GLuint>(state.assets.bricksTexture.texture);
		UVRect brickUV = BricksUVs[brick.type];
		glPushMatrix();
		glTranslatef(brickPos.x, brickPos.y, 0);
		glRotatef(RadiansToDegrees(brickRot), 0, 0, 1);
		glColor4f(1, 1, 1, 1);
		DrawSprite(texId, BrickRadius.x, BrickRadius.y, brickUV);
		glPopMatrix();
	}

#if DRAW_DEBUG
	// Bodies
	const float normalLen = WorldWidth * 0.025f;
	b2Body* body = state.world->GetBodyList();
	while (body != nullptr) {
		if (body->GetUserData() == nullptr) {
			body = body->GetNext();
			continue;
		}
		Entity* entity = (Entity*)body->GetUserData();
		b2Fixture* fixture = body->GetFixtureList();
		b2Vec2 bodyPos = body->GetPosition();
		float bodyRot = body->GetAngle();
		while (fixture != nullptr) {
			switch (fixture->GetType()) {
			case b2Shape::Type::e_circle:
			{
				b2CircleShape* circle = (b2CircleShape*)fixture->GetShape();
				glPushMatrix();
				glTranslatef(bodyPos.x + circle->m_p.x, bodyPos.y + circle->m_p.y, 0);
				glRotatef(RadiansToDegrees(bodyRot), 0, 0, 1);
				DrawCircle(0, 0, circle->m_radius, false, V4f(1, 1, 1, 1));
				glPopMatrix();
			} break;

			case b2Shape::Type::e_polygon:
			{
				b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
				glPushMatrix();
				glTranslatef(bodyPos.x, bodyPos.y, 0);
				glRotatef(RadiansToDegrees(bodyRot), 0, 0, 1);

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
				glRotatef(RadiansToDegrees(bodyRot), 0, 0, 1);
				b2ChainShape* chain = (b2ChainShape*)fixture->GetShape();
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

	char textBuffer[256];
	const float textFrameMargin = BallRadius * 0.25f;
	const float textSize = 0.65f;
	const float textTopMiddle = WorldRadius.y - FrameRadius;
	GLuint fontTexId = PointerToValue<GLuint>(state.assets.fontHud.texture);

	// HUD
	glColor4f(0, 0, 0, 1);

	fplFormatString(textBuffer, fplArrayCount(textBuffer), "Lifes: %d", state.lifes);
	DrawTextFont(textBuffer, fplGetStringLength(textBuffer), &state.assets.fontHud.desc, fontTexId, -WorldRadius.x + FrameRadius * 2.0f + textFrameMargin, textTopMiddle, textSize, 1.0f, 0.0f);

	if (state.totalBricks > 0) {
		int levelPercentage = 100 - (int)((state.remainingBricks / (double)state.totalBricks) * 100);
		fplFormatString(textBuffer, fplArrayCount(textBuffer), "Level: %d (%d %s)", (state.levelsCompleted + 1), levelPercentage, "%");
	} else {
		fplFormatString(textBuffer, fplArrayCount(textBuffer), "Level: %d", (state.levelsCompleted + 1));
	}
	DrawTextFont(textBuffer, fplGetStringLength(textBuffer), &state.assets.fontHud.desc, fontTexId, 0, textTopMiddle, textSize, 0.0f, 0.0f);

	fplFormatString(textBuffer, fplArrayCount(textBuffer), "Score: %d", state.score);
	size_t textCount = fplGetStringLength(textBuffer);
	Vec2f textBounds = GetTextSize(textBuffer, textCount, &state.assets.fontHud.desc, textSize);
	DrawTextFont(textBuffer, textCount, &state.assets.fontHud.desc, fontTexId, WorldRadius.x - FrameRadius * 2.0f - textFrameMargin - textBounds.w, textTopMiddle, textSize, 1.0f, 0.0f);
}

static void BeginMenu(GameState& state) {
	state.menu.itemCount = 0;
	state.menu.hotID = nullptr;
}

static bool PushMenuItem(GameState& state, MenuRenderState& menuRender, const char* itemText) {
	bool result = false;
	int index = state.menu.itemCount++;
	if (index == state.menu.itemIndex) {
		if (state.menu.hotID != itemText) {
			state.menu.hotID = itemText;
		}
		if (state.menu.itemActivated) {
			state.menu.activeID = state.menu.hotID;
			state.menu.itemActivated = false;
			result = true;
		}
		glColor4f(1, 1, 0, 1);
	} else {
		glColor4f(1, 1, 1, 1);
	}
	GLuint fontTexId = PointerToValue<GLuint>(state.assets.fontMenu.texture);
	DrawTextFont(itemText, fplGetStringLength(itemText), &state.assets.fontMenu.desc, fontTexId, 0.0f, menuRender.ypos, menuRender.fontHeight, 0.0f, 0.0f);
	menuRender.ypos -= menuRender.fontHeight;
	return(result);
}

static void DrawTitleMenuMode(GameState& state) {
	// Field
	DrawField(state);

	// Title
	const char* titleText = (state.mode == GameMode::GameOver) ? "Game Over!" : "Crackout";
	float titleFontSize = 2.75f;
	float titlePosY = WorldRadius.y - WorldHeight * 0.35f;
	glColor4f(1, 1, 1, 1);
	GLuint fontTexId = PointerToValue<GLuint>(state.assets.fontMenu.texture);
	DrawTextFont(titleText, fplGetStringLength(titleText), &state.assets.fontMenu.desc, fontTexId, 0.0f, titlePosY, titleFontSize, 0.0f, 0.0f);

	if (state.mode == GameMode::Title || state.mode == GameMode::GameOver) {
		// Title screen
		const char* smallText = "Press spacebar or action-key!";
		const float smallFontSize = 0.9f;
		const float smallPosY = -WorldRadius.y + WorldHeight * 0.275f;
		glColor4f(1, 1, 1, 1);
		DrawTextFont(smallText, fplGetStringLength(smallText), &state.assets.fontMenu.desc, fontTexId, 0.0f, smallPosY, smallFontSize, 0.0f, 0.0f);
	} else {
		// Menu screen
		fplAssert(state.mode == GameMode::Menu);
		const float itemFontSize = 1.1f;

		MenuRenderState menuRender = {};
		menuRender.fontHeight = itemFontSize;
		menuRender.ypos = titlePosY - titleFontSize * 0.5f - itemFontSize * 1.25f;

		BeginMenu(state);
		if (PushMenuItem(state, menuRender, "Start Game")) {
			StartGame(state);
		}
		if (PushMenuItem(state, menuRender, "Exit Game")) {
			state.isExiting = true;
		}
	}
}

extern void GameRender(GameMemory& gameMemory, const float alpha) {
	GameState* state = gameMemory.game;
	fplAssert(state != nullptr);
	RenderState* renderState = gameMemory.render;

	const float w = WorldRadius.x;
	const float h = WorldRadius.y;

	glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float scale = 1.0f;
	glOrtho(-w * scale, w * scale, -h * scale, h * scale, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (state->mode == GameMode::Play) {
		DrawPlayMode(*state);
	} else if (state->mode == GameMode::Title || state->mode == GameMode::GameOver || state->mode == GameMode::Menu) {
		DrawTitleMenuMode(*state);
	}
}

extern void GameUpdateAndRender(GameMemory& gameMemory, const Input& input, const float alpha) {
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char* argv[]) {
	fplSetMaxLogLevel(fplLogLevel_All);

	GameConfiguration config = {};
	config.title = L"FPL Demo | Crackout";
	config.hideMouseCursor = true;
	config.noUpdateRenderSeparation = false;
	config.audioSampleRate = 44100;
	int result = GameMain(config);
	return(result);
}