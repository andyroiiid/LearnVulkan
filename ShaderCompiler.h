//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <glslang/Public/ShaderLang.h>

// Might not be a good practice to keep a glslang instance in memory all the time
// Don't really care right now
class ShaderCompiler {
public:
    static ShaderCompiler &GetInstance();

    ShaderCompiler(const ShaderCompiler &) = delete;

    ShaderCompiler &operator=(const ShaderCompiler &) = delete;

    ShaderCompiler(ShaderCompiler &&) = delete;

    ShaderCompiler &operator=(ShaderCompiler &&) = delete;

    void SetPreamble(std::string preamble);

    bool Compile(EShLanguage stage, const char *source, std::vector<uint32_t> &spirv);

private:
    ShaderCompiler();

    ~ShaderCompiler();

    std::string m_preamble;
};
