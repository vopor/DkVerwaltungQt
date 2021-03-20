
FROM ghcr.io/linuxserver/baseimage-guacgui

#
# appdir
# docker build -t dkverwaltungqt-dev -f Dockerfile .
#
# docker run   -p 8080:8080 -p 3389:3389 -v $HOME/Documents/DkVerwaltungQt/guac-daten:/config -d --restart unless-stopped dkverwaltungqt
#


ENV APPNAME="DkVerwaltungQt"

ENV PUID=1000
ENV PGID=1000
ENV TZ=Europe/London
# abc #optional
# ENV GUAC_USER=
# 900150983cd24fb0d6963f7d28e17f72 #optional
# ENV GUAC_PASS=
# optional
ENV UMASK_SET=022
# optional
# ENV CLI_ARGS=

# VOLUMES /config

ENV DISPLAY=:1

EXPOSE 8080:8080/tcp
EXPOSE 8081:8081/tcp

# Azure
# EXPOSE 80/tcp

# ENV APPNAME="DkVerwaltungQt" UMASK_SET="022"

RUN \
 echo "**** install runtime packages ****" && \
 apt-get update && \
 apt-get install -y --no-install-recommends \
	dbus \
	fcitx-rime \
	fonts-wqy-microhei \
	jq \
	libqpdf21 \
	libxkbcommon-x11-0 \
	libxcb-icccm4 \
	libxcb-image0 \
	libxcb-keysyms1 \
	libxcb-randr0 \
	libxcb-render-util0 \
	libxcb-xinerama0 \
	python3 \
	python3-xdg \
	ttf-wqy-zenhei \
	wget \
	xz-utils

RUN dbus-uuidgen > /etc/machine-id

RUN mkdir -p /opt/DkVerwaltungQt/

COPY . /opt/DkVerwaltungQt/

# RUN echo "**** cleanup ****" && \
# apt-get clean && \
#  rm -rf \
# 	/tmp/* \
# 	/var/lib/apt/lists/* \
# 	/var/tmp/*

# add local files
# COPY root/ /
