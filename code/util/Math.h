/*
  THIS IS A FORK OF HANDMADE MATH

  HandmadeMath.h v2.0.0

  This is a single header file with a bunch of useful types and functions for
  games and graphics. Consider it a lightweight alternative to GLM that works
  both C and C++.

  =============================================================================
  CONFIG
  =============================================================================

  By default, all angles in Handmade Math are specified in radians. However, it
  can be configured to use degrees or turns instead. Use one of the following
  defines to specify the default unit for angles:

    #define HANDMADE_MATH_USE_RADIANS
    #define HANDMADE_MATH_USE_DEGREES
    #define HANDMADE_MATH_USE_TURNS

  Regardless of the default angle, you can use the following functions to
  specify an angle in a particular unit:

    HMM_AngleRad(radians)
    HMM_AngleDeg(degrees)
    HMM_AngleTurn(turns)

  The definitions of these functions change depending on the default unit.

  -----------------------------------------------------------------------------

  Handmade Math ships with SSE (SIMD) implementations of several common
  operations. To disable the use of SSE intrinsics, you must define
  HANDMADE_MATH_NO_SSE before including this file:

    #define HANDMADE_MATH_NO_SSE
    #include "HandmadeMath.h"

  -----------------------------------------------------------------------------

  To use Handmade Math without the C runtime library, you must provide your own
  implementations of basic math functions. Otherwise, HandmadeMath.h will use
  the runtime library implementation of these functions.

  Define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS and provide your own
  implementations of HMM_SINF, HMM_COSF, HMM_TANF, HMM_ACOSF, and HMM_SQRTF
  before including HandmadeMath.h, like so:

    #define HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS
    #define HMM_SINF MySinF
    #define HMM_COSF MyCosF
    #define HMM_TANF MyTanF
    #define HMM_ACOSF MyACosF
    #define HMM_SQRTF MySqrtF
    #include "HandmadeMath.h"
  
  By default, it is assumed that your math functions take radians. To use
  different units, you must define HMM_ANGLE_USER_TO_INTERNAL and
  HMM_ANGLE_INTERNAL_TO_USER. For example, if you want to use degrees in your
  code but your math functions use turns:

    #define HMM_ANGLE_USER_TO_INTERNAL(a) ((a)*HMM_DegToTurn)
    #define HMM_ANGLE_INTERNAL_TO_USER(a) ((a)*HMM_TurnToDeg)

  =============================================================================
  
  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  =============================================================================

  CREDITS

  Originally written by Zakary Strange.

  Functionality:
   Zakary Strange (strangezak@protonmail.com && @strangezak)
   Matt Mascarenhas (@miblo_)
   Aleph
   FieryDrake (@fierydrake)
   Gingerbill (@TheGingerBill)
   Ben Visness (@bvisness)
   Trinton Bullard (@Peliex_Dev)
   @AntonDan
   Logan Forman (@dev_dwarf)

  Fixes:
   Jeroen van Rijn (@J_vanRijn)
   Kiljacken (@Kiljacken)
   Insofaras (@insofaras)
   Daniel Gibson (@DanielGibson)
*/

#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H

// Dummy macros for when test framework is not present.
#ifndef COVERAGE
# define COVERAGE(a, b)
#endif

#ifndef ASSERT_COVERED
# define ASSERT_COVERED(a)
#endif

/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef HANDMADE_MATH__USE_SSE" to check for SSE support below this block! */
#ifndef HANDMADE_MATH_NO_SSE
# ifdef _MSC_VER /* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#  if defined(_M_AMD64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#   define HANDMADE_MATH__USE_SSE 1
#  endif
# else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#  ifdef __SSE__ /* they #define __SSE__ if it's supported */
#   define HANDMADE_MATH__USE_SSE 1
#  endif /*  __SSE__ */
# endif /* not _MSC_VER */
#endif /* #ifndef HANDMADE_MATH_NO_SSE */

#if (!defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
# define HANDMADE_MATH__USE_C11_GENERICS 1
#endif

#ifdef HANDMADE_MATH__USE_SSE
# include <xmmintrin.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wfloat-equal"
# if (defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 8)) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wmissing-braces"
# endif
# ifdef __clang__
#  pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

#if defined(__GNUC__) || defined(__clang__)
# define HMM_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
# define HMM_DEPRECATED(msg) __declspec(deprecated(msg))
#else
# define HMM_DEPRECATED(msg)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(HANDMADE_MATH_USE_DEGREES) \
    && !defined(HANDMADE_MATH_USE_TURNS) \
    && !defined(HANDMADE_MATH_USE_RADIANS)
# define HANDMADE_MATH_USE_RADIANS
#endif
    
#define HMM_PI 3.14159265358979323846
#define HMM_PI32 3.14159265359f
#define HMM_DEG180 180.0
#define HMM_DEG18032 180.0f
#define HMM_TURNHALF 0.5
#define HMM_TURNHALF32 0.5f
#define HMM_RadToDeg ((float)(HMM_DEG180/HMM_PI))
#define HMM_RadToTurn ((float)(HMM_TURNHALF/HMM_PI))
#define HMM_DegToRad ((float)(HMM_PI/HMM_DEG180))
#define HMM_DegToTurn ((float)(HMM_TURNHALF/HMM_DEG180))
#define HMM_TurnToRad ((float)(HMM_PI/HMM_TURNHALF))
#define HMM_TurnToDeg ((float)(HMM_DEG180/HMM_TURNHALF))

#if defined(HANDMADE_MATH_USE_RADIANS)
# define HMM_AngleRad(a) (a)
# define HMM_AngleDeg(a) ((a)*HMM_DegToRad)
# define HMM_AngleTurn(a) ((a)*HMM_TurnToRad)
#elif defined(HANDMADE_MATH_USE_DEGREES)
# define HMM_AngleRad(a) ((a)*HMM_RadToDeg)
# define HMM_AngleDeg(a) (a)
# define HMM_AngleTurn(a) ((a)*HMM_TurnToDeg)
#elif defined(HANDMADE_MATH_USE_TURNS)
# define HMM_AngleRad(a) ((a)*HMM_RadToTurn)
# define HMM_AngleDeg(a) ((a)*HMM_DegToTurn)
# define HMM_AngleTurn(a) (a)
#endif

#if !defined(HANDMADE_MATH_PROVIDE_MATH_FUNCTIONS)
# include <math.h>
# define HMM_SINF sinf
# define HMM_COSF cosf
# define HMM_TANF tanf
# define HMM_SQRTF sqrtf
# define HMM_ACOSF acosf
#endif

#if !defined(HMM_ANGLE_USER_TO_INTERNAL)
# define HMM_ANGLE_USER_TO_INTERNAL(a) (DegreeToRad(a))
#endif

#if !defined(HMM_ANGLE_INTERNAL_TO_USER)
# if defined(HANDMADE_MATH_USE_RADIANS)
#  define HMM_ANGLE_INTERNAL_TO_USER(a) (a) 
# elif defined(HANDMADE_MATH_USE_DEGREES)
#  define HMM_ANGLE_INTERNAL_TO_USER(a) ((a)*HMM_RadToDeg)
# elif defined(HANDMADE_MATH_USE_TURNS)
#  define HMM_ANGLE_INTERNAL_TO_USER(a) ((a)*HMM_RadToTurn)
# endif
#endif

#define Min(a, b) ((a) > (b) ? (b) : (a))
#define Max(a, b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define Modulous(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define Square(x) ((x) * (x))

typedef union vec2
{
    struct
    {
        float X, Y;
    };

    struct
    {
        float U, V;
    };

    struct
    {
        float Left, Right;
    };

    struct
    {
        float Width, Height;
    };

    float Elements[2];

#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }

#endif
} vec2;

typedef union ivec2
{
    struct 
    {
        int32 X;
        int32 Y;
    };
    struct 
    {
        int32 Width;
        int32 Height;
    };
}ivec2;

internal inline vec2
v2Cast(ivec2 A)
{
    vec2 Result = {};
    
    Result.X = real32(A.X);
    Result.Y = real32(A.Y);
    
    return(Result);
}

internal inline ivec2
iv2Cast(vec2 A)
{
    ivec2 Result = {};

    Result.X = int32(A.X);
    Result.Y = int32(A.Y);

    return(Result);
}


typedef union vec3
{
    struct
    {
        float X, Y, Z;
    };

    struct
    {
        float U, V, W;
    };

    struct
    {
        float R, G, B;
    };

    struct
    {
        vec2 XY;
        float _Ignored0;
    };

    struct
    {
        float _Ignored1;
        vec2 YZ;
    };

    struct
    {
        vec2 UV;
        float _Ignored2;
    };

    struct
    {
        float _Ignored3;
        vec2 VW;
    };

    float Elements[3];

#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
} vec3;

internal inline vec3
v2Expand(vec2 A, real32 B)
{
    vec3 Result = {};
    Result.X = A.X;
    Result.Y = A.Y;
    Result.Z = B;

    return(Result);
}

typedef union
{
    int32 Elements[3];
    
    struct
    {
        int32 x;
        int32 y;
        int32 z;
    };
    
    struct
    {
        int32 Width;
        int32 Height;
        int32 Channels;
    };
}ivec3;

typedef union vec4
{
    struct
    {
        union
        {
            vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };

        float W;
    };
    struct
    {
        union
        {
            vec3 RGB;
            struct
            {
                float R, G, B;
            };
        };

        float A;
    };

    struct
    {
        vec2 XY;
        float _Ignored0;
        float _Ignored1;
    };

    struct
    {
        float _Ignored2;
        vec2 YZ;
        float _Ignored3;
    };

    struct
    {
        float _Ignored4;
        float _Ignored5;
        vec2 ZW;
    };

    float Elements[4];

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSE;
#endif

#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
} vec4;

typedef union
{
    int32 Elements[4];
    struct
    {
        int32 X;
        int32 Y;
        int32 Width;
        int32 Height;
    };
}ivec4;

typedef union mat2
{
    float Elements[2][2];
    vec2 Columns[2];

#ifdef __cplusplus
    inline vec2 &operator[](const int &Index)
    {
        return Columns[Index];
    }
#endif
} mat2;
    
typedef union mat3
{
    float Elements[3][3];
    vec3 Columns[3];

#ifdef __cplusplus
    inline vec3 &operator[](const int &Index)
    {
        return Columns[Index];
    }
#endif
} mat3;

typedef union mat4
{
    float Elements[4][4];
    vec4 Columns[4];

#ifdef __cplusplus
    inline vec4 &operator[](const int &Index)
    {
        return Columns[Index];
    }
#endif
} mat4;

typedef union quat
{
    struct
    {
        union
        {
            vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };

        float W;
    };

    float Elements[4];

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSE;
#endif
} quat;

typedef signed int HMM_Bool;

/*
 * Angle unit conversion functions
 */
static inline float DegreeToRad(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle;
#elif defined(HANDMADE_MATH_USE_DEGREES) 
    float Result = Angle * HMM_DegToRad;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle * HMM_TurnToRad;
#endif
    
    return Result;
}

static inline float RadiansToDeg(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle * HMM_RadToDeg;
#elif defined(HANDMADE_MATH_USE_DEGREES) 
    float Result = Angle;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle * HMM_TurnToDeg;
#endif
    
    return Result;
}

static inline float RadianToTurn(float Angle)
{
#if defined(HANDMADE_MATH_USE_RADIANS)
    float Result = Angle * HMM_RadToTurn;
#elif defined(HANDMADE_MATH_USE_DEGREES) 
    float Result = Angle * HMM_DegToTurn;
#elif defined(HANDMADE_MATH_USE_TURNS)
    float Result = Angle;
#endif
    
    return Result;
}

/*
 * Floating-point math functions
 */

COVERAGE(SinF, 1)
static inline float SinF(float Angle)
{
    ASSERT_COVERED(SinF);
    return HMM_SINF(HMM_ANGLE_USER_TO_INTERNAL(Angle));
}

COVERAGE(CosF, 1)
static inline float CosF(float Angle)
{
    ASSERT_COVERED(CosF);
    return HMM_COSF(HMM_ANGLE_USER_TO_INTERNAL(Angle));
}

COVERAGE(TanF, 1)
static inline float TanF(float Angle)
{
    ASSERT_COVERED(TanF);
    return HMM_TANF(HMM_ANGLE_USER_TO_INTERNAL(Angle));
}

COVERAGE(ACosF, 1)
static inline float ACosF(float Arg)
{
    ASSERT_COVERED(ACosF);
    return HMM_ANGLE_INTERNAL_TO_USER(HMM_ACOSF(Arg));
}

COVERAGE(SqrtF, 1)
static inline float SqrtF(float Float)
{
    ASSERT_COVERED(SqrtF);

    float Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Float);
    __m128 Out = _mm_sqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = HMM_SQRTF(Float);
#endif

    return Result;
}

