#!/bin/bash
#==============================================================================
# USD Vulkan Engine - Dependencies Download Script
#==============================================================================
#
# This script downloads ALL dependencies including:
#   - CPM.cmake (package manager)
#   - OpenUSD source (built separately)
#   - Vulkan-Components (Headers, Hpp, Loader)
#   - GLM (math library)
#   - GLFW (windowing)
#   - Dear ImGui (UI)
#   - ImGuiFileDialog (file dialog)
#   - spdlog (logging)
#   - Catch2 (testing)
#
# All packages are downloaded to .cpm-cache/ for use by CPM.cmake.
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
#==============================================================================

set -e  # Exit on error

#==============================================================================
# Configuration
#==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CPM_CACHE_DIR="$PROJECT_ROOT/.cpm-cache"

# Detect WSL2
# /proc/version contains "microsoft" in WSL2 environments
if grep -qi "microsoft" /proc/version 2>/dev/null; then
    # WSL2 environment - use Windows proxy port
    DEFAULT_PROXY_PORT="10808"
    DETECTED_ENV="WSL2 (Windows 11)"
else
    # Native Linux - use Linux proxy port
    DEFAULT_PROXY_PORT="8118"
    DETECTED_ENV="Native Linux"
fi

# Proxy configuration (can be overridden by environment variables)
if [ -z "$HTTP_PROXY" ] && [ -z "$HTTPS_PROXY" ]; then
    export HTTP_PROXY="http://127.0.0.1:${DEFAULT_PROXY_PORT}"
    export HTTPS_PROXY="http://127.0.0.1:${DEFAULT_PROXY_PORT}"
    export ALL_PROXY="$HTTPS_PROXY"
    AUTO_DETECTED_PROXY=1
else
    export ALL_PROXY="${HTTPS_PROXY:-$HTTP_PROXY}"
    AUTO_DETECTED_PROXY=0
fi

# Download timeout (seconds)
DOWNLOAD_TIMEOUT=600

# Retry settings
MAX_RETRIES=3
RETRY_DELAY=5

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

