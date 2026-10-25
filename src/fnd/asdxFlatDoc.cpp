//-----------------------------------------------------------------------------
// File : asdxFlatDoc.cpp
// Desc : Flat Document.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxFlatDoc.h>
#include <fstream>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FlatDoc class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FlatDoc::FlatDoc()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FlatDoc::~FlatDoc()
{
    m_Bool.clear();

    m_Int .clear();
    m_Int2.clear();
    m_Int3.clear();
    m_Int4.clear();

    m_Uint .clear();
    m_Uint2.clear();
    m_Uint3.clear();
    m_Uint4.clear();

    m_Float .clear();
    m_Float2.clear();
    m_Float3.clear();
    m_Float4.clear();

    m_Matrix4x3.clear();
    m_Matrix4x4.clear();

    m_String.clear();
}

//-----------------------------------------------------------------------------
//      ファイルから読み込みます.
//-----------------------------------------------------------------------------
bool FlatDoc::Load(const char* path)
{
    std::ifstream file;

    // ファイルを開く.
    file.open( path, std::ios::in );

    if (!file.is_open())
    { return false; }

    char buf[ 2048 ];

    for(;;)
    {
        // バッファに格納.
        file >> buf;

        // ファイル末端かチェック
        if ( !file )
            break;

        if ( file.eof() )
            break;

        //　コメント
        if ( 0 == strcmp( buf, "#" ) )
        { /* DO_NOTHING */ }
        else if ( 0 == strcmp( buf, "bool") )
        {
            std::string tag;
            bool value;
            file >> tag >> value;
            m_Bool[tag] = value;
        }
        else if ( 0 == strcmp( buf, "int4") )
        {
            std::string tag;
            asdx::Int4 value;
            file >> tag >> value.x >> value.y >> value.z >> value.w;
            m_Int4[tag] = value;
        }
        else if ( 0 == strcmp( buf, "int3") )
        {
            std::string tag;
            asdx::Int3 value;
            file >> tag >> value.x >> value.y >> value.z;
            m_Int3[tag] = value;
        }
        else if ( 0 == strcmp( buf, "int2") )
        {
            std::string tag;
            asdx::Int2 value;
            file >> tag >> value.x >> value.y;
            m_Int2[tag] = value;
        }
        else if ( 0 == strcmp( buf, "int") )
        {
            std::string tag;
            int32_t value;
            file >> tag >> value;
            m_Int[tag] = value;
        }
        else if ( 0 == strcmp( buf, "uint4") )
        {
            std::string tag;
            asdx::Uint4 value;
            file >> tag >> value.x >> value.y >> value.z >> value.w;
            m_Uint4[tag] = value;
        }
        else if ( 0 == strcmp( buf, "uint3") )
        {
            std::string tag;
            asdx::Uint3 value;
            file >> tag >> value.x >> value.y >> value.z;
            m_Uint3[tag] = value;
        }
        else if ( 0 == strcmp( buf, "uint2") )
        {
            std::string tag;
            asdx::Uint2 value;
            file >> tag >> value.x >> value.y;
            m_Uint2[tag] = value;
        }
        else if ( 0 == strcmp( buf, "uint") )
        {
            std::string tag;
            uint32_t value;
            file >> tag >> value;
            m_Uint[tag] = value;
        }
        else if ( 0 == strcmp( buf, "float4") )
        {
            std::string tag;
            asdx::Float4 value;
            file >> tag >> value.x >> value.y >> value.z >> value.w;
            m_Float4[tag] = value;
        }
        else if ( 0 == strcmp( buf, "float3") )
        {
            std::string tag;
            asdx::Float3 value;
            file >> tag >> value.x >> value.y >> value.z;
            m_Float3[tag] = value;
        }
        else if ( 0 == strcmp( buf, "float2") )
        {
            std::string tag;
            asdx::Float2 value;
            file >> tag >> value.x >> value.y;
            m_Float2[tag] = value;
        }
        else if ( 0 == strcmp( buf, "float") )
        {
            std::string tag;
            float value;
            file >> tag >> value;
            m_Float[tag] = value;
        }
        else if ( 0 == strcmp( buf, "matrix4x3") )
        {
            std::string tag;
            float m[12] = {};
            file >> tag >> m[0]  >> m[1]  >> m[2]
                        >> m[3]  >> m[4]  >> m[5]
                        >> m[6]  >> m[7]  >> m[8]
                        >> m[9]  >> m[10] >> m[11];
            m_Matrix4x3[tag] = asdx::Matrix4x3(
                                m[0], m[1], m[2],
                                m[3], m[4], m[5],
                                m[6], m[7], m[8],
                                m[9], m[10], m[11]);
        }
        else if ( 0 == strcmp( buf, "matrix4x4") )
        {
            std::string tag;
            float m[16] = {};
            file >> tag >> m[0]  >> m[1]  >> m[2]  >> m[3] 
                        >> m[4]  >> m[5]  >> m[6]  >> m[7]
                        >> m[8]  >> m[9]  >> m[10] >> m[11]
                        >> m[12] >> m[13] >> m[14] >> m[15];
            m_Matrix4x4[tag] = asdx::Matrix4x4(m);
        }
        else if ( 0 == strcmp( buf, "string") )
        {
            std::string tag;
            std::string value;
            file >> tag >> value;
            m_String[tag] = value;
        }

        file.ignore( 2048, '\n' );
    }

    file.close();

    return true;
}

