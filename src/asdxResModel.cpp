//-----------------------------------------------------------------------------
// File : asdxResModel.cpp
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxResModel.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// TangentSpace
///////////////////////////////////////////////////////////////////////////////
union TangentSpace
{
    struct {
    uint32_t    NormalX             : 10;
    uint32_t    NormalY             : 10;
    uint32_t    CosAngle            : 8;
    uint32_t    CompIndex           : 2;
    uint32_t    TangentHandedness   : 1;
    uint32_t    BinormalHandedness  : 1;
    };
    uint32_t u;
};
static_assert(sizeof(TangentSpace) == sizeof(uint32_t), "TangentSpace Invalid Data Size");

///////////////////////////////////////////////////////////////////////////////
// TexCoord
///////////////////////////////////////////////////////////////////////////////
union TexCoord
{
    struct 
    {
        uint16_t x;
        uint16_t y;
    };
    uint32_t u;
};

///////////////////////////////////////////////////////////////////////////////
// Unorm88
///////////////////////////////////////////////////////////////////////////////
union Unorm8888
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t c;
};

///////////////////////////////////////////////////////////////////////////////
// FP32 
///////////////////////////////////////////////////////////////////////////////
union FP32
{
    uint32_t    u;
    float       f;
    struct
    {
        uint32_t Mantissa   : 23;
        uint32_t Exponent   : 8;
        uint32_t Sign       : 1;
    };
};

///////////////////////////////////////////////////////////////////////////////
// FP16
///////////////////////////////////////////////////////////////////////////////
union FP16
{
    uint16_t u;
    struct
    {
        uint32_t Mantissa   : 10;
        uint32_t Exponent   : 5;
        uint32_t Sign       : 1;
    };
};

///////////////////////////////////////////////////////////////////////////////
// ModelHeader structure
///////////////////////////////////////////////////////////////////////////////
struct ModelHeader
{
    uint8_t     Magic[4];
    uint32_t    Version;
    uint32_t    ModelHash;
    uint32_t    StaticMeshCount;
    uint32_t    SkinningMeshCount;
};

///////////////////////////////////////////////////////////////////////////////
/// MeshHeader structure
///////////////////////////////////////////////////////////////////////////////
struct MeshHeader
{
    uint32_t    MaterialHash;
    uint32_t    VertexCount;
    uint32_t    IndexCount;
    uint32_t    PrimitiveCount;
    uint32_t    MeshletCount;
    uint32_t    CullingInfoCount;
};

static const FP32       kMagic      = { 113 << 23 };
static const uint32_t   kShiftedExp = 0x7c00 << 13;


//-----------------------------------------------------------------------------
//      最大成分を取得します.
//-----------------------------------------------------------------------------
inline float Max3(const asdx::Vector3& value)
{ return asdx::Max(value.x, asdx::Max(value.y, value.z)); }

} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      メッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResStaticMesh& resource)
{
    resource.Vertices.clear();
    resource.Vertices.shrink_to_fit();

    resource.Indices.clear();
    resource.Indices.shrink_to_fit();

    resource.Primitives.clear();
    resource.Primitives.shrink_to_fit();

    resource.Meshlets.clear();
    resource.Meshlets.shrink_to_fit();

    resource.CullingInfos.clear();
    resource.CullingInfos.shrink_to_fit();

    resource.MatrerialHash = 0;
}

//-----------------------------------------------------------------------------
//      メッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResSkinningMesh& resource)
{
    resource.Vertices.clear();
    resource.Vertices.shrink_to_fit();

    resource.Indices.clear();
    resource.Indices.shrink_to_fit();

    resource.Primitives.clear();
    resource.Primitives.shrink_to_fit();

    resource.Meshlets.clear();
    resource.Meshlets.shrink_to_fit();

    resource.CullingInfos.clear();
    resource.CullingInfos.shrink_to_fit();

    resource.MatrerialHash = 0;
}

