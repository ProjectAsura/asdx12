//-----------------------------------------------------------------------------
// File : SampleUtil.h
// Desc : Sample Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxTextureManager.h>
#include <gfx/asdxFont.h>
#if ASDX_ENABLE_SOUND
#include <snd/asdxSoundEngine.h>
#endif//ASDX_ENABLE_SOUND


///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//! @brief      テクスチャをロードします.
//! 
//! @param[in]      path        ファイルパス.
//! @param[out]     holder      テクスチャの格納先.
//! @retval true    ロードに成功.
//! @retval false   ロードに失敗.
//-----------------------------------------------------------------------------
bool LoadTexture(const char* path, asdx::TextureHolder& holder);

//-----------------------------------------------------------------------------
//! @brief      フォントをロードします.
//! 
//! @param[in]      path        ファイルパス.
//! @param[out]     font        フォントの格納先.
//! @retval true    ロードに成功.
//! @retval false   ロードに失敗.
//-----------------------------------------------------------------------------
bool LoadFont(const char* path, asdx::Font& font);

#if ASDX_ENABLE_SOUND
//-----------------------------------------------------------------------------
//! @brief      WAVファイルをロードします.
//! 
//! @param[in]      path        ファイルパス.
//! @param[out]     resource    サウンドリソースの格納先.
//! @param[in]      loop        ループ再生する場合は true.
//! @retval true    ロードに成功.
//! @retval false   ロードに失敗.
//-----------------------------------------------------------------------------
bool LoadWav(const char* path, asdx::SoundResource& resource, bool loop = false);

//-----------------------------------------------------------------------------
//! @brief      Ogg Vorbisをロードします.
//! 
//! @param[in]      path        ファイルパス.
//! @param[out]     resource    サウンドリソースの格納先.
//! @retval true    ロードに成功.
//! @retval false   ロードに失敗.
//-----------------------------------------------------------------------------
bool LoadOgg(const char* path, asdx::SoundResource& resource, bool loop = false);

#endif // ASDX_ENABLE_SOUND
