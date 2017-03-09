#pragma once
#include "stdafx.h"
#include "fbxsdk.h"
#include <DirectXMath.h>
class Vec4
{
public:

	float x, y, z, w;
	
	Vec4() : x(0), y(0), z(0), w(1) {}
	Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

class Vec3
{
public:
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	Vec4 BuildVec4() const;

	Vec3 operator+(const Vec3& _rhs) const;
	Vec3 operator-(const Vec3& _rhs) const;
	Vec3 operator-() const;
};

struct Mat4x4
{
	union
	{
		struct
		{
			float e[16];
		};

		struct
		{
			float ee[4][4];
		};

		struct
		{
			float m_11, m_12, m_13, m_14, m_21, m_22, m_23, m_24, m_31, m_32, m_33, m_34, m_41, m_42, m_43, m_44;
		};

		struct
		{
			float m_xx, m_xy, m_xz, m_xw, m_yx, m_yy, m_yz, m_yw, m_zx, m_zy, m_zz, m_zw, m_wx, m_wy, m_wz, m_ww;
		};
	};

	//static Mat4x4 Identity();
	//static Mat4x4 Scaling(const Vec3& _scale);

	//Mat4x4 Invert();

	//friend Mat4x4 operator*(const Mat4x4& _rhs, const Mat4x4& _lhs);
	//Mat4x4 operator+(const Mat4x4& _rhs) const;
	//Mat4x4 operator-(const Mat4x4& _rhs) const;
	//Mat4x4 operator=(const FbxMatrix& _rhs) const;
	//Mat4x4 operator=(const FbxAMatrix& _rhs) const;
	void SaveMatrix(const FbxAMatrix& _rhs);
	//Mat4x4 Transpose();
};