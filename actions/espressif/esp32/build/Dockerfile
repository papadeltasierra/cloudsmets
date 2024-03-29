# Dockerfile definition for the Ubuntu based EspressIf ESP32-IDF
# References:
# - https://docs.github.com/en/actions/creating-actions/creating-a-docker-container-action:
# - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html
#
# Note that the file locations follow those in the instructions with the exception
# that an explicit _home directory is created in the working directory.
#
FROM ubuntu:20.04

# This is required to ensure that tzdata installs based on the defaulted timezone of UTC.
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# This allows us to pass a value between RUN steps.
ARG ESPTEMP=""

# Currently don't know how to pass this in :-(.
ENV RELEASE=v5.1
ENV HASH=2d1fe74b50f8e351ebf307490e71e14a3753463b4bb060e26567c0849a586a10

ENV FILEPREFIX=esp-idf-release
ENV URLPREFIX=https://github.com/espressif/esp-idf/archive/refs/heads/release

ENV _HOME=home

# !!PDS: Move later when happy.
# GitHub runs command in a Docker container via the entrypoint; note that the
# path to the entry point is relative to where the 'action.yml' file resides.
COPY entrypoint.sh /entrypoint.sh

# Also adding:
# - curl; so we can download the ZIPfile.
# - unzip; so we can unzip it!
# Have to pipe to /dev/null because apt-get will not go '-quiet'!
RUN \
    echo "Installing toolchain prerequisites..." \
    && apt --yes update > /dev/null 2>&1 \
    && apt --yes --option Dpkg::log=/dev/null install \
        wget flex bison gperf python3 python3-pip \
        python3-venv cmake ninja-build ccache \
        libffi-dev libssl-dev dfu-util libusb-1.0-0 \
        curl unzip > /dev/null 2>&1

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
    && mkdir --parents "${_HOME}/esp" \
    && unzip  -q -d "${_HOME}/esp" "${ESPTEMP}/${FILEPREFIX}-${RELEASE}.zip" \
    && echo "Deleting the temporary ZIPfile and directory..." \
    && rm --force --recursive "${ESPTEMP}"

RUN \
    echo "Installing the EspressIF IDF tools..." \
    && PUSHD=$PWD \
    && ls -lsap ${_HOME}/esp \
    && cd "${_HOME}/esp/esp-idf" \
    && ./install.sh esp32 \
    && cd "${PUSHD}" \
    && echo "Done."


ENTRYPOINT ["/entrypoint.sh"]
