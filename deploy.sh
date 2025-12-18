#!/bin/sh

################################
BOARD_USER=root
BOARD_IP=192.168.2.100
BOARD_BASE_DIR=/app

SERVICE_REMOTE_DIR=${BOARD_BASE_DIR}/
CLIENT_REMOTE_DIR=${BOARD_BASE_DIR}/
QT_REMOTE_DIR=/opt/Ticker/bin/
################################

TOPDIR=$(pwd)
SERVICE_DIR=${TOPDIR}/dev-service
QT_DIR=${TOPDIR}/app

PLATFORM=""
DEPLOY_SERVICE=0
DEPLOY_CLIENT=0
DEPLOY_QT=0

log_info() { echo "[INFO] $1"; }
log_error() { echo "[ERROR] $1"; exit 1; }

while [ $# -gt 0 ]; do
    case "$1" in
        -t113)  PLATFORM="t113" ;;
        -t527)  PLATFORM="t527" ;;
        -linux) PLATFORM="linux" ;;
        -service) DEPLOY_SERVICE=1 ;;
        -client)  DEPLOY_CLIENT=1 ;;
        -qt)      DEPLOY_QT=1 ;;
        -all)
            DEPLOY_SERVICE=1
            DEPLOY_CLIENT=1
            DEPLOY_QT=1
            ;;
        *)
            log_error "Unknown option: $1"
            ;;
    esac
    shift
done

[ -z "${PLATFORM}" ] && log_error "No platform specified"

if [ ${DEPLOY_SERVICE} -eq 0 ] && \
   [ ${DEPLOY_CLIENT}  -eq 0 ] && \
   [ ${DEPLOY_QT}      -eq 0 ]; then
    DEPLOY_SERVICE=1
    DEPLOY_CLIENT=1
    DEPLOY_QT=1
fi


# 检查板卡连通性
log_info "Checking board connectivity..."
ping -c 1 ${BOARD_IP} >/dev/null 2>&1 || \
    log_error "Board ${BOARD_IP} unreachable"


# 部署 Service
if [ ${DEPLOY_SERVICE} -eq 1 ]; then
    SERVICE_BIN=${SERVICE_DIR}/dev-service 

    [ ! -f ${SERVICE_BIN} ] && \
        log_error "Service binary not found: ${SERVICE_BIN}"

    log_info "Deploying service..."
    scp ${SERVICE_BIN} \
        ${BOARD_USER}@${BOARD_IP}:${SERVICE_REMOTE_DIR}/
fi


# 部署 Client
if [ ${DEPLOY_CLIENT} -eq 1 ]; then
    CLIENT_BIN=${SERVICE_DIR}/client-${PLATFORM}

    [ ! -f ${CLIENT_BIN} ] && \
        log_error "Client binary not found: ${CLIENT_BIN}"

    log_info "Deploying client test..."
    scp ${CLIENT_BIN} \
        ${BOARD_USER}@${BOARD_IP}:${CLIENT_REMOTE_DIR}/
fi


# 部署 QT
if [ ${DEPLOY_QT} -eq 1 ]; then
    QT_BIN=${QT_DIR}/build-${PLATFORM}/Ticker

    [ ! -f ${QT_BIN} ] && \
        log_error "QT binary not found: ${QT_BIN}"

    log_info "Deploying QT app..."
    scp ${QT_BIN} \
        ${BOARD_USER}@${BOARD_IP}:${QT_REMOTE_DIR}/
fi

log_info "Deploy finished successfully for ${PLATFORM}"