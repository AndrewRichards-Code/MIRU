#pragma once
#include <math.h>

#undef far
#undef near

class Vec3
{
public:
	float x, y, z;
};
class Vec4
{
public:
	float x, y, z, w;

	Vec4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w) {}

	Vec4 operator+ (const Vec4& other) const
	{
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4 operator* (float a) const
	{
		return Vec4(a * x, a * y, a * z, a * w);
	}
};
class Mat4
{
public:
	float a, b, c, d,
		e, f, g, h,
		i, j, k, l,
		m, n, o, p;

	Mat4()
		:a(0), b(0), c(0), d(0), e(0), f(0), g(0), h(0),
		i(0), j(0), k(0), l(0), m(0), n(0), o(0), p(0) {}

	Mat4(float a, float b, float c, float d, float e, float f, float g, float h,
		float i, float j, float k, float l, float m, float n, float o, float p)
		: a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h),
		i(i), j(j), k(k), l(l), m(m), n(n), o(o), p(p) {}

	Mat4(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d)
		: a(a.x), b(a.y), c(a.z), d(a.w), e(b.x), f(b.y), g(b.z), h(b.w),
		i(c.x), j(c.y), k(c.z), l(c.w), m(d.x), n(d.y), o(d.z), p(d.w) {}

	Mat4(float diagonal)
		: a(diagonal), b(0), c(0), d(0), e(0), f(diagonal), g(0), h(0),
		i(0), j(0), k(diagonal), l(0), m(0), n(0), o(0), p(diagonal) {}

	static Mat4 Identity()
	{
		return Mat4(1);
	}

	void Transpose()
	{
		a = a;
		f = f;
		k = k;
		p = p;

		float temp_b = b;
		float temp_c = c;
		float temp_d = d;
		float temp_g = g;
		float temp_h = h;
		float temp_l = l;

		b = e;
		c = i;
		d = m;
		g = j;
		h = n;
		l = o;

		e = temp_b;
		i = temp_c;
		m = temp_d;
		j = temp_g;
		n = temp_h;
		o = temp_l;
	}

	static Mat4 Perspective(double fov, float aspectRatio, float near, float far)
	{
		return Mat4((1 / (aspectRatio * static_cast<float>(tan(fov / 2)))), (0), (0), (0),
			(0), (1 / static_cast<float>(tan(fov / 2))), (0), (0),
			(0), (0), -((far + near) / (far - near)), -((2 * far * near) / (far - near)),
			(0), (0), (-1), (0));
	}

	static Mat4 Translation(const Vec3& translation)
	{
		Mat4 result(1);
		result.d = translation.x;
		result.h = translation.y;
		result.l = translation.z;
		return result;
	}

	static Mat4 Rotation(double angle, const Vec3& axis)
	{
		Mat4 result(1);
		float c_angle = static_cast<float>(cos(angle));
		float s_angle = static_cast<float>(sin(angle));
		float omcos = static_cast<float>(1 - c_angle);

		float x = axis.x;
		float y = axis.y;
		float z = axis.z;

		result.a = x * x * omcos + c_angle;
		result.e = x * y * omcos + z * s_angle;
		result.i = x * z * omcos - y * s_angle;
		result.m = 0;

		result.b = y * x * omcos - z * s_angle;
		result.f = y * y * omcos + c_angle;
		result.j = y * z * omcos + x * s_angle;
		result.n = 0;

		result.c = z * x * omcos + y * s_angle;
		result.g = z * y * omcos - x * s_angle;
		result.k = z * z * omcos + c_angle;
		result.o = 0;

		result.d = 0;
		result.h = 0;
		result.l = 0;
		result.p = 1;

		return result;
	}

	static Mat4 Scale(const Vec3& scale)
	{
		Mat4 result(1);
		result.a = scale.x;
		result.f = scale.y;
		result.k = scale.z;
		return result;
	}

	Mat4 operator*(const Mat4& input) const
	{
		Vec4 input_i(input.a, input.e, input.i, input.m);
		Vec4 input_j(input.b, input.f, input.j, input.n);
		Vec4 input_k(input.c, input.g, input.k, input.o);
		Vec4 input_l(input.d, input.h, input.l, input.p);
		Vec4 output_i = *this * input_i;
		Vec4 output_j = *this * input_j;
		Vec4 output_k = *this * input_k;
		Vec4 output_l = *this * input_l;
		Mat4 output(output_i, output_j, output_k, output_l);
		output.Transpose();
		return output;
	}

	Vec4 operator*(const Vec4& input) const
	{
		float x = input.x;
		float y = input.y;
		float z = input.z;
		float w = input.w;
		Vec4 transform_i(a, e, i, m);
		Vec4 transform_j(b, f, j, n);
		Vec4 transform_k(c, g, k, o);
		Vec4 transform_l(d, h, l, p);
		Vec4 output(transform_i * x + transform_j * y + transform_k * z + transform_l * w);
		return output;
	}
};
