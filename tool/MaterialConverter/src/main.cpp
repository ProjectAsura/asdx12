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
    if (argc < 2)
    {
        printf_s("MateiralConverter Failed. Invalid Arguments.\n");
        return -1;
    }

    cmdline::parser parser;

    // 引数指定を設定.
    parser.add<std::string>("input", 'i', "Input Mep file path", true, "");
    parser.add<std::string>("output", 'o', "Output Material Binary Path", true, "");

    // 解析実行.
    parser.parse_check(argc, argv);

    MaterialConverter::Desc desc = {};
    desc.InputPath  = parser.get<std::string>("input");
    desc.OutputPath = parser.get<std::string>("output");

    auto ret = MaterialConverter().Convert(desc);
    if (ret)
    {
        fprintf_s(stdout, "Material Convert Success! OutputPath = %s\n", desc.OutputPath.c_str());
        return 0;
    }

    fprintf_s(stderr, "Material Convert Failed... InputPath = %s\n", desc.InputPath.c_str());
    return 0;
}
