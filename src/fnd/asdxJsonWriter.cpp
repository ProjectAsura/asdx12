//-----------------------------------------------------------------------------
// File : asdxJsonWriter.h
// Desc : Json Writer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <fnd/asdxJsonWriter.h>
#include <fnd/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// JsonWriter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
JsonWriter::JsonWriter()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
JsonWriter::~JsonWriter()
{ m_Stream.clear(); }

//-----------------------------------------------------------------------------
//      ファイルに出力します.
//-----------------------------------------------------------------------------
bool JsonWriter::Save(const char* path)
{
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "w");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path);
        return false;
    }

    auto view = m_Stream.view();
    fprintf_s(fp, "%s", view.data());
    fclose(fp);
    return true;
}

//-----------------------------------------------------------------------------
//      文字列を取得します.
//-----------------------------------------------------------------------------
std::string JsonWriter::GetString() const
{ return m_Stream.str(); }

//-----------------------------------------------------------------------------
//      文字列ビューを取得します.
//-----------------------------------------------------------------------------
std::string_view JsonWriter::GetStringView() const
{ return m_Stream.view(); }

//-----------------------------------------------------------------------------
//      文字列をクリアします.
//-----------------------------------------------------------------------------
void JsonWriter::Clear()
{ m_Stream.clear(); }

//-----------------------------------------------------------------------------
//      記録開始処理.
//-----------------------------------------------------------------------------
void JsonWriter::Begin()
{
    m_Stream << "{" << std::endl;
    m_Indent++;
}

//-----------------------------------------------------------------------------
//      記録終了処理.
//-----------------------------------------------------------------------------
void JsonWriter::End()
{
    m_Indent--;
    m_Stream << "}" << std::endl;
}

//-----------------------------------------------------------------------------
//      インデントを書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::WriteIndent()
{
    if (m_Indent == 0)
        return;

    for(auto i=0u; i<m_Indent; ++i)
        m_Stream << " ";
}

//-----------------------------------------------------------------------------
//      セクション開始を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::WriteBeginSection(const char* tag)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": {";
    m_Indent++;
}

//-----------------------------------------------------------------------------
//      セクション終了を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::WriteEndSection()
{
    assert(m_Indent > 0);
    m_Indent--;
    WriteIndent();
    m_Stream << "}";
}

//-----------------------------------------------------------------------------
//      配列開始を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::WriteBeginArray(const char* tag)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": [";
    m_Indent++;
}

//-----------------------------------------------------------------------------
//      配列終了を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::WriteEndArray()
{
    assert(m_Indent > 0);
    m_Indent--;
    WriteIndent();
    m_Stream << "]";
}

//-----------------------------------------------------------------------------
//      改行を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::WriteNextLine(bool comma)
{
    if (comma)
        m_Stream << ",";
    m_Stream << std::endl;
}

//-----------------------------------------------------------------------------
//      bool型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, bool value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      int8_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, int8_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      int16_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, int16_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      int32_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, int32_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      int64_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, int64_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      uint8_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, uint8_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      uint16_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, uint16_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      uint32_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, uint32_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      uint64_t型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, uint64_t value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      float型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, float value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      double型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, double value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": " << std::to_string(value);
}

//-----------------------------------------------------------------------------
//      文字列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const char* text)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": \"" << text << "\"";
}

//-----------------------------------------------------------------------------
//      std::string型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const std::string& value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": \"" << value << "\"";
}

//-----------------------------------------------------------------------------
//      std::string型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const std::string_view& value)
{
    WriteIndent();
    m_Stream << "\"" << tag << "\": \"" << value.data() << "\"";
}

//-----------------------------------------------------------------------------
//      Vector2型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const Vector2& value)
{
    WriteBeginSection(tag); WriteNextLine(false);
    Write("x", value.x); WriteNextLine(true);
    Write("y", value.y); WriteNextLine(false);
    WriteEndSection();
}

//-----------------------------------------------------------------------------
//      Vector3型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const Vector3& value)
{
    WriteBeginSection(tag);  WriteNextLine(false);
    Write("x", value.x); WriteNextLine(true);
    Write("y", value.y); WriteNextLine(true);
    Write("z", value.z); WriteNextLine(false);
    WriteEndSection();
}

//-----------------------------------------------------------------------------
//      Vector4型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const Vector4& value)
{
    WriteBeginSection(tag); WriteNextLine(false);
    Write("x", value.x); WriteNextLine(true);
    Write("y", value.y); WriteNextLine(true);
    Write("z", value.z); WriteNextLine(true);
    Write("w", value.w); WriteNextLine(false);
    WriteEndSection();
}

//-----------------------------------------------------------------------------
//      Quaternion型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const Quaternion& value)
{
    WriteBeginSection(tag); WriteNextLine(false);
    Write("qx", value.x); WriteNextLine(true);
    Write("qy", value.y); WriteNextLine(true);
    Write("qz", value.z); WriteNextLine(true);
    Write("qw", value.w); WriteNextLine(false);
    WriteEndSection();
}

