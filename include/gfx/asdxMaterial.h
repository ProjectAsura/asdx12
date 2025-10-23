//-----------------------------------------------------------------------------
// File : asdxMaterial.h
// Desc : Material System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include <fnd/asdxRef.h>
#include <fnd/asdxStringView.h>


namespace asdx {

class ConstantBuffer;
class Texture;

///////////////////////////////////////////////////////////////////////////////
// PropertyType enum
///////////////////////////////////////////////////////////////////////////////
enum PropertyType
{
    Bool,
    S8,
    S16,
    S32,
    S64,
    U8,
    U16,
    U32,
    U64,
    F32,
    F64,
};

///////////////////////////////////////////////////////////////////////////////
// ResProperty structure
///////////////////////////////////////////////////////////////////////////////
struct ResProperty
{
    StringView      Name;       //!< プロパティ名.
    PropertyType    Type;       //!< データ型.
    uint32_t        Offset;     //!< オフセット.
    uint32_t        Size;       //!< サイズ.
};

///////////////////////////////////////////////////////////////////////////////
// IMaterial interface
///////////////////////////////////////////////////////////////////////////////
struct IMaterial : public IReference
{
    virtual ~IMaterial()
    {}

    virtual const std::string& GetShaderPath() const = 0;

    virtual ConstantBuffer& GetConstantBuffer() = 0;

    virtual uint32_t GetTextureCount() const = 0;

    virtual const Texture& GetTexture(uint32_t index) const = 0;

    virtual void GetParamInfo(size_t index, ResProperty& prop) const = 0;

    virtual bool FindParamInfo(const char* name, ResProperty& prop) const = 0;

};

///////////////////////////////////////////////////////////////////////////////
// IMaterialInstance interface
///////////////////////////////////////////////////////////////////////////////
struct IMaterialInstance : public IMaterial
{
    virtual ~IMaterialInstance()
    {}

    virtual void GetParam(void* dst, uint32_t offset, uint32_t size) const = 0;

    virtual void SetParam(const void* dst, uint32_t offset, uint32_t size) = 0;
};

struct IMaterialManager
{
    virtual ~IMaterialManager()
    {}

    virtual bool CreateMaterial(const std::vector<uint8_t>& blob, IMaterial** ppMaterial) = 0;

    virtual bool CreateMaterialInstance(IMaterial* pMasterMaterial, IMaterialInstance** ppInstance) = 0;
};

} // namespace asdx
