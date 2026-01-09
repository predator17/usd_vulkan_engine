#!/bin/bash
#==============================================================================
# USD Vulkan Engine - Dependencies Download Script
#==============================================================================
#
# This script downloads ALL CPM.cmake dependencies from their official
# GitHub repositories using git clone. Packages are cached in .cpm-cache/
# for use by CPM.cmake during cmake configuration.
#
# Usage:
#   ./scripts/download_dependencies.sh
#
# Environment:
#   HTTP_PROXY - Proxy server (auto-detected, can be overridden)
#   HTTPS_PROXY - Proxy server (auto-detected, can be overridden)
#
# To bypass proxy, set HTTP_PROXY="" and HTTPS_PROXY=""
#
# Proxy Auto-Detection:
#   Native Linux (Rocky Linux 9.7):   http://127.0.0.1:8118
#   WSL2 (Windows 11 Rocky Linux):    http://127.0.0.1:10808
#
# After running this script, run ./scripts/setup_usd.sh to build OpenUSD.
#
#==============================================================================

set -e  # Exit on error

#==============================================================================
# Configuration
#==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CPM_CACHE_DIR="$PROJECT_ROOT/.cpm-cache"

# Dependency versions
CPM_VERSION="v0.42.0"
CATCH2_VERSION="v3.12.0"
GLM_VERSION="1.0.2"
VULKAN_HEADERS_VERSION="v1.4.337"
VULKAN_HPP_VERSION="v1.4.337"
VULKAN_LOADER_VERSION="v1.4.337"
GLFW_VERSION="3.4"
IMGUI_VERSION="v1.92.2"
IMGUI_FILE_DIALOG_VERSION="v0.6.7"
SPDLOG_VERSION="v1.16.0"

# Detect WSL2
if grep -qi "microsoft" /proc/version 2>/dev/null; then
    DEFAULT_PROXY_PORT="10808"
    DETECTED_ENV="WSL2 (Windows 11)"
else
    DEFAULT_PROXY_PORT="8118"
    DETECTED_ENV="Native Linux"
fi

# Proxy configuration
if [ -z "$HTTP_PROXY" ] && [ -z "$HTTPS_PROXY" ]; then
    export HTTP_PROXY="http://127.0.0.1:${DEFAULT_PROXY_PORT}"
    export HTTPS_PROXY="http://127.0.0.1:${DEFAULT_PROXY_PORT}"
    export ALL_PROXY="$HTTPS_PROXY"
    AUTO_DETECTED_PROXY=1
else
    export ALL_PROXY="${HTTPS_PROXY:-$HTTP_PROXY}"
    AUTO_DETECTED_PROXY=0
fi

# Download settings
DOWNLOAD_TIMEOUT=600
MAX_RETRIES=3
RETRY_DELAY=5

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

#==============================================================================
# Functions
#==============================================================================

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Clone Git repository with retry
clone_repo() {
    local repo_url="$1"
    local tag="$2"
    local dest_dir="$3"
    local description="$4"

    log_info "Cloning $description..."

    if [ -d "$dest_dir" ] && [ -f "$dest_dir/.git/config" ]; then
        log_info "  Already exists at $dest_dir, skipping..."
        return 0
    fi

    mkdir -p "$(dirname "$dest_dir")"

    local attempt=1
    while [ $attempt -le $MAX_RETRIES ]; do
        if GIT_TERMINAL_PROMPT=0 git clone --depth 1 --branch "$tag" --single-branch \
                -c http.proxy="$HTTP_PROXY" \
                -c https.proxy="$HTTPS_PROXY" \
                "$repo_url" "$dest_dir" 2>/dev/null; then
            log_success "Cloned: $description"
            return 0
        else
            rm -rf "$dest_dir"
            log_warning "Attempt $attempt/$MAX_RETRIES failed for $description"
            if [ $attempt -lt $MAX_RETRIES ]; then
                log_info "Retrying in ${RETRY_DELAY}s..."
                sleep $RETRY_DELAY
            fi
            ((attempt++))
        fi
    done

    log_error "Failed to clone: $description after $MAX_RETRIES attempts"
    return 1
}

# Download file with retry
download_file() {
    local url="$1"
    local output="$2"
    local description="$3"

    log_info "Downloading $description..."

    local attempt=1
    while [ $attempt -le $MAX_RETRIES ]; do
        if curl --proxy "$HTTP_PROXY" \
                --fail \
                --location \
                --retry $MAX_RETRIES \
                --retry-delay $RETRY_DELAY \
                --max-time $DOWNLOAD_TIMEOUT \
                --show-error \
                -o "$output" \
                "$url"; then
            local size=$(du -h "$output" | cut -f1)
            log_success "Downloaded: $description ($size)"
            return 0
        else
            log_warning "Attempt $attempt/$MAX_RETRIES failed for $description"
            if [ $attempt -lt $MAX_RETRIES ]; then
                log_info "Retrying in ${RETRY_DELAY}s..."
                sleep $RETRY_DELAY
            fi
            ((attempt++))
        fi
    done

    log_error "Failed to download: $description after $MAX_RETRIES attempts"
    return 1
}

#==============================================================================
# Main
#==============================================================================

