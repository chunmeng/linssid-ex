#!/bin/bash

# Extract entrypoint and parameters if any
if ! test -z "$1"; then
    ENTRYPOINT="--entrypoint $1"
fi;
if ! test -z "$2"; then
    PARAMETERS="${@:2}"
fi;

# Handle TTY presence/absence
if test -t 1; then
    TTY="-t"
fi;

PASSWD_FILE=/etc/passwd
GROUP_FILE=/etc/group
if [ "$(uname -s)" == "Darwin" ]; then
PASSWD_FILE=/private${PASSWD_FILE}
GROUP_FILE=/private${GROUP_FILE}
fi

# Build and run the container
docker build docker/ -t linssid-builder
docker run -i $TTY --rm \
    --user `id -u`:`id -g` \
    --volume `pwd`:/src \
    --volume ${PASSWD_FILE}:/etc/passwd \
    --volume ${GROUP_FILE}:/etc/group \
    --volume $HOME:$HOME \
    --network=host \
    $ENTRYPOINT \
    linssid-builder \
    $PARAMETERS
