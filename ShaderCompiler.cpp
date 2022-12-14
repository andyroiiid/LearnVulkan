//
// Created by andyroiiid on 12/12/2022.
//

#include "ShaderCompiler.h"

#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "Debug.h"

ShaderCompiler &ShaderCompiler::GetInstance() {
    static ShaderCompiler instance;
    return instance;
}

ShaderCompiler::ShaderCompiler() {
    DebugInfo("glslang version: {}", glslang::GetGlslVersionString());
    DebugCheckCritical(glslang::InitializeProcess(), "Failed to initialize glslang.");
}

ShaderCompiler::~ShaderCompiler() {
    glslang::FinalizeProcess();
}

void ShaderCompiler::SetPreamble(std::string preamble) {
    m_preamble = std::move(preamble);
}

bool ShaderCompiler::Compile(const EShLanguage stage, const char *source, std::vector<uint32_t> &spirv) {
    glslang::TShader shader(stage);
    shader.setStrings(&source, 1);
    shader.setPreamble(m_preamble.c_str());
    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

    if (!shader.parse(GetDefaultResources(), 100, false, EShMsgDefault)) {
        DebugError("Failed to parse shader: {}", shader.getInfoLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault)) {
        DebugError("Failed to link program: {}", program.getInfoLog());
        return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
    return true;
}
