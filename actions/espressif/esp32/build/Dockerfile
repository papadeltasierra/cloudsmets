# Dockerfile definition for the Ubuntu based EspressIf ESP32-IDF
# References:
# - https://docs.github.com/en/actions/creating-actions/creating-a-docker-container-action:
# - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html
FROM ubuntu:20.04

# This is required to ensure that tzdata installs based on the defaulted timezone of UTC.
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# This allows us to pass a value between RUN steps.
ARG ESPTEMP=""

# Currently don't know how to pass this in :-(.
ENV RELEASE=v5.1
ENV HASH=2d1fe74b50f8e351ebf307490e71e14a3753463b4bb060e26567c0849a586a10

ENV DIRNAME=/opt/espressif-idf
ENV FILEPREFIX=esp-idf-release
ENV URLPREFIX=https://github.com/espressif/esp-idf/archive/refs/heads/release

# Also adding:
# - curl; so we can download the ZIPfile.
# - unzip; so we can unzip it!
RUN \
    echo "Installing toolchain prerequisites..." \
    && apt --yes update \
    && apt --yes install \
        wget flex bison gperf python3 python3-pip \
        python3-venv cmake ninja-build ccache \
        libffi-dev libssl-dev dfu-util libusb-1.0-0 \
        curl unzip

# Note the need for the semi-colons in the 'if' test below.
# Also note that we cannot pass ESPTEMP between steps so we have to merge them
# here :-(.
RUN \
    echo "Downloading the EspressIF IDE..." \
    && ESPTEMP=$(mktemp --directory espressif.XXXXXX) \
    && echo "Temporary download directory: ${ESPTEMP}." \
    && PUSHD=${PWD} \
    && cd "${ESPTEMP}" \
    && curl \
        --location \
        --output "${FILEPREFIX}-${RELEASE}.zip" \
        "${URLPREFIX}/${RELEASE}.zip" \
    && echo "Debug 1" \
    && cd "${PUSHD}" \
    && echo "Debug 2" \
    && REMOTE_HASH=$(sha256sum "${ESPTEMP}/${FILEPREFIX}-${RELEASE}.zip" | cut -d " " -f 1 ) \
    && echo "Debug 3" \
    && echo "A:${REMOTE_HASH}A:" \
    && echo "B:${HASH}:B" \
    && if [ "${REMOTE_HASH}" != "${HASH}" ]; \
       then \
           echo "EspressIF ZIPfile hash, '${REMOTE_HASH}',' has changed!"; \
           exit 1; \
       fi \
    && echo "Extracting the EspressIF IDE..." \
    && mkdir --parents "${DIRNAME}" \
    && unzip  -q -d "${DIRNAME}" "${ESPTEMP}/${FILEPREFIX}-${RELEASE}.zip" \
    && echo "Deleting the temporary ZIPfile and directory..." \
    && rm --force --recursive "${ESPTEMP}"

RUN \
    echo "Installing the EspressIF IDF tools..." \
    && PUSHD=$PWD \
    && cd "${DIRNAME}/${FILEPREFIX}-${RELEASE}" \
    && ./install.sh esp32 \
    && cd "${PUSHD}" \
    && echo "Done."