main() {
    echo "=========================================="
    echo "USD Vulkan Engine - Dependencies Download"
    echo "=========================================="
    echo ""
    log_info "Environment: $DETECTED_ENV"
    if [ "$AUTO_DETECTED_PROXY" -eq 1 ]; then
        log_info "Proxy: $HTTP_PROXY (auto-detected)"
    else
        log_info "Proxy: $HTTP_PROXY (user-specified)"
    fi
    log_info "Project Root: $PROJECT_ROOT"
    log_info "CPM Cache: $CPM_CACHE_DIR"
    echo ""

    # Test proxy connection
    log_info "Testing proxy connection..."
    if curl --proxy "$HTTP_PROXY" --max-time 10 -s -o /dev/null https://www.google.com; then
        log_success "Proxy connection OK"
    else
        log_warning "Proxy connection test failed (this may be OK if proxy is optional)"
    fi
    echo ""

    # Create directories
    log_info "Creating directory structure..."
    mkdir -p "$PROJECT_ROOT/cmake"
    mkdir -p "$CPM_CACHE_DIR"
    log_success "Directories created"
    echo ""

    local TOTAL_STEPS=10
    local CURRENT_STEP=0

    #==========================================================================
    # Step 1: CPM.cmake
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: CPM.cmake"
    log_info "=========================================="
    echo ""

    if [ ! -f "$PROJECT_ROOT/cmake/CPM.cmake" ]; then
        download_file \
            "https://github.com/cpm-cmake/CPM.cmake/releases/download/${CPM_VERSION}/CPM.cmake" \
            "$PROJECT_ROOT/cmake/CPM.cmake" \
            "CPM.cmake ${CPM_VERSION}"
    else
        log_info "CPM.cmake already exists, skipping..."
    fi
    echo ""

    #==========================================================================
    # Step 2: Catch2
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Catch2"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/catchorg/Catch2.git" \
        "$CATCH2_VERSION" \
        "$CPM_CACHE_DIR/catch2/catch2" \
        "Catch2 ${CATCH2_VERSION}"
    echo ""

    #==========================================================================
    # Step 3: GLM
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: GLM"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/g-truc/glm.git" \
        "$GLM_VERSION" \
        "$CPM_CACHE_DIR/glm/glm" \
        "GLM ${GLM_VERSION}"
    echo ""

    #==========================================================================
    # Step 4: Vulkan-Headers
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Vulkan-Headers"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/KhronosGroup/Vulkan-Headers.git" \
        "$VULKAN_HEADERS_VERSION" \
        "$CPM_CACHE_DIR/vulkan-headers/vulkan-headers" \
        "Vulkan-Headers ${VULKAN_HEADERS_VERSION}"
    echo ""

    #==========================================================================
    # Step 5: Vulkan-Hpp
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Vulkan-Hpp"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/KhronosGroup/Vulkan-Hpp.git" \
        "$VULKAN_HPP_VERSION" \
        "$CPM_CACHE_DIR/vulkan-hpp/vulkan-hpp" \
        "Vulkan-Hpp ${VULKAN_HPP_VERSION}"
    echo ""

    #==========================================================================
    # Step 6: Vulkan-Loader
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Vulkan-Loader"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/KhronosGroup/Vulkan-Loader.git" \
        "$VULKAN_LOADER_VERSION" \
        "$CPM_CACHE_DIR/vulkan-loader/vulkan-loader" \
        "Vulkan-Loader ${VULKAN_LOADER_VERSION}"
    echo ""

    #==========================================================================
    # Step 7: GLFW
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: GLFW"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/glfw/glfw.git" \
        "$GLFW_VERSION" \
        "$CPM_CACHE_DIR/glfw/glfw" \
        "GLFW ${GLFW_VERSION}"
    echo ""

    #==========================================================================
    # Step 8: Dear ImGui
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Dear ImGui"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/ocornut/imgui.git" \
        "$IMGUI_VERSION" \
        "$CPM_CACHE_DIR/imgui/imgui" \
        "Dear ImGui ${IMGUI_VERSION}"
    echo ""

    #==========================================================================
    # Step 9: ImGuiFileDialog
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: ImGuiFileDialog"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/aiekick/ImGuiFileDialog.git" \
        "$IMGUI_FILE_DIALOG_VERSION" \
        "$CPM_CACHE_DIR/imguifiledialog/ImGuiFileDialog" \
        "ImGuiFileDialog ${IMGUI_FILE_DIALOG_VERSION}"
    echo ""

    #==========================================================================
    # Step 10: spdlog
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: spdlog"
    log_info "=========================================="
    echo ""

    clone_repo \
        "https://github.com/gabime/spdlog.git" \
        "$SPDLOG_VERSION" \
        "$CPM_CACHE_DIR/spdlog/spdlog" \
        "spdlog ${SPDLOG_VERSION}"
    echo ""

    #==========================================================================
    # Summary
    #==========================================================================
    log_info "=========================================="
    log_info "Download Summary"
    log_info "=========================================="
    echo ""

    log_success "All CPM dependencies downloaded successfully!"
    echo ""

    log_info "CPM Cache contents:"
    ls -1 "$CPM_CACHE_DIR" 2>/dev/null | sed 's/^/  /' || echo "  (empty)"
    echo ""

    log_info "Next steps:"
    log_info "  1. Run: ./scripts/setup_usd.sh        (Download and build OpenUSD)"
    log_info "  2. Run: HTTP_PROXY=$HTTP_PROXY HTTPS_PROXY=$HTTPS_PROXY cmake -B build -DCMAKE_BUILD_TYPE=Release"
    log_info "  3. Run: cd build && make -j\$(nproc)"
    echo ""
}

# Run main
main "$@"
