#pragma once

#include <string>
#include <vector>
#include "shader.hpp"

/**
 * @brief Shader compilation utilities for GLSL to SPIR-V conversion
 *
 * Provides runtime GLSL compilation using system glslangValidator.
 * Falls back to pre-compiled SPIR-V if compilation is not available.
 */
class ShaderCompiler {
public:
    struct CompileResult {
        bool success = false;
        std::string errorMessage;
        std::vector<uint32_t> spirvCode;

        static CompileResult ok(const std::vector<uint32_t>& code) {
            return {true, "", code};
        }

        static CompileResult error(const std::string& msg) {
            return {false, msg, {}};
        }
    };

    /**
     * @brief Compile GLSL source code to SPIR-V
     */
    static CompileResult compileGLSL(const std::string& glslCode, Shader::Stage stage);

    /**
     * @brief Compile GLSL file to SPIR-V
     */
    static CompileResult compileGLSLFile(const std::string& path, Shader::Stage stage);

    /**
     * @brief Compile GLSL file and save SPIR-V to output path
     */
    static bool compileAndSave(const std::string& glslPath,
                               const std::string& spirvPath,
                               Shader::Stage stage);

    /**
     * @brief Check if glslangValidator is available
     */
    static bool isCompilerAvailable();

    /**
     * @brief Get GLSL definition for a shader stage
     */
    static std::string getStageDefinition(Shader::Stage stage);

private:
    static bool findGlslangValidator();
    static std::string s_glslangValidatorPath;
    static bool s_compilerChecked;
};
