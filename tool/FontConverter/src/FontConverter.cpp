//-----------------------------------------------------------------------------
// File : FontConverter.cpp
// Desc : Font Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <Windows.h>
#include <FontConverter.h>
#include <simdjson.h>
#include <DirectXTex.h>
#include <FontBinary_generated.h>


namespace {

static constexpr uint32_t kCurVersion = 1u;

} // namespace


///////////////////////////////////////////////////////////////////////////////
// FontConvert class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool FontConverter::Convert(const Desc& desc)
{
    {
        simdjson::ondemand::parser parser;
        auto json = simdjson::padded_string::load(desc.JsonPath.c_str());
        auto doc  = parser.iterate(json);

        auto atlas   = doc["atlas"];
        auto metrics = doc["metric"];
        auto glyphs  = doc["glyphs"].get_array();

        for(auto g : glyphs)
        {
            auto unicode = uint32_t(g["unicode"].get_uint64().value());
            auto advance = float(g["advance"].get_double().value());
            auto planeBounds = g["plane_bounds"];
            {
                auto left   = float(planeBounds["left"]  .get_double().value());
                auto right  = float(planeBounds["right"] .get_double().value());
                auto top    = float(planeBounds["top"]   .get_double().value());
                auto bottom = float(planeBounds["bottom"].get_double().value());
            }

            auto atlasBounds = g["atlas_bounds"];
            {
                auto left   = float(atlasBounds["left"]  .get_double().value());
                auto right  = float(atlasBounds["right"] .get_double().value());
                auto top    = float(atlasBounds["top"]   .get_double().value());
                auto bottom = float(atlasBounds["bottom"].get_double().value());

            }

            printf_s("%f\n", advance);
        }
    }

    return true;
}