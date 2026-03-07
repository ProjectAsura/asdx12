//-----------------------------------------------------------------------------
// File : LevelConverter.h
// Desc : Level Binary (*.lvb) Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <vector>
#include <fnd/asdxMath.h>


///////////////////////////////////////////////////////////////////////////////
// EditModelInstance structure
///////////////////////////////////////////////////////////////////////////////
struct EditModelInstance
{
    std::string         Tag;        //!< 名前.
    std::string         Path;       //!< mdb ファイルパス.
    asdx::Vector3       Position;   //!< 位置座標.
    asdx::Quaternion    Rotation;   //!< 回転量.
    asdx::Vector3       Scale;      //!< 拡大・縮小値.
    uint64_t            UserId;     //!< ユーザーデータ.
};

///////////////////////////////////////////////////////////////////////////////
// EditPointLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditPointLight
{
    std::string         Tag;        //!< 名前.
    asdx::Vector3       Position;   //!< 位置座標.
    asdx::Vector3       Color;      //!< 色.
    float               Intensity;  //!< 強度.
    float               Radius;     //!< 半径.
};

///////////////////////////////////////////////////////////////////////////////
// EditSpotLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditSpotLight
{
    std::string         Tag;        //!< 名前.
    asdx::Vector3       Position;   //!< 位置座標.
    asdx::Vector3       Direction;  //!< 照射方向.
    asdx::Vector3       Color;      //!< 色.
    float               Intensity;  //!< 強度.
    float               Radius;     //!< 半径.
    float               InnerAngle; //!< 内角(radian).
    float               OuterAngle; //!< 外角(radian).
};

///////////////////////////////////////////////////////////////////////////////
// EditDirectionalLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditDirectionalLight
{
    std::string         Tag;        //!< 名前.
    asdx::Vector3       Direction;  //!< 照射方向.
    asdx::Vector3       Color;      //!< 色.
    float               Intensity;  //!< 強度.
};

///////////////////////////////////////////////////////////////////////////////
// EditImageBasedLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditImageBasedLight
{
    std::string         Tag;        //!< 名前.
    std::string         Path;       //!< txb ファイルパス.
    float               Intensity;  //!< 強度.
};

///////////////////////////////////////////////////////////////////////////////
// EditPin structure
///////////////////////////////////////////////////////////////////////////////
struct EditPin
{
    std::string     Tag;        //!< 名前.
    asdx::Vector3   Position;   //!< 位置座標.
    uint64_t        UserId;     //!< ユーザーデータ.
};

///////////////////////////////////////////////////////////////////////////////
// EditVolume structure
///////////////////////////////////////////////////////////////////////////////
struct EditVolume
{
    std::string         Tag;        //!< 名前.
    asdx::Vector3       Position;   //!< 位置座標.
    asdx::Quaternion    Rotation;   //!< 回転量.
    asdx::Vector3       Scale;      //!< 拡大・縮小値.
    uint64_t            UserId;     //!< ユーザーデータ.
};

///////////////////////////////////////////////////////////////////////////////
// EditLevel structure
///////////////////////////////////////////////////////////////////////////////
struct EditLevel
{
    std::vector<EditModelInstance>      ModelInstances;     //!< モデルインスタンス.
    std::vector<EditPointLight>         PointLights;        //!< ポイントライト.
    std::vector<EditSpotLight>          SpotLights;         //!< スポットライト.
    std::vector<EditDirectionalLight>   DirLights;          //!< ディレクショナルライト.
    std::vector<EditImageBasedLight>    IblLights;          //!< IBLライト.
    std::vector<EditPin>                Pins;               //!< 汎用ピン
    std::vector<EditVolume>             Volumes;            //!< 汎用ボリューム.
};

///////////////////////////////////////////////////////////////////////////////
// LevelConverter
///////////////////////////////////////////////////////////////////////////////
class LevelConverter
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    ///////////////////////////////////////////////////////////////////////////
    // Desc structure
    ///////////////////////////////////////////////////////////////////////////
    struct Desc
    {
        std::string InputPath;  //!< 入力ファイルパス (*.json).
        std::string OutputPath; //!< 出力ファイルパス (*.lvb).
    };

    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      現在のバイナリバージョンを取得します.
    //! 
    //! @return     現在のバイナリバージョンを返却します.
    //-------------------------------------------------------------------------
    static uint32_t GetCurrentVersion();

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      desc        変換設定.
    //! @retval true    変換に成功.
    //! @retval false   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const Desc& desc);

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      path        入力ファイルパス.
    //! @param[out]     binary      出力バイナリ.
    //! @retval true    変換に成功.
    //! @retval false   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const std::string& path, std::vector<uint8_t>& binary);

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      level       編集可能レベルデータ.
    //! @param[out]     binary      出力バイナリ.
    //! @retval true    変換に成功.
    //! @retval fasle   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const EditLevel& level, std::vector<uint8_t>& binary);

    //-------------------------------------------------------------------------
    //! @brief      逆変換処理を行います.
    //! 
    //! @param[in]      binary      入力バイナリ.
    //! @param[out]     level       編集可能レベルデータ.
    //! @retval true    逆変換に成功.
    //! @retval false   逆変換に成功.
    //-------------------------------------------------------------------------
    static bool ReverseConvert(const std::vector<uint8_t>& binary, EditLevel& level);

    //-------------------------------------------------------------------------
    //! @brief      逆変換処理を行います.
    //! 
    //! @param[in]      binary      入力バイナリ.
    //! @param[in]      path        出力 json ファイルパス.
    //! @retval true    逆変換に成功.
    //! @retval false   逆変換に成功.
    //-------------------------------------------------------------------------
    static bool ReverseConvert(const std::vector<uint8_t>& binary, const std::string& path);

    //-------------------------------------------------------------------------
    //! @brief      編集可能レベルデータを保存します.
    //! 
    //! @param[in]      path        json ファイルパス.
    //! @param[in]      level       保存するレベルデータ.
    //! @retval true    保存に成功.
    //! @retval false   保存に失敗.
    //-------------------------------------------------------------------------
    static bool Save(const std::string& path, const EditLevel& level);

    //-------------------------------------------------------------------------
    //! @brief      編集可能レベルデータを読み込みます.
    //! 
    //! @param[in]      path        json ファイルパス.
    //! @param[out]     level       レベルデータの格納先.
    //! @retval true    保存に成功.
    //! @retval false   保存に失敗.
    //-------------------------------------------------------------------------
    static bool Load(const std::string& path, EditLevel& level);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};