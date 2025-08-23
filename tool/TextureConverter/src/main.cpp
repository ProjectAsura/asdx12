//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Texture Converter Exe.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cmdline.h>
#include <TextureConverter.h>


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf_s("TextureConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    cmdline::parser parser;

    // 引数指定を設定.
    parser.add<std::string>("input",  'i', "input texture file path", true, "");
    parser.add<std::string>("output", 'o', "output texture file path", true, "");

    // 解析実行.
    parser.parse_check(argc, argv);

    // 変換設定.
    asdx::TextureConverter::Desc desc = {};
    desc.InputPath  = parser.get<std::string>("input");
    desc.OutputPath = parser.get<std::string>("output");

    // 変換処理実行.
    auto ret = asdx::TextureConverter().Convert(desc);

    // 正常終了.
    if (ret)
    {
        fprintf_s(stdout, "Texture Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "Texture Convert Failed... InputPath = %s\n", desc.InputPath.c_str());
    return -1;
}
