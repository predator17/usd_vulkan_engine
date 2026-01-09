#!/bin/bash
#==============================================================================
# USD Vulkan Engine - Dependencies Download Script
#==============================================================================
#
# This script downloads CPM.cmake and OpenUSD source.
#
# All other dependencies (Vulkan, GLM, ImGui, etc.) are downloaded
# automatically by CPM.cmake during cmake configuration.
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

# Verify tar.gz archive
verify_archive() {
    local file="$1"
    local description="$2"

    if [ ! -f "$file" ]; then
        log_error "File not found: $file"
        return 1
    fi

    # Try to list archive contents
    if tar -tzf "$file" > /dev/null 2>&1; then
        log_success "Archive verified: $description"
        return 0
    else
        log_error "Archive verification failed: $description"
        return 1
    fi
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
    log_success "Directories created"
    echo ""

    #==========================================================================
    # Step 1: Download CPM.cmake
    #==========================================================================
    log_info "=========================================="
    log_info "Step 1: Downloading CPM.cmake"
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
    # Step 2: Download OpenUSD
    #==========================================================================
    log_info "=========================================="
    log_info "Step 2: Downloading OpenUSD v25.11"
    log_info "=========================================="
    echo ""

    local USD_VERSION="v25.11"
    local USD_URL="https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/${USD_VERSION}.tar.gz"
    local USD_OUTPUT="$PROJECT_ROOT/OpenUSD/src/OpenUSD-${USD_VERSION}.tar.gz"

    if [ -f "$USD_OUTPUT" ]; then
        log_info "OpenUSD ${USD_VERSION} already exists, skipping..."
    else
        download_file "$USD_URL" "$USD_OUTPUT" "OpenUSD ${USD_VERSION}"
        verify_archive "$USD_OUTPUT" "OpenUSD ${USD_VERSION}"
    fi
    echo ""

    #==========================================================================
    # Summary
    #==========================================================================
    log_info "=========================================="
    log_info "Download Summary"
    log_info "=========================================="
    echo ""

    log_info "Downloaded files:"
    echo "  cmake/CPM.cmake"
    echo "  OpenUSD/src/OpenUSD-${USD_VERSION}.tar.gz"
    echo ""

    log_info "Note: Other dependencies (Vulkan, GLM, ImGui, GLFW, spdlog)"
    log_info "will be downloaded automatically by CPM.cmake during cmake."
    echo ""

    log_success "Setup complete!"
    echo ""
    log_info "Next steps:"
    log_info "  1. Build OpenUSD (see README.md 'Build USD' section)"
    log_info "  2. Run: cmake -B build -DCMAKE_BUILD_TYPE=Release"
    log_info "  3. Run: cd build && make -j\$(nproc)"
}

# Run main
main "$@"