COVERAGE(InvSqrtf, 1)
static inline float InvSqrtf(float Float)
{
    ASSERT_COVERED(InvSqrtf);

    float Result;

    Result = 1.0f/SqrtF(Float);

    return Result;
}


/*
 * Utility functions
 */

COVERAGE(Lerp, 1)
static inline float Lerp(float A, float Time, float B)
{
    ASSERT_COVERED(Lerp);
    return (1.0f - Time) * A + Time * B;
}

COVERAGE(Clamp, 1)
static inline float Clamp(float Min, float Value, float Max)
{
    ASSERT_COVERED(Clamp);

    float Result = Value;

    if (Result < Min)
    {
        Result = Min;
    }

    if (Result > Max)
    {
        Result = Max;
    }

    return Result;
}


/*
 * Vector initialization
 */

COVERAGE(v2Create, 1)
static inline vec2 v2Create(float X, float Y)
{
    ASSERT_COVERED(v2Create);

    vec2 Result;
    Result.X = X;
    Result.Y = Y;

    return Result;
}

COVERAGE(v3Create, 1)
static inline vec3 v3Create(float X, float Y, float Z)
{
    ASSERT_COVERED(v3Create);

    vec3 Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

COVERAGE(v4Create, 1)
static inline vec4 v4Create(float X, float Y, float Z, float W)
{
    ASSERT_COVERED(v4Create);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif

    return Result;
}

COVERAGE(v3Expand, 1)
static inline vec4 v3Expand(vec3 Vector, float W)
{
    ASSERT_COVERED(v3Expand);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(Vector.X, Vector.Y, Vector.Z, W);
#else
    Result.XYZ = Vector;
    Result.W = W;
#endif

    return Result;
}


/*
 * Binary vector operations
 */

COVERAGE(v2Add, 1)
static inline vec2 v2Add(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2Add);

    vec2 Result;
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;

    return Result;
}

COVERAGE(v3Add, 1)
static inline vec3 v3Add(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Add);

    vec3 Result;
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;

    return Result;
}

COVERAGE(v4Add, 1)
static inline vec4 v4Add(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4Add);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif

    return Result;
}

COVERAGE(v2Subtract, 1)
static inline vec2 v2Subtract(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2Subtract);

    vec2 Result;
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;

    return Result;
}

COVERAGE(v3Subtract, 1)
static inline vec3 v3Subtract(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Subtract);

    vec3 Result;
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;

    return Result;
}

COVERAGE(v4Subtract, 1)
static inline vec4 v4Subtract(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4Subtract);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif

    return Result;
}

COVERAGE(v2Multiply, 1)
static inline vec2 v2Multiply(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2Multiply);

    vec2 Result;
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;

    return Result;
}

COVERAGE(v2Scale, 1)
static inline vec2 v2Scale(vec2 Left, float Right)
{
    ASSERT_COVERED(v2Scale);

    vec2 Result;
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;

    return Result;
}

COVERAGE(v3Multiply, 1)
static inline vec3 v3Multiply(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Multiply);

    vec3 Result;
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;

    return Result;
}

COVERAGE(v3Scale, 1)
static inline vec3 v3Scale(vec3 Left, float Right)
{
    ASSERT_COVERED(v3Scale);

    vec3 Result;
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;

    return Result;
}

COVERAGE(v4Multiply, 1)
static inline vec4 v4Multiply(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4Multiply);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_mul_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    Result.W = Left.W * Right.W;
#endif

    return Result;
}

COVERAGE(v4Scale, 1)
static inline vec4 v4Scale(vec4 Left, float Right)
{
    ASSERT_COVERED(v4Scale);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    Result.W = Left.W * Right;
#endif

    return Result;
}

COVERAGE(v2Divide, 1)
static inline vec2 v2Divide(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2Divide);

    vec2 Result;
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;

    return Result;
}

COVERAGE(v2Reduce, 1)
static inline vec2 v2Reduce(vec2 Left, float Right)
{
    ASSERT_COVERED(v2Reduce);

    vec2 Result;
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;

    return Result;
}

COVERAGE(v3Divide, 1)
static inline vec3 v3Divide(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Divide);

    vec3 Result;
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;

    return Result;
}

COVERAGE(v3Reduce, 1)
static inline vec3 v3Reduce(vec3 Left, float Right)
{
    ASSERT_COVERED(v3Reduce);

    vec3 Result;
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;

    return Result;
}

COVERAGE(v4Divide, 1)
static inline vec4 v4Divide(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4Divide);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_div_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    Result.W = Left.W / Right.W;
#endif

    return Result;
}

COVERAGE(v4Reduce, 1)
static inline vec4 v4Reduce(vec4 Left, float Right)
{
    ASSERT_COVERED(v4Reduce);

    vec4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    Result.W = Left.W / Right;
#endif

    return Result;
}

COVERAGE(v2Equal, 1)
static inline HMM_Bool v2Equal(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2Equal);
    return Left.X == Right.X && Left.Y == Right.Y;
}

COVERAGE(v3Equal, 1)
static inline HMM_Bool v3Equal(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Equal);
    return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z;
}

COVERAGE(v4Equal, 1)
static inline HMM_Bool v4Equal(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4Equal);
    return Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z && Left.W == Right.W;
}

COVERAGE(v2Dot, 1)
static inline float v2Dot(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2Dot);
    return (Left.X * Right.X) + (Left.Y * Right.Y);
}

COVERAGE(v3Dot, 1)
static inline float v3Dot(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Dot);
    return (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z);
}

COVERAGE(v4Dot, 1)
static inline float v4Dot(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4Dot);

    float Result;

    // NOTE(zak): IN the future if we wanna check what version SSE is support
    // we can use _mm_dp_ps (4.3) but for now we will use the old way.
    // Or a r = _mm_mul_ps(v1, v2), r = _mm_hadd_ps(r, r), r = _mm_hadd_ps(r, r) for SSE3
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = ((Left.X * Right.X) + (Left.Z * Right.Z)) + ((Left.Y * Right.Y) + (Left.W * Right.W));
#endif

    return Result;
}

COVERAGE(v3Cross, 1)
static inline vec3 v3Cross(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3Cross);

    vec3 Result;
    Result.X = (Left.Y * Right.Z) - (Left.Z * Right.Y);
    Result.Y = (Left.Z * Right.X) - (Left.X * Right.Z);
    Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X);

    return Result;
}


/*
 * Unary vector operations
 */

COVERAGE(v2LengthSquared, 1)
static inline float v2LengthSquared(vec2 A)
{
    ASSERT_COVERED(v2LengthSquared);
    return v2Dot(A, A);
}

COVERAGE(v3LengthSquared, 1)
static inline float v3LengthSquared(vec3 A)
{
    ASSERT_COVERED(v3LengthSquared);
    return v3Dot(A, A);
}

COVERAGE(v4LengthSquared, 1)
static inline float v4LengthSquared(vec4 A)
{
    ASSERT_COVERED(v4LengthSquared);
    return v4Dot(A, A);
}

COVERAGE(v2Length, 1)
static inline float v2Length(vec2 A)
{
    ASSERT_COVERED(v2Length);
    return SqrtF(v2LengthSquared(A));
}

COVERAGE(v3Length, 1)
static inline float v3Length(vec3 A)
{
    ASSERT_COVERED(v3Length);
    return SqrtF(v3LengthSquared(A));
}

COVERAGE(v4Length, 1)
static inline float v4Length(vec4 A)
{
    ASSERT_COVERED(v4Length);
    return SqrtF(v4LengthSquared(A));
}

COVERAGE(v2Normalize, 1)
static inline vec2 v2Normalize(vec2 A)
{
    ASSERT_COVERED(v2Normalize);
    return v2Scale(A, InvSqrtf(v2Dot(A, A)));
}

COVERAGE(v3Normalize, 1)
static inline vec3 v3Normalize(vec3 A)
{
    ASSERT_COVERED(v3Normalize);
    return v3Scale(A, InvSqrtf(v3Dot(A, A)));
}

COVERAGE(v4Normalize, 1)
static inline vec4 v4Normalize(vec4 A)
{
    ASSERT_COVERED(v4Normalize);
    return v4Scale(A, InvSqrtf(v4Dot(A, A)));
}

/*
 * Utility vector functions
 */

COVERAGE(v2Lerp, 1)
static inline vec2 v2Lerp(vec2 A, float Time, vec2 B) 
{
    ASSERT_COVERED(v2Lerp);
    return v2Add(v2Scale(A, 1.0f - Time), v2Scale(B, Time));
}

COVERAGE(v3Lerp, 1)
static inline vec3 v3Lerp(vec3 A, float Time, vec3 B) 
{
    ASSERT_COVERED(v3Lerp);
    return v3Add(v3Scale(A, 1.0f - Time), v3Scale(B, Time));
}

COVERAGE(v4Lerp, 1)
static inline vec4 v4Lerp(vec4 A, float Time, vec4 B) 
{
    ASSERT_COVERED(v4Lerp);
    return v4Add(v4Scale(A, 1.0f - Time), v4Scale(B, Time));
}

/*
 * SSE stuff
 */

COVERAGE(mat4MakeTransform, 1)
static inline vec4 mat4MakeTransform(vec4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4MakeTransform);

    vec4 Result;
#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x00), Right.Columns[0].SSE);
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x55), Right.Columns[1].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xaa), Right.Columns[2].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xff), Right.Columns[3].SSE));
#else
    Result.X = Left.Elements[0] * Right.Columns[0].X;
    Result.Y = Left.Elements[0] * Right.Columns[0].Y;
    Result.Z = Left.Elements[0] * Right.Columns[0].Z;
    Result.W = Left.Elements[0] * Right.Columns[0].W;

    Result.X += Left.Elements[1] * Right.Columns[1].X;
    Result.Y += Left.Elements[1] * Right.Columns[1].Y;
    Result.Z += Left.Elements[1] * Right.Columns[1].Z;
    Result.W += Left.Elements[1] * Right.Columns[1].W;

    Result.X += Left.Elements[2] * Right.Columns[2].X;
    Result.Y += Left.Elements[2] * Right.Columns[2].Y;
    Result.Z += Left.Elements[2] * Right.Columns[2].Z;
    Result.W += Left.Elements[2] * Right.Columns[2].W;

    Result.X += Left.Elements[3] * Right.Columns[3].X;
    Result.Y += Left.Elements[3] * Right.Columns[3].Y;
    Result.Z += Left.Elements[3] * Right.Columns[3].Z;
    Result.W += Left.Elements[3] * Right.Columns[3].W;
#endif

    return Result;
}

/*
 * 2x2 Matrices
 */

COVERAGE(mat2Create, 1)
static inline mat2 mat2Create(void)
{
    ASSERT_COVERED(mat2Create);
    mat2 Result = {0};
    return Result;
}

COVERAGE(mat2CreateD, 1)
static inline mat2 mat2CreateD(float Diagonal)
{
    ASSERT_COVERED(mat2CreateD);
    
    mat2 Result = {0};
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;

    return Result;
}

COVERAGE(mat2Transpose, 1)
static inline mat2 mat2Transpose(mat2 Matrix)
{
    ASSERT_COVERED(mat2Transpose);
    
    mat2 Result = Matrix;

    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    
    return Result;
}

COVERAGE(mat2Add, 1)
static inline mat2 mat2Add(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2Add);
    
    mat2 Result;

    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
   
    return Result;    
}

COVERAGE(mat2Subtract, 1)
static inline mat2 mat2Subtract(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2Subtract);
    
    mat2 Result;

    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    
    return Result;
}

COVERAGE(mat2Transform, 1)
static inline vec2 mat2Transform(mat2 Matrix, vec2 Vector)
{
    ASSERT_COVERED(mat2Transform);
    
    vec2 Result;

    Result.X = Vector.Elements[0] * Matrix.Columns[0].X;
    Result.Y = Vector.Elements[0] * Matrix.Columns[0].Y;

    Result.X += Vector.Elements[1] * Matrix.Columns[1].X;
    Result.Y += Vector.Elements[1] * Matrix.Columns[1].Y;

    return Result;    
}

COVERAGE(mat2Multiply, 1)
static inline mat2 mat2Multiply(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2Multiply);
    
    mat2 Result;
    Result.Columns[0] = mat2Transform(Left, Right.Columns[0]);
    Result.Columns[1] = mat2Transform(Left, Right.Columns[1]);

    return Result;    
}

COVERAGE(mat2ScaleMatrix, 1)
static inline mat2 mat2ScaleMatrix(mat2 Matrix, float Scalar)
{
    ASSERT_COVERED(mat2ScaleMatrix);
    
    mat2 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    
    return Result;
}

COVERAGE(mat2Reduce, 1)
static inline mat2 mat2Reduce(mat2 Matrix, float Scalar)
{
    ASSERT_COVERED(mat2Reduce);
    
    mat2 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;

    return Result;
}

