#ifndef VECTOR_H_
#define VECTOR_H_

template<int T>
union vec
{
	real32 e[T];

	real32&
	operator[](int index)
	{
		return e[index];
	}
	const real32&
	operator[](int index) const
	{
		return e[index];
	}
};

template<>
union vec<2>
{
	struct
	{
		real32 x, y;
	};
	real32 e[2];
};

template<>
union vec<3>
{
	struct
	{
		real32 x, y, z;
	};
	struct
	{
		real32 r, g, b;
	};
	struct
	{
		vec<2> xy;
		real32 pad_;
	};
	real32 e[3];
};

template<>
union vec<4>
{
	struct
	{
		real32 x, y, z, w;
	};
	struct
	{
		real32 r, g, b, a;
	};
	struct
	{
		vec<3> xyz;
		real32 pad_;
	};
	real32 e[4];
};

typedef vec<2> vec2_t;
typedef vec<3> vec3_t;
typedef vec<4> vec4_t;

#if 0
//!!! DANGER !!!
template <int T> vec<T>
Vec(real32 e, ...)
{
    vec<T> result;
    va_list args;
    va_start(args, e);

    result.e[0] = e;
    for (int i = 1; i < T; ++i) {
        result.e[i] = (real32)va_arg(args, real64);
    }

    va_end(args);

    return result;
}
#endif

template<int T, class... Args>
vec<T>
Vec(Args... args)
{
	const int length = sizeof...(args);
	static_assert(length == T, "");
	vec<T> result = { static_cast<real32>(args)... };
	return result;
}

template<int T>
vec<T>
operator*(real32 scalar, vec<T> v)
{
	vec<T> result;
	for (int i = 0; i < T; ++i) {
		result.e[i] = scalar * v.e[i];
	}

	return result;
}

template<int T>
vec<T>
operator*(vec<T> v, real32 scalar)
{
	vec<T> result = scalar * v;
	return result;
}

template<int T>
vec<T>&
operator*=(vec<T>& v, real32 scalar)
{
	v = scalar * v;
	return v;
}

template<int T>
vec<T>
operator-(vec<T> v)
{
	vec<T> result;
	for (int i = 0; i < T; ++i) {
		result.e[i] = -v.e[i];
	}

	return result;
}

template<int T>
vec<T>
operator+(vec<T> A, vec<T> B)
{
	vec<T> result;
	for (int i = 0; i < T; ++i) {
		result.e[i] = A.e[i] + B.e[i];
	}

	return result;
}

template<int T>
vec<T>&
operator+=(vec<T>& A, vec<T> B)
{
	A = A + B;
	return A;
}

template<int T>
vec<T>
operator-(vec<T> A, vec<T> B)
{
	vec<T> result;
	for (int i = 0; i < T; ++i) {
		result.e[i] = A.e[i] - B.e[i];
	}

	return result;
}

template<int T>
vec<T>&
operator-=(vec<T>& A, vec<T> B)
{
	A = A - B;
	return A;
}

template<int T>
vec<T>
Hadamard(vec<T> A, vec<T> B)
{
	vec<T> result;
	for (int i = 0; i < T; ++i) {
		result.e[i] = A.e[i] * B.e[i];
	}

	return result;
}

template<int T>
real32
Dot(vec<T> A, vec<T> B)
{
	real32 result = 0.f;
	for (int i = 0; i < T; ++i) {
		result += (A.e[i] * B.e[i]);
	}

	return result;
}

template<int T>
real32
LengthSq(vec<T> v)
{
	real32 result = Dot(v, v);
	return result;
}

template<int T>
real32
Length(vec<T> v)
{
	real32 result = Sqrt(LengthSq(v));
	return result;
}

#endif	// VECTOR_H_
