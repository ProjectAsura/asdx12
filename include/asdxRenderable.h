﻿//-----------------------------------------------------------------------------
// File : asdxRenderable.h
// Desc : Renderable Object.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMath.h>


#ifndef ASDX_UNUSED
#define ASDX_UNUSED(x)  (void)x
#endif//ASDX_UNUSED

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IRenderable structure
///////////////////////////////////////////////////////////////////////////////
struct IRenderable
{
    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    virtual ~IRenderable()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      カリングを行います.
    //!
    //! @param[in]      planes      視錐台を構成する6平面.
    //! @return     カリングする場合は true を返却します.
    //-------------------------------------------------------------------------
    virtual bool OnCulling(const asdx::Vector4* planes)
    {
        ASDX_UNUSED(planes);
        return false;
    }
};

} // namespace asdx