COVERAGE(mat2Determinate, 1)
static inline float mat2Determinate(mat2 Matrix) 
{
    ASSERT_COVERED(mat2Determinate);
    return Matrix.Elements[0][0]*Matrix.Elements[1][1] - Matrix.Elements[0][1]*Matrix.Elements[1][0];
}


COVERAGE(mat2InvDeterminate, 1)
static inline mat2 mat2InvDeterminate(mat2 Matrix) 
{
    ASSERT_COVERED(mat2InvDeterminate);

    mat2 Result;
    float InvDeterminant = 1.0f / mat2Determinate(Matrix);
    Result.Elements[0][0] = InvDeterminant * +Matrix.Elements[1][1];
    Result.Elements[1][1] = InvDeterminant * +Matrix.Elements[0][0];
    Result.Elements[0][1] = InvDeterminant * -Matrix.Elements[0][1];
    Result.Elements[1][0] = InvDeterminant * -Matrix.Elements[1][0];

    return Result;
}

/*
 * 3x3 Matrices
 */

COVERAGE(mat3Create, 1)
static inline mat3 mat3Create(void)
{
    ASSERT_COVERED(mat3Create);
    mat3 Result = {0};
    return Result;
}

COVERAGE(mat3CreateD, 1)
static inline mat3 mat3CreateD(float Diagonal)
{
    ASSERT_COVERED(mat3CreateD);
    
    mat3 Result = {0};
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;

    return Result;
}

COVERAGE(mat3Transpose, 1)
static inline mat3 mat3Transpose(mat3 Matrix)
{
    ASSERT_COVERED(mat3Transpose);

    mat3 Result = Matrix;

    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[0][2] = Matrix.Elements[2][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    Result.Elements[1][2] = Matrix.Elements[2][1];
    Result.Elements[2][1] = Matrix.Elements[1][2];
    Result.Elements[2][0] = Matrix.Elements[0][2];
    
    return Result;
}

COVERAGE(mat3Add, 1)
static inline mat3 mat3Add(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3Add);
    
    mat3 Result;
    
    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] + Right.Elements[0][2];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] + Right.Elements[1][2];
    Result.Elements[2][0] = Left.Elements[2][0] + Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] + Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] + Right.Elements[2][2];

    return Result;    
}

COVERAGE(mat3Subtract, 1)
static inline mat3 mat3Subtract(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3Subtract);

    mat3 Result;

    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] - Right.Elements[0][2];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] - Right.Elements[1][2];
    Result.Elements[2][0] = Left.Elements[2][0] - Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] - Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] - Right.Elements[2][2];

    return Result;
}

COVERAGE(mat3Transform, 1)
static inline vec3 mat3Transform(mat3 Matrix, vec3 Vector)
{
    ASSERT_COVERED(mat3Transform);
    
    vec3 Result;

    Result.X = Vector.Elements[0] * Matrix.Columns[0].X;
    Result.Y = Vector.Elements[0] * Matrix.Columns[0].Y;
    Result.Z = Vector.Elements[0] * Matrix.Columns[0].Z;

    Result.X += Vector.Elements[1] * Matrix.Columns[1].X;
    Result.Y += Vector.Elements[1] * Matrix.Columns[1].Y;
    Result.Z += Vector.Elements[1] * Matrix.Columns[1].Z;

    Result.X += Vector.Elements[2] * Matrix.Columns[2].X;
    Result.Y += Vector.Elements[2] * Matrix.Columns[2].Y;
    Result.Z += Vector.Elements[2] * Matrix.Columns[2].Z;
    
    return Result;    
}

COVERAGE(mat3Multiply, 1)
static inline mat3 mat3Multiply(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3Multiply);

    mat3 Result;
    Result.Columns[0] = mat3Transform(Left, Right.Columns[0]);
    Result.Columns[1] = mat3Transform(Left, Right.Columns[1]);
    Result.Columns[2] = mat3Transform(Left, Right.Columns[2]);

    return Result;    
}

COVERAGE(mat3ScaleMatrix, 1)
static inline mat3 mat3ScaleMatrix(mat3 Matrix, float Scalar)
{
    ASSERT_COVERED(mat3ScaleMatrix);

    mat3 Result;

    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] * Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] * Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] * Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] * Scalar;

    return Result;            
}

COVERAGE(HMM_DivM3, 1)
static inline mat3 mat3Reduce(mat3 Matrix, float Scalar)
{
    ASSERT_COVERED(HMM_DivM3);

    mat3 Result;
    
    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] / Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] / Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] / Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] / Scalar;

    return Result;                    
}

COVERAGE(mat3Determinate, 1)
static inline float mat3Determinate(mat3 Matrix) 
{
    ASSERT_COVERED(mat3Determinate);

    mat3 Cross;
    Cross.Columns[0] = v3Cross(Matrix.Columns[1], Matrix.Columns[2]);
    Cross.Columns[1] = v3Cross(Matrix.Columns[2], Matrix.Columns[0]);
    Cross.Columns[2] = v3Cross(Matrix.Columns[0], Matrix.Columns[1]);

    return v3Dot(Cross.Columns[2], Matrix.Columns[2]);
}

COVERAGE(mat3InvDeterminate, 1)
static inline mat3 mat3InvDeterminate(mat3 Matrix) 
{
    ASSERT_COVERED(mat3InvDeterminate);

    mat3 Cross;
    Cross.Columns[0] = v3Cross(Matrix.Columns[1], Matrix.Columns[2]);
    Cross.Columns[1] = v3Cross(Matrix.Columns[2], Matrix.Columns[0]);
    Cross.Columns[2] = v3Cross(Matrix.Columns[0], Matrix.Columns[1]);

    float InvDeterminant = 1.0f / v3Dot(Cross.Columns[2], Matrix.Columns[2]);

    mat3 Result;
    Result.Columns[0] = v3Scale(Cross.Columns[0], InvDeterminant);
    Result.Columns[1] = v3Scale(Cross.Columns[1], InvDeterminant);
    Result.Columns[2] = v3Scale(Cross.Columns[2], InvDeterminant);

    return mat3Transpose(Result);
}

/*
 * 4x4 Matrices
 */

COVERAGE(mat4Create, 1)
static inline mat4 mat4Create(void)
{
    ASSERT_COVERED(mat4Create);
    mat4 Result = {0};
    return Result;
}

COVERAGE(mat4Identity, 1)
static inline mat4 mat4Identity(float Diagonal)
{
    ASSERT_COVERED(mat4Identity);

    mat4 Result = {0};
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;
    Result.Elements[3][3] = Diagonal;

    return Result;
}

COVERAGE(mat4Transpose, 1)
static inline mat4 mat4Transpose(mat4 Matrix)
{
    ASSERT_COVERED(mat4Transpose);

    mat4 Result = Matrix;
#ifdef HANDMADE_MATH__USE_SSE
    _MM_TRANSPOSE4_PS(Result.Columns[0].SSE, Result.Columns[1].SSE, Result.Columns[2].SSE, Result.Columns[3].SSE);
#else
    Result.Elements[0][1] = Matrix.Elements[1][0];
    Result.Elements[0][2] = Matrix.Elements[2][0];
    Result.Elements[0][3] = Matrix.Elements[3][0];
    Result.Elements[1][0] = Matrix.Elements[0][1];
    Result.Elements[1][2] = Matrix.Elements[2][1];
    Result.Elements[1][3] = Matrix.Elements[3][1];
    Result.Elements[2][1] = Matrix.Elements[1][2];
    Result.Elements[2][0] = Matrix.Elements[0][2];
    Result.Elements[2][3] = Matrix.Elements[3][2];
    Result.Elements[3][1] = Matrix.Elements[1][3];
    Result.Elements[3][2] = Matrix.Elements[2][3];
    Result.Elements[3][0] = Matrix.Elements[0][3];
#endif

    return Result;
}

COVERAGE(mat4Add, 1)
static inline mat4 mat4Add(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4Add);

    mat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0].SSE = _mm_add_ps(Left.Columns[0].SSE, Right.Columns[0].SSE);
    Result.Columns[1].SSE = _mm_add_ps(Left.Columns[1].SSE, Right.Columns[1].SSE);
    Result.Columns[2].SSE = _mm_add_ps(Left.Columns[2].SSE, Right.Columns[2].SSE);
    Result.Columns[3].SSE = _mm_add_ps(Left.Columns[3].SSE, Right.Columns[3].SSE);
#else
    Result.Elements[0][0] = Left.Elements[0][0] + Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] + Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] + Right.Elements[0][2];
    Result.Elements[0][3] = Left.Elements[0][3] + Right.Elements[0][3];
    Result.Elements[1][0] = Left.Elements[1][0] + Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] + Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] + Right.Elements[1][2];
    Result.Elements[1][3] = Left.Elements[1][3] + Right.Elements[1][3];
    Result.Elements[2][0] = Left.Elements[2][0] + Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] + Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] + Right.Elements[2][2];
    Result.Elements[2][3] = Left.Elements[2][3] + Right.Elements[2][3];
    Result.Elements[3][0] = Left.Elements[3][0] + Right.Elements[3][0];
    Result.Elements[3][1] = Left.Elements[3][1] + Right.Elements[3][1];
    Result.Elements[3][2] = Left.Elements[3][2] + Right.Elements[3][2];
    Result.Elements[3][3] = Left.Elements[3][3] + Right.Elements[3][3];
#endif

    return Result;
}

COVERAGE(mat4Subtract, 1)
static inline mat4 mat4Subtract(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4Subtract);

    mat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0].SSE = _mm_sub_ps(Left.Columns[0].SSE, Right.Columns[0].SSE);
    Result.Columns[1].SSE = _mm_sub_ps(Left.Columns[1].SSE, Right.Columns[1].SSE);
    Result.Columns[2].SSE = _mm_sub_ps(Left.Columns[2].SSE, Right.Columns[2].SSE);
    Result.Columns[3].SSE = _mm_sub_ps(Left.Columns[3].SSE, Right.Columns[3].SSE);
#else
    Result.Elements[0][0] = Left.Elements[0][0] - Right.Elements[0][0];
    Result.Elements[0][1] = Left.Elements[0][1] - Right.Elements[0][1];
    Result.Elements[0][2] = Left.Elements[0][2] - Right.Elements[0][2];
    Result.Elements[0][3] = Left.Elements[0][3] - Right.Elements[0][3];
    Result.Elements[1][0] = Left.Elements[1][0] - Right.Elements[1][0];
    Result.Elements[1][1] = Left.Elements[1][1] - Right.Elements[1][1];
    Result.Elements[1][2] = Left.Elements[1][2] - Right.Elements[1][2];
    Result.Elements[1][3] = Left.Elements[1][3] - Right.Elements[1][3];
    Result.Elements[2][0] = Left.Elements[2][0] - Right.Elements[2][0];
    Result.Elements[2][1] = Left.Elements[2][1] - Right.Elements[2][1];
    Result.Elements[2][2] = Left.Elements[2][2] - Right.Elements[2][2];
    Result.Elements[2][3] = Left.Elements[2][3] - Right.Elements[2][3];
    Result.Elements[3][0] = Left.Elements[3][0] - Right.Elements[3][0];
    Result.Elements[3][1] = Left.Elements[3][1] - Right.Elements[3][1];
    Result.Elements[3][2] = Left.Elements[3][2] - Right.Elements[3][2];
    Result.Elements[3][3] = Left.Elements[3][3] - Right.Elements[3][3];
#endif
 
    return Result;
}

COVERAGE(mat4Multiply, 1)
static inline mat4 mat4Multiply(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4Multiply);

    mat4 Result;
    Result.Columns[0] = mat4MakeTransform(Right.Columns[0], Left);
    Result.Columns[1] = mat4MakeTransform(Right.Columns[1], Left);
    Result.Columns[2] = mat4MakeTransform(Right.Columns[2], Left);
    Result.Columns[3] = mat4MakeTransform(Right.Columns[3], Left);

    return Result;
}

COVERAGE(mat4ScaleMatrix, 1)
static inline mat4 mat4ScaleMatrix(mat4 Matrix, float Scalar)
{
    ASSERT_COVERED(mat4ScaleMatrix);

    mat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_mul_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_mul_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_mul_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_mul_ps(Matrix.Columns[3].SSE, SSEScalar);
#else
    Result.Elements[0][0] = Matrix.Elements[0][0] * Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] * Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] * Scalar;
    Result.Elements[0][3] = Matrix.Elements[0][3] * Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] * Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] * Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] * Scalar;
    Result.Elements[1][3] = Matrix.Elements[1][3] * Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] * Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] * Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] * Scalar;
    Result.Elements[2][3] = Matrix.Elements[2][3] * Scalar;
    Result.Elements[3][0] = Matrix.Elements[3][0] * Scalar;
    Result.Elements[3][1] = Matrix.Elements[3][1] * Scalar;
    Result.Elements[3][2] = Matrix.Elements[3][2] * Scalar;
    Result.Elements[3][3] = Matrix.Elements[3][3] * Scalar;
#endif

    return Result;
}