//-----------------------------------------------------------------------------
//      Matrix型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const Matrix& value)
{
    WriteBeginSection(tag); WriteNextLine(false);
    Write("e11", value._11); WriteNextLine(true);
    Write("e12", value._12); WriteNextLine(true);
    Write("e13", value._13); WriteNextLine(true);
    Write("e14", value._14); WriteNextLine(true);
    Write("e21", value._21); WriteNextLine(true);
    Write("e22", value._22); WriteNextLine(true);
    Write("e23", value._23); WriteNextLine(true);
    Write("e24", value._24); WriteNextLine(true);
    Write("e31", value._31); WriteNextLine(true);
    Write("e32", value._32); WriteNextLine(true);
    Write("e33", value._33); WriteNextLine(true);
    Write("e34", value._34); WriteNextLine(true);
    Write("e41", value._41); WriteNextLine(true);
    Write("e42", value._42); WriteNextLine(true);
    Write("e43", value._43); WriteNextLine(true);
    Write("e44", value._44); WriteNextLine(false);
    WriteEndSection();
}

//-----------------------------------------------------------------------------
//      Transform3x4型を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, const Transform3x4& value)
{
    WriteBeginSection(tag); WriteNextLine(false);
    Write("e11", value._11); WriteNextLine(true);
    Write("e12", value._12); WriteNextLine(true);
    Write("e13", value._13); WriteNextLine(true);
    Write("e14", value._14); WriteNextLine(true);
    Write("e21", value._21); WriteNextLine(true);
    Write("e22", value._22); WriteNextLine(true);
    Write("e23", value._23); WriteNextLine(true);
    Write("e24", value._24); WriteNextLine(true);
    Write("e31", value._31); WriteNextLine(true);
    Write("e32", value._32); WriteNextLine(true);
    Write("e33", value._33); WriteNextLine(true);
    Write("e34", value._34); WriteNextLine(false);
    WriteEndSection();
}

//-----------------------------------------------------------------------------
//      bool型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<bool> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      int8_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<int8_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      int16_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<int16_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      int32_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<int32_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      int64_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<int64_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      uint8_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<uint8_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      uint16_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<uint16_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      uint32_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<uint32_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      uint64_t型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<uint64_t> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      float型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<float> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      double型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<double> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << std::to_string(values[i]);
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      Vector2型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<Vector2> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "{ \"x\": " << values[i].x << ", \"y\": " << values[i].y << " }";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      Vector3型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<Vector3> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "{ \"x\": " << values[i].x << ", \"y\": " << values[i].y << ", \"z\": "<< values[i].z << " }";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      Vector4型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<Vector4> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "{ \"x\": " << values[i].x << ", \"y\": " << values[i].y << ", \"z\": "<< values[i].z << ", \"w\": " << values[i].w << " }";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      Quaternion型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<Quaternion> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "{ \"qx\": " << values[i].x << ", \"qy\": " << values[i].y << ", \"qz\": "<< values[i].z << ", \"qw\": " << values[i].w << " }";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      Matrix型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<Matrix> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Indent++;
        m_Stream << "{"; WriteNextLine(false);
        WriteIndent(); m_Stream << "\"e11\": " << values[i]._11; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e12\": " << values[i]._12; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e13\": " << values[i]._13; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e14\": " << values[i]._14; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e21\": " << values[i]._21; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e22\": " << values[i]._22; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e23\": " << values[i]._23; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e24\": " << values[i]._24; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e31\": " << values[i]._31; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e32\": " << values[i]._32; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e33\": " << values[i]._33; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e34\": " << values[i]._34; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e41\": " << values[i]._41; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e42\": " << values[i]._42; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e43\": " << values[i]._43; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e44\": " << values[i]._44; WriteNextLine(false);
        m_Indent--;
        WriteIndent(); m_Stream << "}";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      Transform3x4型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<Transform3x4> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Indent++;
        m_Stream << "{"; WriteNextLine(false);
        WriteIndent(); m_Stream << "\"e11\": " << values[i]._11; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e12\": " << values[i]._12; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e13\": " << values[i]._13; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e14\": " << values[i]._14; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e21\": " << values[i]._21; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e22\": " << values[i]._22; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e23\": " << values[i]._23; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e24\": " << values[i]._24; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e31\": " << values[i]._31; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e32\": " << values[i]._32; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e33\": " << values[i]._33; WriteNextLine(true);
        WriteIndent(); m_Stream << "\"e34\": " << values[i]._34; WriteNextLine(true);
        m_Indent--;
        WriteIndent(); m_Stream << "}";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      文字列配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<const char*> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "\"" << values[i] << "\"";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      std::string型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<std::string> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "\"" << values[i] << "\"";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

//-----------------------------------------------------------------------------
//      std::string_view型配列を書き込みます.
//-----------------------------------------------------------------------------
void JsonWriter::Write(const char* tag, std::span<std::string_view> values)
{
    WriteBeginArray(tag); WriteNextLine(false);
    for(size_t i=0; i<values.size(); ++i)
    {
        WriteIndent();
        m_Stream << "\"" << values[i].data() << "\"";
        WriteNextLine(i != values.size() - 1);
    }
    WriteEndArray();
}

} // namespace asdx
