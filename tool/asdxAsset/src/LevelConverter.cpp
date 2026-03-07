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

    std::vector<flatbuffers::Offset<asdx::res::ModelInstance>>      dstModels;
    std::vector<flatbuffers::Offset<asdx::res::PointLight>>         dstPointLights;
    std::vector<flatbuffers::Offset<asdx::res::SpotLight>>          dstSpotLights;
    std::vector<flatbuffers::Offset<asdx::res::DirectionalLight>>   dstDirLights;
    std::vector<flatbuffers::Offset<asdx::res::ImageBasedLight>>    dstIblLights;
    std::vector<flatbuffers::Offset<asdx::res::Pin>>                dstPins;
    std::vector<flatbuffers::Offset<asdx::res::Volume>>             dstVolumes;

    // モデルデータ変換.
    for(auto& srcModel : level.ModelInstances)
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
            &rotation,
            srcModel.UserId);

        dstModels.emplace_back(instance);
    }

    // ポイントライトデータ変換.
    for(auto& srcLight : level.PointLights)
    {
        auto pos = asdx::ToFloat3(srcLight.Position);
        auto col = asdx::ToFloat3(srcLight.Color);

        auto item = asdx::res::CreatePointLightDirect(
            builder,
            srcLight.Tag.c_str(),
            &pos,
            &col,
            srcLight.Intensity,
            srcLight.Radius);

        dstPointLights.emplace_back(item);
    }

    // スポットライトデータ変換.
    for(auto& srcLight : level.SpotLights)
    {
        auto pos = asdx::ToFloat3(srcLight.Position);
        auto dir = asdx::ToFloat3(srcLight.Direction);
        auto col = asdx::ToFloat3(srcLight.Color);

        auto item = asdx::res::CreateSpotLightDirect(
            builder,
            srcLight.Tag.c_str(),
            &pos,
            &dir,
            &col,
            srcLight.Intensity,
            srcLight.Radius,
            srcLight.InnerAngle,
            srcLight.OuterAngle);

        dstSpotLights.emplace_back(item);
    }

    // ディレクショナルライトデータ変換.
    for(auto& srcLight : level.DirLights)
    {
        auto dir = asdx::ToFloat3(srcLight.Direction);
        auto col = asdx::ToFloat3(srcLight.Color);

        auto item = asdx::res::CreateDirectionalLightDirect(
            builder,
            srcLight.Tag.c_str(),
            &dir,
            &col,
            srcLight.Intensity);

        dstDirLights.emplace_back(item);
    }

    // IBLライトデータ変換.
    for(auto& srcLight : level.IblLights)
    {
        auto item = asdx::res::CreateImageBasedLightDirect(
            builder,
            srcLight.Tag.c_str(),
            srcLight.Path.c_str(),
            srcLight.Intensity);

        dstIblLights.emplace_back(item);
    }

    // ピンデータ変換.
    for(auto& srcPin : level.Pins)
    {
        auto pos = asdx::ToFloat3(srcPin.Position);

        auto item = asdx::res::CreatePinDirect(
            builder,
            srcPin.Tag.c_str(),
            &pos,
            srcPin.UserId);

        dstPins.emplace_back(item);
    }

    // ボリュームデータ変換.
    for(auto& srcVolume : level.Volumes)
    {
        auto pos      = asdx::ToFloat3(srcVolume.Position);
        auto scale    = asdx::ToFloat3(srcVolume.Scale);
        auto rotation = asdx::ToQuaternion(srcVolume.Rotation);

        auto item = asdx::res::CreateVolumeDirect(
            builder,
            srcVolume.Tag.c_str(),
            &pos,
            &scale,
            &rotation,
            srcVolume.UserId);

        dstVolumes.emplace_back(item);
    }

    // バイナリ作成.
    auto resource = asdx::res::CreateLevelBinaryDirect(
        builder,
        &dstModels,
        &dstPointLights,
        &dstSpotLights,
        &dstDirLights,
        &dstIblLights,
        &dstPins,
        &dstVolumes);
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
    auto models = bin->ModelInstances();
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

            output.ModelInstances.emplace_back(instance);
        }
    }

    auto pointLights = bin->PointLights();
    if (pointLights != nullptr)
    {
        for(auto i=0u; i<pointLights->size(); ++i)
        {
            auto srcLight = pointLights->Get(i);

            EditPointLight instance = {};
            instance.Tag        = srcLight->Tag()->c_str();
            instance.Position   = asdx::FromFloat3(*srcLight->Position());
            instance.Color      = asdx::FromFloat3(*srcLight->Color());
            instance.Intensity  = srcLight->Intensity();
            instance.Radius     = srcLight->Radius();

            output.PointLights.emplace_back(instance);
        }
    }

    auto spotLights = bin->SpotLights();
    if (spotLights != nullptr)
    {
        for(auto i=0u; i<spotLights->size(); ++i)
        {
            auto srcLight = spotLights->Get(i);

            EditSpotLight instance = {};
            instance.Tag        = srcLight->Tag()->c_str();
            instance.Position   = asdx::FromFloat3(*srcLight->Position());
            instance.Direction  = asdx::FromFloat3(*srcLight->Direction());
            instance.Color      = asdx::FromFloat3(*srcLight->Color());
            instance.Intensity  = srcLight->Intensity();
            instance.InnerAngle = srcLight->InnerAngle();
            instance.OuterAngle = srcLight->OuterAngle();
            instance.Radius     = srcLight->Radius();

            output.SpotLights.emplace_back(instance);
        }
    }

    auto dirLights = bin->DirLights();
    if (dirLights != nullptr)
    {
        for(auto i=0u; i<dirLights->size(); ++i)
        {
            auto srcLight = dirLights->Get(i);

            EditDirectionalLight instance = {};
            instance.Tag        = srcLight->Tag()->c_str();
            instance.Direction  = asdx::FromFloat3(*srcLight->Direction());
            instance.Color      = asdx::FromFloat3(*srcLight->Color());
            instance.Intensity  = srcLight->Intensity();

            output.DirLights.emplace_back(instance);
        }
    }

    auto iblLights = bin->IblLights();
    if (iblLights != nullptr)
    {
        for(auto i=0u; i<iblLights->size(); ++i)
        {
            auto srcLight = iblLights->Get(i);

            EditImageBasedLight instance = {};
            instance.Tag        = srcLight->Tag()->c_str();
            instance.Path       = srcLight->Path()->c_str();
            instance.Intensity  = srcLight->Intensity();

            output.IblLights.emplace_back(instance);
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

    // ボリュームデータ逆変換.
    auto volumes = bin->Volumes();
    if (volumes != nullptr)
    {
        for(auto i=0u; i<volumes->size(); ++i)
        {
            auto srcVolume = volumes->Get(i);

            EditVolume instance = {};
            instance.Tag        = srcVolume->Tab()->c_str();
            instance.Position   = asdx::FromFloat3(*srcVolume->Position());
            instance.Scale      = asdx::FromFloat3(*srcVolume->Scale());
            instance.Rotation   = asdx::FromQuaternion(*srcVolume->Rotation());
            instance.UserId     = srcVolume->UserId();

            output.Volumes.emplace_back(instance);
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
    if (!level.ModelInstances.empty())
    {
        fprintf_s(fp, "  \"Models\": [\n");
        for (size_t i = 0; i < level.ModelInstances.size(); ++i)
        {
            const auto& m = level.ModelInstances[i];

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
            fprintf_s(fp, "      \"UserId\": %llu\n", m.UserId);

            fprintf_s(fp, "    }%s\n", (i + 1 < level.ModelInstances.size()) ? "," : "");
        }
        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    if (!level.PointLights.empty())
    {
        if (hasPrev)
        { fprintf_s(fp, ",\n"); }

        fprintf_s(fp, "  \"PointLights\": [\n");
        for (size_t i = 0; i < level.PointLights.size(); ++i)
        {
            const auto& l = level.PointLights[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", l.Tag.c_str());
            fprintf_s(fp, "      \"Position\": ");
            WriteVector3(fp, l.Position);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Color\": ");
            WriteVector3(fp, l.Color);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Intensity\": %.6f,\n", l.Intensity);
            fprintf_s(fp, "      \"Radius\": %.6f\n", l.Radius);
            fprintf_s(fp, "    }%s\n", (i + 1 < level.PointLights.size()) ? "," : "");
        }

        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    if (!level.SpotLights.empty())
    {
        if (hasPrev)
        { fprintf_s(fp, ",\n"); }

        fprintf_s(fp, "  \"SpotLights\": [\n");
        for (size_t i = 0; i < level.SpotLights.size(); ++i)
        {
            const auto& l = level.SpotLights[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", l.Tag.c_str());
            fprintf_s(fp, "      \"Position\": ");
            WriteVector3(fp, l.Position);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Direction\": ");
            WriteVector3(fp, l.Direction);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Color\": ");
            WriteVector3(fp, l.Color);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Intensity\": %.6f,\n", l.Intensity);
            fprintf_s(fp, "      \"Radius\": %.6f,\n", l.Radius);
            fprintf_s(fp, "      \"InnerAngle\": %.6f,\n", l.InnerAngle);
            fprintf_s(fp, "      \"OuterAngle\": %.6f\n", l.OuterAngle);
            fprintf_s(fp, "    }%s\n", (i + 1 < level.SpotLights.size()) ? "," : "");
        }

        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    if (!level.DirLights.empty())
    {
        if (hasPrev)
        { fprintf_s(fp, ",\n"); }

        fprintf_s(fp, "  \"DirLights\": [\n");
        for (size_t i = 0; i < level.DirLights.size(); ++i)
        {
            const auto& l = level.DirLights[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", l.Tag.c_str());
            WriteVector3(fp, l.Direction);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Color\": ");
            WriteVector3(fp, l.Color);
            fprintf_s(fp, ",\n");
            fprintf_s(fp, "      \"Intensity\": %.6f\n", l.Intensity);
            fprintf_s(fp, "    }%s\n", (i + 1 < level.DirLights.size()) ? "," : "");
        }
        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    if (!level.IblLights.empty())
    {
        if (hasPrev)
        { fprintf_s(fp, ",\n"); }

        fprintf_s(fp, "  \"IblLights\": [\n");
        for (size_t i = 0; i < level.DirLights.size(); ++i)
        {
            const auto& l = level.IblLights[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", l.Tag.c_str());
            fprintf_s(fp, "      \"Path\": \"%s\",\n", l.Path.c_str());
            fprintf_s(fp, "      \"Intensity\": %.6f\n", l.Intensity);
            fprintf_s(fp, "    }%s\n", (i + 1 < level.IblLights.size()) ? "," : "");
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
            fprintf_s(fp, "      \"UserId\": %llu\n", p.UserId);
            fprintf_s(fp, "    }%s\n", (i + 1 < level.Pins.size()) ? "," : "");
        }

        fprintf_s(fp, "  ]");
        hasPrev = true;
    }

    if (!level.Volumes.empty())
    {
        fprintf_s(fp, "  \"Volumes\": [\n");
        for (size_t i = 0; i < level.Volumes.size(); ++i)
        {
            const auto& v = level.Volumes[i];

            fprintf_s(fp, "    {\n");
            fprintf_s(fp, "      \"Tag\": \"%s\",\n", v.Tag.c_str());

            fprintf_s(fp, "      \"Position\": ");
            WriteVector3(fp, v.Position);
            fprintf_s(fp, ",\n");

            fprintf_s(fp, "      \"Rotation\": ");
            WriteQuaternion(fp, v.Rotation);
            fprintf_s(fp, ",\n");

            fprintf_s(fp, "      \"Scale\": ");
            WriteVector3(fp, v.Scale);
            fprintf_s(fp, "\n");
            fprintf_s(fp, "      \"UserId\": %llu\n", v.UserId);

            fprintf_s(fp, "    }%s\n", (i + 1 < level.ModelInstances.size()) ? "," : "");
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

            auto userId = obj["UserId"].get_uint64().value();

            EditModelInstance instance = {};
            instance.Tag        = tag;
            instance.Path       = path;
            instance.Scale      = s;
            instance.Rotation   = r;
            instance.Position   = t;
            instance.UserId     = userId;
            level.ModelInstances.emplace_back(instance);
        }
    }

    // ライトデータ読み込み.
    auto pointLights = doc["PointLights"];
    if (pointLights.error() == simdjson::SUCCESS)
    {
        for (auto light : pointLights.get_array())
        {
            auto obj = light.get_object();

            EditPointLight instance = {};
            auto tag        = std::string(std::string_view(obj["Tag"]));
            auto pos        = ReadVector3(obj["Position"].get_object());
            auto color      = ReadVector3(obj["Color"].get_object());
            auto intensity  = float(double(obj["Intensity"]));
            auto radius     = float(double(obj["Radius"]));

            instance.Tag       = tag;
            instance.Position  = pos;
            instance.Color     = color;
            instance.Intensity = intensity;
            instance.Radius    = radius;

            level.PointLights.emplace_back(instance);
        }
    }

    auto spotLights = doc["SpotLights"];
    if (spotLights.error() == simdjson::SUCCESS)
    {
        for(auto light : spotLights.get_array())
        {
            auto obj = light.get_object();

            EditSpotLight instance = {};
            auto tag        = std::string(std::string_view(obj["Tag"]));
            auto pos        = ReadVector3(obj["Position"].get_object());
            auto dir        = ReadVector3(obj["Direction"].get_object());
            auto color      = ReadVector3(obj["Color"].get_object());
            auto intensity  = float(double(obj["Intensity"]));
            auto radius     = float(double(obj["Radius"]));
            auto inner      = float(double(obj["InnerAngle"]));
            auto outer      = float(double(obj["OuterAngle"]));

            instance.Tag           = tag;
            instance.Position      = pos;
            instance.Direction     = dir;
            instance.Color         = color;
            instance.Intensity     = intensity;
            instance.Radius        = radius;
            instance.InnerAngle    = inner;
            instance.OuterAngle    = outer;

            level.SpotLights.emplace_back(instance);
        }
    }
            
    auto dirLights = doc["DirLights"];
    if (dirLights.error() == simdjson::SUCCESS)
    {
        for(auto light : dirLights.get_array())
        {
            auto obj = light.get_object();

            EditDirectionalLight instance = {};
            auto tag        = std::string(std::string_view(obj["Tag"]));
            auto dir        = ReadVector3(obj["Direction"].get_object());
            auto color      = ReadVector3(obj["Color"].get_object());
            auto intensity  = float(double(obj["Intensity"]));

            instance.Tag        = tag;
            instance.Direction  = dir;
            instance.Color      = color;
            instance.Intensity  = intensity;

            level.DirLights.emplace_back(instance);
        }
    }

    auto iblLights = doc["IblLights"];
    if (iblLights.error() == simdjson::SUCCESS)
    {
        for(auto light : iblLights.get_array())
        {
            auto obj = light.get_object();

            EditImageBasedLight instance = {};
            auto tag        = std::string(std::string_view(obj["Tag"]));
            auto path       = std::string(std::string_view(obj["Path"]));
            auto intensity  = float(double(obj["Intensity"]));

            instance.Tag        = tag;
            instance.Path       = path;
            instance.Intensity  = intensity;

            level.IblLights.emplace_back(instance);
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
            instance.UserId     = obj["UserId"].get_uint64().value();

            level.Pins.emplace_back(instance);
        }
    }

    auto volumes = doc["Volumes"];
    if (volumes.error() == simdjson::SUCCESS)
    {
        for(auto volume : volumes.get_array())
        {
            auto obj  = volume.get_object();
            auto tag  = std::string(std::string_view(obj["Tag"]));
            auto path = std::string(std::string_view(obj["Path"]));

            auto s = ReadVector3   (obj["Scale"]   .get_object());
            auto r = ReadQuaternion(obj["Rotation"].get_object());
            auto t = ReadVector3   (obj["Position"].get_object());

            EditVolume instance = {};
            instance.Tag        = tag;
            instance.Position   = t;
            instance.Scale      = s;
            instance.Rotation   = r;
            instance.UserId     = obj["UserId"].get_uint64().value();

            level.Volumes.emplace_back(instance);

        }
    }

    // 正常終了.
    return true;
}