COVERAGE(mat4Transform, 1)
static inline vec4 mat4Transform(mat4 Matrix, vec4 Vector)
{
    ASSERT_COVERED(mat4Transform);
    return mat4MakeTransform(Vector, Matrix);
}

COVERAGE(mat4Reduce, 1)
static inline mat4 mat4Reduce(mat4 Matrix, float Scalar)
{
    ASSERT_COVERED(mat4Reduce);

    mat4 Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_div_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_div_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_div_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_div_ps(Matrix.Columns[3].SSE, SSEScalar);
#else
    Result.Elements[0][0] = Matrix.Elements[0][0] / Scalar;
    Result.Elements[0][1] = Matrix.Elements[0][1] / Scalar;
    Result.Elements[0][2] = Matrix.Elements[0][2] / Scalar;
    Result.Elements[0][3] = Matrix.Elements[0][3] / Scalar;
    Result.Elements[1][0] = Matrix.Elements[1][0] / Scalar;
    Result.Elements[1][1] = Matrix.Elements[1][1] / Scalar;
    Result.Elements[1][2] = Matrix.Elements[1][2] / Scalar;
    Result.Elements[1][3] = Matrix.Elements[1][3] / Scalar;
    Result.Elements[2][0] = Matrix.Elements[2][0] / Scalar;
    Result.Elements[2][1] = Matrix.Elements[2][1] / Scalar;
    Result.Elements[2][2] = Matrix.Elements[2][2] / Scalar;
    Result.Elements[2][3] = Matrix.Elements[2][3] / Scalar;
    Result.Elements[3][0] = Matrix.Elements[3][0] / Scalar;
    Result.Elements[3][1] = Matrix.Elements[3][1] / Scalar;
    Result.Elements[3][2] = Matrix.Elements[3][2] / Scalar;
    Result.Elements[3][3] = Matrix.Elements[3][3] / Scalar;
#endif

    return Result;
}

COVERAGE(mat4Determinate, 1)
static inline float mat4Determinate(mat4 Matrix) 
{
    ASSERT_COVERED(mat4Determinate);

    vec3 C01 = v3Cross(Matrix.Columns[0].XYZ, Matrix.Columns[1].XYZ);
    vec3 C23 = v3Cross(Matrix.Columns[2].XYZ, Matrix.Columns[3].XYZ);
    vec3 B10 = v3Subtract(v3Scale(Matrix.Columns[0].XYZ, Matrix.Columns[1].W), v3Scale(Matrix.Columns[1].XYZ, Matrix.Columns[0].W));
    vec3 B32 = v3Subtract(v3Scale(Matrix.Columns[2].XYZ, Matrix.Columns[3].W), v3Scale(Matrix.Columns[3].XYZ, Matrix.Columns[2].W));
    
    return v3Dot(C01, B32) + v3Dot(C23, B10);
}

COVERAGE(mat4Inverse, 1)
// Returns a general-purpose inverse of an mat4. Note that special-purpose inverses of many transformations
// are available and will be more efficient.
static inline mat4 mat4Inverse(mat4 Matrix) 
{
    ASSERT_COVERED(mat4Inverse);

    vec3 C01 = v3Cross(Matrix.Columns[0].XYZ, Matrix.Columns[1].XYZ);
    vec3 C23 = v3Cross(Matrix.Columns[2].XYZ, Matrix.Columns[3].XYZ);
    vec3 B10 = v3Subtract(v3Scale(Matrix.Columns[0].XYZ, Matrix.Columns[1].W), v3Scale(Matrix.Columns[1].XYZ, Matrix.Columns[0].W));
    vec3 B32 = v3Subtract(v3Scale(Matrix.Columns[2].XYZ, Matrix.Columns[3].W), v3Scale(Matrix.Columns[3].XYZ, Matrix.Columns[2].W));
    
    float InvDeterminant = 1.0f / (v3Dot(C01, B32) + v3Dot(C23, B10));
    C01 = v3Scale(C01, InvDeterminant);
    C23 = v3Scale(C23, InvDeterminant);
    B10 = v3Scale(B10, InvDeterminant);
    B32 = v3Scale(B32, InvDeterminant);

    mat4 Result;
    Result.Columns[0] = v3Expand(v3Add(v3Cross(Matrix.Columns[1].XYZ, B32), v3Scale(C23, Matrix.Columns[1].W)), -v3Dot(Matrix.Columns[1].XYZ, C23));
    Result.Columns[1] = v3Expand(v3Subtract(v3Cross(B32, Matrix.Columns[0].XYZ), v3Scale(C23, Matrix.Columns[0].W)), +v3Dot(Matrix.Columns[0].XYZ, C23));
    Result.Columns[2] = v3Expand(v3Add(v3Cross(Matrix.Columns[3].XYZ, B10), v3Scale(C01, Matrix.Columns[3].W)), -v3Dot(Matrix.Columns[3].XYZ, C01));
    Result.Columns[3] = v3Expand(v3Subtract(v3Cross(B10, Matrix.Columns[2].XYZ), v3Scale(C01, Matrix.Columns[2].W)), +v3Dot(Matrix.Columns[2].XYZ, C01));
        
    return mat4Transpose(Result);
}

/*
 * Common graphics transformations
 */

COVERAGE(mat4RHGLOrtho, 1)
// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4RHGLOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(mat4RHGLOrtho);

    mat4 Result = {0};

    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 2.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;

    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Near + Far) / (Near - Far);

    return Result;
}

COVERAGE(mat4RHDXOrtho, 1)
// Produces a right-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4RHDXOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(mat4RHDXOrtho);

    mat4 Result = {0};

    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 1.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;

    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Near) / (Near - Far);

    return Result;
}

COVERAGE(mat4LHGLOrtho, 1)
// Produces a left-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4LHGLOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(mat4LHGLOrtho);

    mat4 Result = mat4RHGLOrtho(Left, Right, Bottom, Top, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    
    return Result;
}

COVERAGE(mat4LHDXOrtho, 1)
// Produces a left-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline mat4 mat4LHDXOrtho(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    ASSERT_COVERED(mat4LHDXOrtho);

    mat4 Result = mat4RHDXOrtho(Left, Right, Bottom, Top, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    
    return Result;
}

COVERAGE(mat4OrthoInverse, 1)
// Returns an inverse for the given orthographic projection matrix. Works for all orthographic
// projection matrices, regardless of handedness or NDC convention.
static inline mat4 mat4OrthoInverse(mat4 OrthoMatrix)
{
    ASSERT_COVERED(mat4OrthoInverse);

    mat4 Result = {0};
    Result.Elements[0][0] = 1.0f / OrthoMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / OrthoMatrix.Elements[1][1];
    Result.Elements[2][2] = 1.0f / OrthoMatrix.Elements[2][2];
    Result.Elements[3][3] = 1.0f;
    
    Result.Elements[3][0] = -OrthoMatrix.Elements[3][0] * Result.Elements[0][0];
    Result.Elements[3][1] = -OrthoMatrix.Elements[3][1] * Result.Elements[1][1];
    Result.Elements[3][2] = -OrthoMatrix.Elements[3][2] * Result.Elements[2][2];

    return Result;
}

COVERAGE(mat4RHGLMakePerspective, 1)
static inline mat4 mat4RHGLMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{
    ASSERT_COVERED(mat4RHGLMakePerspective);

    mat4 Result = {0};

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

    float Cotangent = 1.0f / TanF(FOV / 2.0f);
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;

    Result.Elements[2][2] = (Near + Far) / (Near - Far);
    Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);
    
    return Result;
}

COVERAGE(mat4RLDXMakePerspective, 1)
static inline mat4 mat4RLDXMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{
    ASSERT_COVERED(mat4RLDXMakePerspective);

    mat4 Result = {0};

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

    float Cotangent = 1.0f / TanF(FOV / 2.0f);
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;

    Result.Elements[2][2] = (Far) / (Near - Far);
    Result.Elements[3][2] = (Near * Far) / (Near - Far);

    return Result;
}

COVERAGE(mat4LHGLMakePerspective, 1)
static inline mat4 mat4LHGLMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{ 
    ASSERT_COVERED(mat4LHGLMakePerspective);

    mat4 Result = mat4RHGLMakePerspective(FOV, AspectRatio, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    Result.Elements[2][3] = -Result.Elements[2][3];
    
    return Result;
}

COVERAGE(mat4LHDXMakePerspective, 1)
static inline mat4 mat4LHDXMakePerspective(float FOV, float AspectRatio, float Near, float Far)
{ 
    ASSERT_COVERED(mat4LHDXMakePerspective);

    mat4 Result = mat4RLDXMakePerspective(FOV, AspectRatio, Near, Far);
    Result.Elements[2][2] = -Result.Elements[2][2];
    Result.Elements[2][3] = -Result.Elements[2][3];
    
    return Result;
}

COVERAGE(mat4RHInversePerp, 1)
static inline mat4 mat4RHInversePerp(mat4 PerspectiveMatrix)
{
    ASSERT_COVERED(mat4RHInversePerp);

    mat4 Result = {0};
    Result.Elements[0][0] = 1.0f / PerspectiveMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / PerspectiveMatrix.Elements[1][1];
    Result.Elements[2][2] = 0.0f;

    Result.Elements[2][3] = 1.0f / PerspectiveMatrix.Elements[3][2];
    Result.Elements[3][3] = PerspectiveMatrix.Elements[2][2] * Result.Elements[2][3];
    Result.Elements[3][2] = PerspectiveMatrix.Elements[2][3];

    return Result;
}

COVERAGE(mat4LHInversePerp, 1)
static inline mat4 mat4LHInversePerp(mat4 PerspectiveMatrix)
{
    ASSERT_COVERED(mat4LHInversePerp);

    mat4 Result = {0};
    Result.Elements[0][0] = 1.0f / PerspectiveMatrix.Elements[0][0];
    Result.Elements[1][1] = 1.0f / PerspectiveMatrix.Elements[1][1];
    Result.Elements[2][2] = 0.0f;

    Result.Elements[2][3] = 1.0f / PerspectiveMatrix.Elements[3][2];
    Result.Elements[3][3] = PerspectiveMatrix.Elements[2][2] * -Result.Elements[2][3];
    Result.Elements[3][2] = PerspectiveMatrix.Elements[2][3];

    return Result;
}

COVERAGE(mat4Translate, 1)
static inline mat4 mat4Translate(vec3 Translation)
{
    ASSERT_COVERED(mat4Translate);

    mat4 Result = mat4Identity(1.0f);
    Result.Elements[3][0] = Translation.X;
    Result.Elements[3][1] = Translation.Y;
    Result.Elements[3][2] = Translation.Z;

    return Result;
}

COVERAGE(mat4InvTranslate, 1)
static inline mat4 mat4InvTranslate(mat4 TranslationMatrix)
{
    ASSERT_COVERED(mat4InvTranslate);

    mat4 Result = TranslationMatrix;
    Result.Elements[3][0] = -Result.Elements[3][0];
    Result.Elements[3][1] = -Result.Elements[3][1];
    Result.Elements[3][2] = -Result.Elements[3][2];

    return Result;
}

COVERAGE(mat4RHRotation, 1)
static inline mat4 mat4RHRotation(float Angle, vec3 Axis)
{
    ASSERT_COVERED(mat4RHRotation);

    mat4 Result = mat4Identity(1.0f);

    Axis = v3Normalize(Axis);

    float SinTheta = SinF(Angle);
    float CosTheta = CosF(Angle);
    float CosValue = 1.0f - CosTheta;

    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);

    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);

    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;

    return Result;
}

COVERAGE(mat4LHRotation, 1)
static inline mat4 mat4LHRotation(float Angle, vec3 Axis)
{
    ASSERT_COVERED(mat4LHRotation);
    /* NOTE(lcf): Matrix will be inverse/transpose of RH. */
    return mat4RHRotation(-Angle, Axis);
}

COVERAGE(mat4InverseRotate, 1)
static inline mat4 mat4InverseRotate(mat4 RotationMatrix)
{
    ASSERT_COVERED(mat4InverseRotate);
    return mat4Transpose(RotationMatrix);
}

COVERAGE(mat4MakeScale, 1)
static inline mat4 mat4MakeScale(vec3 Scale)
{
    ASSERT_COVERED(mat4MakeScale);

    mat4 Result = mat4Identity(1.0f);
    Result.Elements[0][0] = Scale.X;
    Result.Elements[1][1] = Scale.Y;
    Result.Elements[2][2] = Scale.Z;

    return Result;
}

COVERAGE(mat4InvScale, 1)
static inline mat4 mat4InvScale(mat4 ScaleMatrix) 
{
    ASSERT_COVERED(mat4InvScale);

    mat4 Result = ScaleMatrix;
    Result.Elements[0][0] = 1.0f / Result.Elements[0][0];
    Result.Elements[1][1] = 1.0f / Result.Elements[1][1];
    Result.Elements[2][2] = 1.0f / Result.Elements[2][2];

    return Result;
}