#==============================================================================
# Functions
#==============================================================================

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Clone Git repository
clone_repo() {
    local repo_url="$1"
    local tag="$2"
    local dest_dir="$3"
    local description="$4"

    log_info "Cloning $description..."

    if [ -d "$dest_dir" ] && [ -f "$dest_dir/.git/config" ]; then
        log_info "  Already exists, skipping..."
        return 0
    fi

    # Create parent directory
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
            # Clean up failed clone attempt
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

# Download file (for tarballs)
download_file() {
    local url="$1"
    local output="$2"
    local description="$3"

    log_info "Downloading $description..."
    log_info "  URL: $url"

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
    mkdir -p "$PROJECT_ROOT/OpenUSD/src"
    mkdir -p "$PROJECT_ROOT/OpenUSD/build"
    mkdir -p "$PROJECT_ROOT/OpenUSD/install"
    mkdir -p "$CPM_CACHE_DIR"
    log_success "Directories created"
    echo ""

    local TOTAL_STEPS=11
    local CURRENT_STEP=0

    #==========================================================================
    # Step 1: Download CPM.cmake
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading CPM.cmake"
    log_info "=========================================="
    echo ""

    local CPM_VERSION="v0.42.0"
    local CPM_URL="https://github.com/cpm-cmake/CPM.cmake/releases/download/${CPM_VERSION}/CPM.cmake"

    if [ ! -f "$PROJECT_ROOT/cmake/CPM.cmake" ]; then
        download_file "$CPM_URL" "$PROJECT_ROOT/cmake/CPM.cmake" "CPM.cmake ${CPM_VERSION}"
    else
        log_info "CPM.cmake already exists, skipping..."
    fi
    echo ""

    #==========================================================================
    # Step 2: Download Catch2
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading Catch2"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/catchorg/Catch2.git" \
               "v3.12.0" \
               "$CPM_CACHE_DIR/catch2/catch2" \
               "Catch2 v3.12.0"
    echo ""

    #==========================================================================
    # Step 3: Download GLM
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading GLM"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/g-truc/glm.git" \
               "1.0.2" \
               "$CPM_CACHE_DIR/glm/glm" \
               "GLM 1.0.2"
    echo ""

    #==========================================================================
    # Step 4: Download Vulkan-Headers
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading Vulkan-Headers"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/KhronosGroup/Vulkan-Headers.git" \
               "v1.4.337" \
               "$CPM_CACHE_DIR/vulkan-headers/vulkan-headers" \
               "Vulkan-Headers v1.4.337"
    echo ""

    #==========================================================================
    # Step 5: Download Vulkan-Hpp
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading Vulkan-Hpp"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/KhronosGroup/Vulkan-Hpp.git" \
               "v1.4.337" \
               "$CPM_CACHE_DIR/vulkan-hpp/vulkan-hpp" \
               "Vulkan-Hpp v1.4.337"
    echo ""

    #==========================================================================
    # Step 6: Download Vulkan-Loader
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading Vulkan-Loader"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/KhronosGroup/Vulkan-Loader.git" \
               "v1.4.337" \
               "$CPM_CACHE_DIR/vulkan-loader/vulkan-loader" \
               "Vulkan-Loader v1.4.337"
    echo ""

    #==========================================================================
    # Step 7: Download GLFW
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading GLFW"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/glfw/glfw.git" \
               "3.4" \
               "$CPM_CACHE_DIR/glfw/glfw" \
               "GLFW 3.4"
    echo ""

    #==========================================================================
    # Step 8: Download Dear ImGui
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading Dear ImGui"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/ocornut/imgui.git" \
               "v1.92.2" \
               "$CPM_CACHE_DIR/imgui/imgui" \
               "Dear ImGui v1.92.2"
    echo ""

    #==========================================================================
    # Step 9: Download ImGuiFileDialog
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading ImGuiFileDialog"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/aiekick/ImGuiFileDialog.git" \
               "v0.6.7" \
               "$CPM_CACHE_DIR/imguifiledialog/ImGuiFileDialog" \
               "ImGuiFileDialog v0.6.7"
    echo ""

    #==========================================================================
    # Step 10: Download spdlog
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading spdlog"
    log_info "=========================================="
    echo ""

    clone_repo "https://github.com/gabime/spdlog.git" \
               "v1.16.0" \
               "$CPM_CACHE_DIR/spdlog/spdlog" \
               "spdlog v1.16.0"
    echo ""

    #==========================================================================
    # Step 11: Download OpenUSD
    #==========================================================================
    ((CURRENT_STEP++))
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading OpenUSD"
    log_info "=========================================="
    echo ""

    local USD_VERSION="v25.11"
    local USD_URL="https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/${USD_VERSION}.tar.gz"
    local USD_OUTPUT="$PROJECT_ROOT/OpenUSD/src/OpenUSD-${USD_VERSION}.tar.gz"

    if [ -f "$USD_OUTPUT" ]; then
        log_info "OpenUSD ${USD_VERSION} already exists, skipping..."
    else
        download_file "$USD_URL" "$USD_OUTPUT" "OpenUSD ${USD_VERSION}"
    fi
    echo ""

    #==========================================================================
    # Summary
    #==========================================================================
    log_info "=========================================="
    log_info "Download Summary"
    log_info "=========================================="
    echo ""

    log_success "All dependencies downloaded successfully!"
    echo ""

    log_info "CPM Cache contents:"
    ls -1 "$CPM_CACHE_DIR" 2>/dev/null | sed 's/^/  /' || echo "  (empty)"
    echo ""

    log_info "Next steps:"
    log_info "  1. Build OpenUSD (see README.md 'Build USD' section)"
    log_info "  2. Run: HTTP_PROXY=$HTTP_PROXY HTTPS_PROXY=$HTTPS_PROXY cmake -B build -DCMAKE_BUILD_TYPE=Release"
    log_info "  3. Run: cd build && make -j\$(nproc)"
    echo ""
}

# Run main
main "$@"
