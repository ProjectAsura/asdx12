//------------------------------------------------------------q----------------
// File : LevelConverter.cpp
// Desc : Level Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <LevelConverter.h>
#include <simdjson.h>
#include <LevelBinary_generated.h>
#include <res/asdxResHelper.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
// Constant values.
//-----------------------------------------------------------------------------
constexpr uint32_t CURRENT_VERSION = 1u;    //!< 現在サポートされているバージョン.

//-----------------------------------------------------------------------------
//      Vector3型に変換します.
//-----------------------------------------------------------------------------
static asdx::Vector3 ReadVector3(simdjson::ondemand::object obj)
{
    asdx::Vector3 v;
    v.x = float(double(obj["X"]));
    v.y = float(double(obj["Y"]));
    v.z = float(double(obj["Z"]));
    return v;
}

//-----------------------------------------------------------------------------
//      Quatenrion型に変換します.
//-----------------------------------------------------------------------------
static asdx::Quaternion ReadQuaternion(simdjson::ondemand::object obj)
{
    asdx::Quaternion q;
    q.x = float(double(obj["X"]));
    q.y = float(double(obj["Y"]));
    q.z = float(double(obj["Z"]));
    q.w = float(double(obj["W"]));
    return q;
}

//-----------------------------------------------------------------------------
//      Vector3型を json フォーマットで書き込みます.
//-----------------------------------------------------------------------------
static void WriteVector3(FILE* fp, const asdx::Vector3& v)
{
    fprintf_s(fp, "{ \"X\": %.6f, \"Y\": %.6f, \"Z\": %.6f }",
        v.x, v.y, v.z);
}

