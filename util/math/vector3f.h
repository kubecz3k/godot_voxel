#ifndef ZYLANN_VECTOR3F_H
#define ZYLANN_VECTOR3F_H

#include "../errors.h"
#include "funcs.h"

namespace zylann {

// 32-bit float precision 3D vector.
// Because Godot's `Vector3` uses `real_t`, so when `real_t` is `double` it forces some things to use double-precision
// vectors while they dont need that amount of precision. This is also a problem for some third-party libraries
// that do not support `double` as a result.
struct Vector3f {
	static const unsigned int AXIS_X = 0;
	static const unsigned int AXIS_Y = 1;
	static const unsigned int AXIS_Z = 2;
	static const unsigned int AXIS_COUNT = 3;

	union {
		struct {
			float x;
			float y;
			float z;
		};
		float coords[3];
	};

	Vector3f() : x(0), y(0), z(0) {}

	// It is recommended to use `explicit` because otherwise it would open the door to plenty of implicit conversions
	// which would make many cases ambiguous.
	explicit Vector3f(float p_v) : x(p_v), y(p_v), z(p_v) {}

	Vector3f(float p_x, float p_y, float p_z) : x(p_x), y(p_y), z(p_z) {}

	inline float length_squared() const {
		return x * x + y * y + z * z;
	}

	inline float length() const {
		return Math::sqrt(length_squared());
	}

	inline float distance_squared_to(const Vector3f &p_to) const {
		return (p_to - *this).length_squared();
	}

	inline Vector3f cross(const Vector3f &p_with) const {
		const Vector3f ret( //
				(y * p_with.z) - (z * p_with.y), //
				(z * p_with.x) - (x * p_with.z), //
				(x * p_with.y) - (y * p_with.x));
		return ret;
	}

	inline float dot(const Vector3f &p_with) const {
		return x * p_with.x + y * p_with.y + z * p_with.z;
	}

	inline void normalize() {
		const float lengthsq = length_squared();
		if (lengthsq == 0) {
			x = y = z = 0;
		} else {
			const float length = Math::sqrt(lengthsq);
			x /= length;
			y /= length;
			z /= length;
		}
	}

	inline Vector3f normalized() const {
		Vector3f v = *this;
		v.normalize();
		return v;
	}

	inline const float &operator[](const unsigned int p_axis) const {
#ifdef DEBUG_ENABLED
		ZN_ASSERT(p_axis < AXIS_COUNT);
#endif
		return coords[p_axis];
	}

	inline float &operator[](const unsigned int p_axis) {
#ifdef DEBUG_ENABLED
		ZN_ASSERT(p_axis < AXIS_COUNT);
#endif
		return coords[p_axis];
	}

	inline Vector3f &operator+=(const Vector3f &p_v) {
		x += p_v.x;
		y += p_v.y;
		z += p_v.z;
		return *this;
	}

	inline Vector3f operator+(const Vector3f &p_v) const {
		return Vector3f(x + p_v.x, y + p_v.y, z + p_v.z);
	}

	inline Vector3f &operator-=(const Vector3f &p_v) {
		x -= p_v.x;
		y -= p_v.y;
		z -= p_v.z;
		return *this;
	}

	inline Vector3f operator-(const Vector3f &p_v) const {
		return Vector3f(x - p_v.x, y - p_v.y, z - p_v.z);
	}

	inline Vector3f &operator*=(const Vector3f &p_v) {
		x *= p_v.x;
		y *= p_v.y;
		z *= p_v.z;
		return *this;
	}

	inline Vector3f operator*(const Vector3f &p_v) const {
		return Vector3f(x * p_v.x, y * p_v.y, z * p_v.z);
	}

	inline Vector3f &operator/=(const Vector3f &p_v) {
		x /= p_v.x;
		y /= p_v.y;
		z /= p_v.z;
		return *this;
	}

	inline Vector3f operator/(const Vector3f &p_v) const {
		return Vector3f(x / p_v.x, y / p_v.y, z / p_v.z);
	}

	inline Vector3f &operator*=(const float p_scalar) {
		x *= p_scalar;
		y *= p_scalar;
		z *= p_scalar;
		return *this;
	}

	inline Vector3f operator*(const float p_scalar) const {
		return Vector3f(x * p_scalar, y * p_scalar, z * p_scalar);
	}

	inline Vector3f &operator/=(const float p_scalar) {
		x /= p_scalar;
		y /= p_scalar;
		z /= p_scalar;
		return *this;
	}

	inline Vector3f operator/(const float p_scalar) const {
		return Vector3f(x / p_scalar, y / p_scalar, z / p_scalar);
	}

	inline Vector3f operator-() const {
		return Vector3f(-x, -y, -z);
	}

	inline bool operator==(const Vector3f &p_v) const {
		return x == p_v.x && y == p_v.y && z == p_v.z;
	}

	inline bool operator!=(const Vector3f &p_v) const {
		return x != p_v.x || y != p_v.y || z != p_v.z;
	}

	inline bool operator<(const Vector3f &p_v) const {
		if (x == p_v.x) {
			if (y == p_v.y) {
				return z < p_v.z;
			}
			return y < p_v.y;
		}
		return x < p_v.x;
	}
};

inline Vector3f operator*(float p_scalar, const Vector3f &v) {
	return v * p_scalar;
}

namespace math {

// Trilinear interpolation between corner values of a unit-sized cube.
// `v***` arguments are corner values named as `vXYZ`, where a coordinate is 0 or 1 on the cube.
// Coordinates of `p` are in 0..1, but are not clamped so extrapolation is possible.
//
//      6---------------7
//     /|              /|
//    / |             / |
//   5---------------4  |
//   |  |            |  |
//   |  |            |  |
//   |  |            |  |
//   |  2------------|--3        Y
//   | /             | /         | Z
//   |/              |/          |/
//   1---------------0      X----o
//
// p000, p100, p101, p001, p010, p110, p111, p011
template <typename T>
inline T interpolate_trilinear(const T v000, const T v100, const T v101, const T v001, const T v010, const T v110,
		const T v111, const T v011, Vector3f p) {
	//
	const T v00 = v000 + p.x * (v100 - v000);
	const T v10 = v010 + p.x * (v110 - v010);
	const T v01 = v001 + p.x * (v101 - v001);
	const T v11 = v011 + p.x * (v111 - v011);

	const T v0 = v00 + p.y * (v10 - v00);
	const T v1 = v01 + p.y * (v11 - v01);

	const T v = v0 + p.z * (v1 - v0);

	return v;
}

inline Vector3f min(const Vector3f a, const Vector3f b) {
	return Vector3f(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

inline Vector3f max(const Vector3f a, const Vector3f b) {
	return Vector3f(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

inline Vector3f floor(const Vector3f a) {
	return Vector3f(Math::floor(a.x), Math::floor(a.y), Math::floor(a.z));
}

inline Vector3f ceil(const Vector3f a) {
	return Vector3f(Math::ceil(a.x), Math::ceil(a.y), Math::ceil(a.z));
}

inline Vector3f lerp(const Vector3f a, const Vector3f b, const float t) {
	return Vector3f(Math::lerp(a.x, b.x, t), Math::lerp(a.y, b.y, t), Math::lerp(a.z, b.z, t));
}

} // namespace math
} // namespace zylann

#endif // ZYLANN_VECTOR3F_H
