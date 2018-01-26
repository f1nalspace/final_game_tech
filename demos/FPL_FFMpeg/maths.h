#pragma once

//
// Vec2f (2D 32-bit float)
//
union Vec2f {
	struct {
		float x;
		float y;
	};
	struct {
		float w;
		float h;
	};
	struct {
		float u;
		float v;
	};
	float elements[2];

	inline Vec2f(const float xy = 0.0f) {
		x = y = xy;
	}
	inline Vec2f(const float newX, const float newY) {
		x = newX;
		y = newY;
	}
	inline Vec2f(const Vec2f &from) {
		x = from.x;
		y = from.y;
	}

	// @NOTE: Static initialization is done in the source file
	static const Vec2f &Up;
	static const Vec2f &Down;
	static const Vec2f &Left;
	static const Vec2f &Right;
};

//
// Vec3f (3D 32-bit float)
//
union Vec3f {
	struct {
		float x;
		float y;
		float z;
	};
	struct {
		float w;
		float h;
		float d;
	};
	struct {
		Vec2f xy;
		float z;
	};
	struct {
		float x;
		Vec2f yz;
	};
	struct {
		Vec2f st;
		float u;
	};
	struct {
		float s;
		Vec2f tu;
	};
	struct {
		float r;
		float g;
		float b;
	};
	struct {
		Vec2f rg;
		float b;
	};
	struct {
		float r;
		Vec2f gb;
	};
	float elements[3];

	inline Vec3f(const float xyz = 0.0f) {
		x = y = z = xyz;
	}
	inline Vec3f(const float newX, const float newY, const float newZ = 0.0f) {
		x = newX;
		y = newY;
		z = newZ;
	}
	inline Vec3f(const Vec2f &from, const float newZ = 0.0f) {
		x = from.x;
		y = from.y;
		z = newZ;
	}
	inline Vec3f(const Vec3f &from) {
		x = from.x;
		y = from.y;
		z = from.z;
	}

	// @NOTE: Static initialization is done in the source file
	static const Vec3f &Up;
	static const Vec3f &Down;
	static const Vec3f &Left;
	static const Vec3f &Right;
};

//
// Vec4f (4D 32-bit float)
//
union Vec4f {
	struct {
		float x;
		float y;
		float z;
		float w;
	};
	struct {
		Vec2f xy;
		float z;
		float w;
	};
	struct {
		float x;
		Vec2f yz;
		float w;
	};
	struct {
		float x;
		float y;
		Vec2f zw;
	};
	struct {
		Vec2f xy;
		Vec2f zw;
	};
	struct {
		Vec3f xyz;
		float w;
	};
	struct {
		float x;
		Vec3f yzw;
	};
	struct {
		float r;
		float g;
		float b;
		float a;
	};
	struct {
		Vec3f rgb;
		float a;
	};
	struct {
		float r;
		Vec3f gba;
	};
	struct {
		Vec2f rg;
		float b;
		float a;
	};
	struct {
		Vec2f rg;
		Vec2f ba;
	};
	struct {
		float r;
		Vec2f gb;
		float a;
	};
	struct {
		float r;
		float g;
		Vec2f ba;
	};
	float elements[4];

	inline Vec4f(const float newW = 1.0f) {
		x = y = z = 0;
		w = newW;
	}
	inline Vec4f(const float xyz, const float newW = 1.0f) {
		x = y = z = xyz;
		w = newW;
	}
	inline Vec4f(const float newX, const float newY, const float newZ, const float newW = 1.0f) {
		x = newX;
		y = newY;
		z = newZ;
		w = newW;
	}
	inline Vec4f(const Vec2f &from, const float newZ = 1.0f, const float newW = 1.0f) {
		x = from.x;
		y = from.y;
		z = newZ;
		w = newW;
	}
	inline Vec4f(const Vec3f &from, const float newW = 1.0f) {
		x = from.x;
		y = from.y;
		z = from.z;
		w = newW;
	}
	inline Vec4f(const Vec4f &from) {
		x = from.x;
		y = from.y;
		z = from.z;
		w = from.w;
	}

	static const Vec4f &White;
	static const Vec4f &Black;
	static const Vec4f &Red;
	static const Vec4f &Green;
	static const Vec4f &Blue;
	static const Vec4f &Yellow;
};

