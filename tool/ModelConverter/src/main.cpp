//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ModelConverter.h>
#include <cmdline.h>


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf_s("ModelConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    cmdline::parser parser;

    // 引数を設定.
    parser.add<std::string>("input",  'i', "input model file path", true, "");
    parser.add<std::string>("output", 'o', "output model file path", true, "");

    // 解析実行.
    parser.parse_check(argc, argv);

    // 変換設定.
    ModelConverter::Desc desc = {};
    desc.InputPath  = parser.get<std::string>("input");
    desc.OutputPath = parser.get<std::string>("output");

    // 変換処理実行.
    auto ret = ModelConverter::Convert(desc);

    // 正常終了.
    if (ret)
    {
        fprintf_s(stdout, "Model Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "Model Convert Failed... InputPath = %s\n", desc.InputPath.c_str());
    return -1;
}

