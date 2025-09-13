//-----------------------------------------------------------------------------
// File : asdxScript.cpp
// Desc : Script System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxScript.h>
#include <fnd/asdxSpinLock.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMath.h>
#include <CflatHelper.h>


namespace {

//----------------------------------------------------------------------------
// Global Variables.
//----------------------------------------------------------------------------
Cflat::Environment  g_Env      = {};
asdx::SpinLock      g_SpinLock = {};

} // namespace


namespace CflatGlobal {

//-----------------------------------------------------------------------------
//      環境を取得します.
//-----------------------------------------------------------------------------
Cflat::Environment* getEnvironment()
{ return &g_Env; }

//-----------------------------------------------------------------------------
//      環境をロックします.
//-----------------------------------------------------------------------------
void lockEnvironment()
{ g_SpinLock.lock(); }

//-----------------------------------------------------------------------------
//      環境のロックを解除します.
//-----------------------------------------------------------------------------
void unlockEnvironment()
{ g_SpinLock.unlock(); }

//-----------------------------------------------------------------------------
//      エラー時にメッセージを表示します.
//-----------------------------------------------------------------------------
void onError(const char* msg)
{ ELOG("%s", msg); }

} // namespace CflatGlobal


namespace asdx {

//-----------------------------------------------------------------------------
//      Cflat::Environmentを取得します
//-----------------------------------------------------------------------------
Cflat::Environment* CflatEnv()
{ return &g_Env; }

//-----------------------------------------------------------------------------
//      算術演算タイプを登録します.
//-----------------------------------------------------------------------------
void RegisterMathTypes()
{
    auto pEnv = CflatEnv();

    using namespace asdx;
    auto ns = pEnv->requestNamespace("asdx");

    // Vector2
    {
        CflatRegisterStruct(ns, Vector2);
        CflatStructAddMember(ns, Vector2, float, x);
        CflatStructAddMember(ns, Vector2, float, y);
        CflatStructAddConstructor(ns, Vector2);
        CflatStructAddConstructorParams1(ns, Vector2, const float*);
        CflatStructAddConstructorParams2(ns, Vector2, float, float);

        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2&, operator+=, const Vector2&);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2&, operator-=, const Vector2&);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2&, operator*=, float);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2&, operator/=, float);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2&, operator=, const Vector2&);
        CflatStructAddMethodReturn(ns, Vector2, Vector2, operator+);
        CflatStructAddMethodReturn(ns, Vector2, Vector2, operator-);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2, operator+, const Vector2&);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2, operator-, const Vector2&);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2, operator*, float);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2, operator/, float);
        CflatStructAddMethodReturnParams1(ns, Vector2, bool, operator==, const Vector2&);
        CflatStructAddMethodReturnParams1(ns, Vector2, bool, operator!=, const Vector2&);
        CflatStructAddMethodReturn(ns, Vector2, float, Length);
        CflatStructAddMethodReturn(ns, Vector2, float, LengthSq);
        CflatStructAddMethodReturn(ns, Vector2, Vector2&, Normalize);
        CflatStructAddMethodReturnParams1(ns, Vector2, Vector2&, SafeNormalize, const Vector2&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector2, Vector2, Abs, const Vector2&);
        CflatStructAddStaticMethodReturnParams3(ns, Vector2, Vector2, Clamp, const Vector2&, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector2, Vector2, Saturate, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, float, Distance, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, float, DistanceSq, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, float, Dot, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector2, Vector2, Normalize, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, SafeNormalize, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, float, ComputeCrossingAngle, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, Min, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, Max, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, Reflect, const Vector2&, const Vector2&);
        CflatStructAddStaticMethodReturnParams3(ns, Vector2, Vector2, Refract, const Vector2&, const Vector2&, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector2, Vector2, Barycentric, const Vector2&, const Vector2&, const Vector2&, float, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector2, Vector2, Hermite, const Vector2&, const Vector2&, const Vector2&, const Vector2&, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector2, Vector2, CatmullRom, const Vector2&, const Vector2&, const Vector2&, const Vector2&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Vector2, Vector2, Lerp, const Vector2&, const Vector2&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Vector2, Vector2, SmoothStep, const Vector2&, const Vector2&, float);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, Transform, const Vector2&, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, TransformNormal, const Vector2&, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector2, Vector2, TransformCoord, const Vector2&, const Matrix&);
    }

    // Vector3
    {
        CflatRegisterStruct(ns, Vector3);
        CflatStructAddMember(ns, Vector3, float, x);
        CflatStructAddMember(ns, Vector3, float, y);
        CflatStructAddMember(ns, Vector3, float, z);
        CflatStructAddConstructor(ns, Vector3);
        CflatStructAddConstructorParams1(ns, Vector3, const float*);
        CflatStructAddConstructorParams2(ns, Vector3, const Vector2&, float);
        CflatStructAddConstructorParams3(ns, Vector3, float, float, float);

        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3&, operator+=, const Vector3&);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3&, operator-=, const Vector3&);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3&, operator*=, float);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3&, operator/=, float);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3&, operator=, const Vector3&);
        CflatStructAddMethodReturn(ns, Vector3, Vector3, operator+);
        CflatStructAddMethodReturn(ns, Vector3, Vector3, operator-);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3, operator+, const Vector3&);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3, operator-, const Vector3&);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3, operator*, float);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3, operator/, float);
        CflatStructAddMethodReturnParams1(ns, Vector3, bool, operator==, const Vector3&);
        CflatStructAddMethodReturnParams1(ns, Vector3, bool, operator!=, const Vector3&);
        CflatStructAddMethodReturn(ns, Vector3, float, Length);
        CflatStructAddMethodReturn(ns, Vector3, float, LengthSq);
        CflatStructAddMethodReturn(ns, Vector3, Vector3&, Normalize);
        CflatStructAddMethodReturnParams1(ns, Vector3, Vector3&, SafeNormalize, const Vector3&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector3, Vector3, Abs, const Vector3&);
        CflatStructAddStaticMethodReturnParams3(ns, Vector3, Vector3, Clamp, const Vector3&, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector3, Vector3, Saturate, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, float, Distance, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, float, DistanceSq, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, float, Dot, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, Cross, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector3, Vector3, Normalize, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, SafeNormalize, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, float, ComputeCrossingAngle, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, Min, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, Max, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, Reflect, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams3(ns, Vector3, Vector3, Refract, const Vector3&, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector3, Vector3, Barycentric, const Vector3&, const Vector3&, const Vector3&, float, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector3, Vector3, Hermite, const Vector3&, const Vector3&, const Vector3&, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector3, Vector3, CatmullRom, const Vector3&, const Vector3&, const Vector3&, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Vector3, Vector3, Lerp, const Vector3&, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Vector3, Vector3, SmoothStep, const Vector3&, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, Transform, const Vector3&, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, TransformNormal, const Vector3&, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector3, Vector3, TransformCoord, const Vector3&, const Matrix&);
    }

    // Vector4
    {
        CflatRegisterStruct(ns, Vector4);
        CflatStructAddMember(ns, Vector4, float, x);
        CflatStructAddMember(ns, Vector4, float, y);
        CflatStructAddMember(ns, Vector4, float, z);
        CflatStructAddMember(ns, Vector4, float, z);
        CflatStructAddConstructor(ns, Vector4);
        CflatStructAddConstructorParams1(ns, Vector4, const float*);
        CflatStructAddConstructorParams3(ns, Vector4, const Vector2&, float, float);
        CflatStructAddConstructorParams2(ns, Vector4, const Vector3&, float);
        CflatStructAddConstructorParams4(ns, Vector4, float, float, float, float);

        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4&, operator+=, const Vector4&);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4&, operator-=, const Vector4&);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4&, operator*=, float);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4&, operator/=, float);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4&, operator=, const Vector4&);
        CflatStructAddMethodReturn(ns, Vector4, Vector4, operator+);
        CflatStructAddMethodReturn(ns, Vector4, Vector4, operator-);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4, operator+, const Vector4&);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4, operator-, const Vector4&);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4, operator*, float);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4, operator/, float);
        CflatStructAddMethodReturnParams1(ns, Vector4, bool, operator==, const Vector4&);
        CflatStructAddMethodReturnParams1(ns, Vector4, bool, operator!=, const Vector4&);
        CflatStructAddMethodReturn(ns, Vector4, float, Length);
        CflatStructAddMethodReturn(ns, Vector4, float, LengthSq);
        CflatStructAddMethodReturn(ns, Vector4, Vector4&, Normalize);
        CflatStructAddMethodReturnParams1(ns, Vector4, Vector4&, SafeNormalize, const Vector4&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector4, Vector4, Abs, const Vector4&);
        CflatStructAddStaticMethodReturnParams3(ns, Vector4, Vector4, Clamp, const Vector4&, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector4, Vector4, Saturate, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, float, Distance, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, float, DistanceSq, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, float, Dot, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams1(ns, Vector4, Vector4, Normalize, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, Vector4, SafeNormalize, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, float, ComputeCrossingAngle, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, Vector4, Min, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, Vector4, Max, const Vector4&, const Vector4&);
        CflatStructAddStaticMethodReturnParams5(ns, Vector4, Vector4, Barycentric, const Vector4&, const Vector4&, const Vector4&, float, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector4, Vector4, Hermite, const Vector4&, const Vector4&, const Vector4&, const Vector4&, float);
        CflatStructAddStaticMethodReturnParams5(ns, Vector4, Vector4, CatmullRom, const Vector4&, const Vector4&, const Vector4&, const Vector4&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Vector4, Vector4, Lerp, const Vector4&, const Vector4&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Vector4, Vector4, SmoothStep, const Vector4&, const Vector4&, float);
        CflatStructAddStaticMethodReturnParams2(ns, Vector4, Vector4, Transform, const Vector4&, const Matrix&);
    }

    // Matrix
    {
        CflatRegisterStruct(ns, Matrix);
        CflatStructAddMember(ns, Matrix, float, _11);
        CflatStructAddMember(ns, Matrix, float, _12);
        CflatStructAddMember(ns, Matrix, float, _13);
        CflatStructAddMember(ns, Matrix, float, _14);
        CflatStructAddMember(ns, Matrix, float, _21);
        CflatStructAddMember(ns, Matrix, float, _22);
        CflatStructAddMember(ns, Matrix, float, _23);
        CflatStructAddMember(ns, Matrix, float, _24);
        CflatStructAddMember(ns, Matrix, float, _31);
        CflatStructAddMember(ns, Matrix, float, _32);
        CflatStructAddMember(ns, Matrix, float, _33);
        CflatStructAddMember(ns, Matrix, float, _34);
        CflatStructAddMember(ns, Matrix, float, _41);
        CflatStructAddMember(ns, Matrix, float, _42);
        CflatStructAddMember(ns, Matrix, float, _43);
        CflatStructAddMember(ns, Matrix, float, _44);

        CflatStructAddConstructor(ns, Matrix);
        CflatStructAddConstructorParams4(ns, Matrix, const Vector4&, const Vector4&, const Vector4&, const Vector4&);

        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix&, operator+=, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix&, operator-=, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix&, operator*=, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix&, operator*=, float);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix&, operator/=, float);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix&, operator=, const Matrix&);
        CflatStructAddMethodReturn(ns, Matrix, Matrix, operator+);
        CflatStructAddMethodReturn(ns, Matrix, Matrix, operator-);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix, operator+, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix, operator-, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix, operator+, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix, operator*, float);
        CflatStructAddMethodReturnParams1(ns, Matrix, Matrix, operator/, float);
        CflatStructAddMethodReturnParams1(ns, Matrix, bool, operator==, const Matrix&);
        CflatStructAddMethodReturnParams1(ns, Matrix, bool, operator!=, const Matrix&);
        CflatStructAddMethodReturn(ns, Matrix, float, Determinant);

        CflatStructAddStaticMethodReturn(ns, Matrix, Matrix, CreateIdentity);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, bool, IsIdentity, const Matrix&);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, Transpose, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Matrix, Matrix, Multiply, const Matrix&, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Matrix, Matrix, Multiply, const Matrix&, float);
        CflatStructAddStaticMethodReturnParams2(ns, Matrix, Matrix, MultiplyTranspose, const Matrix&, const Matrix&);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, Invert, const Matrix&);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateScale, float);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreateScale, float, float, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateScale, const Vector3&);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreateTranslation, float, float, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateTranslation, const Vector3&);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateRotationX, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateRotationY, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateRotationZ, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateFromQuaternion, const Quaternion&);
        CflatStructAddStaticMethodReturnParams2(ns, Matrix, Matrix, CreateFromAxisAngle, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreateRotationFromYawPitchRoll, float, float, float);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreateLookAt, const Vector3&, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreateLookTo, const Vector3&, const Vector3&, const Vector3&);
        CflatStructAddStaticMethodReturnParams4(ns, Matrix, Matrix, CreatePerspective, float, float, float, float);
        CflatStructAddStaticMethodReturnParams4(ns, Matrix, Matrix, CreatePerspectiveFieldOfView, float, float, float, float);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreatePerspectiveFieldOfViewReverseZ, float, float, float);
        CflatStructAddStaticMethodReturnParams6(ns, Matrix, Matrix, CreatePerspectiveOffCenter, float, float, float, float, float, float);
        CflatStructAddStaticMethodReturnParams4(ns, Matrix, Matrix, CreateOrthographic, float, float, float, float);
        CflatStructAddStaticMethodReturnParams6(ns, Matrix, Matrix, CreateOrthographicOffCenter, float, float, float, float, float, float);
        CflatStructAddStaticMethodReturnParams6(ns, Matrix, Matrix, CreateOrthographicOffCenterReverseZ, float, float, float, float, float, float);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, Lerp, const Matrix&, const Matrix&, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateBillboard, const Matrix&);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateBillboardAxisY, const Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Matrix, Matrix, AppendTranslation, const Vector3&, Matrix&);
        CflatStructAddStaticMethodReturnParams2(ns, Matrix, Matrix, AppendScale, const Vector3&, Matrix&);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateBrightnessMatrix, float);
        CflatStructAddStaticMethodReturnParams3(ns, Matrix, Matrix, CreateSaturationMatrix, float, float, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateSaturationMatrix, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateContrastMatrix, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateHueMatrix, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateSepiaMatrix, float);
        CflatStructAddStaticMethodReturnParams1(ns, Matrix, Matrix, CreateGrayScaleMatrix, float);
        CflatStructAddStaticMethodReturn(ns, Matrix, Matrix, CreateReverseColorMatrix);
    }

    // Quaternion
    {
        CflatRegisterStruct(ns, Quaternion);
        CflatStructAddMember(ns, Quaternion, float, x);
        CflatStructAddMember(ns, Quaternion, float, y);
        CflatStructAddMember(ns, Quaternion, float, z);
        CflatStructAddMember(ns, Quaternion, float, w);

        CflatStructAddConstructor(ns, Quaternion);
        CflatStructAddConstructorParams1(ns, Quaternion, const float*);
        CflatStructAddConstructorParams4(ns, Quaternion, float, float, float, float);

        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, operator+=, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, operator-=, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, operator*=, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, operator*=, float);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, operator/=, float);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, operator=, const Quaternion&);
        CflatStructAddMethodReturn(ns, Quaternion, Quaternion, operator+);
        CflatStructAddMethodReturn(ns, Quaternion, Quaternion, operator-);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion, operator+, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion, operator-, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion, operator+, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion, operator*, float);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion, operator/, float);
        CflatStructAddMethodReturnParams1(ns, Quaternion, bool, operator==, const Quaternion&);
        CflatStructAddMethodReturnParams1(ns, Quaternion, bool, operator!=, const Quaternion&);
        CflatStructAddMethodReturn(ns, Quaternion, float, Length);
        CflatStructAddMethodReturn(ns, Quaternion, float, LengthSq);
        CflatStructAddMethodReturn(ns, Quaternion, Quaternion&, Normalize);
        CflatStructAddMethodReturnParams1(ns, Quaternion, Quaternion&, SafeNormalize, const Quaternion&);
        CflatStructAddMethodReturn(ns, Quaternion, Quaternion&, Identity);

        CflatStructAddStaticMethodReturn(ns, Quaternion, Quaternion, CreateIdentity);
        CflatStructAddStaticMethodReturnParams1(ns, Quaternion, bool, IsIdentity, const Quaternion&);
        CflatStructAddStaticMethodReturnParams1(ns, Quaternion, bool, IsNormalized, const Quaternion&);
        CflatStructAddStaticMethodReturnParams2(ns, Quaternion, Quaternion, Multiply, const Quaternion&, const Quaternion&);
        CflatStructAddStaticMethodReturnParams2(ns, Quaternion, float, Dot, const Quaternion&, const Quaternion&);
        CflatStructAddStaticMethodReturnParams1(ns, Quaternion, Quaternion, Conjugate, const Quaternion&);
        CflatStructAddStaticMethodReturnParams1(ns, Quaternion, Quaternion, Normalize, const Quaternion&);
        CflatStructAddStaticMethodReturnParams2(ns, Quaternion, Quaternion, SafeNormalize, const Quaternion&, const Quaternion&);
        CflatStructAddStaticMethodReturnParams3(ns, Quaternion, Quaternion, CreateFromYawPitchRoll, float, float, float);
        CflatStructAddStaticMethodReturnParams2(ns, Quaternion, Quaternion, CreateFromAxisAngle, const Vector3&, float);
        CflatStructAddStaticMethodReturnParams1(ns, Quaternion, Quaternion, CreateFromRotationMatrix, const Matrix&);
        CflatStructAddStaticMethodReturnParams1(ns, Quaternion, Vector3, ToAxisAngle, const Quaternion&);
        CflatStructAddStaticMethodReturnParams3(ns, Quaternion, Quaternion, Slerp, const Quaternion&, const Quaternion&, float);
        CflatStructAddStaticMethodReturnParams5(ns, Quaternion, Quaternion, Squad, const Quaternion&, const Quaternion&, const Quaternion&, const Quaternion&, float);
    }

    // XorShift
    {
        CflatRegisterClass(ns, XorShift);
        CflatClassAddConstructorParams1(ns, XorShift, int);
        CflatClassAddCopyConstructor(ns, XorShift);
        CflatClassAddDestructor(ns, XorShift);

        CflatClassAddMethodVoidParams1(ns, XorShift, void, SetSeed, uint32_t);
        CflatClassAddMethodReturn(ns, XorShift, uint32_t, GetValue);
        CflatStructAddMethodReturnParams1(ns, XorShift, XorShift&, operator=, const XorShift&);
        CflatStructAddMethodReturnParams1(ns, XorShift, bool, operator==, const XorShift&);
        CflatStructAddMethodReturnParams1(ns, XorShift, bool, operator!=, const XorShift&);
    }

    // PCG
    {
        CflatRegisterClass(ns, PCG);
        CflatClassAddConstructorParams1(ns, PCG, uint64_t);
        CflatClassAddCopyConstructor(ns, PCG);
        CflatClassAddDestructor(ns, PCG);

        CflatClassAddMethodVoidParams1(ns, PCG, void, SetSeed, uint64_t);
        CflatClassAddMethodReturn(ns, PCG, uint32_t, GetValue);
        CflatStructAddMethodReturnParams1(ns, PCG, PCG&, operator=, const PCG&);
        CflatStructAddMethodReturnParams1(ns, PCG, bool, operator==, const PCG&);
        CflatStructAddMethodReturnParams1(ns, PCG, bool, operator!=, const PCG&);
    }

    // RandomHelper
    {
        CflatRegisterClass(ns, RandomHelper);
        CflatClassAddStaticMethodReturnParams1(ns, RandomHelper, int, GetAsInt, uint32_t);
        CflatClassAddStaticMethodReturnParams3(ns, RandomHelper, int, GetAsInt, uint32_t, int, int);
        CflatClassAddStaticMethodReturnParams1(ns, RandomHelper, float, GetAsFloat, uint32_t);
        CflatClassAddStaticMethodReturnParams3(ns, RandomHelper, float, GetAsFloat, uint32_t, float, float);
    }

    // Quad2
    {
        CflatRegisterClass(ns, Quad2);
        CflatClassAddConstructor(ns, Quad2);
        CflatClassAddConstructorParams4(ns, Quad2, int, int, int, int);

        CflatClassAddMethodReturnParams2(ns, Quad2, Quad2&, Move, int, int);
        CflatStructAddStaticMethodReturnParams2(ns, Quad2, bool, Contains, const Quad2&, const Quad2&);
        CflatStructAddStaticMethodReturnParams3(ns, Quad2, bool, Contains, int, int, const Quad2&);
    }
}

//-----------------------------------------------------------------------------
//      Cflatの初期化処理を行います.
//-----------------------------------------------------------------------------
void InitCflat()
{
    auto pEnv = CflatEnv();
    Cflat::Helper::registerStdString(pEnv);
    Cflat::Helper::registerStdOut(pEnv);
    Cflat::Helper::registerPrintfFamily(pEnv);

    RegisterMathTypes();
}

//-----------------------------------------------------------------------------
//      Cflatの終了処理を行います.
//-----------------------------------------------------------------------------
void TermCflat()
{
    /* DO_NOTHING */
}

} // namespace asdx
