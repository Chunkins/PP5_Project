#pragma once
#include "LinearPrimitives.h"

Vec4 Vec3::BuildVec4() const
{
	return Vec4(x, y, z, 1);
}

Vec3 Vec3::operator+(const Vec3& _rhs) const
{
	return Vec3(x + _rhs.x, y + _rhs.y, z + _rhs.z);
}

Vec3 Vec3::operator-(const Vec3& _rhs) const
{
	return Vec3(x - _rhs.x, y - _rhs.y, z - _rhs.z);
}

Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

//Mat4x4 Mat4x4::Identity()
//{
//	Mat4x4 ret;
//	for (int i = 0; i < 4; i++)
//		for (int j = 0; j < 4; j++)
//			ret.ee[i][j] = i == j ? 1.0f : 0.0f;
//	return ret;
//}
//
//Mat4x4 Mat4x4::Scaling(const Vec3& _scale)
//{
//	Mat4x4 ret = Identity();
//	ret.m_xx = _scale.x;
//	ret.m_yy = _scale.y;
//	ret.m_zz = _scale.z;
//	return ret;
//}
//
//Mat4x4 Mat4x4::Invert()
//{
//	float det;
//	Mat4x4 ret;
//	ret.e[0] = e[5] * e[10] * e[15] -
//		e[5] * e[11] * e[14] -
//		e[9] * e[6] * e[15] +
//		e[9] * e[7] * e[14] +
//		e[13] * e[6] * e[11] -
//		e[13] * e[7] * e[10];
//
//	ret.e[4] = -e[4] * e[10] * e[15] +
//		e[4] * e[11] * e[14] +
//		e[8] * e[6] * e[15] -
//		e[8] * e[7] * e[14] -
//		e[12] * e[6] * e[11] +
//		e[12] * e[7] * e[10];
//
//	ret.e[8] = e[4] * e[9] * e[15] -
//		e[4] * e[11] * e[13] -
//		e[8] * e[5] * e[15] +
//		e[8] * e[7] * e[13] +
//		e[12] * e[5] * e[11] -
//		e[12] * e[7] * e[9];
//
//	ret.e[12] = -e[4] * e[9] * e[14] +
//		e[4] * e[10] * e[13] +
//		e[8] * e[5] * e[14] -
//		e[8] * e[6] * e[13] -
//		e[12] * e[5] * e[10] +
//		e[12] * e[6] * e[9];
//
//	ret.e[1] = -e[1] * e[10] * e[15] +
//		e[1] * e[11] * e[14] +
//		e[9] * e[2] * e[15] -
//		e[9] * e[3] * e[14] -
//		e[13] * e[2] * e[11] +
//		e[13] * e[3] * e[10];
//
//	ret.e[5] = e[0] * e[10] * e[15] -
//		e[0] * e[11] * e[14] -
//		e[8] * e[2] * e[15] +
//		e[8] * e[3] * e[14] +
//		e[12] * e[2] * e[11] -
//		e[12] * e[3] * e[10];
//
//	ret.e[9] = -e[0] * e[9] * e[15] +
//		e[0] * e[11] * e[13] +
//		e[8] * e[1] * e[15] -
//		e[8] * e[3] * e[13] -
//		e[12] * e[1] * e[11] +
//		e[12] * e[3] * e[9];
//
//	ret.e[13] = e[0] * e[9] * e[14] -
//		e[0] * e[10] * e[13] -
//		e[8] * e[1] * e[14] +
//		e[8] * e[2] * e[13] +
//		e[12] * e[1] * e[10] -
//		e[12] * e[2] * e[9];
//
//	ret.e[2] = e[1] * e[6] * e[15] -
//		e[1] * e[7] * e[14] -
//		e[5] * e[2] * e[15] +
//		e[5] * e[3] * e[14] +
//		e[13] * e[2] * e[7] -
//		e[13] * e[3] * e[6];
//
//	ret.e[6] = -e[0] * e[6] * e[15] +
//		e[0] * e[7] * e[14] +
//		e[4] * e[2] * e[15] -
//		e[4] * e[3] * e[14] -
//		e[12] * e[2] * e[7] +
//		e[12] * e[3] * e[6];
//
//	ret.e[10] = e[0] * e[5] * e[15] -
//		e[0] * e[7] * e[13] -
//		e[4] * e[1] * e[15] +
//		e[4] * e[3] * e[13] +
//		e[12] * e[1] * e[7] -
//		e[12] * e[3] * e[5];
//
//	ret.e[14] = -e[0] * e[5] * e[14] +
//		e[0] * e[6] * e[13] +
//		e[4] * e[1] * e[14] -
//		e[4] * e[2] * e[13] -
//		e[12] * e[1] * e[6] +
//		e[12] * e[2] * e[5];
//
//	ret.e[3] = -e[1] * e[6] * e[11] +
//		e[1] * e[7] * e[10] +
//		e[5] * e[2] * e[11] -
//		e[5] * e[3] * e[10] -
//		e[9] * e[2] * e[7] +
//		e[9] * e[3] * e[6];
//
//	ret.e[7] = e[0] * e[6] * e[11] -
//		e[0] * e[7] * e[10] -
//		e[4] * e[2] * e[11] +
//		e[4] * e[3] * e[10] +
//		e[8] * e[2] * e[7] -
//		e[8] * e[3] * e[6];
//
//	ret.e[11] = -e[0] * e[5] * e[11] +
//		e[0] * e[7] * e[9] +
//		e[4] * e[1] * e[11] -
//		e[4] * e[3] * e[9] -
//		e[8] * e[1] * e[7] +
//		e[8] * e[3] * e[5];
//
//	ret.e[15] = e[0] * e[5] * e[10] -
//		e[0] * e[6] * e[9] -
//		e[4] * e[1] * e[10] +
//		e[4] * e[2] * e[9] +
//		e[8] * e[1] * e[6] -
//		e[8] * e[2] * e[5];
//
//	det = e[0] * ret.e[0] + e[1] * ret.e[4] + e[2] * ret.e[8] + e[3] * ret.e[12];
//	det = 1.0f / det;
//	for (int i = 0; i < 16; i++)
//		ret.e[i] = ret.e[i] * det;
//	return ret;
//}