//-----------------------------------------------------------------------------
//      モデルの破棄処理を行います.
//-----------------------------------------------------------------------------
void Dispose(ResModel& resource)
{
    for(size_t i=0; i<resource.StaticMeshes.size(); ++i)
    { Dispose(resource.StaticMeshes[i]); }

    for(size_t i=0; i<resource.SkinningMeshes.size(); ++i)
    { Dispose(resource.SkinningMeshes[i]); }

    resource.StaticMeshes.clear();
    resource.StaticMeshes.shrink_to_fit();

    resource.SkinningMeshes.clear();
    resource.SkinningMeshes.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      半精度浮動小数に変換します.
//-----------------------------------------------------------------------------
uint16_t ToHalf(float value)
{
    FP32 f;
    f.f = value;

    FP16 o = { 0 };

    if (f.Exponent == 0)
    { o.Exponent = 0; }

    else if (f.Exponent == 255)
    {
        o.Exponent = 31;
        o.Mantissa = f.Mantissa ? 0x200 : 0;
    }
    else
    {
        int newexp = f.Exponent - 127 + 15;
        
        if (newexp >= 31)
        { o.Exponent = 31; }
 
        else if (newexp <= 0)
        {
            if ((14 - newexp) <= 24)
            {
                uint16_t mant = f.Mantissa | 0x800000;
                o.Mantissa = mant >> (14 - newexp);
                if ((mant >> (13 - newexp)) & 1)
                { o.u++; }
            }
        }
        else
        {
            o.Exponent = newexp;
            o.Mantissa = f.Mantissa >> 13;
            if (f.Mantissa & 0x1000)
            { o.u++; }
        }
    }

    o.Sign = f.Sign;
    return o.u;
}

//-----------------------------------------------------------------------------
//      単精度浮動小数に変換します.
//-----------------------------------------------------------------------------
float ToFloat(uint16_t v)
{
    FP16 h;
    h.u = v;

    FP32 o;

    o.u = (h.u & 0x7fff) << 13;
    uint32_t exp = kShiftedExp & o.u;
    o.u += (127 - 15) << 23;

    if (exp == kShiftedExp)
    { o.u += (128 - 16) << 23; }
    else if (exp == 0)
    {
        o.u += 1 << 23;
        o.f -= kMagic.f;
    }

    o.u |= (h.u & 0x8000) << 16;
    return o.f;
}

//-----------------------------------------------------------------------------
//      R8G8B8A8_UNORM形式に変換します.
//-----------------------------------------------------------------------------
uint32_t ToUnorm(const Vector4& value)
{
    Unorm8888 result;
    result.r = uint8_t(value.x * 255.0f);
    result.g = uint8_t(value.y * 255.0f);
    result.b = uint8_t(value.z * 255.0f);
    result.a = uint8_t(value.w * 255.0f);
    return result.c;
}

//-----------------------------------------------------------------------------
//      R8G8B8A8_UNORM形式からVector4に変換します.
//-----------------------------------------------------------------------------
Vector4 FromUnorm(uint32_t value)
{
    Unorm8888 packed;
    packed.c = value;

    Vector4 result;
    result.x = float(packed.r) / 255.0f;
    result.y = float(packed.g) / 255.0f;
    result.z = float(packed.b) / 255.0f;
    result.w = float(packed.a) / 255.0f;
    return result;
}

//-----------------------------------------------------------------------------
//      テクスチャ座標を圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeTexCoord(const Vector2& value)
{
    TexCoord packed;
    packed.x = ToHalf(value.x);
    packed.y = ToHalf(value.y);
    return packed.u;
}

//-----------------------------------------------------------------------------
//      圧縮されたテクスチャ座標を展開します.
//-----------------------------------------------------------------------------
Vector2 DecodeTexCoord(uint32_t value)
{
    TexCoord packed;
    packed.u = value;

    Vector2 result;
    result.x = ToFloat(packed.x);
    result.y = ToFloat(packed.y);
    return result;
}

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
Vector2 OctWrap(const Vector2& value)
{
    Vector2 result;
    result.x = (1.0f - abs(value.y)) * (value.x >= 0.0f ? 1.0f : -1.0f);
    result.y = (1.0f - abs(value.x)) * (value.y >= 0.0f ? 1.0f : -1.0f);
    return result;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
Vector2 PackNormal(const Vector3& value)
{
    auto mag = abs(value.x) + abs(value.y) + abs(value.z);
    auto n = value / mag;
    Vector2 result(n.x, n.y);
    result = (n.z >= 0.0f) ? result : OctWrap(result);
    result *= 0.5f;
    result += Vector2(0.5f, 0.5f);
    return result;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
Vector3 UnpackNormal(const Vector2& value)
{
    auto encoded = value * 2.0f - Vector2(1.0f, 1.0f);
    auto n = Vector3(encoded.x, encoded.y, 1.0f - abs(encoded.x) - abs(encoded.y));
    auto t = Saturate(-n.z);
    n.x += (n.x >= 0.0f) ? -t : t;
    n.y += (n.y >= 0.0f) ? -t : t;
    return Vector3::Normalize(n);
}

//-----------------------------------------------------------------------------
//      接線空間を圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeTBN(const Vector3& normal, const Vector3& tangent, uint8_t binormalHandedness)
{
    auto packedNormal = PackNormal(normal);

    TangentSpace packed;
    packed.NormalX = uint32_t(packedNormal.x * 1023.0);
    packed.NormalY = uint32_t(packedNormal.y * 1023.0);

    auto tangentAbs = Vector3::Abs(tangent);
    auto maxComp = Max3(tangentAbs);

    Vector3 refVector;
    uint32_t compIndex = 0;
    if (maxComp == tangentAbs.x)
    {
        refVector = Vector3(1.0f, 0.0f, 0.0f);
        compIndex = 0;
    }
    else if (maxComp == tangentAbs.y)
    {
        refVector = Vector3(0.0f, 1.0f, 0.0f);
        compIndex = 1;
    }
    else if (maxComp == tangentAbs.z)
    {
        refVector = Vector3(0.0f, 0.0f, 1.0f);
        compIndex = 2;
    }

    auto orthoA = Vector3::Normalize(Vector3::Cross(normal, refVector));
    auto orthoB = Vector3::Cross(normal, orthoA);
    uint8_t cosAngle = uint8_t((Vector3::Dot(tangent, orthoA) * 0.5f + 0.5f) * 255.0f);
    uint8_t tangentHandedness = (Vector3::Dot(tangent, orthoB) > 0.0001f) ? 1 : 0;

    packed.CompIndex            = compIndex;
    packed.CosAngle             = cosAngle;
    packed.TangentHandedness    = tangentHandedness;
    packed.BinormalHandedness   = binormalHandedness;

    return packed.u;
}

//-----------------------------------------------------------------------------
//      圧縮された接線空間を展開します.
//-----------------------------------------------------------------------------
void DecodeTBN(uint32_t encoded, Vector3& tangent, Vector3& bitangent, Vector3& normal)
{
    TangentSpace packed;
    packed.u = encoded;

    normal = UnpackNormal(Vector2(packed.NormalX / 1023.0f, packed.NormalY / 1023.0f));

    Vector3 refVector;
    uint8_t compIndex = (packed.CompIndex);
    if (compIndex == 0)
    {
        refVector = Vector3(1.0f, 0.0f, 0.0f);
    }
    else if (compIndex == 1)
    {
        refVector = Vector3(0.0f, 1.0f, 0.0f);
    }
    else if (compIndex == 2)
    {
        refVector = Vector3(0.0f, 0.0f, 1.0f);
    }

    float cosAngle = (packed.CosAngle / 255.0f) * 2.0f - 1.0f;
    float sinAngle = sqrt(Saturate(1.0f - cosAngle * cosAngle));
    sinAngle = (packed.TangentHandedness == 0) ? -sinAngle : sinAngle;

    auto orthoA = Vector3::Normalize(Vector3::Cross(normal, refVector));
    auto orhotB = Vector3::Cross(normal, orthoA);
    tangent = Vector3::Normalize((cosAngle * orthoA) + (sinAngle * orhotB));

    bitangent = Vector3::Cross(normal, tangent);
    bitangent = (packed.BinormalHandedness == 0) ? bitangent : -bitangent; 
}

//-----------------------------------------------------------------------------
//      モデルを保存します.
//-----------------------------------------------------------------------------
bool SaveModel(const char* path, const ResModel& model)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "wb");
    if (err != 0)
    { return false; }

    ModelHeader header;
    header.Magic[0] = 'M';
    header.Magic[1] = 'D';
    header.Magic[2] = 'L';
    header.Magic[3] = '\0';
    header.Version = 0x1;

    header.ModelHash            = model.ModelHash;
    header.StaticMeshCount      = uint32_t(model.StaticMeshes.size());
    header.SkinningMeshCount    = uint32_t(model.SkinningMeshes.size());

    fwrite(&header, sizeof(header), 1, pFile);

    for(size_t i=0; i<model.StaticMeshes.size(); ++i)
    {
        auto& mesh = model.StaticMeshes[i];

        MeshHeader meshHeader;
        meshHeader.MaterialHash     = mesh.MatrerialHash;
        meshHeader.VertexCount      = uint32_t(mesh.Vertices.size());
        meshHeader.IndexCount       = uint32_t(mesh.Indices.size());
        meshHeader.PrimitiveCount   = uint32_t(mesh.Primitives.size());
        meshHeader.MeshletCount     = uint32_t(mesh.Meshlets.size());
        meshHeader.CullingInfoCount = uint32_t(mesh.CullingInfos.size());

        fwrite(&meshHeader, sizeof(meshHeader), 1, pFile);

        for(size_t j=0; j<mesh.Vertices.size(); ++j)
        { fwrite(&mesh.Vertices[j], sizeof(mesh.Vertices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Indices.size(); ++j)
        { fwrite(&mesh.Indices[j], sizeof(mesh.Indices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Primitives.size(); ++j)
        { fwrite(&mesh.Primitives[j], sizeof(mesh.Primitives[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Meshlets.size(); ++j)
        { fwrite(&mesh.Meshlets[j], sizeof(mesh.Meshlets[j]), 1, pFile); }

        for(size_t j=0; j<mesh.CullingInfos.size(); ++j)
        { fwrite(&mesh.CullingInfos[j], sizeof(mesh.CullingInfos[j]), 1, pFile); }
    }

    for(size_t i=0; i<model.SkinningMeshes.size(); ++i)
    {
        auto& mesh = model.SkinningMeshes[i];

        MeshHeader meshHeader;
        meshHeader.MaterialHash     = mesh.MatrerialHash;
        meshHeader.VertexCount      = uint32_t(mesh.Vertices.size());
        meshHeader.IndexCount       = uint32_t(mesh.Indices.size());
        meshHeader.PrimitiveCount   = uint32_t(mesh.Primitives.size());
        meshHeader.MeshletCount     = uint32_t(mesh.Meshlets.size());
        meshHeader.CullingInfoCount = uint32_t(mesh.CullingInfos.size());

        fwrite(&meshHeader, sizeof(meshHeader), 1, pFile);

        for(size_t j=0; j<mesh.Vertices.size(); ++j)
        { fwrite(&mesh.Vertices[j], sizeof(mesh.Vertices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Indices.size(); ++j)
        { fwrite(&mesh.Indices[j], sizeof(mesh.Indices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Primitives.size(); ++j)
        { fwrite(&mesh.Primitives[j], sizeof(mesh.Primitives[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Meshlets.size(); ++j)
        { fwrite(&mesh.Meshlets[j], sizeof(mesh.Meshlets[j]), 1, pFile); }

        for(size_t j=0; j<mesh.CullingInfos.size(); ++j)
        { fwrite(&mesh.CullingInfos[j], sizeof(mesh.CullingInfos[j]), 1, pFile); }
    }

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      モデルを読込します.
//-----------------------------------------------------------------------------
bool LoadModel(const char* path, ResModel& model)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "rb");
    if (err != 0)
    {
        return false;
    }

    ModelHeader modelHeader;
    fread(&modelHeader, sizeof(modelHeader), 1, pFile);

    model.ModelHash = modelHeader.ModelHash;
    model.StaticMeshes.resize(modelHeader.StaticMeshCount);
    model.SkinningMeshes.resize(modelHeader.SkinningMeshCount);

    for(size_t i=0; i<model.StaticMeshes.size(); ++i)
    {
        MeshHeader meshHeader;
        fread(&meshHeader, sizeof(meshHeader), 1, pFile);

        auto& mesh = model.StaticMeshes[i];

        mesh.MatrerialHash = meshHeader.MaterialHash;
        mesh.Vertices    .resize(meshHeader.VertexCount);
        mesh.Indices     .resize(meshHeader.IndexCount);
        mesh.Primitives  .resize(meshHeader.PrimitiveCount);
        mesh.Meshlets    .resize(meshHeader.MeshletCount);
        mesh.CullingInfos.resize(meshHeader.CullingInfoCount);

        for(size_t j=0; j<mesh.Vertices.size(); ++j)
        { fread(&mesh.Vertices[j], sizeof(mesh.Vertices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Indices.size(); ++j)
        { fread(&mesh.Indices[j], sizeof(mesh.Indices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Primitives.size(); ++j)
        { fread(&mesh.Primitives[j], sizeof(mesh.Primitives[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Meshlets.size(); ++j)
        { fread(&mesh.Meshlets[j], sizeof(mesh.Meshlets[j]), 1, pFile); }

        for(size_t j=0; j<mesh.CullingInfos.size(); ++j)
        { fread(&mesh.CullingInfos[j], sizeof(mesh.CullingInfos[j]), 1, pFile); }
    }

    for(size_t i=0; i<model.SkinningMeshes.size(); ++i)
    {
        MeshHeader meshHeader;
        fread(&meshHeader, sizeof(meshHeader), 1, pFile);

        auto& mesh = model.SkinningMeshes[i];

        mesh.MatrerialHash = meshHeader.MaterialHash;
        mesh.Vertices    .resize(meshHeader.VertexCount);
        mesh.Indices     .resize(meshHeader.IndexCount);
        mesh.Primitives  .resize(meshHeader.PrimitiveCount);
        mesh.Meshlets    .resize(meshHeader.MeshletCount);
        mesh.CullingInfos.resize(meshHeader.CullingInfoCount);

        for(size_t j=0; j<mesh.Vertices.size(); ++j)
        { fread(&mesh.Vertices[j], sizeof(mesh.Vertices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Indices.size(); ++j)
        { fread(&mesh.Indices[j], sizeof(mesh.Indices[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Primitives.size(); ++j)
        { fread(&mesh.Primitives[j], sizeof(mesh.Primitives[j]), 1, pFile); }

        for(size_t j=0; j<mesh.Meshlets.size(); ++j)
        { fread(&mesh.Meshlets[j], sizeof(mesh.Meshlets[j]), 1, pFile); }

        for(size_t j=0; j<mesh.CullingInfos.size(); ++j)
        { fread(&mesh.CullingInfos[j], sizeof(mesh.CullingInfos[j]), 1, pFile); }
    }

    fclose(pFile);
    return true;
}

} // namespace asdx
