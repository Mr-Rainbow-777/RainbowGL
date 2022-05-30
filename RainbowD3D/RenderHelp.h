#ifndef _RENDER_HELP_
#define _RENDER_HELP_



#include "Vector.h";
#include "Matrix.h";


//---------------------------------------------------------------------
// ���ߺ���
//---------------------------------------------------------------------
template<typename T> inline T Abs(T x) { return (x < 0) ? (-x) : x; }
template<typename T> inline T Max(T x, T y) { return (x < y) ? y : x; }
template<typename T> inline T Min(T x, T y) { return (x > y) ? y : x; }

template<typename T> inline bool NearEqual(T x, T y, T error) {
	return (Abs(x - y) < error);
}

template<typename T> inline T Between(T xmin, T xmax, T x) {
	return Min(Max(xmin, x), xmax);
}

// ��ȡ [0, 1] �ķ�Χ
template<typename T> inline T Saturate(T x) {
	return Between<T>(0, 1, x);
}

// ���ͱ���
typedef Vector<2, float>  Vec2f;
typedef Vector<2, double> Vec2d;
typedef Vector<2, int>    Vec2i;
typedef Vector<3, float>  Vec3f;
typedef Vector<3, double> Vec3d;
typedef Vector<3, int>    Vec3i;
typedef Vector<4, float>  Vec4f;
typedef Vector<4, double> Vec4d;
typedef Vector<4, int>    Vec4i;

typedef Matrix<4, 4, float> Mat4x4f;
typedef Matrix<3, 3, float> Mat3x3f;
typedef Matrix<4, 3, float> Mat4x3f;
typedef Matrix<3, 4, float> Mat3x4f;


//---------------------------------------------------------------------
// 3D ��ѧ����
//---------------------------------------------------------------------


// ʸ��ת������ɫ
inline static uint32_t vector_to_color(const Vec4f& color) {
	uint32_t r = (uint32_t)Between(0, 255, (int)(color.r * 255.0f));
	uint32_t g = (uint32_t)Between(0, 255, (int)(color.g * 255.0f));
	uint32_t b = (uint32_t)Between(0, 255, (int)(color.b * 255.0f));
	uint32_t a = (uint32_t)Between(0, 255, (int)(color.a * 255.0f));
	return (r << 16) | (g << 8) | b | (a << 24);
}

// ʸ��ת��������ɫ
inline static uint32_t vector_to_color(const Vec3f& color) {
	return vector_to_color(color.xyz1());
}

// ������ɫ��ʸ��
inline static Vec4f vector_from_color(uint32_t rgba) {
	Vec4f out;
	out.r = ((rgba >> 16) & 0xff) / 255.0f;
	out.g = ((rgba >> 8) & 0xff) / 255.0f;
	out.b = ((rgba >> 0) & 0xff) / 255.0f;
	out.a = ((rgba >> 24) & 0xff) / 255.0f;
	return out;
}

// matrix set to zero
inline static Mat4x4f matrix_set_zero() {
	Mat4x4f m;
	m.m[0][0] = m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][1] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][2] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = m.m[3][3] = 0.0f;
	return m;
}

// set to identity
inline static Mat4x4f matrix_set_identity() {
	Mat4x4f m;
	m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
	m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	return m;
}

// ƽ�Ʊ任
inline static Mat4x4f matrix_set_translate(float x, float y, float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[3][0] = x;
	m.m[3][1] = y;
	m.m[3][2] = z;
	return m;
}

// ���ű任
inline static Mat4x4f matrix_set_scale(float x, float y, float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[0][0] = x;
	m.m[1][1] = y;
	m.m[2][2] = z;
	return m;
}

// Local2World����
inline static Mat4x4f Model2World_Matrix(const Vec3f& pos,const Vec3f& xAxis, const Vec3f& yAxis, const Vec3f& zAxis) {
	Mat4x4f m;
	m.SetRow(0, Vec4f(xAxis.x, xAxis.y, xAxis.z, 0));
	m.SetRow(1, Vec4f(yAxis.x, yAxis.y, yAxis.z, 0));
	m.SetRow(2, Vec4f(zAxis.x, zAxis.y, zAxis.z, 0));
	m.SetCol(3, Vec4f(pos.x, pos.y, pos.z, 1.0f));
	return m;
}

// ��Ӱ���任����eye/�ӵ�λ�ã�at/�������up/ָ���Ϸ���ʸ��  
//Camera2World����
inline static Mat4x4f matrix_set_lookat(const Vec3f& eye, const Vec3f& at, const Vec3f& up) {
	Vec3f zaxis = vector_normalize(eye-at);
	Vec3f xaxis = vector_normalize(vector_cross(up, zaxis));
	Vec3f yaxis = vector_cross(xaxis, zaxis);
	Mat4x4f m;
	m.SetRow(0, Vec4f(xaxis.x, xaxis.y, xaxis.z, 0));
	m.SetRow(1, Vec4f(yaxis.x, yaxis.y, yaxis.z, 0));
	m.SetRow(2, Vec4f(zaxis.x, zaxis.y, zaxis.z, 0));
	m.SetCol(3, Vec4f(eye.x,eye.y,eye.z, 1.0f));
	return m;
}


//View����  (�ҳ�)
inline static Mat4x4f World2Camera_Matrix(const Vec3f& eye, const Vec3f& at, const Vec3f& up)
{
	Mat4x4f m = matrix_set_lookat(eye, at, up);
	return matrix_invert(m);
}


/// <summary>
/// ͶӰ����
/// </summary>
/// <param name="fovy">����:thera��</param>
/// <param name="zn"></param>
/// <param name="zf"></param>
/// <returns></returns>
inline static Mat4x4f Projection_Matrix(float fovy, float zn, float zf) {
	float scale = 1.0f / (float)tan(fovy * 0.5f);
	Mat4x4f m = matrix_set_zero();
	m.m[0][0] = scale;
	m.m[1][1] = scale;
	m.m[2][2] = zf / (zf - zn);
	m.m[3][2] = -zn * zf / (zf - zn);
	m.m[2][3] = 1;
	return m;
}







#endif // !_RENDER_HELP_