static inline mat4 _HMM_LookAt(vec3 F,  vec3 S, vec3 U,  vec3 Eye)
{
    mat4 Result;

    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;
    Result.Elements[0][3] = 0.0f;

    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;
    Result.Elements[1][3] = 0.0f;

    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;
    Result.Elements[2][3] = 0.0f;

    Result.Elements[3][0] = -v3Dot(S, Eye);
    Result.Elements[3][1] = -v3Dot(U, Eye);
    Result.Elements[3][2] = v3Dot(F, Eye);
    Result.Elements[3][3] = 1.0f;

    return Result;
}


COVERAGE(HMM_LookAt_RH, 1)
static inline mat4 HMM_LookAt_RH(vec3 Eye, vec3 Center, vec3 Up)
{
    ASSERT_COVERED(HMM_LookAt_RH);

    vec3 F = v3Normalize(v3Subtract(Center, Eye));
    vec3 S = v3Normalize(v3Cross(F, Up));
    vec3 U = v3Cross(S, F);

    return _HMM_LookAt(F, S, U, Eye);
}

COVERAGE(HMM_LookAt_LH, 1)
static inline mat4 HMM_LookAt_LH(vec3 Eye, vec3 Center, vec3 Up)
{
    ASSERT_COVERED(HMM_LookAt_LH);

    vec3 F = v3Normalize(v3Subtract(Eye, Center));
    vec3 S = v3Normalize(v3Cross(F, Up));
    vec3 U = v3Cross(S, F);

    return _HMM_LookAt(F, S, U, Eye);
}

COVERAGE(HMM_InvLookAt, 1)
static inline mat4 HMM_InvLookAt(mat4 Matrix)
{
    ASSERT_COVERED(HMM_InvLookAt);
    mat4 Result;

    mat3 Rotation = {0};
    Rotation.Columns[0] = Matrix.Columns[0].XYZ;
    Rotation.Columns[1] = Matrix.Columns[1].XYZ;
    Rotation.Columns[2] = Matrix.Columns[2].XYZ;
    Rotation = mat3Transpose(Rotation);

    Result.Columns[0] = v3Expand(Rotation.Columns[0], 0.0f);
    Result.Columns[1] = v3Expand(Rotation.Columns[1], 0.0f);
    Result.Columns[2] = v3Expand(Rotation.Columns[2], 0.0f);
    Result.Columns[3] = v4Scale(Matrix.Columns[3], -1.0f);
    Result.Elements[3][0] = -1.0f * Matrix.Elements[3][0] /
        (Rotation.Elements[0][0] + Rotation.Elements[0][1] + Rotation.Elements[0][2]);
    Result.Elements[3][1] = -1.0f * Matrix.Elements[3][1] /
        (Rotation.Elements[1][0] + Rotation.Elements[1][1] + Rotation.Elements[1][2]);
    Result.Elements[3][2] = -1.0f * Matrix.Elements[3][2] /
        (Rotation.Elements[2][0] + Rotation.Elements[2][1] + Rotation.Elements[2][2]);
    Result.Elements[3][3] = 1.0f;

    return Result;
}

/*
 * Quaternion operations
 */

COVERAGE(quatCreate, 1)
static inline quat quatCreate(float X, float Y, float Z, float W)
{
    ASSERT_COVERED(quatCreate);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif

    return Result;
}

COVERAGE(quatCreateV4, 1)
static inline quat quatCreateV4(vec4 Vector)
{
    ASSERT_COVERED(quatCreateV4);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = Vector.SSE;
#else
    Result.X = Vector.X;
    Result.Y = Vector.Y;
    Result.Z = Vector.Z;
    Result.W = Vector.W;
#endif

    return Result;
}

COVERAGE(quatAdd, 1)
static inline quat quatAdd(quat Left, quat Right)
{
    ASSERT_COVERED(quatAdd);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#else

    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif

    return Result;
}

COVERAGE(quatSubtract, 1)
static inline quat quatSubtract(quat Left, quat Right)
{
    ASSERT_COVERED(quatSubtract);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif

    return Result;
}

COVERAGE(HMM_MulQ, 1)
static inline quat HMM_MulQ(quat Left, quat Right)
{
    ASSERT_COVERED(HMM_MulQ);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
    __m128 SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 SSEResultThree = _mm_mul_ps(SSEResultTwo, SSEResultOne);

    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(1, 1, 1, 1)) , _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(1, 0, 3, 2));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));

    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));

    SSEResultOne = _mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(3, 3, 3, 3));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(3, 2, 1, 0));
    Result.SSE = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
#else
    Result.X =  Right.Elements[3] * +Left.Elements[0];
    Result.Y =  Right.Elements[2] * -Left.Elements[0];
    Result.Z =  Right.Elements[1] * +Left.Elements[0];
    Result.W =  Right.Elements[0] * -Left.Elements[0];

    Result.X += Right.Elements[2] * +Left.Elements[1];
    Result.Y += Right.Elements[3] * +Left.Elements[1];
    Result.Z += Right.Elements[0] * -Left.Elements[1];
    Result.W += Right.Elements[1] * -Left.Elements[1];
    
    Result.X += Right.Elements[1] * -Left.Elements[2];
    Result.Y += Right.Elements[0] * +Left.Elements[2];
    Result.Z += Right.Elements[3] * +Left.Elements[2];
    Result.W += Right.Elements[2] * -Left.Elements[2];

    Result.X += Right.Elements[0] * +Left.Elements[3];
    Result.Y += Right.Elements[1] * +Left.Elements[3];
    Result.Z += Right.Elements[2] * +Left.Elements[3];
    Result.W += Right.Elements[3] * +Left.Elements[3];
#endif

    return Result;
}

COVERAGE(quatScale, 1)
static inline quat quatScale(quat Left, float Multiplicative)
{
    ASSERT_COVERED(quatScale);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Multiplicative);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X * Multiplicative;
    Result.Y = Left.Y * Multiplicative;
    Result.Z = Left.Z * Multiplicative;
    Result.W = Left.W * Multiplicative;
#endif

    return Result;
}

COVERAGE(quatReduce, 1)
static inline quat quatReduce(quat Left, float Divnd)
{
    ASSERT_COVERED(quatReduce);

    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Divnd);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#else
    Result.X = Left.X / Divnd;
    Result.Y = Left.Y / Divnd;
    Result.Z = Left.Z / Divnd;
    Result.W = Left.W / Divnd;
#endif

    return Result;
}

COVERAGE(HMM_DotQ, 1)
static inline float HMM_DotQ(quat Left, quat Right)
{
    ASSERT_COVERED(HMM_DotQ);

    float Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = ((Left.X * Right.X) + (Left.Z * Right.Z)) + ((Left.Y * Right.Y) + (Left.W * Right.W));
#endif

    return Result;
}

COVERAGE(HMM_InvQ, 1)
static inline quat HMM_InvQ(quat Left)
{
    ASSERT_COVERED(HMM_InvQ);
    
    quat Result;
    Result.X = -Left.X;
    Result.Y = -Left.Y;
    Result.Z = -Left.Z;
    Result.W = Left.W;

    return quatReduce(Result, (HMM_DotQ(Left, Left)));
}

COVERAGE(HMM_NormQ, 1)
static inline quat HMM_NormQ(quat Quat)
{
    ASSERT_COVERED(HMM_NormQ);

    /* NOTE(lcf): Take advantage of SSE implementation in v4Normalize */
    vec4 Vec = {Quat.X, Quat.Y, Quat.Z, Quat.W};
    Vec = v4Normalize(Vec);
    quat Result = {Vec.X, Vec.Y, Vec.Z, Vec.W};

    return Result;
}

static inline quat _HMM_MixQ(quat Left, float MixLeft, quat Right, float MixRight) {
    quat Result;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 ScalarLeft = _mm_set1_ps(MixLeft);
    __m128 ScalarRight = _mm_set1_ps(MixRight);
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, ScalarLeft);
    __m128 SSEResultTwo = _mm_mul_ps(Right.SSE, ScalarRight);
    Result.SSE = _mm_add_ps(SSEResultOne, SSEResultTwo);
#else
    Result.X = Left.X*MixLeft + Right.X*MixRight;
    Result.Y = Left.Y*MixLeft + Right.Y*MixRight;
    Result.Z = Left.Z*MixLeft + Right.Z*MixRight;
    Result.W = Left.W*MixLeft + Right.W*MixRight;
#endif

    return Result;
}

COVERAGE(HMM_NLerp, 1)
static inline quat HMM_NLerp(quat Left, float Time, quat Right)
{
    ASSERT_COVERED(HMM_NLerp);

    quat Result = _HMM_MixQ(Left, 1.0f-Time, Right, Time);
    Result = HMM_NormQ(Result);

    return Result;
}

COVERAGE(HMM_SLerp, 1)
static inline quat HMM_SLerp(quat Left, float Time, quat Right)
{
    ASSERT_COVERED(HMM_SLerp);

    quat Result;

    float Cos_Theta = HMM_DotQ(Left, Right);

    if (Cos_Theta < 0.0f) { /* NOTE(lcf): Take shortest path on Hyper-sphere */
        Cos_Theta = -Cos_Theta;
        Right = quatCreate(-Right.X, -Right.Y, -Right.Z, -Right.W);
    }
    
    /* NOTE(lcf): Use Normalized Linear interpolation when vectors are roughly not L.I. */
    if (Cos_Theta > 0.9995f) {
        Result = HMM_NLerp(Left, Time, Right);
    } else {
        float Angle = ACosF(Cos_Theta);
        float MixLeft = SinF((1.0f - Time) * Angle);
        float MixRight = SinF(Time * Angle);

        Result = _HMM_MixQ(Left, MixLeft, Right, MixRight);
        Result = HMM_NormQ(Result);
    }
    
    return Result;
}

COVERAGE(quatCreateToM4, 1)
static inline mat4 quatCreateToM4(quat Left)
{
    ASSERT_COVERED(quatCreateToM4);

    mat4 Result;

    quat NormalizedQ = HMM_NormQ(Left);

    float XX, YY, ZZ,
          XY, XZ, YZ,
          WX, WY, WZ;

    XX = NormalizedQ.X * NormalizedQ.X;
    YY = NormalizedQ.Y * NormalizedQ.Y;
    ZZ = NormalizedQ.Z * NormalizedQ.Z;
    XY = NormalizedQ.X * NormalizedQ.Y;
    XZ = NormalizedQ.X * NormalizedQ.Z;
    YZ = NormalizedQ.Y * NormalizedQ.Z;
    WX = NormalizedQ.W * NormalizedQ.X;
    WY = NormalizedQ.W * NormalizedQ.Y;
    WZ = NormalizedQ.W * NormalizedQ.Z;

    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);
    Result.Elements[0][3] = 0.0f;

    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);
    Result.Elements[1][3] = 0.0f;

    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);
    Result.Elements[2][3] = 0.0f;

    Result.Elements[3][0] = 0.0f;
    Result.Elements[3][1] = 0.0f;
    Result.Elements[3][2] = 0.0f;
    Result.Elements[3][3] = 1.0f;

    return Result;
}

// This method taken from Mike Day at Insomniac Games.
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
//
// Note that as mentioned at the top of the paper, the paper assumes the matrix
// would be *post*-multiplied to a vector to rotate it, meaning the matrix is
// the transpose of what we're dealing with. But, because our matrices are
// stored in column-major order, the indices *appear* to match the paper.
//
// For example, m12 in the paper is row 1, column 2. We need to transpose it to
// row 2, column 1. But, because the column comes first when referencing
// elements, it looks like M.Elements[1][2].
//
// Don't be confused! Or if you must be confused, at least trust this
// comment. :)
COVERAGE(mat4CreateToQ_RH, 4)
static inline quat mat4CreateToQ_RH(mat4 M)
{
    float T;
    quat Q;

    if (M.Elements[2][2] < 0.0f) {
        if (M.Elements[0][0] > M.Elements[1][1]) {
            ASSERT_COVERED(mat4CreateToQ_RH);

            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                T,
                M.Elements[0][1] + M.Elements[1][0],
                M.Elements[2][0] + M.Elements[0][2],
                M.Elements[1][2] - M.Elements[2][1]
            );
        } else {
            ASSERT_COVERED(mat4CreateToQ_RH);

            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                M.Elements[0][1] + M.Elements[1][0],
                T,
                M.Elements[1][2] + M.Elements[2][1],
                M.Elements[2][0] - M.Elements[0][2]
            );
        }
    } else {
        if (M.Elements[0][0] < -M.Elements[1][1]) {
            ASSERT_COVERED(mat4CreateToQ_RH);

            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                M.Elements[2][0] + M.Elements[0][2],
                M.Elements[1][2] + M.Elements[2][1],
                T,
                M.Elements[0][1] - M.Elements[1][0]
            );
        } else {
            ASSERT_COVERED(mat4CreateToQ_RH);

            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                M.Elements[1][2] - M.Elements[2][1],
                M.Elements[2][0] - M.Elements[0][2],
                M.Elements[0][1] - M.Elements[1][0],
                T
            );
        }
    }

    Q = quatScale(Q, 0.5f / SqrtF(T));

    return Q;
}

