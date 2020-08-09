//-----------------------------------------------------------------------------
// File : asdxRootSignature.h
// Desc : Root Signature.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <list>
#include <map>
#include <string>
#include <d3d12.h>
#include <asdxRef.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SHADER_VISIBILITY enum
///////////////////////////////////////////////////////////////////////////////
enum SHADER_VISIBILITY
{
    SV_ALL  = D3D12_SHADER_VISIBILITY_ALL,
    SV_VS   = D3D12_SHADER_VISIBILITY_VERTEX,
    SV_HS   = D3D12_SHADER_VISIBILITY_HULL,
    SV_DS   = D3D12_SHADER_VISIBILITY_DOMAIN,
    SV_GS   = D3D12_SHADER_VISIBILITY_GEOMETRY,
    SV_PS   = D3D12_SHADER_VISIBILITY_PIXEL,
    SV_AS   = D3D12_SHADER_VISIBILITY_AMPLIFICATION,
    SV_MS   = D3D12_SHADER_VISIBILITY_MESH
};

///////////////////////////////////////////////////////////////////////////////
// STATIC_SAMPLER_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum STATIC_SAMPLER_TYPE
{
    SST_POINT_CLAMP,
    SST_POINT_WRAP,
    SST_POINT_MIRROR,
    SST_LINEAR_CLAMP,
    SST_LINEAR_WRAP,
    SST_LINEAR_MIRROR,
    SST_ANISOTROPIC_CLAMP,
    SST_ANISOTROPIC_WRAP,
    SST_ANISOTROPIC_MIRROR,
};

///////////////////////////////////////////////////////////////////////////////
// ROOT_SIGNATURE_FLAG enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_SIGNATURE_FLAG
{
    RSF_NONE        = D3D12_ROOT_SIGNATURE_FLAG_NONE,
    RSF_DENY_VS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS,
    RSF_DENY_GS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS,
    RSF_DENY_HS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS,
    RSF_DENY_DS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS,
    RSF_DENY_PS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS,
    RSF_DENY_AS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS,
    RSF_DENY_MS     = D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS,
    RSF_ALLOW_IA    = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
    RSF_ALLOW_SO    = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT,
};

///////////////////////////////////////////////////////////////////////////////
// RootSignatureDesc class
///////////////////////////////////////////////////////////////////////////////
class RootSignatureDesc
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    friend class RootSignature;

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    RootSignatureDesc();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~RootSignatureDesc();

    //-------------------------------------------------------------------------
    //! @brief      定数バッファビューを追加します.
    //!
    //! @param[in]      visibility      シェーダアクセス設定.
    //! @param[in]      shaderRegister  レジスタ番号.
    //! @param[in]      registerSpace   レジスタスペース.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddCBV(
        const char*         tag,
        SHADER_VISIBILITY   visibility,
        uint32_t            shaderRegister,
        uint32_t            registerSpace = 0);

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを追加します.
    //!
    //! @param[in]      visibility      シェーダアクセス設定.
    //! @param[in]      shaderRegister  レジスタ番号.
    //! @param[in]      registerSpace   レジスタスペース.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddSRV(
        const char*         tag,
        SHADER_VISIBILITY   visibility,
        uint32_t            shaderRegister,
        uint32_t            registerSpace = 0);

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを追加します.
    //!
    //! @param[in]      visibility      シェーダアクセス設定.
    //! @param[in]      shaderRegister  レジスタ番号.
    //! @param[in]      registerSpace   レジスタスペース.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddUAV(
        const char*         tag,
        SHADER_VISIBILITY   visibility,
        uint32_t            shaderRegister,
        uint32_t            registerSpace = 0);

    //-------------------------------------------------------------------------
    //! @brief      サンプラーを追加します.
    //!
    //! @param[in]      visibility      シェーダアクセス設定.
    //! @param[in]      shaderRegister  レジスタ番号.
    //! @param[in]      registerSpace   レジスタスペース.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddSampler(
        const char*         tag,
        SHADER_VISIBILITY   visibility,
        uint32_t            shaderRegister,
        uint32_t            registerSpace = 0);

    //-------------------------------------------------------------------------
    //! @brief      32bit定数を追加します.
    //!
    //! @param[in]      visibility      シェーダアクセス設定.
    //! @param[in]      count32Bitvalue     定数の数.
    //! @param[in]      shaderRegister      レジスタ番号.
    //! @param[in]      registerSpace       レジスタスペース.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddConstant(
        const char*         tag,
        SHADER_VISIBILITY   visibility,
        uint32_t            count32BitValues,
        uint32_t            shaderRegister,
        uint32_t            registerSpace = 0);

    //-------------------------------------------------------------------------
    //! @brief      静的サンプラーを追加します.
    //!
    //! @param[in]      visibility      シェーダアクセス設定.
    //! @param[in]      type            サンプラータイプ.
    //! @param[in]      shaderRegister  レジスタ番号.
    //! @param[in]      registerSpace   レジスタスペース.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddStaticSampler(
        SHADER_VISIBILITY   visibility,
        STATIC_SAMPLER_TYPE type,
        uint32_t            shaderRegister,
        uint32_t            registerSpace = 0);

    //-------------------------------------------------------------------------
    //! @brief      静的サンプラーを追加します.
    //!
    //! @param[in]      deec            静的サンプラー設定.
    //! @return     ルートパラメータ番号を返却します.
    //-------------------------------------------------------------------------
    uint32_t AddStaticSampler(D3D12_STATIC_SAMPLER_DESC desc);

    //-------------------------------------------------------------------------
    //! @brief      フラグを設定します.
    //!
    //! @param[in]      flags           設定するフラグ.
    //-------------------------------------------------------------------------
    void SetFlag(uint32_t flags);

    //-------------------------------------------------------------------------
    //! @brief      シェーダコードからルートパラメータを追加します.
    //!
    //! @param[in]      pByteCode       シェーダバイトコードです.
    //! @param[in]      byteCodeSize    バイトコードサイズです.
    //-------------------------------------------------------------------------
    RootSignatureDesc& AddFromShader(const void* pByteCode, size_t byteCodeSize);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::vector<D3D12_ROOT_PARAMETER>       m_Params;
    std::vector<D3D12_STATIC_SAMPLER_DESC>  m_Samplers;
    std::list<D3D12_DESCRIPTOR_RANGE*>      m_pRange;
    D3D12_ROOT_SIGNATURE_FLAGS              m_Flags;
    std::vector<std::string>                m_Tags;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// RootSignature class
///////////////////////////////////////////////////////////////////////////////
class RootSignature
{
    //=========================================================================
    // friend classes and methods.
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

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    RootSignature();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~RootSignature();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      desc        ルートシグニチャ設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* pDevice, RootSignatureDesc& desc);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ルートパラメータ番号を探索します.
    //!
    //! @param[in]      tag     タグ名です.
    //! @return     ルートパラメータ番号を返却します. 
    //!             見つからなかった場合はUINT32_MAXを返却します.
    //-------------------------------------------------------------------------
    uint32_t Find(const char* tag) const;

    //-------------------------------------------------------------------------
    //! @brief      ルートシグニチャを取得します.
    //!
    //! @return     ルートシグニチャを返却します.
    //-------------------------------------------------------------------------
    ID3D12RootSignature* GetPtr() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::RefPtr<ID3D12RootSignature>   m_RootSignature;
    std::map<std::string, uint32_t>     m_RootParamIndex;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
