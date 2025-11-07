//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Animation Clip Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <AnimationClipConverter.h>
#include <cmdline.h>


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf_s("AnimationClipConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    cmdline::parser parser;

    // 引数を設定.
    parser.add<std::string>("input",  'i', "input animation file path", true, "");
    parser.add<std::string>("output", 'o', "output animation file path", true, "");

    // 解析実行.
    parser.parse_check(argc, argv);

    // 変換設定.
    asdx::AnimationClipConverter::Desc desc = {};
    desc.InputPath  = parser.get<std::string>("input");
    desc.OutputPath = parser.get<std::string>("output");

    // 変換処理実行.
    auto ret = asdx::AnimationClipConverter::Convert(desc);

    // 正常終了.
    if (ret)
    {
        fprintf_s(stdout, "AnimationClip Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "AnimationClip Convert Failed... InputPath = %s\n", desc.InputPath.c_str());
    return -1;
}