//Mat4x4 operator*(const Mat4x4& _rhs, const Mat4x4& _lhs)
//{
//	Mat4x4 ret;
//	ZMem(ret);
//	for (int i = 0; i < 4; i++) {
//		for (int j = 0; j < 4; j++) {
//			for (int k = 0; k < 4; k++) {
//				ret.ee[i][j] += _lhs.ee[i][k] * _rhs.ee[k][j];
//			}
//		}
//	}
//	return ret;
//}

//Mat4x4 Mat4x4::operator+(const Mat4x4& _rhs) const
//{
//	Mat4x4 ret;
//	for (int i = 0; i < 16; i++)
//		ret.e[i] = e[i] + _rhs.e[i];
//	return ret;
//}
//
//Mat4x4 Mat4x4::operator-(const Mat4x4& _rhs) const
//{
//	Mat4x4 ret;
//	for (int i = 0; i < 16; i++)
//		ret.e[i] = e[i] - _rhs.e[i];
//	return ret;
//}
//
//Mat4x4 Mat4x4::operator=(const FbxMatrix & _rhs) const
//{
//	Mat4x4 ret;
//	for (int i = 0; i < 4; i++)
//		for (int j = 0; j < 4; j++)
//			ret.ee[i][j] = (float)_rhs.Get(i, j);
//	return ret;
//}
//
//Mat4x4 Mat4x4::operator=(const FbxAMatrix & _rhs) const
//{
//	Mat4x4 ret = Identity();
//	for (int i = 0; i < 4; i++) {
//		for (int j = 0; j < 4; j++) {
//			ret.ee[i][j] = (float)_rhs.Get(i, j);
//		}
//	}
//	return ret;
//}

void Mat4x4::SaveMatrix(const FbxAMatrix & _rhs)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ee[i][j] = (float)_rhs.Get(i, j);
		}
	}
}

//Mat4x4 Mat4x4::Transpose()
//{
//	Mat4x4 ret;
//	for (int i = 0; i < 4; i++)
//	{
//		for (int j = 0; j < 4; j++)
//		{
//			ret.ee[i][j] = ee[j][i];
//		}
//	}
//	return ret;
//}