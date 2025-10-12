//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <MapChipConverter.h>
#include <cmdline.h>


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf_s("MapChipConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    cmdline::parser parser;

    // 引数指定を設定.
    parser.add<std::string>("input",  'i', "Input Txm File Path.", true, "");
    parser.add<std::string>("output", 'o', "Output MapChip Binary Path", true, "");

    // 解析実行.
    parser.parse_check(argc, argv);

    MapChipConverter::Desc desc = {};
    desc.InputPath  = parser.get<std::string>("input");
    desc.OutputPath = parser.get<std::string>("output");

    auto ret = MapChipConverter().Convert(desc);
    if (ret)
    {
        fprintf_s(stdout, "MapChip Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "MapChip Convert Failed... InputPath = %s, OutputPath = %s\n", desc.InputPath.c_str(), desc.OutputPath.c_str());
    return -1;
}
