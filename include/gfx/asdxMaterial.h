//-----------------------------------------------------------------------------
// File : asdxMaterial.h
// Desc : Material System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxRef.h>
#include <res/asdxResMaterial.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IMaterial interface
///////////////////////////////////////////////////////////////////////////////
struct IMaterial : public IReference
{
    virtual ~IMaterial()
    {}


};

///////////////////////////////////////////////////////////////////////////////
// IMaterialInstance interface
///////////////////////////////////////////////////////////////////////////////
struct IMaterialInstance : public IMaterial
{
    virtual ~IMaterialInstance()
    {}

    virtual bool SetProperty(uint32_t index, const void* data, uint32_t size) = 0;

    template<typename T>
    bool SetPropertyAs(uint32_t index, const T& data)
    { return SetProperty(index, &data, sizeof(T)); }
};

struct IMaterialManager
{
    virtual ~IMaterialManager()
    {}

    virtual bool CreateMaterial(const std::vector<uint8_t>& blob, IMaterial** ppMaterial) = 0;

    virtual bool CreateMaterialInstance(IMaterial* pMasterMaterial, IMaterialInstance** ppInstance) = 0;
};

} // namespace asdx
