set(SPDLOG_FOUND TRUE)
set(SPDLOG_INCLUDE_DIRS "/mnt/wd_hdd_18t/rd/github/usd_vulkan_engine/external/spdlog/include")
set(SPDLOG_LIBRARIES "")
add_library(spdlog::spdlog INTERFACE IMPORTED)
set_target_properties(spdlog::spdlog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "/mnt/wd_hdd_18t/rd/github/usd_vulkan_engine/external/spdlog/include"
)
