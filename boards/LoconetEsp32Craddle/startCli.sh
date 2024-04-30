#!/bin/sh
LOCAL_BASE=$(git rev-parse --show-toplevel)
SUBDIR=$(git rev-parse --show-prefix)
KIBOT_DIR=$LOCAL_BASE/kibot
PASS_FILE=/etc/passwd
export USER_ID=$(id -u)
export GROUP_ID=$(id -g)

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac
echo ${machine}

if [ ! $machine == "Mac" ]
then
    echo "This Script is for Mac Os"
fi

if [ ! -d $KIBOT_DIR/docker.tmp/config ]
then
    mkdir -p $KIBOT_DIR/docker.tmp/config
fi
if [ ! -d $KIBOT_DIR/docker.tmp/cache ]
then
    mkdir -p $KIBOT_DIR/docker.tmp/cache
fi
if [ ! -d $KIBOT_DIR/docker.tmp/local ]
then
    mkdir -p $KIBOT_DIR/docker.tmp/local
fi

if [ ! -f $KIBOT_DIR/docker.tmp/passwd ]
then
    cp /etc/passwd $KIBOT_DIR/docker.tmp/.
    echo "$USER:x:$USER_ID:$GROUP_ID:$USER:/home/$USER:/usr/sbin/nologin" >> $KIBOT_DIR/docker.tmp/passwd
fi

VERSION=ghcr.io/inti-cmnb/kicad7_auto_full:latest
docker run --rm -it  \
    -v $LOCAL_BASE:/home/$USER/workdir \
    -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=host.docker.internal:0 \
    --user $USER_ID:$GROUP_ID \
    --env NO_AT_BRIDGE=1 \
    --env USER=$USER \
    --env HOME=/home/$USER/ \
    --env KICAD_CONFIG_HOME=/home/$USER/.config/kicad \
    --env-file $KIBOT_DIR/docker.env\
    --workdir="/home/$USER/" \
    --volume="$KIBOT_DIR/docker.tmp/passwd:/etc/passwd:ro" \
    --volume="$KIBOT_DIR/docker.tmp/config:/home/$USER/.config:rw" \
    --volume="$KIBOT_DIR/docker.tmp/cache:/home/$USER/.cache:rw" \
    --volume="$KIBOT_DIR/docker.tmp/local:/home/$USER/.local:rw" \
    --rm \
    --hostname kibot \
    $VERSION  /bin/bash -c "cd workdir/$SUBDIR; bash"



    

#    --volume="$PASS_FILE:/etc/passwd:ro" \
#    --volume="/etc/shadow:/etc/shadow:ro" \
#    --volume="/etc/group:/etc/group:ro" \

#    --rm \
#    --hostname kibot \
#    /bin/bash -c "cd workdir/$SUBDIR; bash ./runKibot.sh  $@"