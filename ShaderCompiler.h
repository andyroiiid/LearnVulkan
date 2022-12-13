//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <glslang/Public/ShaderLang.h>

class ShaderCompiler {
public:
    ShaderCompiler();

    ~ShaderCompiler();

    void SetPreamble(std::string preamble);

    bool Compile(EShLanguage stage, const char *source, std::vector<unsigned int> &spirv);

private:
    std::string m_preamble;
};
