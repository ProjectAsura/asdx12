//-----------------------------------------------------------------------------
// File : asdxFlatDoc.h
// Desc : Flat Document.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <map>
#include <fnd/asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FlatDoc class
///////////////////////////////////////////////////////////////////////////////
class FlatDoc
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    FlatDoc();
    ~FlatDoc();

    bool Load(const char* path);
    bool Save(const char* path);

    bool GetBool(const char* tag, bool  defVal = false) const;

    int32_t    GetInt (const char* tag, int32_t defVal = 0) const;
    asdx::Int2 GetInt2(const char* tag, asdx::Int2 defVal = asdx::Int2(0, 0)) const;
    asdx::Int3 GetInt3(const char* tag, asdx::Int3 defVal = asdx::Int3(0, 0, 0)) const;
    asdx::Int4 GetInt4(const char* tag, asdx::Int4 defVal = asdx::Int4(0, 0, 0, 0)) const;

    uint32_t    GetUint (const char* tag, uint32_t defVal = 0) const;
    asdx::Uint2 GetUint2(const char* tag, asdx::Uint2 defVal = asdx::Uint2(0, 0)) const;
    asdx::Uint3 GetUint3(const char* tag, asdx::Uint3 defVal = asdx::Uint3(0, 0, 0)) const;
    asdx::Uint4 GetUint4(const char* tag, asdx::Uint4 defVal = asdx::Uint4(0, 0, 0, 0)) const;

    float        GetFloat (const char* tag, float defVal = 0.0f) const;
    asdx::Float2 GetFloat2(const char* tag, asdx::Float2 defVal = asdx::Float2(0.0f, 0.0f)) const;
    asdx::Float3 GetFloat3(const char* tag, asdx::Float3 defVal = asdx::Float3(0.0f, 0.0f, 0.0f)) const;
    asdx::Float4 GetFloat4(const char* tag, asdx::Float4 defVal = asdx::Float4(0.0f, 0.0f, 0.0f, 0.0f)) const;

    asdx::Matrix4x3 GetMatrix4x3(const char* tag, asdx::Matrix4x3 defVal = asdx::Matrix4x3::CreateIdentity()) const;
    asdx::Matrix4x4 GetMatrix4x4(const char* tag, asdx::Matrix4x4 defVal = asdx::Matrix4x4::CreateIdentity()) const;

    std::string GetString(const char* tag, std::string defVal = "") const;

    void SetBool(const char* tag, bool  value);

    void SetInt (const char* tag, int32_t    value);
    void SetInt2(const char* tag, const asdx::Int2& value);
    void SetInt3(const char* tag, const asdx::Int3& value);
    void SetInt4(const char* tag, const asdx::Int4& value);

    void SetUint (const char* tag, uint32_t value);
    void SetUint2(const char* tag, const asdx::Uint2& value);
    void SetUint3(const char* tag, const asdx::Uint3& value);
    void SetUint4(const char* tag, const asdx::Uint4& value);

    void SetFloat (const char* tag, float value);
    void SetFloat2(const char* tag, const asdx::Float2& value);
    void SetFloat3(const char* tag, const asdx::Float3& value);
    void SetFloat4(const char* tag, const asdx::Float4& value);

    void SetMatrix4x3(const char* tag, const asdx::Matrix4x3& value);
    void SetMatrix4x4(const char* tag, const asdx::Matrix4x4& value);

    void SetString(const char* tag, const std::string&   value);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::map<std::string, bool>             m_Bool;

    std::map<std::string, int32_t>          m_Int;
    std::map<std::string, asdx::Int2>       m_Int2;
    std::map<std::string, asdx::Int3>       m_Int3;
    std::map<std::string, asdx::Int4>       m_Int4;

    std::map<std::string, uint32_t>         m_Uint;
    std::map<std::string, asdx::Uint2>      m_Uint2;
    std::map<std::string, asdx::Uint3>      m_Uint3;
    std::map<std::string, asdx::Uint4>      m_Uint4;

    std::map<std::string, float>            m_Float;
    std::map<std::string, asdx::Float2>     m_Float2;
    std::map<std::string, asdx::Float3>     m_Float3;
    std::map<std::string, asdx::Float4>     m_Float4;

    std::map<std::string, asdx::Matrix4x3>  m_Matrix4x3;
    std::map<std::string, asdx::Matrix4x4>  m_Matrix4x4;

    std::map<std::string, std::string>      m_String;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx