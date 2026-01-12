#/bin/sh

# 顶层目录路径
TOPDIR=$(pwd)

# 编译输出目录
OUTDIR=${TOPDIR}/out

# 工具链相关路径
PLATFORM=""
TOOLCHAIN_TOPDIR=${TOPDIR}/toolchain

# 服务器代码路径
SERVICE_DIR=${TOPDIR}/dev-service

# 用于测试服务器的客户端代码路径
CLIENT_TEST_DIR=${SERVICE_DIR}/test

# qt客户端代码
QT_DIR=${TOPDIR}/app
QMAKE_PATH=""
QT_PRJ_DIR=${QT_DIR}/Ticker

# 默认值
PLATFORM=""
BUILD_SERVICE=0
BUILD_CLIENT=0
BUILD_QT=0
DO_CLEAN=0

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
        QMAKE_PATH="/home/cohen/platform/t113/xiaozhi/tina5.0-v1.2/app/Qt-5.15.9/Qt-install-5.15.9/bin/qmake"
    elif [ "${PLATFORM}" = "t527" ]; then
        export ARCH=aarch64
        export CROSS_COMPILE=aarch64-none-linux-gnu-
        export PATH=$PATH:${TOPDIR}/toolchain/t527/bin
        QMAKE_PATH=""
    elif [ "${PLATFORM}" = "linux" ]; then
        export ARCH=x86_64
        export CROSS_COMPILE=""
        QMAKE_PATH="/opt/qt/qt-creator-5.14.2/5.14.2/gcc_64/bin/qmake"
    else
        log_error "Unsupported platform: ${PLATFORM}"
        exit 1
    fi
}

build_qt()
{
    log_info " =========================== Build QT Client for ${PLATFORM}... ==========================="

    # 检查 qmake 是否存在
    if [ ! -f ${QMAKE_PATH} ]; then
        log_error "Source directory ${QMAKE_PATH} not found!"
        exit 1
    fi

    # 检查项目工程是否存在
    if [ ! -f ${QT_PRJ_DIR}/Ticker.pro ]; then
        log_error "QT project file Ticker.pro not found!"
        exit 1
    fi

    # 检查编译输出目录是否存在
    if [ ! -d ${QT_DIR}/build-${PLATFORM} ]; then
        mkdir ${QT_DIR}/build-${PLATFORM}
    fi
    rm -rf ${QT_DIR}/build-${PLATFORM}/*

    cd ${QT_DIR}/build-${PLATFORM}
    ${QMAKE_PATH} ${QT_PRJ_DIR}/Ticker.pro
    make -j12

    # 复制编译结果到输出目录，并统一重命名为 Ticker-app
    cp -r ${QT_DIR}/build-${PLATFORM}/Ticker ${OUTDIR}/Ticker-app

    log_info " =========================== QT Client built successfully for ${PLATFORM} ==========================="
}

build_service()
{
    log_info " =========================== Build Service for ${PLATFORM}... ==========================="

    if [ ! -d ${SERVICE_DIR} ]; then
        log_error "Source directory ${SERVICE_DIR} not found!"
        exit 1
    fi

    cd ${SERVICE_DIR}
    make clean
    make -j12

    # 复制编译结果到输出目录，并统一重命名为 Ticker-service
    cp ${SERVICE_DIR}/dev-service ${OUTDIR}/Ticker-service

    log_info " =========================== Service built successfully for ${PLATFORM} ==========================="
}

build_client()
{
    log_info " =========================== Build Client Test for ${PLATFORM}... ==========================="

    if [ ! -d ${SERVICE_DIR}/test ]; then
        log_error "Source directory ${SERVICE_DIR}/test not found!"
        exit 1
    fi

    cd ${SERVICE_DIR}/test
    ${CROSS_COMPILE}gcc -o client-${PLATFORM} rpc_client.c

    if [ -f client-${PLATFORM} ]; then
        mv client-${PLATFORM} ${SERVICE_DIR}
    else
        log_error "Failed to build Client Test."
        exit 1
    fi

    # 复制编译结果到输出目录，并统一重命名为 Ticker-client-test
    cp ${SERVICE_DIR}/client-${PLATFORM} ${OUTDIR}/Ticker-client-test

    log_info " =========================== Client Test built successfully for ${PLATFORM} ==========================="
}

show_help()
{
    echo "Usage: ./build.sh [platform] [options]"
    echo "  Platforms:"
    echo "    -t113          Build for t113 platform"
    echo "    -t527          Build for t527 platform"
    echo "    -linux         Build for Linux platform"
    echo "  Options:"
    echo "    -service       Build only the service"
    echo "    -client        Build only the client test"
    echo "    -qt            Build only the QT client"
    echo "    -all           Build service, client test, and QT client (default)"
    echo "    -clean         Clean build outputs for the specified platform"
    echo "    -h, --help     Show this help message"
}

while [ $# -gt 0 ]; do
    case "$1" in
        -t113)
            PLATFORM="t113"
            ;;
        -t527)
            PLATFORM="t527"
            ;;
        -linux)
            PLATFORM="linux"
            ;;
        -service)
            BUILD_SERVICE=1
            ;;
        -client)
            BUILD_CLIENT=1
            ;;
        -qt)
            BUILD_QT=1
            ;;
        -all)
            BUILD_SERVICE=1
            BUILD_CLIENT=1
            BUILD_QT=1
            ;;
        -clean)
            DO_CLEAN=1
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done

if [ -z "${PLATFORM}" ]; then
    log_error "No platform specified. Use -t113 | -t527 | -linux"
    exit 1
fi

if [ ${BUILD_SERVICE} -eq 0 ] && \
   [ ${BUILD_CLIENT} -eq 0 ] && \
   [ ${BUILD_QT} -eq 0 ]; then
    BUILD_SERVICE=1
    BUILD_CLIENT=1
    BUILD_QT=1
fi

# 目前 t527 不支持 QT 编译
if [ "${PLATFORM}" == "t527" ] && [ ${BUILD_QT} -eq 1 ]; then
    log_warn "QT build is not supported for t527 platform. Skipping QT build."
    BUILD_QT=0
fi

if [ ${DO_CLEAN} -eq 1 ]; then
    log_info "Cleaning build outputs for platform ${PLATFORM}"

    cd ${SERVICE_DIR} && make clean

    rm -f ${SERVICE_DIR}/client-${PLATFORM}

    rm -rf ${QT_DIR}/build-${PLATFORM}

    exit 0
fi

prepare_toolchain

# 检查并创建输出目录
if [ ! -d ${OUTDIR} ]; then
    mkdir ${OUTDIR}
fi

[ ${BUILD_SERVICE} -eq 1 ] && build_service
[ ${BUILD_CLIENT}  -eq 1 ] && build_client
[ ${BUILD_QT}      -eq 1 ] && build_qt

log_info "Build finished successfully for ${PLATFORM}"