COVERAGE(mat4CreateToQ_LH, 4)
static inline quat mat4CreateToQ_LH(mat4 M)
{
    float T;
    quat Q;

    if (M.Elements[2][2] < 0.0f) {
        if (M.Elements[0][0] > M.Elements[1][1]) {
            ASSERT_COVERED(mat4CreateToQ_LH);

            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                T,
                M.Elements[0][1] + M.Elements[1][0],
                M.Elements[2][0] + M.Elements[0][2],
                M.Elements[2][1] - M.Elements[1][2]
            );
        } else {
            ASSERT_COVERED(mat4CreateToQ_LH);

            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = quatCreate(
                M.Elements[0][1] + M.Elements[1][0],
                T,
                M.Elements[1][2] + M.Elements[2][1],
                M.Elements[0][2] - M.Elements[2][0]
            );
        }
    } else {
        if (M.Elements[0][0] < -M.Elements[1][1]) {
            ASSERT_COVERED(mat4CreateToQ_LH);

            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                M.Elements[2][0] + M.Elements[0][2],
                M.Elements[1][2] + M.Elements[2][1],
                T,
                M.Elements[1][0] - M.Elements[0][1]
            );
        } else {
            ASSERT_COVERED(mat4CreateToQ_LH);

            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = quatCreate(
                M.Elements[2][1] - M.Elements[1][2],
                M.Elements[0][2] - M.Elements[2][0],
                M.Elements[1][0] - M.Elements[0][2],
                T
            );
        }
    }

    Q = quatScale(Q, 0.5f / SqrtF(T));

    return Q;
}


COVERAGE(quatCreateFromAxisAngle_RH, 1)
static inline quat quatCreateFromAxisAngle_RH(vec3 Axis, float AngleOfRotation)
{
    ASSERT_COVERED(quatCreateFromAxisAngle_RH);

    quat Result;

    vec3 AxisNormalized = v3Normalize(Axis);
    float SineOfRotation = SinF(AngleOfRotation / 2.0f);

    Result.XYZ = v3Scale(AxisNormalized, SineOfRotation);
    Result.W = CosF(AngleOfRotation / 2.0f);

    return Result;
}

COVERAGE(quatCreateFromAxisAngle_LH, 1)
static inline quat quatCreateFromAxisAngle_LH(vec3 Axis, float AngleOfRotation)
{
    ASSERT_COVERED(quatCreateFromAxisAngle_LH);

    return quatCreateFromAxisAngle_RH(Axis, -AngleOfRotation);
}


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

COVERAGE(v2LengthCPP, 1)
static inline float HMM_Len(vec2 A)
{
    ASSERT_COVERED(v2LengthCPP);
    return v2Length(A);
}

COVERAGE(v3LengthCPP, 1)
static inline float HMM_Len(vec3 A)
{
    ASSERT_COVERED(v3LengthCPP);
    return v3Length(A);
}

COVERAGE(v4LengthCPP, 1)
static inline float HMM_Len(vec4 A)
{
    ASSERT_COVERED(v4LengthCPP);
    return v4Length(A);
}

COVERAGE(v2LengthSquaredCPP, 1)
static inline float HMM_LenSqr(vec2 A)
{
    ASSERT_COVERED(v2LengthSquaredCPP);
    return v2LengthSquared(A);
}

COVERAGE(v3LengthSquaredCPP, 1)
static inline float HMM_LenSqr(vec3 A)
{
    ASSERT_COVERED(v3LengthSquaredCPP);
    return v3LengthSquared(A);
}

COVERAGE(v4LengthSquaredCPP, 1)
static inline float HMM_LenSqr(vec4 A)
{
    ASSERT_COVERED(v4LengthSquaredCPP);
    return v4LengthSquared(A);
}

COVERAGE(v2NormalizeCPP, 1)
static inline vec2 HMM_Norm(vec2 A)
{
    ASSERT_COVERED(v2NormalizeCPP);
    return v2Normalize(A);
}

COVERAGE(v3NormalizeCPP, 1)
static inline vec3 HMM_Norm(vec3 A)
{
    ASSERT_COVERED(v3NormalizeCPP);
    return v3Normalize(A);
}

COVERAGE(v4NormalizeCPP, 1)
static inline vec4 HMM_Norm(vec4 A)
{
    ASSERT_COVERED(v4NormalizeCPP);
    return v4Normalize(A);
}

COVERAGE(HMM_NormQCPP, 1)
static inline quat HMM_Norm(quat A)
{
    ASSERT_COVERED(HMM_NormQCPP);
    return HMM_NormQ(A);
}

COVERAGE(v2DotCPP, 1)
static inline float HMM_Dot(vec2 Left, vec2 VecTwo)
{
    ASSERT_COVERED(v2DotCPP);
    return v2Dot(Left, VecTwo);
}

COVERAGE(v3DotCPP, 1)
static inline float HMM_Dot(vec3 Left, vec3 VecTwo)
{
    ASSERT_COVERED(v3DotCPP);
    return v3Dot(Left, VecTwo);
}

COVERAGE(v4DotCPP, 1)
static inline float HMM_Dot(vec4 Left, vec4 VecTwo)
{
    ASSERT_COVERED(v4DotCPP);
    return v4Dot(Left, VecTwo);
}
 
COVERAGE(v2LerpCPP, 1)
static inline vec2 Lerp(vec2 Left, float Time, vec2 Right) 
{
    ASSERT_COVERED(v2LerpCPP);
    return v2Lerp(Left, Time, Right);
}

COVERAGE(v3LerpCPP, 1)
static inline vec3 Lerp(vec3 Left, float Time, vec3 Right) 
{
    ASSERT_COVERED(v3LerpCPP);
    return v3Lerp(Left, Time, Right);
}

COVERAGE(v4LerpCPP, 1)
static inline vec4 Lerp(vec4 Left, float Time, vec4 Right) 
{
    ASSERT_COVERED(v4LerpCPP);
    return v4Lerp(Left, Time, Right);
}

COVERAGE(mat2TransposeCPP, 1)
static inline mat2 HMM_Transpose(mat2 Matrix)
{
    ASSERT_COVERED(mat2TransposeCPP);
    return mat2Transpose(Matrix);
}

COVERAGE(mat3TransposeCPP, 1)
static inline mat3 HMM_Transpose(mat3 Matrix)
{
    ASSERT_COVERED(mat3TransposeCPP);
    return mat3Transpose(Matrix);
}

COVERAGE(mat4TransposeCPP, 1)
static inline mat4 HMM_Transpose(mat4 Matrix)
{
    ASSERT_COVERED(mat4TransposeCPP);
    return mat4Transpose(Matrix);
}

COVERAGE(mat2DeterminateCPP, 1)
static inline float HMM_Determinant(mat2 Matrix)
{
    ASSERT_COVERED(mat2DeterminateCPP);
    return mat2Determinate(Matrix);
}

COVERAGE(mat3DeterminateCPP, 1)
static inline float HMM_Determinant(mat3 Matrix)
{
    ASSERT_COVERED(mat3DeterminateCPP);
    return mat3Determinate(Matrix);
}

COVERAGE(mat4DeterminateCPP, 1)
static inline float HMM_Determinant(mat4 Matrix)
{
    ASSERT_COVERED(mat4DeterminateCPP);
    return mat4Determinate(Matrix);
}

COVERAGE(mat2InvDeterminateCPP, 1)
static inline mat2 HMM_InvGeneral(mat2 Matrix)
{
    ASSERT_COVERED(mat2InvDeterminateCPP);
    return mat2InvDeterminate(Matrix);
}

COVERAGE(mat3InvDeterminateCPP, 1)
static inline mat3 HMM_InvGeneral(mat3 Matrix)
{
    ASSERT_COVERED(mat3InvDeterminateCPP);
    return mat3InvDeterminate(Matrix);
}

COVERAGE(mat4InverseCPP, 1)
static inline mat4 HMM_InvGeneral(mat4 Matrix)
{
    ASSERT_COVERED(mat4InverseCPP);
    return mat4Inverse(Matrix);
}

COVERAGE(HMM_DotQCPP, 1)
static inline float HMM_Dot(quat QuatOne, quat QuatTwo)
{
    ASSERT_COVERED(HMM_DotQCPP);
    return HMM_DotQ(QuatOne, QuatTwo);
}

COVERAGE(v2AddCPP, 1)
static inline vec2 HMM_Add(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2AddCPP);
    return v2Add(Left, Right);
}

COVERAGE(v3AddCPP, 1)
static inline vec3 HMM_Add(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3AddCPP);
    return v3Add(Left, Right);
}

COVERAGE(v4AddCPP, 1)
static inline vec4 HMM_Add(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4AddCPP);
    return v4Add(Left, Right);
}

COVERAGE(mat2AddCPP, 1)
static inline mat2 HMM_Add(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2AddCPP);
    return mat2Add(Left, Right);
}

COVERAGE(mat3AddCPP, 1)
static inline mat3 HMM_Add(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3AddCPP);
    return mat3Add(Left, Right);
}

COVERAGE(mat4AddCPP, 1)
static inline mat4 HMM_Add(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4AddCPP);
    return mat4Add(Left, Right);
}

COVERAGE(quatAddCPP, 1)
static inline quat HMM_Add(quat Left, quat Right)
{
    ASSERT_COVERED(quatAddCPP);
    return quatAdd(Left, Right);
}

COVERAGE(v2SubtractCPP, 1)
static inline vec2 HMM_Sub(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2SubtractCPP);
    return v2Subtract(Left, Right);
}

COVERAGE(v3SubtractCPP, 1)
static inline vec3 HMM_Sub(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3SubtractCPP);
    return v3Subtract(Left, Right);
}

COVERAGE(v4SubtractCPP, 1)
static inline vec4 HMM_Sub(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4SubtractCPP);
    return v4Subtract(Left, Right);
}

COVERAGE(mat2SubtractCPP, 1)
static inline mat2 HMM_Sub(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2SubtractCPP);
    return mat2Subtract(Left, Right);
}

COVERAGE(mat3SubtractCPP, 1)
static inline mat3 HMM_Sub(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3SubtractCPP);
    return mat3Subtract(Left, Right);
}

COVERAGE(mat4SubtractCPP, 1)
static inline mat4 HMM_Sub(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4SubtractCPP);
    return mat4Subtract(Left, Right);
}

COVERAGE(quatSubtractCPP, 1)
static inline quat HMM_Sub(quat Left, quat Right)
{
    ASSERT_COVERED(quatSubtractCPP);
    return quatSubtract(Left, Right);
}

COVERAGE(v2MultiplyCPP, 1)
static inline vec2 HMM_Mul(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2MultiplyCPP);
    return v2Multiply(Left, Right);
}

COVERAGE(v2ScaleCPP, 1)
static inline vec2 HMM_Mul(vec2 Left, float Right)
{
    ASSERT_COVERED(v2ScaleCPP);
    return v2Scale(Left, Right);
}

COVERAGE(v3MultiplyCPP, 1)
static inline vec3 HMM_Mul(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3MultiplyCPP);
    return v3Multiply(Left, Right);
}

COVERAGE(v3ScaleCPP, 1)
static inline vec3 HMM_Mul(vec3 Left, float Right)
{
    ASSERT_COVERED(v3ScaleCPP);
    return v3Scale(Left, Right);
}

COVERAGE(v4MultiplyCPP, 1)
static inline vec4 HMM_Mul(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4MultiplyCPP);
    return v4Multiply(Left, Right);
}

COVERAGE(v4ScaleCPP, 1)
static inline vec4 HMM_Mul(vec4 Left, float Right)
{
    ASSERT_COVERED(v4ScaleCPP);
    return v4Scale(Left, Right);
}

COVERAGE(mat2MultiplyCPP, 1)
static inline mat2 HMM_Mul(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2MultiplyCPP);
    return mat2Multiply(Left, Right);
}

COVERAGE(mat3MultiplyCPP, 1)
static inline mat3 HMM_Mul(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3MultiplyCPP);
    return mat3Multiply(Left, Right);
}

COVERAGE(mat4MultiplyCPP, 1)
static inline mat4 HMM_Mul(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4MultiplyCPP);
    return mat4Multiply(Left, Right);
}

COVERAGE(mat2ScaleMatrixCPP, 1)
static inline mat2 HMM_Mul(mat2 Left, float Right)
{
    ASSERT_COVERED(mat2ScaleMatrixCPP);
    return mat2ScaleMatrix(Left, Right);
}

COVERAGE(mat3ScaleMatrixCPP, 1)
static inline mat3 HMM_Mul(mat3 Left, float Right)
{
    ASSERT_COVERED(mat3ScaleMatrixCPP);
    return mat3ScaleMatrix(Left, Right);
}

