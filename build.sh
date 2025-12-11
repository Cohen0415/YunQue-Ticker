#/bin/sh

TOPDIR=$(pwd)
CDIR=${TOPDIR}/dev-service

PLATFORM=""
TOOLCHAIN_TOPDIR=${TOPDIR}/toolchain

log_info() 
{
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() 
{
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() 
{
    echo -e "${RED}[ERROR]${NC} $1"
}

prepare_toolchain()
{
    if [ "${PLATFORM}" != "linux" ]; then
        if [ ! -d ${TOOLCHAIN_TOPDIR}/${PLATFORM} ]; then
            log_error "Toolchain directory ${TOOLCHAIN_TOPDIR}/${PLATFORM} not found!"
            exit 1
        fi
    fi

    if [ "${PLATFORM}" = "t113" ]; then
        export ARCH=arm
        export CROSS_COMPILE=arm-linux-gnueabi-
        export PATH=$PATH:${TOPDIR}/toolchain/t113/bin
    elif [ "${PLATFORM}" = "t527" ]; then
        export ARCH=aarch64
        export CROSS_COMPILE=aarch64-none-linux-gnu-
        export PATH=$PATH:${TOPDIR}/toolchain/t527/bin
    elif [ "${PLATFORM}" = "linux" ]; then
        export ARCH=x86_64
        export CROSS_COMPILE=""
    else
        log_error "Unsupported platform: ${PLATFORM}"
        exit 1
    fi
}

build()
{
    echo "Build C Code for ${PLATFORM}..."

    if [ ! -d ${CDIR} ]; then
        log_error "Source directory ${CDIR} not found!"
        exit 1
    fi

    cd ${CDIR}
    make clean
    make 
}

build_client()
{
    echo "Build Client Test for ${PLATFORM}..."

    if [ ! -d ${CDIR}/test ]; then
        log_error "Source directory ${CDIR}/test not found!"
        exit 1
    fi

    cd ${CDIR}/test
    ${CROSS_COMPILE}gcc -o client-${PLATFORM} rpc_client.c

    if [ -f client-${PLATFORM} ]; then
        log_info "Client Test built successfully: client-${PLATFORM}"
        mv client-${PLATFORM} ${CDIR}
    else
        log_error "Failed to build Client Test."
        exit 1
    fi
}

if [ "$1" = "-t113" ]; then
    PLATFORM="t113"
elif [ "$1" = "-t527" ]; then
    PLATFORM="t527"
elif [ "$1" = "-linux" ]; then
    PLATFORM="linux"
elif [ "$1" = "-clean" ]; then
    echo "Cleaning build files..."
    cd ${CDIR}
    make clean
    echo "Cleaning client test..."
    rm -f ${CDIR}/client-*
    exit 0
else
    log_warn "No valid build target specified. Please check."
    log_info "Usage: ./build.sh <target>"
    log_info "Available platform:"
    log_info "  -t113        Build T113 platform"
    log_info "  -t527        Build T527 platform"
    log_info "  -linux       Build Linux platform"
    log_info "  -clean       Clean build"
fi

prepare_toolchain
build
build_client