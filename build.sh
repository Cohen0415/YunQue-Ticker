#/bin/sh

TOPDIR=$(pwd)
CDIR=${TOPDIR}/dev-service

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

build_c()
{
    echo "Build C Code ..."

    if [ ! -d "${TOPDIR}/toolchain/host" ]; then
        log_error "Toolchain not found! Please set up the toolchain first."
        exit 1
    fi

    export ARCH=arm
    export CROSS_COMPILE=arm-linux-gnueabi-
    export PATH=$PATH:${TOPDIR}/toolchain/host/bin

    if [ ! -d ${CDIR} ]; then
        log_error "Source directory ${CDIR} not found!"
        exit 1
    fi

    cd ${CDIR}
    make clean
    make 
}

if [ "$1" = "-c" ]; then
    build_c
else
    log_warn "No valid build target specified. Use 'c' to build C code."
    log_info "Usage: ./build.sh <target>"
    log_info "Available targets:"
    log_info "  -c    Build C code"
fi