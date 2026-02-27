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
// EditLightType enum
///////////////////////////////////////////////////////////////////////////////
enum EditLightType
{
    Directional,        //!< 指向性ライト.
    Point,              //!< 点光源.
    Spot,               //!< スポットライト.
    ImageBased,         //!< イメージベースドライト.
};

///////////////////////////////////////////////////////////////////////////////
// EditModelInstance structure
///////////////////////////////////////////////////////////////////////////////
struct EditModelInstance
{
    std::string         Path;       //!< mdb ファイルパス.
    asdx::Vector3       Position;   //!< 位置座標.
    asdx::Quaternion    Rotation;   //!< 回転量.
    asdx::Vector3       Scale;      //!< 拡大・縮小値.
};

///////////////////////////////////////////////////////////////////////////////
// EditPointLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditPointLight
{
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
    asdx::Vector3       Direction;  //!< 照射方向.
    asdx::Vector3       Color;      //!< 色.
    float               Intensity;  //!< 強度.
};

///////////////////////////////////////////////////////////////////////////////
// EditImageBasedLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditImageBasedLight
{
    std::string         Path;       //!< txb ファイルパス.
    float               Intensity;  //!< 強度.
};

///////////////////////////////////////////////////////////////////////////////
// EditLight structure
///////////////////////////////////////////////////////////////////////////////
struct EditLight
{
    EditLightType   Type;       //!< ライトタイプ.
    union
    {
        EditDirectionalLight    Directional;    //!< 指向性ライト.
        EditPointLight          Point;          //!< 点光源.
        EditSpotLight           Spot;           //!< スポットライト.
        EditImageBasedLight     ImageBased;     //!< イメージベースドライト.
    };

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    EditLight()
    : Type(EditLightType::Directional)
    {
        Directional.Direction = asdx::Vector3(0.0f, -1.0f, 0.0f);
        Directional.Color     = asdx::Vector3(1.0f, 1.0f, 1.0f);
        Directional.Intensity = 1.0f;
    }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~EditLight()
    {
        if (Type == EditLightType::ImageBased)
        { ImageBased.Path.~basic_string(); }
    }

    //-------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //-------------------------------------------------------------------------
    EditLight(const EditLight& other)
    {
        Type = other.Type;
        switch(Type)
        {
            case EditLightType::Directional: { Directional = other.Directional; } break;
            case EditLightType::Point:       { Point       = other.Point;       } break;
            case EditLightType::Spot:        { Spot        = other.Spot;        } break;
            case EditLightType::ImageBased:  { ImageBased  = other.ImageBased;  } break;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      ムーブコンストラクタです.
    //-------------------------------------------------------------------------
    EditLight(EditLight&& other) noexcept
    {
        Type = other.Type;
        switch(Type)
        {
            case EditLightType::Directional: { Directional  = std::move(other.Directional); } break;
            case EditLightType::Point:       { Point        = std::move(other.Point);       } break;
            case EditLightType::Spot:        { Spot         = std::move(other.Spot);        } break;
            case EditLightType::ImageBased:  { ImageBased   = std::move(other.ImageBased);  } break;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
// EditPin structure
///////////////////////////////////////////////////////////////////////////////
struct EditPin
{
    std::string     Tag;        //!< タグ.
    asdx::Vector3   Position;   //!< 位置座標.
};

///////////////////////////////////////////////////////////////////////////////
// EditLevel structure
///////////////////////////////////////////////////////////////////////////////
struct EditLevel
{
    std::vector<EditModelInstance>  Models;     //!< モデルインスタンス.
    std::vector<EditLight>          Lights;     //!< ライト.
    std::vector<EditPin>            Pins;       //!< ピン
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