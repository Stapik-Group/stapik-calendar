#!/usr/bin/env bash
set -euo pipefail

APP_NAME="stapikcalendar"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "${SCRIPT_DIR}")"
BUILD_DIR="${PROJECT_ROOT}/cmake-build-release"

INSTALL_DIR="$HOME/.local/share/${APP_NAME}"
BIN_DIR="$HOME/.local/bin"
DESKTOP_DIR="$HOME/.local/share/applications"

echo "Installing ${APP_NAME}..."

mkdir -p "${INSTALL_DIR}/bin"
mkdir -p "${BIN_DIR}"
mkdir -p "${DESKTOP_DIR}"

cp "${BUILD_DIR}/${APP_NAME}" "${INSTALL_DIR}/bin/"
cp -r "${BUILD_DIR}/resources" "${INSTALL_DIR}/"
cp "${SCRIPT_DIR}/icon.png" "${INSTALL_DIR}/icon.png"

chmod +x "${INSTALL_DIR}/bin/${APP_NAME}"

ln -sf "${INSTALL_DIR}/bin/${APP_NAME}" "${BIN_DIR}/${APP_NAME}"

sed "s|%INSTALL_DIR%|${INSTALL_DIR}|g" "${SCRIPT_DIR}/stapikcalendar.desktop.template" \
    > "${DESKTOP_DIR}/stapikcalendar.desktop"

update-desktop-database "${DESKTOP_DIR}" 2>/dev/null || true

echo "Installed. Run via application menu or with command: ${APP_NAME}"