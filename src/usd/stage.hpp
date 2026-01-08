#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class USDStage {
public:
    USDStage();
    ~USDStage();

    bool load(const std::string& path);
    bool isValid() const { return m_valid; }

    std::string getPath() const { return m_path; }

    // Time range
    double getStartTimeCode() const;
    double getEndTimeCode() const;

private:
    std::string m_path;
    bool m_valid = false;

    // USD internals (pimpl to avoid USD headers in this header)
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