COVERAGE(mat4ScaleMatrixCPP, 1)
static inline mat4 HMM_Mul(mat4 Left, float Right)
{
    ASSERT_COVERED(mat4ScaleMatrixCPP);
    return mat4ScaleMatrix(Left, Right);
}

COVERAGE(mat2TransformCPP, 1)
static inline vec2 HMM_Mul(mat2 Matrix, vec2 Vector)
{
    ASSERT_COVERED(mat2TransformCPP);
    return mat2Transform(Matrix, Vector);
}

COVERAGE(mat3TransformCPP, 1)
static inline vec3 HMM_Mul(mat3 Matrix, vec3 Vector)
{
    ASSERT_COVERED(mat3TransformCPP);
    return mat3Transform(Matrix, Vector);
}

COVERAGE(mat4TransformCPP, 1)
static inline vec4 HMM_Mul(mat4 Matrix, vec4 Vector)
{
    ASSERT_COVERED(mat4TransformCPP);
    return mat4Transform(Matrix, Vector);
}

COVERAGE(HMM_MulQCPP, 1)
static inline quat HMM_Mul(quat Left, quat Right)
{
    ASSERT_COVERED(HMM_MulQCPP);
    return HMM_MulQ(Left, Right);
}

COVERAGE(quatScaleCPP, 1)
static inline quat HMM_Mul(quat Left, float Right)
{
    ASSERT_COVERED(quatScaleCPP);
    return quatScale(Left, Right);
}

COVERAGE(v2DivideCPP, 1)
static inline vec2 HMM_Div(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2DivideCPP);
    return v2Divide(Left, Right);
}

COVERAGE(v2ReduceCPP, 1)
static inline vec2 HMM_Div(vec2 Left, float Right)
{
    ASSERT_COVERED(v2ReduceCPP);
    return v2Reduce(Left, Right);
}

COVERAGE(v3DivideCPP, 1)
static inline vec3 HMM_Div(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3DivideCPP);
    return v3Divide(Left, Right);
}

COVERAGE(v3ReduceCPP, 1)
static inline vec3 HMM_Div(vec3 Left, float Right)
{
    ASSERT_COVERED(v3ReduceCPP);
    return v3Reduce(Left, Right);
}

COVERAGE(v4DivideCPP, 1)
static inline vec4 HMM_Div(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4DivideCPP);
    return v4Divide(Left, Right);
}

COVERAGE(v4ReduceCPP, 1)
static inline vec4 HMM_Div(vec4 Left, float Right)
{
    ASSERT_COVERED(v4ReduceCPP);
    return v4Reduce(Left, Right);
}

COVERAGE(mat2ReduceCPP, 1)
static inline mat2 HMM_Div(mat2 Left, float Right)
{
    ASSERT_COVERED(mat2ReduceCPP);
    return mat2Reduce(Left, Right);
}

COVERAGE(mat3ReduceCPP, 1)
static inline mat3 HMM_Div(mat3 Left, float Right)
{
    ASSERT_COVERED(mat3ReduceCPP);
    return mat3Reduce(Left, Right);
}

COVERAGE(mat4ReduceCPP, 1)
static inline mat4 HMM_Div(mat4 Left, float Right)
{
    ASSERT_COVERED(mat4ReduceCPP);
    return mat4Reduce(Left, Right);
}

COVERAGE(quatReduceCPP, 1)
static inline quat HMM_Div(quat Left, float Right)
{
    ASSERT_COVERED(quatReduceCPP);
    return quatReduce(Left, Right);
}

COVERAGE(v2EqualCPP, 1)
static inline HMM_Bool HMM_Eq(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2EqualCPP);
    return v2Equal(Left, Right);
}

COVERAGE(v3EqualCPP, 1)
static inline HMM_Bool HMM_Eq(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3EqualCPP);
    return v3Equal(Left, Right);
}

COVERAGE(v4EqualCPP, 1)
static inline HMM_Bool HMM_Eq(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4EqualCPP);
    return v4Equal(Left, Right);
}

COVERAGE(v2AddOp, 1)
static inline vec2 operator+(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2AddOp);
    return v2Add(Left, Right);
}

COVERAGE(v3AddOp, 1)
static inline vec3 operator+(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3AddOp);
    return v3Add(Left, Right);
}

COVERAGE(v4AddOp, 1)
static inline vec4 operator+(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4AddOp);
    return v4Add(Left, Right);
}

COVERAGE(mat2AddOp, 1)
static inline mat2 operator+(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2AddOp);
    return mat2Add(Left, Right);
}

COVERAGE(mat3AddOp, 1)
static inline mat3 operator+(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3AddOp);
    return mat3Add(Left, Right);
}

COVERAGE(mat4AddOp, 1)
static inline mat4 operator+(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4AddOp);
    return mat4Add(Left, Right);
}

COVERAGE(quatAddOp, 1)
static inline quat operator+(quat Left, quat Right)
{
    ASSERT_COVERED(quatAddOp);
    return quatAdd(Left, Right);
}

COVERAGE(v2SubtractOp, 1)
static inline vec2 operator-(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2SubtractOp);
    return v2Subtract(Left, Right);
}

COVERAGE(v3SubtractOp, 1)
static inline vec3 operator-(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3SubtractOp);
    return v3Subtract(Left, Right);
}

COVERAGE(v4SubtractOp, 1)
static inline vec4 operator-(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4SubtractOp);
    return v4Subtract(Left, Right);
}

COVERAGE(mat2SubtractOp, 1)
static inline mat2 operator-(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2SubtractOp);
    return mat2Subtract(Left, Right);
}

COVERAGE(mat3SubtractOp, 1)
static inline mat3 operator-(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3SubtractOp);
    return mat3Subtract(Left, Right);
}

COVERAGE(mat4SubtractOp, 1)
static inline mat4 operator-(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4SubtractOp);
    return mat4Subtract(Left, Right);
}

COVERAGE(quatSubtractOp, 1)
static inline quat operator-(quat Left, quat Right)
{
    ASSERT_COVERED(quatSubtractOp);
    return quatSubtract(Left, Right);
}

COVERAGE(v2MultiplyOp, 1)
static inline vec2 operator*(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2MultiplyOp);
    return v2Multiply(Left, Right);
}

COVERAGE(v3MultiplyOp, 1)
static inline vec3 operator*(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3MultiplyOp);
    return v3Multiply(Left, Right);
}

COVERAGE(v4MultiplyOp, 1)
static inline vec4 operator*(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4MultiplyOp);
    return v4Multiply(Left, Right);
}

COVERAGE(mat2MultiplyOp, 1)
static inline mat2 operator*(mat2 Left, mat2 Right)
{
    ASSERT_COVERED(mat2MultiplyOp);
    return mat2Multiply(Left, Right);
}

COVERAGE(mat3MultiplyOp, 1)
static inline mat3 operator*(mat3 Left, mat3 Right)
{
    ASSERT_COVERED(mat3MultiplyOp);
    return mat3Multiply(Left, Right);
}

COVERAGE(mat4MultiplyOp, 1)
static inline mat4 operator*(mat4 Left, mat4 Right)
{
    ASSERT_COVERED(mat4MultiplyOp);
    return mat4Multiply(Left, Right);
}

COVERAGE(HMM_MulQOp, 1)
static inline quat operator*(quat Left, quat Right)
{
    ASSERT_COVERED(HMM_MulQOp);
    return HMM_MulQ(Left, Right);
}

COVERAGE(v2ScaleOp, 1)
static inline vec2 operator*(vec2 Left, float Right)
{
    ASSERT_COVERED(v2ScaleOp);
    return v2Scale(Left, Right);
}

COVERAGE(v3ScaleOp, 1)
static inline vec3 operator*(vec3 Left, float Right)
{
    ASSERT_COVERED(v3ScaleOp);
    return v3Scale(Left, Right);
}

COVERAGE(v4ScaleOp, 1)
static inline vec4 operator*(vec4 Left, float Right)
{
    ASSERT_COVERED(v4ScaleOp);
    return v4Scale(Left, Right);
}

COVERAGE(mat2ScaleMatrixOp, 1)
static inline mat2 operator*(mat2 Left, float Right)
{
    ASSERT_COVERED(mat2ScaleMatrixOp);
    return mat2ScaleMatrix(Left, Right);
}

COVERAGE(mat3ScaleMatrixOp, 1)
static inline mat3 operator*(mat3 Left, float Right)
{
    ASSERT_COVERED(mat3ScaleMatrixOp);
    return mat3ScaleMatrix(Left, Right);
}

COVERAGE(mat4ScaleMatrixOp, 1)
static inline mat4 operator*(mat4 Left, float Right)
{
    ASSERT_COVERED(mat4ScaleMatrixOp);
    return mat4ScaleMatrix(Left, Right);
}

COVERAGE(quatScaleOp, 1)
static inline quat operator*(quat Left, float Right)
{
    ASSERT_COVERED(quatScaleOp);
    return quatScale(Left, Right);
}

COVERAGE(v2ScaleOpLeft, 1)
static inline vec2 operator*(float Left, vec2 Right)
{
    ASSERT_COVERED(v2ScaleOpLeft);
    return v2Scale(Right, Left);
}

COVERAGE(v3ScaleOpLeft, 1)
static inline vec3 operator*(float Left, vec3 Right)
{
    ASSERT_COVERED(v3ScaleOpLeft);
    return v3Scale(Right, Left);
}

COVERAGE(v4ScaleOpLeft, 1)
static inline vec4 operator*(float Left, vec4 Right)
{
    ASSERT_COVERED(v4ScaleOpLeft);
    return v4Scale(Right, Left);
}

COVERAGE(mat2ScaleMatrixOpLeft, 1)
static inline mat2 operator*(float Left, mat2 Right)
{
    ASSERT_COVERED(mat2ScaleMatrixOpLeft);
    return mat2ScaleMatrix(Right, Left);
}

COVERAGE(mat3ScaleMatrixOpLeft, 1)
static inline mat3 operator*(float Left, mat3 Right)
{
    ASSERT_COVERED(mat3ScaleMatrixOpLeft);
    return mat3ScaleMatrix(Right, Left);
}

COVERAGE(mat4ScaleMatrixOpLeft, 1)
static inline mat4 operator*(float Left, mat4 Right)
{
    ASSERT_COVERED(mat4ScaleMatrixOpLeft);
    return mat4ScaleMatrix(Right, Left);
}

COVERAGE(quatScaleOpLeft, 1)
static inline quat operator*(float Left, quat Right)
{
    ASSERT_COVERED(quatScaleOpLeft);
    return quatScale(Right, Left);
}

COVERAGE(mat2TransformOp, 1)
static inline vec2 operator*(mat2 Matrix, vec2 Vector)
{
    ASSERT_COVERED(mat2TransformOp);
    return mat2Transform(Matrix, Vector);
}

COVERAGE(mat3TransformOp, 1)
static inline vec3 operator*(mat3 Matrix, vec3 Vector)
{
    ASSERT_COVERED(mat3TransformOp);
    return mat3Transform(Matrix, Vector);
}

COVERAGE(mat4TransformOp, 1)
static inline vec4 operator*(mat4 Matrix, vec4 Vector)
{
    ASSERT_COVERED(mat4TransformOp);
    return mat4Transform(Matrix, Vector);
}

COVERAGE(v2DivideOp, 1)
static inline vec2 operator/(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2DivideOp);
    return v2Divide(Left, Right);
}

COVERAGE(v3DivideOp, 1)
static inline vec3 operator/(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3DivideOp);
    return v3Divide(Left, Right);
}

COVERAGE(v4DivideOp, 1)
static inline vec4 operator/(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4DivideOp);
    return v4Divide(Left, Right);
}

COVERAGE(v2ReduceOp, 1)
static inline vec2 operator/(vec2 Left, float Right)
{
    ASSERT_COVERED(v2ReduceOp);
    return v2Reduce(Left, Right);
}

COVERAGE(v3ReduceOp, 1)
static inline vec3 operator/(vec3 Left, float Right)
{
    ASSERT_COVERED(v3ReduceOp);
    return v3Reduce(Left, Right);
}

COVERAGE(v4ReduceOp, 1)
static inline vec4 operator/(vec4 Left, float Right)
{
    ASSERT_COVERED(v4ReduceOp);
    return v4Reduce(Left, Right);
}

COVERAGE(mat4ReduceOp, 1)
static inline mat4 operator/(mat4 Left, float Right)
{
    ASSERT_COVERED(mat4ReduceOp);
    return mat4Reduce(Left, Right);
}

COVERAGE(mat3ReduceOp, 1)
static inline mat3 operator/(mat3 Left, float Right)
{
    ASSERT_COVERED(mat3ReduceOp);
    return mat3Reduce(Left, Right);
}

COVERAGE(mat2ReduceOp, 1)
static inline mat2 operator/(mat2 Left, float Right)
{
    ASSERT_COVERED(mat2ReduceOp);
    return mat2Reduce(Left, Right);
}