//
// Mat4f (4 x 4 float matrix)
//
__declspec(align(16)) union Mat4f {
	struct {
		Vec4f col1;
		Vec4f col2;
		Vec4f col3;
		Vec4f col4;
	};
	struct {
		float elements[4][4];
	};
	float m[16];

	Mat4f(const float d = 1.0f) {
		m[0] = d;
		m[1] = 0;
		m[2] = 0;
		m[3] = 0;

		m[4] = 0;
		m[5] = d;
		m[6] = 0;
		m[7] = 0;

		m[8] = 0;
		m[9] = 0;
		m[10] = d;
		m[11] = 0;

		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = d;
	}

	Mat4f(const Mat4f &other) {
		m[0] = other.m[0];
		m[1] = other.m[1];
		m[2] = other.m[2];
		m[3] = other.m[3];

		m[4] = other.m[4];
		m[5] = other.m[5];
		m[6] = other.m[6];
		m[7] = other.m[7];

		m[8] = other.m[8];
		m[9] = other.m[9];
		m[10] = other.m[10];
		m[11] = other.m[11];

		m[12] = other.m[12];
		m[13] = other.m[13];
		m[14] = other.m[14];
		m[15] = other.m[15];
	}

	Mat4f(const float values[16]) {
		m[0] = values[0];
		m[1] = values[1];
		m[2] = values[2];
		m[3] = values[3];

		m[4] = values[4];
		m[5] = values[5];
		m[6] = values[6];
		m[7] = values[7];

		m[8] = values[8];
		m[9] = values[9];
		m[10] = values[10];
		m[11] = values[11];

		m[12] = values[12];
		m[13] = values[13];
		m[14] = values[14];
		m[15] = values[15];
	}

	Mat4f(const Vec4f &newCol1, const Vec4f &newCol2, const Vec4f &newCol3, const Vec4f &newCol4) {
		col1 = newCol1;
		col2 = newCol2;
		col3 = newCol3;
		col4 = newCol4;
	}

	static inline Mat4f CreateOrtho(const float left, const float right, const float bottom, const float top) {
		Mat4f result = Mat4f(1);
		result.elements[0][0] = 2.0f / (right - left);
		result.elements[1][1] = 2.0f / (top - bottom);
		result.elements[2][2] = -1.0f;
		result.elements[3][0] = -(right + left) / (right - left);
		result.elements[3][1] = -(top + bottom) / (top - bottom);
		return(result);
	}

	static inline Mat4f CreateFrustumLH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar, const bool zeroToOneClipSpace = false) {
		Mat4f result = Mat4f(0.0f);
		result.elements[0][0] = (2.0f * zNear) / (right - left);
		result.elements[1][1] = (2.0f * zNear) / (top - bottom);
		result.elements[2][0] = (right + left) / (right - left);
		result.elements[2][1] = (top + bottom) / (top - bottom);
		result.elements[2][3] = 1.0f;
		if (zeroToOneClipSpace) {
			result.elements[2][2] = zFar / (zFar - zNear);
			result.elements[3][2] = -(zFar * zNear) / (zFar - zNear);
		} else {
			result.elements[2][2] = (zFar + zNear) / (zFar - zNear);
			result.elements[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
		}
		return(result);
	}

	static inline Mat4f CreateFrustumRH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar, const bool zeroToOneClipSpace = false) {
		Mat4f result = Mat4f(0.0f);
		result.elements[0][0] = (2.0f * zNear) / (right - left);
		result.elements[1][1] = (2.0f * zNear) / (top - bottom);
		result.elements[2][0] = (right + left) / (right - left);
		result.elements[2][1] = (top + bottom) / (top - bottom);
		result.elements[2][3] = -1.0f;
		if (zeroToOneClipSpace) {
			result.elements[2][2] = zFar / (zNear - zFar);
			result.elements[3][2] = -(zFar * zNear) / (zFar - zNear);
		} else {
			result.elements[2][2] = -(zFar + zNear) / (zFar - zNear);
			result.elements[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
		}
		return(result);
	}

	static inline Mat4f CreateOrthoLH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar, const bool zeroToOneClipSpace = false) {
		Mat4f result = Mat4f(1);
		result.elements[0][0] = 2.0f / (right - left);
		result.elements[1][1] = 2.0f / (top - bottom);
		result.elements[3][0] = -(right + left) / (right - left);
		result.elements[3][1] = -(top + bottom) / (top - bottom);
		if (zeroToOneClipSpace) {
			result.elements[2][2] = 1.0f / (zFar - zNear);
			result.elements[3][2] = -zNear / (zFar - zNear);
		} else {
			result.elements[2][2] = 2.0f / (zFar - zNear);
			result.elements[3][2] = -(zFar + zNear) / (zFar - zNear);
		}
		return(result);
	}

	static inline Mat4f CreateOrthoRH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar, const bool zeroToOneClipSpace = false) {
		Mat4f result = Mat4f(1);
		result.elements[0][0] = 2.0f / (right - left);
		result.elements[1][1] = 2.0f / (top - bottom);
		result.elements[3][0] = -(right + left) / (right - left);
		result.elements[3][1] = -(top + bottom) / (top - bottom);
		if (zeroToOneClipSpace) {
			result.elements[2][2] = -1.0f / (zFar - zNear);
			result.elements[3][2] = -zNear / (zFar - zNear);
		} else {
			result.elements[2][2] = -2.0f / (zFar - zNear);
			result.elements[3][2] = -(zFar + zNear) / (zFar - zNear);
		}
		return(result);
	}

	static const Mat4f &Identity;
};
