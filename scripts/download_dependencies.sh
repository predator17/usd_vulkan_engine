#!/bin/bash
#==============================================================================
# USD Vulkan Engine - Dependencies Download Script
#==============================================================================
#
# This script downloads all project dependencies from their official sources.
#
# Usage:
#   ./scripts/download_dependencies.sh
#
# Environment:
#   HTTP_PROXY - Proxy server (default: http://127.0.0.1:8118)
#   HTTPS_PROXY - Proxy server (default: http://127.0.0.1:8118)
#
# To bypass proxy, set HTTP_PROXY="" and HTTPS_PROXY=""
#
#==============================================================================

set -e  # Exit on error

#==============================================================================
# Configuration
#==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Proxy configuration
export HTTP_PROXY="${HTTP_PROXY:-http://127.0.0.1:8118}"
export HTTPS_PROXY="${HTTPS_PROXY:-http://127.0.0.1:8118}"
export ALL_PROXY="$HTTPS_PROXY"

# Download timeout (seconds)
DOWNLOAD_TIMEOUT=300

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

# Download with retry
download_file() {
    local url="$1"
    local output="$2"
    local description="$3"

    log_info "Downloading $description..."
    log_info "  URL: $url"
    log_info "  To: $output"
    log_info "  Proxy: $HTTP_PROXY"

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
            log_success "Downloaded: $description"
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

# Verify tar.gz archive
verify_archive() {
    local file="$1"
    local description="$2"

    log_info "Verifying $description..."

    if [ ! -f "$file" ]; then
        log_error "File not found: $file"
        return 1
    fi

    # Try to list archive contents
    if tar -tzf "$file" > /dev/null 2>&1; then
        local size=$(du -h "$file" | cut -f1)
        log_success "Archive verified: $description ($size)"
        return 0
    else
        log_error "Archive verification failed: $description"
        return 1
    fi
}

#==============================================================================
# Dependency Definitions
#==============================================================================

declare -A DEPS=(
    # [name]="url|version|output_dir|filename|description"
    [cpm]="https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.0/CPM.cmake|0.42.0|cmake|CPM.cmake"
    [catch2]="https://github.com/catchorg/Catch2/archive/refs/tags/v3.12.0.tar.gz|3.12.0|cpm-sources|catch2-3.12.0.tar.gz|Catch2 v3.12.0"
    [glm]="https://github.com/g-truc/glm/archive/refs/tags/1.0.2.tar.gz|1.0.2|cpm-sources|glm-1.0.2.tar.gz|GLM 1.0.2"
    [imgui]="https://github.com/ocornut/imgui/archive/refs/tags/v1.92.2.tar.gz|1.92.2|cpm-sources|imgui-1.92.2.tar.gz|Dear ImGui v1.92.2"
    [spdlog]="https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz|1.16.0|cpm-sources|spdlog-1.16.0.tar.gz|spdlog v1.16.0"
    [imguifiledialog]="https://github.com/aiekick/ImGuiFileDialog/archive/refs/tags/v0.6.7.tar.gz|0.6.7|cpm-sources|ImGuiFileDialog-0.6.7.tar.gz|ImGuiFileDialog v0.6.7"
    [glfw]="https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz|3.4|cpm-sources|glfw-3.4.tar.gz|GLFW 3.4"
    [vulkan-headers]="https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.4.337.tar.gz|1.4.337|cpm-sources|Vulkan-Headers-1.4.337.tar.gz|Vulkan-Headers v1.4.337"
    [vulkan-hpp]="https://github.com/KhronosGroup/Vulkan-Hpp/archive/refs/tags/v1.4.337.tar.gz|1.4.337|cpm-sources|Vulkan-Hpp-1.4.337.tar.gz|Vulkan-Hpp v1.4.337"
    [usd]="https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v25.11.tar.gz|25.11|OpenUSD/src|OpenUSD-25.11.tar.gz|OpenUSD v25.11"
)

#==============================================================================
# Main
#==============================================================================

main() {
    echo "=========================================="
    echo "USD Vulkan Engine - Dependencies Download"
    echo "=========================================="
    echo ""
    log_info "Proxy: $HTTP_PROXY"
    log_info "Project Root: $PROJECT_ROOT"
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
    mkdir -p "$PROJECT_ROOT/.cpm-cache/_ext"
    log_success "Directories created"
    echo ""

    # Download CPM.cmake first (it's small and needed for others)
    log_info "=========================================="
    log_info "Step 1: Downloading CPM.cmake"
    log_info "=========================================="

    local cpm_info="${DEPS[cpm]}"
    IFS='|' read -r cpm_url cpm_ver cpm_dir cpm_file cpm_desc <<< "$cpm_info"

    if [ ! -f "$PROJECT_ROOT/cmake/CPM.cmake" ]; then
        download_file "$cpm_url" "$PROJECT_ROOT/cmake/CPM.cmake" "$cpm_desc"
    else
        log_info "CPM.cmake already exists, skipping..."
    fi
    echo ""

    # Download CPM-managed dependencies
    log_info "=========================================="
    log_info "Step 2: Downloading CPM Dependencies"
    log_info "=========================================="
    echo ""

    local cpm_sources_dir="$PROJECT_ROOT/.cpm-cache/_ext"

    for dep in catch2 glm imgui spdlog imguifiledialog glfw vulkan-headers vulkan-hpp; do
        local info="${DEPS[$dep]}"
        IFS='|' read -r url version dir filename description <<< "$info"

        local output_path="$PROJECT_ROOT/$dir/$filename"

        if [ -f "$output_path" ]; then
            log_info "$description already exists, skipping..."
            echo ""
            continue
        fi

        mkdir -p "$PROJECT_ROOT/$dir"
        download_file "$url" "$output_path" "$description"
        verify_archive "$output_path" "$description"
        echo ""
    done

    # Download OpenUSD (large, separate)
    log_info "=========================================="
    log_info "Step 3: Downloading OpenUSD"
    log_info "=========================================="
    echo ""

    local usd_info="${DEPS[usd]}"
    IFS='|' read -r usd_url usd_ver usd_dir usd_file usd_desc <<< "$usd_info"

    local usd_output="$PROJECT_ROOT/$usd_dir/$usd_file"

    if [ -f "$usd_output" ]; then
        log_info "$usd_desc already exists, skipping..."
    else
        download_file "$usd_url" "$usd_output" "$usd_desc"
        verify_archive "$usd_output" "$usd_desc"

        # Get file size
        local size=$(du -h "$usd_output" | cut -f1)
        log_success "OpenUSD downloaded: $size"
    fi
    echo ""

    # Summary
    log_info "=========================================="
    log_info "Download Summary"
    log_info "=========================================="
    echo ""

    log_info "Downloaded files:"
    echo "  cmake/CPM.cmake"
    echo "  .cpm-cache/_ext/"
    echo "    ├── catch2-3.12.0.tar.gz"
    echo "    ├── glm-1.0.2.tar.gz"
    echo "    ├── imgui-1.92.2.tar.gz"
    echo "    ├── spdlog-1.16.0.tar.gz"
    echo "    ├── ImGuiFileDialog-0.6.7.tar.gz"
    echo "    ├── glfw-3.4.tar.gz"
    echo "    ├── Vulkan-Headers-1.4.337.tar.gz"
    echo "    └── Vulkan-Hpp-1.4.337.tar.gz"
    echo "  OpenUSD/src/"
    echo "    └── OpenUSD-25.11.tar.gz"
    echo ""

    log_success "All dependencies downloaded successfully!"
    echo ""
    log_info "Next steps:"
    log_info "  1. Review and approve the refactor_dependencies.md document"
    log_info "  2. Run: ./scripts/build_dependencies.sh (to build dependencies)"
    log_info "  3. Run: cmake -B build (to configure the project)"
}

# Run main
main "$@"