COVERAGE(quatReduceOp, 1)
static inline quat operator/(quat Left, float Right)
{
    ASSERT_COVERED(quatReduceOp);
    return quatReduce(Left, Right);
}

COVERAGE(v2AddAssign, 1)
static inline vec2 &operator+=(vec2 &Left, vec2 Right)
{
    ASSERT_COVERED(v2AddAssign);
    return Left = Left + Right;
}

COVERAGE(v3AddAssign, 1)
static inline vec3 &operator+=(vec3 &Left, vec3 Right)
{
    ASSERT_COVERED(v3AddAssign);
    return Left = Left + Right;
}

COVERAGE(v4AddAssign, 1)
static inline vec4 &operator+=(vec4 &Left, vec4 Right)
{
    ASSERT_COVERED(v4AddAssign);
    return Left = Left + Right;
}

COVERAGE(mat2AddAssign, 1)
static inline mat2 &operator+=(mat2 &Left, mat2 Right)
{
    ASSERT_COVERED(mat2AddAssign);
    return Left = Left + Right;
}

COVERAGE(mat3AddAssign, 1)
static inline mat3 &operator+=(mat3 &Left, mat3 Right)
{
    ASSERT_COVERED(mat3AddAssign);
    return Left = Left + Right;
}

COVERAGE(mat4AddAssign, 1)
static inline mat4 &operator+=(mat4 &Left, mat4 Right)
{
    ASSERT_COVERED(mat4AddAssign);
    return Left = Left + Right;
}

COVERAGE(quatAddAssign, 1)
static inline quat &operator+=(quat &Left, quat Right)
{
    ASSERT_COVERED(quatAddAssign);
    return Left = Left + Right;
}

COVERAGE(v2SubtractAssign, 1)
static inline vec2 &operator-=(vec2 &Left, vec2 Right)
{
    ASSERT_COVERED(v2SubtractAssign);
    return Left = Left - Right;
}

COVERAGE(v3SubtractAssign, 1)
static inline vec3 &operator-=(vec3 &Left, vec3 Right)
{
    ASSERT_COVERED(v3SubtractAssign);
    return Left = Left - Right;
}

COVERAGE(v4SubtractAssign, 1)
static inline vec4 &operator-=(vec4 &Left, vec4 Right)
{
    ASSERT_COVERED(v4SubtractAssign);
    return Left = Left - Right;
}

COVERAGE(mat2SubtractAssign, 1)
static inline mat2 &operator-=(mat2 &Left, mat2 Right)
{
    ASSERT_COVERED(mat2SubtractAssign);
    return Left = Left - Right;
}

COVERAGE(mat3SubtractAssign, 1)
static inline mat3 &operator-=(mat3 &Left, mat3 Right)
{
    ASSERT_COVERED(mat3SubtractAssign);
    return Left = Left - Right;
}

COVERAGE(mat4SubtractAssign, 1)
static inline mat4 &operator-=(mat4 &Left, mat4 Right)
{
    ASSERT_COVERED(mat4SubtractAssign);
    return Left = Left - Right;
}

COVERAGE(quatSubtractAssign, 1)
static inline quat &operator-=(quat &Left, quat Right)
{
    ASSERT_COVERED(quatSubtractAssign);
    return Left = Left - Right;
}

COVERAGE(v2MultiplyAssign, 1)
static inline vec2 &operator*=(vec2 &Left, vec2 Right)
{
    ASSERT_COVERED(v2MultiplyAssign);
    return Left = Left * Right;
}

COVERAGE(v3MultiplyAssign, 1)
static inline vec3 &operator*=(vec3 &Left, vec3 Right)
{
    ASSERT_COVERED(v3MultiplyAssign);
    return Left = Left * Right;
}

COVERAGE(v4MultiplyAssign, 1)
static inline vec4 &operator*=(vec4 &Left, vec4 Right)
{
    ASSERT_COVERED(v4MultiplyAssign);
    return Left = Left * Right;
}

COVERAGE(v2ScaleAssign, 1)
static inline vec2 &operator*=(vec2 &Left, float Right)
{
    ASSERT_COVERED(v2ScaleAssign);
    return Left = Left * Right;
}

COVERAGE(v3ScaleAssign, 1)
static inline vec3 &operator*=(vec3 &Left, float Right)
{
    ASSERT_COVERED(v3ScaleAssign);
    return Left = Left * Right;
}

COVERAGE(v4ScaleAssign, 1)
static inline vec4 &operator*=(vec4 &Left, float Right)
{
    ASSERT_COVERED(v4ScaleAssign);
    return Left = Left * Right;
}

COVERAGE(mat2ScaleMatrixAssign, 1)
static inline mat2 &operator*=(mat2 &Left, float Right)
{
    ASSERT_COVERED(mat2ScaleMatrixAssign);
    return Left = Left * Right;
}

COVERAGE(mat3ScaleMatrixAssign, 1)
static inline mat3 &operator*=(mat3 &Left, float Right)
{
    ASSERT_COVERED(mat3ScaleMatrixAssign);
    return Left = Left * Right;
}

COVERAGE(mat4ScaleMatrixAssign, 1)
static inline mat4 &operator*=(mat4 &Left, float Right)
{
    ASSERT_COVERED(mat4ScaleMatrixAssign);
    return Left = Left * Right;
}

COVERAGE(quatScaleAssign, 1)
static inline quat &operator*=(quat &Left, float Right)
{
    ASSERT_COVERED(quatScaleAssign);
    return Left = Left * Right;
}

COVERAGE(v2DivideAssign, 1)
static inline vec2 &operator/=(vec2 &Left, vec2 Right)
{
    ASSERT_COVERED(v2DivideAssign);
    return Left = Left / Right;
}

COVERAGE(v3DivideAssign, 1)
static inline vec3 &operator/=(vec3 &Left, vec3 Right)
{
    ASSERT_COVERED(v3DivideAssign);
    return Left = Left / Right;
}

COVERAGE(v4DivideAssign, 1)
static inline vec4 &operator/=(vec4 &Left, vec4 Right)
{
    ASSERT_COVERED(v4DivideAssign);
    return Left = Left / Right;
}

COVERAGE(v2ReduceAssign, 1)
static inline vec2 &operator/=(vec2 &Left, float Right)
{
    ASSERT_COVERED(v2ReduceAssign);
    return Left = Left / Right;
}

COVERAGE(v3ReduceAssign, 1)
static inline vec3 &operator/=(vec3 &Left, float Right)
{
    ASSERT_COVERED(v3ReduceAssign);
    return Left = Left / Right;
}

COVERAGE(v4ReduceAssign, 1)
static inline vec4 &operator/=(vec4 &Left, float Right)
{
    ASSERT_COVERED(v4ReduceAssign);
    return Left = Left / Right;
}

COVERAGE(mat4ReduceAssign, 1)
static inline mat4 &operator/=(mat4 &Left, float Right)
{
    ASSERT_COVERED(mat4ReduceAssign);
    return Left = Left / Right;
}

COVERAGE(quatReduceAssign, 1)
static inline quat &operator/=(quat &Left, float Right)
{
    ASSERT_COVERED(quatReduceAssign);
    return Left = Left / Right;
}

COVERAGE(v2EqualOp, 1)
static inline HMM_Bool operator==(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2EqualOp);
    return v2Equal(Left, Right);
}

COVERAGE(v3EqualOp, 1)
static inline HMM_Bool operator==(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3EqualOp);
    return v3Equal(Left, Right);
}

COVERAGE(v4EqualOp, 1)
static inline HMM_Bool operator==(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4EqualOp);
    return v4Equal(Left, Right);
}

COVERAGE(v2EqualOpNot, 1)
static inline HMM_Bool operator!=(vec2 Left, vec2 Right)
{
    ASSERT_COVERED(v2EqualOpNot);
    return !v2Equal(Left, Right);
}

COVERAGE(v3EqualOpNot, 1)
static inline HMM_Bool operator!=(vec3 Left, vec3 Right)
{
    ASSERT_COVERED(v3EqualOpNot);
    return !v3Equal(Left, Right);
}

COVERAGE(v4EqualOpNot, 1)
static inline HMM_Bool operator!=(vec4 Left, vec4 Right)
{
    ASSERT_COVERED(v4EqualOpNot);
    return !v4Equal(Left, Right);
}

COVERAGE(HMM_UnaryMinusV2, 1)
static inline vec2 operator-(vec2 In)
{
    ASSERT_COVERED(HMM_UnaryMinusV2);

    vec2 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;

    return Result;
}

COVERAGE(HMM_UnaryMinusV3, 1)
static inline vec3 operator-(vec3 In)
{
    ASSERT_COVERED(HMM_UnaryMinusV3);

    vec3 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;

    return Result;
}

COVERAGE(HMM_UnaryMinusV4, 1)
static inline vec4 operator-(vec4 In)
{
    ASSERT_COVERED(HMM_UnaryMinusV4);

    vec4 Result;
#if HANDMADE_MATH__USE_SSE
    Result.SSE = _mm_xor_ps(In.SSE, _mm_set1_ps(-0.0f));
#else
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;
    Result.W = -In.W;
#endif

    return Result;
}

#endif /* __cplusplus*/

#ifdef HANDMADE_MATH__USE_C11_GENERICS
#define HMM_Add(A, B) _Generic((A), \
        vec2: v2Add, \
        vec3: v3Add, \
        vec4: v4Add, \
        mat2: mat2Add, \
        mat3: mat3Add, \
        mat4: mat4Add, \
        quat: quatAdd \
)(A, B)

#define HMM_Sub(A, B) _Generic((A), \
        vec2: v2Subtract, \
        vec3: v3Subtract, \
        vec4: v4Subtract, \
        mat2: mat2Subtract, \
        mat3: mat3Subtract, \
        mat4: mat4Subtract, \
        quat: quatSubtract \
)(A, B)

#define HMM_Mul(A, B) _Generic((B), \
     float: _Generic((A), \
        vec2: v2Scale, \
        vec3: v3Scale, \
        vec4: v4Scale, \
        mat2: mat2ScaleMatrix, \
        mat3: mat3ScaleMatrix, \
        mat4: mat4ScaleMatrix, \
        quat: quatScale \
     ), \
     mat2: mat2Multiply, \
     mat3: mat3Multiply, \
     mat4: mat4Multiply, \
     quat: HMM_MulQ, \
     default: _Generic((A), \
        vec2: v2Multiply, \
        vec3: v3Multiply, \
        vec4: v4Multiply, \
        mat2: mat2Transform, \
        mat3: mat3Transform, \
        mat4: mat4Transform \
    ) \
)(A, B)

#define HMM_Div(A, B) _Generic((B), \
     float: _Generic((A), \
        mat2: mat2Reduce, \
        mat3: mat3Reduce, \
        mat4: mat4Reduce, \
        vec2: v2Reduce, \
        vec3: v3Reduce, \
        vec4: v4Reduce, \
        quat: quatReduce  \
     ), \
     mat2: HMM_DivM2, \
     mat3: HMM_DivM3, \
     mat4: HMM_DivM4, \
     quat: HMM_DivQ, \
     default: _Generic((A), \
        vec2: v2Divide, \
        vec3: v3Divide, \
        vec4: v4Divide  \
    ) \
)(A, B)

#define HMM_Len(A) _Generic((A), \
        vec2: v2Length, \
        vec3: v3Length, \
        vec4: v4Length  \
)(A)

#define HMM_LenSqr(A) _Generic((A), \
        vec2: v2LengthSquared, \
        vec3: v3LengthSquared, \
        vec4: v4LengthSquared  \
)(A)

#define HMM_Norm(A) _Generic((A), \
        vec2: v2Normalize, \
        vec3: v3Normalize, \
        vec4: v4Normalize  \
)(A)

#define HMM_Dot(A, B) _Generic((A), \
        vec2: v2Dot, \
        vec3: v3Dot, \
        vec4: v4Dot  \
)(A, B)

#define Lerp(A, T, B) _Generic((A), \
        float: Lerp, \
        vec2: v2Lerp, \
        vec3: v3Lerp, \
        vec4: v4Lerp \
)(A, T, B)

#define HMM_Eq(A, B) _Generic((A), \
        vec2: v2Equal, \
        vec3: v3Equal, \
        vec4: v4Equal  \
)(A, B)

#define HMM_Transpose(M) _Generic((M), \
        mat2: mat2Transpose, \
        mat3: mat3Transpose, \
        mat4: mat4Transpose  \
)(M)

#define HMM_Determinant(M) _Generic((M), \
        mat2: mat2Determinate, \
        mat3: mat3Determinate, \
        mat4: mat4Determinate  \
)(M)

#define HMM_InvGeneral(M) _Generic((M), \
        mat2: mat2InvDeterminate, \
        mat3: mat3InvDeterminate, \
        mat4: mat4Inverse  \
)(M)

#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif /* HANDMADE_MATH_H */