//-----------------------------------------------------------------------------
//      ファイルに保存します.
//-----------------------------------------------------------------------------
bool FlatDoc::Save(const char* path)
{
    FILE* pFile;

    auto ret = fopen_s(&pFile, path, "wb");
    if (ret != 0)
    { return false; }

    for(auto& itr : m_Bool)
    { fprintf_s(pFile, "bool %s %d\n", itr.first.c_str(), itr.second); }


    for(auto& itr : m_Int)
    { fprintf_s(pFile, "int %s %d\n", itr.first.c_str(), itr.second); }

    for(auto& itr : m_Int2)
    { fprintf_s(pFile, "int2 %s %d %d\n", itr.first.c_str(), itr.second.x, itr.second.y); }

    for(auto& itr : m_Int3)
    { fprintf_s(pFile, "int3 %s %d %d %d\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z); }

    for(auto& itr : m_Int4)
    { fprintf_s(pFile, "int4 %s %d %d %d %d\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z, itr.second.w); }


    for(auto& itr : m_Uint)
    { fprintf_s(pFile, "uint %s %u\n", itr.first.c_str(), itr.second); }

    for(auto& itr : m_Uint2)
    { fprintf_s(pFile, "uint2 %s %u %u\n", itr.first.c_str(), itr.second.x, itr.second.y); }

    for(auto& itr : m_Uint3)
    { fprintf_s(pFile, "uint3 %s %u %u %u\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z); }

    for(auto& itr : m_Uint4)
    { fprintf_s(pFile, "uint4 %s %u %u %u %u\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z, itr.second.w); }


    for(auto& itr : m_Float)
    { fprintf_s(pFile, "float %s %f\n", itr.first.c_str(), itr.second); }

    for(auto& itr : m_Float2)
    { fprintf_s(pFile, "float2 %s %f %f\n", itr.first.c_str(), itr.second.x, itr.second.y); }

    for(auto& itr : m_Float3)
    { fprintf_s(pFile, "float3 %s %f %f %f\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z); }

    for(auto& itr : m_Float4)
    { fprintf_s(pFile, "float4 %s %f %f %f %f\n", itr.first.c_str(), itr.second.x, itr.second.y, itr.second.z, itr.second.w); }


    for(auto& itr : m_Matrix4x3)
    { 
        fprintf_s(pFile, "matrix4x3 %s %f %f %f %f %f %f %f %f %f %f %f %f\n",
            itr.first.c_str(),
            itr.second._11, itr.second._12, itr.second._13,
            itr.second._21, itr.second._22, itr.second._23,
            itr.second._31, itr.second._32, itr.second._33,
            itr.second._41, itr.second._42, itr.second._43);
    }

    for(auto& itr : m_Matrix4x4)
    { 
        fprintf_s(pFile, "matrix4x4 %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
            itr.first.c_str(),
            itr.second._11, itr.second._12, itr.second._13, itr.second._14,
            itr.second._21, itr.second._22, itr.second._23, itr.second._24,
            itr.second._31, itr.second._32, itr.second._33, itr.second._34,
            itr.second._41, itr.second._42, itr.second._43, itr.second._44);
    }


    for(auto& itr : m_String)
    { fprintf_s(pFile, "string %s %s\n", itr.first.c_str(), itr.second.c_str()); }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      ブール値を取得します.
//-----------------------------------------------------------------------------
bool FlatDoc::GetBool(const char* tag, bool defVal) const
{
    if (m_Bool.find(tag) != std::end(m_Bool))
    { return m_Bool.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
int32_t FlatDoc::GetInt(const char* tag, int32_t defVal) const
{
    if (m_Int.find(tag) != std::end(m_Int))
    { return m_Int.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
asdx::Int2 FlatDoc::GetInt2(const char* tag, asdx::Int2 defVal) const
{
    if (m_Int2.find(tag) != std::end(m_Int2))
    { return m_Int2.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
asdx::Int3 FlatDoc::GetInt3(const char* tag, asdx::Int3 defVal) const
{
    if (m_Int3.find(tag) != std::end(m_Int3))
    { return m_Int3.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
asdx::Int4 FlatDoc::GetInt4(const char* tag, asdx::Int4 defVal) const
{
    if (m_Int4.find(tag) != std::end(m_Int4))
    { return m_Int4.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
uint32_t FlatDoc::GetUint(const char* tag, uint32_t defVal) const
{
    if (m_Int.find(tag) != std::end(m_Int))
    { return m_Int.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
asdx::Uint2 FlatDoc::GetUint2(const char* tag, asdx::Uint2 defVal) const
{
    if (m_Uint2.find(tag) != std::end(m_Uint2))
    { return m_Uint2.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
asdx::Uint3 FlatDoc::GetUint3(const char* tag, asdx::Uint3 defVal) const
{
    if (m_Uint3.find(tag) != std::end(m_Uint3))
    { return m_Uint3.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      整数値を取得します.
//-----------------------------------------------------------------------------
asdx::Uint4 FlatDoc::GetUint4(const char* tag, asdx::Uint4 defVal) const
{
    if (m_Uint4.find(tag) != std::end(m_Uint4))
    { return m_Uint4.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      浮動小数値を取得します.
//-----------------------------------------------------------------------------
float FlatDoc::GetFloat(const char* tag, float defVal) const
{
    if (m_Float.find(tag) != std::end(m_Float))
    { return m_Float.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      浮動小数値を取得します.
//-----------------------------------------------------------------------------
asdx::Float2 FlatDoc::GetFloat2(const char* tag, asdx::Float2 defVal) const
{
    if (m_Float2.find(tag) != std::end(m_Float2))
    { return m_Float2.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      浮動小数値を取得します.
//-----------------------------------------------------------------------------
asdx::Float3 FlatDoc::GetFloat3(const char* tag, asdx::Float3 defVal) const
{
    if (m_Float3.find(tag) != std::end(m_Float3))
    { return m_Float3.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      浮動小数値を取得します.
//-----------------------------------------------------------------------------
asdx::Float4 FlatDoc::GetFloat4(const char* tag, asdx::Float4 defVal) const
{
    if (m_Float4.find(tag) != std::end(m_Float4))
    { return m_Float4.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      行列を取得します.
//-----------------------------------------------------------------------------
asdx::Matrix4x3 FlatDoc::GetMatrix4x3(const char* tag, asdx::Matrix4x3 defVal) const
{
    if (m_Matrix4x3.find(tag) != std::end(m_Matrix4x3))
    { return m_Matrix4x3.at(tag); }

    return defVal;
}


//-----------------------------------------------------------------------------
//      行列を取得します.
//-----------------------------------------------------------------------------
asdx::Matrix4x4 FlatDoc::GetMatrix4x4(const char* tag, asdx::Matrix4x4 defVal) const
{
    if (m_Matrix4x4.find(tag) != std::end(m_Matrix4x4))
    { return m_Matrix4x4.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      テキストを取得します.
//-----------------------------------------------------------------------------
std::string FlatDoc::GetString(const char* tag, std::string defVal) const
{
    if (m_String.find(tag) != std::end(m_String))
    { return m_String.at(tag); }

    return defVal;
}

//-----------------------------------------------------------------------------
//      ブール値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetBool(const char* tag, bool value)
{ m_Bool[tag] = value; }

//-----------------------------------------------------------------------------
//      整数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetInt(const char* tag, int value)
{ m_Int[tag] = value; }

//-----------------------------------------------------------------------------
//      整数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetInt2(const char* tag, const asdx::Int2& value)
{ m_Int2[tag] = value; }

//-----------------------------------------------------------------------------
//      整数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetInt3(const char* tag, const asdx::Int3& value)
{ m_Int3[tag] = value; }

//-----------------------------------------------------------------------------
//      整数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetInt4(const char* tag, const asdx::Int4& value)
{ m_Int4[tag] = value; }

//-----------------------------------------------------------------------------
//      浮動小数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetFloat(const char* tag, float value)
{ m_Float[tag] = value; }

//-----------------------------------------------------------------------------
//      浮動小数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetFloat2(const char* tag, const asdx::Float2& value)
{ m_Float2[tag] = value; }

//-----------------------------------------------------------------------------
//      浮動小数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetFloat3(const char* tag, const asdx::Float3& value)
{ m_Float3[tag] = value; }

//-----------------------------------------------------------------------------
//      浮動小数値を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetFloat4(const char* tag, const asdx::Float4& value)
{ m_Float4[tag] = value; }

//-----------------------------------------------------------------------------
//      行列を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetMatrix4x3(const char* tag, const asdx::Matrix4x3& value)
{ m_Matrix4x3[tag] = value; }

//-----------------------------------------------------------------------------
//      行列を設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetMatrix4x4(const char* tag, const asdx::Matrix4x4& value)
{ m_Matrix4x4[tag] = value; }

//-----------------------------------------------------------------------------
//      テキストを設定します.
//-----------------------------------------------------------------------------
void FlatDoc::SetString(const char* tag, const std::string& value)
{ m_String[tag] = value; }

} // namespace asdx