//-----------------------------------------------------------------------------
//      Quaternion型を json フォーマットで書き込みます.
//-----------------------------------------------------------------------------
static void WriteQuaternion(FILE* fp, const asdx::Quaternion& q)
{
    fprintf_s(fp, "{ \"X\": %.6f, \"Y\": %.6f, \"Z\": %.6f, \"W\": %.6f }",
        q.x, q.y, q.z, q.w);
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// LevelConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      現在のバイナリバージョンを取得します.
//-----------------------------------------------------------------------------
uint32_t LevelConverter::GetCurrentVersion()
{ return CURRENT_VERSION; }

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath, binary))
    {
        ELOG("Error : Convert Failed.");
        return false;
    }

    // バイナリファイルに出力.
    {
        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : Output File Open Failed. path = %s", desc.OutputPath.c_str());
            return false;
        }

        fwrite(binary.data(), binary.size(), 1, fp);
        fclose(fp);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::Convert(const std::string& path, std::vector<uint8_t>& binary)
{
    EditLevel level;
    if (!Load(path, level))
    {
        ELOG("Error : Load() Failed. path = %s", path.c_str());
        return false;
    }

    if (!Convert(level, binary))
    {
        ELOG("Error : Binary Convert Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::Convert(const EditLevel& level, std::vector<uint8_t>& output)
{
    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<asdx::res::ModelInstance>> dstModels;
    std::vector<flatbuffers::Offset<asdx::res::Light>> dstLights;
    std::vector<flatbuffers::Offset<asdx::res::Pin>> dstPins;

    // モデルデータ変換.
    for(auto& srcModel : level.Models)
    {
        auto pos      = asdx::ToFloat3(srcModel.Position);
        auto scale    = asdx::ToFloat3(srcModel.Scale);
        auto rotation = asdx::ToQuaternion(srcModel.Rotation);

        auto instance = asdx::res::CreateModelInstanceDirect(
            builder,
            srcModel.Tag.c_str(),
            srcModel.Path.c_str(),
            &pos,
            &scale,
            &rotation);

        dstModels.emplace_back(instance);
    }

    // ライトデータ変換.
    for(auto& srcLight : level.Lights)
    {
        switch(srcLight.Type)
        {
        case EditLightType::Point:
            {
                auto pos = asdx::ToFloat3(srcLight.Point.Position);
                auto col = asdx::ToFloat3(srcLight.Point.Color);

                auto instance = asdx::res::CreatePointLight(
                    builder, 
                    &pos,
                    &col,
                    srcLight.Point.Intensity,
                    srcLight.Point.Radius);

                auto item = asdx::res::CreateLightDirect(
                    builder,
                    srcLight.Tag.c_str(),
                    asdx::res::LightUnion_PointLight,
                    instance.Union());

                dstLights.emplace_back(item);
            }
            break;

        case EditLightType::Spot:
            {
                auto pos = asdx::ToFloat3(srcLight.Spot.Position);
                auto dir = asdx::ToFloat3(srcLight.Spot.Direction);
                auto col = asdx::ToFloat3(srcLight.Spot.Color);

                auto instance = asdx::res::CreateSpotLight(
                    builder,
                    &pos,
                    &dir,
                    &col,
                    srcLight.Spot.Intensity,
                    srcLight.Spot.Radius,
                    srcLight.Spot.InnerAngle,
                    srcLight.Spot.OuterAngle);

                auto item = asdx::res::CreateLightDirect(
                    builder,
                    srcLight.Tag.c_str(),
                    asdx::res::LightUnion_SpotLight,
                    instance.Union());

                dstLights.emplace_back(item);
            }
            break;

        case EditLightType::Directional:
            {
                auto dir = asdx::ToFloat3(srcLight.Directional.Direction);
                auto col = asdx::ToFloat3(srcLight.Directional.Color);

                auto instance = asdx::res::CreateDirectionalLight(
                    builder,
                    &dir,
                    &col,
                    srcLight.Directional.Intensity);

                auto item = asdx::res::CreateLightDirect(
                    builder,
                    srcLight.Tag.c_str(),
                    asdx::res::LightUnion_DirectionalLight,
                    instance.Union());

                dstLights.emplace_back(item);
            }
            break;

        case EditLightType::ImageBased:
            {
                auto instance = asdx::res::CreateImageBasedLightDirect(
                    builder,
                    srcLight.ImageBased.Path.c_str(),
                    srcLight.ImageBased.Intensity);

                auto item = asdx::res::CreateLightDirect(
                    builder,
                    srcLight.Tag.c_str(),
                    asdx::res::LightUnion_ImageBasedLight,
                    instance.Union());

                dstLights.emplace_back(item);
            }
            break;
        }
    }

    // ピンデータ変換.
    for(auto& srcPin : level.Pins)
    {
        auto pos = asdx::ToFloat3(srcPin.Position);

        auto instance = asdx::res::CreatePinDirect(
            builder,
            srcPin.Tag.c_str(),
            &pos);

        dstPins.emplace_back(instance);
    }

    // バイナリ作成.
    auto resource = asdx::res::CreateLevelBinaryDirect(
        builder,
        &dstModels,
        &dstLights,
        &dstPins);
    builder.Finish(resource);

    // バイナリを出力.
    output.resize(builder.GetSize());
    memcpy(output.data(), builder.GetBufferPointer(), builder.GetSize());

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::ReverseConvert(const std::vector<uint8_t>& input, EditLevel& output)
{
    if (input.empty())
    {
        ELOG("Error : Invalid Argument");
        return false;
    }

    // バイナリデータに変換.
    auto bin = asdx::res::GetLevelBinary(input.data());

    // モデルデータ逆変換.
    auto models = bin->Models();
    if (models != nullptr)
    {
        for(auto i=0u; i<models->size(); ++i)
        {
            auto srcModel = models->Get(i);

            EditModelInstance instance = {};
            instance.Tag        = srcModel->Tag()->c_str();
            instance.Path       = srcModel->Path()->c_str();
            instance.Position   = asdx::FromFloat3(*srcModel->Position());
            instance.Scale      = asdx::FromFloat3(*srcModel->Scale());
            instance.Rotation   = asdx::FromQuaternion(*srcModel->Rotation());

            output.Models.emplace_back(instance);
        }
    }

    // ライトデータ逆変換.
    auto lights = bin->Lights();
    if (lights != nullptr)
    {
        for(auto i=0u; i<lights->size(); ++i)
        {
            auto srcLight = lights->Get(i);
            EditLight instance = {};

            switch(srcLight->data_type())
            {
            case asdx::res::LightUnion_PointLight:
                {
                    auto lightData = srcLight->data_as_PointLight();

                    instance.Tag                = srcLight->Tag()->c_str();
                    instance.Type               = EditLightType::Point;
                    instance.Point.Position     = asdx::FromFloat3(*lightData->Position());
                    instance.Point.Color        = asdx::FromFloat3(*lightData->Color());
                    instance.Point.Intensity    = lightData->Intensity();
                    instance.Point.Radius       = lightData->Radius();

                    output.Lights.emplace_back(instance);
                }
                break;

            case asdx::res::LightUnion_SpotLight:
                {
                    auto lightData = srcLight->data_as_SpotLight();

                    instance.Tag                = srcLight->Tag()->c_str();
                    instance.Type               = EditLightType::Spot;
                    instance.Spot.Position      = asdx::FromFloat3(*lightData->Position());
                    instance.Spot.Direction     = asdx::FromFloat3(*lightData->Direction());
                    instance.Spot.Color         = asdx::FromFloat3(*lightData->Color());
                    instance.Spot.Intensity     = lightData->Intensity();
                    instance.Spot.Radius        = lightData->Radius();
                    instance.Spot.InnerAngle    = lightData->InnerAngle();
                    instance.Spot.OuterAngle    = lightData->OuterAngle();

                    output.Lights.emplace_back(instance);
                }
                break;

            case asdx::res::LightUnion_DirectionalLight:
                {
                    auto lightData = srcLight->data_as_DirectionalLight();

                    instance.Tag                    = srcLight->Tag()->c_str();
                    instance.Type                   = EditLightType::Directional;
                    instance.Directional.Direction  = asdx::FromFloat3(*lightData->Direction());
                    instance.Directional.Color      = asdx::FromFloat3(*lightData->Color());
                    instance.Directional.Intensity  = lightData->Intensity();

                    output.Lights.emplace_back(instance);
                }
                break;

            case asdx::res::LightUnion_ImageBasedLight:
                {
                    auto lightData = srcLight->data_as_ImageBasedLight();

                    instance.Tag                    = srcLight->Tag()->c_str();
                    instance.Type                   = EditLightType::ImageBased;
                    instance.ImageBased.Path        = lightData->Path()->c_str();
                    instance.ImageBased.Intensity   = lightData->Intensity();

                    output.Lights.emplace_back(instance);
                }
                break;
            }
        }
    }

    // ピンデータ逆変換.
    auto pins = bin->Pins();
    if (pins != nullptr)
    {
        for(auto i=0u; i<pins->size(); ++i)
        {
            auto srcPin = pins->Get(i);

            EditPin instance = {};
            instance.Tag        = srcPin->Tag()->c_str();
            instance.Position   = asdx::FromFloat3(*srcPin->Position());

            output.Pins.emplace_back(instance);
        }
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::ReverseConvert(const std::vector<uint8_t>& binary, const std::string& path)
{
    // 逆変換実行.
    EditLevel level;
    if (!ReverseConvert(binary, level))
    {
        ELOG("Error : Reverse Convert Failed.");
        return false;
    }

    // json ファイルに保存.
    return Save(path, level);
}

//-----------------------------------------------------------------------------
//      jsonファイルに保存します.
//-----------------------------------------------------------------------------
bool LevelConverter::Save(const std::string& path, const EditLevel& level)
{
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path.c_str(), "w");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path.c_str());
        return false;
    }

    fprintf_s(fp, "{\n");

    bool hasPrev = false;

    // モデルデータ書き込み.
    if (!level.Models.empty())
    {
        fprintf_s(fp, "  \"Models\": [\n");
        for (size_t i = 0; i < level.Models.size(); ++i)
        {
            const auto& m = level.Models[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", m.Tag.c_str());
            fprintf_s(fp, "      \"Path\": \"%s\",\n", m.Path.c_str());

            fprintf_s(fp, "      \"Position\": ");
            WriteVector3(fp, m.Position);
            fprintf_s(fp, ",\n");

            fprintf_s(fp, "      \"Rotation\": ");
            WriteQuaternion(fp, m.Rotation);
            fprintf_s(fp, ",\n");

            fprintf_s(fp, "      \"Scale\": ");
            WriteVector3(fp, m.Scale);
            fprintf_s(fp, "\n");

            fprintf_s(fp, "    }%s\n", (i + 1 < level.Models.size()) ? "," : "");
        }
        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    // ライトデータ書き込み.
    if (!level.Lights.empty())
    {
        if (hasPrev)
        { fprintf_s(fp, ",\n"); }

        fprintf_s(fp, "  \"Lights\": [\n");
        for (size_t i = 0; i < level.Lights.size(); ++i)
        {
            const auto& l = level.Lights[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", l.Tag.c_str());

            if (l.Type == Point)
            {
                fprintf_s(fp, "      \"Type\": \"Point\",\n");

                fprintf_s(fp, "      \"Position\": ");
                WriteVector3(fp, l.Point.Position);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Color\": ");
                WriteVector3(fp, l.Point.Color);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Intensity\": %.6f,\n", l.Point.Intensity);
                fprintf_s(fp, "      \"Radius\": %.6f\n", l.Point.Radius);
            }
            else if (l.Type == Spot)
            {
                fprintf_s(fp, "      \"Type\": \"Spot\",\n");

                fprintf_s(fp, "      \"Position\": ");
                WriteVector3(fp, l.Spot.Position);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Direction\": ");
                WriteVector3(fp, l.Spot.Direction);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Color\": ");
                WriteVector3(fp, l.Spot.Color);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Intensity\": %.6f,\n", l.Spot.Intensity);
                fprintf_s(fp, "      \"Radius\": %.6f,\n", l.Spot.Radius);
                fprintf_s(fp, "      \"InnerAngle\": %.6f,\n", l.Spot.InnerAngle);
                fprintf_s(fp, "      \"OuterAngle\": %.6f\n", l.Spot.OuterAngle);
            }
            else if (l.Type == Directional)
            {
                fprintf_s(fp, "      \"Type\": \"Directional\",\n");

                fprintf_s(fp, "      \"Direction\": ");
                WriteVector3(fp, l.Directional.Direction);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Color\": ");
                WriteVector3(fp, l.Directional.Color);
                fprintf_s(fp, ",\n");

                fprintf_s(fp, "      \"Intensity\": %.6f\n", l.Directional.Intensity);
            }
            else if (l.Type == ImageBased)
            {
                fprintf_s(fp, "      \"Type\": \"ImageBased\",\n");
                fprintf_s(fp, "      \"Path\": \"%s\",\n", l.ImageBased.Path.c_str());
                fprintf_s(fp, "      \"Intensity\": %.6f\n", l.ImageBased.Intensity);
            }

            fprintf_s(fp, "    }%s\n", (i + 1 < level.Lights.size()) ? "," : "");
        }

        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    // ピンデータ書き込み.
    if (!level.Pins.empty())
    {
        if (hasPrev)
        { fprintf_s(fp, ",\n"); }

        fprintf_s(fp, "  \"Pins\": [\n");
        for(size_t i=0; i<level.Pins.size(); ++i)
        {
            const auto& p = level.Pins[i];
            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", p.Tag.c_str());
            fprintf_s(fp, "      \"Position\": ");
            WriteVector3(fp, p.Position);
            fprintf_s(fp, "    }%s\n", (i + 1 < level.Pins.size()) ? "," : "");
        }

        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    if (hasPrev)
    { fprintf_s(fp, "\n"); }

    fprintf_s(fp, "}\n");

    fclose(fp);
    return true;
}

//-----------------------------------------------------------------------------
//      jsonファイルから読み込みします.
//-----------------------------------------------------------------------------
bool LevelConverter::Load(const std::string& path, EditLevel& level)
{
    simdjson::ondemand::parser parser;

    simdjson::padded_string json;
    if (simdjson::padded_string::load(path).get(json))
    {
        ELOG("Error : Json Load Failed. path = %s", path.c_str());
        return false;
    }

    auto doc = parser.iterate(json);

    // モデルデータ読み込み.
    auto models = doc["Models"];
    if (models.error() == simdjson::SUCCESS)
    {
        for (auto model : models.get_array())
        {
            auto obj = model.get_object();

            auto tag  = std::string(std::string_view(obj["Tag"]));
            auto path = std::string(std::string_view(obj["Path"]));

            auto s = ReadVector3   (obj["Scale"]   .get_object());
            auto r = ReadQuaternion(obj["Rotation"].get_object());
            auto t = ReadVector3   (obj["Position"].get_object());

            EditModelInstance instance = {};
            instance.Tag        = tag;
            instance.Path       = path;
            instance.Scale      = s;
            instance.Rotation   = r;
            instance.Position   = t;
            level.Models.emplace_back(instance);
        }
    }

    // ライトデータ読み込み.
    auto lights = doc["Lights"];
    if (lights.error() == simdjson::SUCCESS)
    {
        for (auto light : lights.get_array())
        {
            auto obj = light.get_object();

            EditLight instance = {};
            auto tag  = std::string(std::string_view(obj["Tag"]));
            auto type = std::string(std::string_view(obj["Type"]));
            instance.Tag = tag;

            if (type == "Point")
            {
                auto pos        = ReadVector3(obj["Position"].get_object());
                auto color      = ReadVector3(obj["Color"].get_object());
                auto intensity  = float(double(obj["Intensity"]));
                auto radius     = float(double(obj["Radius"]));

                instance.Type            = EditLightType::Point;
                instance.Point.Position  = pos;
                instance.Point.Color     = color;
                instance.Point.Intensity = intensity;
                instance.Point.Radius    = radius;

                level.Lights.emplace_back(instance);
            }
            else if (type == "Spot")
            {
                auto pos        = ReadVector3(obj["Position"].get_object());
                auto dir        = ReadVector3(obj["Direction"].get_object());
                auto color      = ReadVector3(obj["Color"].get_object());
                auto intensity  = float(double(obj["Intensity"]));
                auto radius     = float(double(obj["Radius"]));
                auto inner      = float(double(obj["InnerAngle"]));
                auto outer      = float(double(obj["OuterAngle"]));

                instance.Type               = EditLightType::Spot;
                instance.Spot.Position      = pos;
                instance.Spot.Direction     = dir;
                instance.Spot.Color         = color;
                instance.Spot.Intensity     = intensity;
                instance.Spot.Radius        = radius;
                instance.Spot.InnerAngle    = inner;
                instance.Spot.OuterAngle    = outer;

                level.Lights.emplace_back(instance);
            }
            else if (type == "Directional")
            {
                auto dir        = ReadVector3(obj["Direction"].get_object());
                auto color      = ReadVector3(obj["Color"].get_object());
                auto intensity  = float(double(obj["Intensity"]));

                instance.Type                   = EditLightType::Directional;
                instance.Directional.Direction  = dir;
                instance.Directional.Color      = color;
                instance.Directional.Intensity  = intensity;

                level.Lights.emplace_back(instance);
            }
            else if (type == "ImageBased")
            {
                auto path       = std::string(std::string_view(obj["Path"]));
                auto intensity  = float(double(obj["Intensity"]));

                instance.Type                   = EditLightType::ImageBased;
                instance.ImageBased.Path        = path;
                instance.ImageBased.Intensity   = intensity;

                level.Lights.emplace_back(instance);
            }
        }
    }

    // ピンデータ読み込み.
    auto pins = doc["Pins"];
    if (pins.error() == simdjson::SUCCESS)
    {
        for(auto pin : pins.get_array())
        {
            auto obj = pin.get_object();
            auto tag = std::string(std::string_view(obj["Tag"]));
            auto pos = ReadVector3(obj["Position"]);

            EditPin instance = {};
            instance.Tag        = tag;
            instance.Position   = pos;

            level.Pins.emplace_back(instance);
        }
    }

    // 正常終了.
    return true;
}