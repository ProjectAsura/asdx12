//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <FontConverter.h>
#include <cmdline.h>

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf_s("FontConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    cmdline::parser parser;

    // 引数指定を設定.
    parser.add<std::string>("texture", 't', "Input DDS file Path", true, "");
    parser.add<std::string>("json", 'j', "Input Glyph JSON File Path", true, "");
    parser.add<std::string>("output", 'o', "Output Font Binary Path", true, "");

    // 解析実行.
    parser.parse_check(argc, argv);

    FontConverter::Desc desc = {};
    desc.DdsPath    = parser.get<std::string>("texture");
    desc.JsonPath   = parser.get<std::string>("json");
    desc.OutputPath = parser.get<std::string>("output");

    auto ret = FontConverter().Convert(desc);

    if (ret)
    {
        fprintf_s(stdout, "Font Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "Font Convert Failed... DdsPath = %s, JsonPath = %s\n", desc.DdsPath.c_str(), desc.JsonPath.c_str());
    return -1;
}
