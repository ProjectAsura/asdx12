//-----------------------------------------------------------------------------
// File : asdxUploadResource.h
// Desc : Upload Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <core/asdxRef.h>
#include <asdxResTexture.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IUploadResource interface
///////////////////////////////////////////////////////////////////////////////
struct IUploadResource : public IReference
{
    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    virtual ~IUploadResource() 
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      アップロードコマンドを発行します.
    //!
    //! @param[in]      pCmdList        コマンドリスト.
    //-------------------------------------------------------------------------
    virtual void Upload(ID3D12GraphicsCommandList* pCmdList) = 0;
};

//-----------------------------------------------------------------------------
//! @brief      アップロードテクスチャリソースを生成します.
//! 
//! @param[in]      pDest       転送先リソースです.
//! @param[in]      resource    テクスチャリソース.
//! @param[out]     ppResource  格納先.
//! @retval true    生成に成功.
//! @retval fasle   生成に失敗.
//-----------------------------------------------------------------------------
bool CreateUploadTexture(
    ID3D12Resource*     pDest,
    const ResTexture&   resource,
    IUploadResource**   ppResource);

//-----------------------------------------------------------------------------
//! @brief      アップロードバッファリソースを生成します.
//! 
//! @param[in]      pDest       転送先リソースです.
//! @param[in]      resource    リソース
//! @param[out]     ppResource  格納先
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-----------------------------------------------------------------------------
bool CreateUploadBuffer(
    ID3D12Resource* pDest,
    const void* resource,
    IUploadResource** ppResource);

//-----------------------------------------------------------------------------
//! @brief      アップロードリソースを更新します.
//! 
//! @param[in]      pResource       アップロードリソース
//! @param[in]      resource        更新データ.
//-----------------------------------------------------------------------------
void UpdateTextureResource(IUploadResource* pResource, const ResTexture& resource);

//-----------------------------------------------------------------------------
//! @brief      アップロードリソースを更新します.
//! 
//! @param[in]      pResource       アップロードリソース
//! @param[in]      resource        更新データ.
//-----------------------------------------------------------------------------
void UpdateBufferResource(IUploadResource* pResource, const void* resource);

} // namespace asdx