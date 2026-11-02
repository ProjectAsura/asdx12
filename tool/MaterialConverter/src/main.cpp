//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cmdline.h>
#include <MaterialConverter.h>


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    // 引数チェック.
    if (argc < 2)
    {
        printf_s("MaterialConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    // コマンドラインの解析
    cmdline::parser parser;
    parser.add<std::string>("input",    'i', "Input file path.", true);
    parser.add<std::string>("output",   'o', "Output file path.", true);
    parser.add("help", 'h', "Show help message.");
    parser.parse_check(argc, argv);

    // 変換設定.
    MaterialConverter::Desc desc;
    desc.InputPath  = parser.get<std::string>("input");
    desc.OutputPath = parser.get<std::string>("output");

    // 変換処理実行.
    auto ret = MaterialConverter::Convert(desc);
    if (ret)
    {
        fprintf_s(stdout, "Material Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "Material Convert Failed... InputPath = %s\n", desc.InputPath.c_str());;
    return -1;
}
