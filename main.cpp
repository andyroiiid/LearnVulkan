#include "Window.h"
#include "ShaderCompiler.h"
#include "Debug.h"

extern "C" {
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
__declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 0x00000001;
}

int main() {
    ShaderCompiler shaderCompiler;

    shaderCompiler.SetPreamble(R"GLSL(
#define RED vec4(1, 0, 0, 1)
)GLSL");

    const char *vertexSource = R"GLSL(
#version 450 core

void main()
{
    const vec3 POSITIONS[3] = vec3[3](
            vec3(1, 1, 0),
            vec3(-1, 1, 0),
            vec3(0, -1, 0)
    );
    gl_Position = vec4(POSITIONS[gl_VertexIndex], 1);
}
)GLSL";
    const char *fragmentSource = R"GLSL(
#version 450 core

layout (location = 0) out vec4 fColor;

void main()
{
    fColor = RED;
}
)GLSL";

    std::vector<uint32_t> vertexSpirv;
    std::vector<uint32_t> fragmentSpirv;

    DebugCheckCritical(
            shaderCompiler.Compile(EShLangVertex, vertexSource, vertexSpirv),
            "Failed to compile vertex shader."
    );
    DebugCheckCritical(
            shaderCompiler.Compile(EShLangFragment, fragmentSource, fragmentSpirv),
            "Failed to compile fragment shader."
    );

    DebugInfo("vertex spirv size = {}", vertexSpirv.size() * 4);
    DebugInfo("fragment spirv size = {}", fragmentSpirv.size() * 4);

    Window window;
    window.MainLoop();
    return 0;
}
