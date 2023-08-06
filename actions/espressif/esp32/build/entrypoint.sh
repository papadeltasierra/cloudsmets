#!/usr/bin/bash
# Shell script to run Espressif builds in the Ubuntu Docker container.
#
# Ref: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html

# Expect the following variables.
# - `$1`; the nameof the application to be built.

# Call getopt to validate the provided input.
if ! options=$(getopt --options 'p:' --longoptions 'project:' -- "$@")
then
    echo "Invalid arguments."
    echo "usage: -p/--project <project directory>"
    exit 1
fi

PROJECT=""
eval set -- "$options"
while true
do
    case "$1" in
        -p|--project)
            shift
            PROJECT=$1
            ;;
        --)
            shift
            break
            ;;
    esac
    shift
done

echo "Setting ESP-IDF environment variables..."
. /opt/espressif-idf/export.sh

# Ref: https://docs.github.com/en/actions/creating-actions/dockerfile-support-for-github-actions#workdir
# Note that GitHub mounts the (GitHub) working directory path in the GITHUB_WORKSPACE
# so the working directory will be the GITHUB_WORKSPACE.  This "should" mean that
# generated firmware is available to be store to artifacts after the build job
# finishes.

echo "Changing project directory '${PROJECT}'..."
pushd "${PROJECT}" || exit 1

echo "Setting target to be ESP32...."
idf.py set-target esp32

# We configure the application outside of the CI.
# idf.py menuconfig

echo "Building the project..."
idf.py build

# We do not flash the firmware here, rather we will store the generated files
# as artifacts for use later.
# idf.py -p PORT flash

# Finally return to the working directory.
popd || exit 1