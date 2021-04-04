
FROM ghcr.io/linuxserver/baseimage-guacgui

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

#
# cd $HOME/Documents/GitHub/DkVerwaltungQt/build-DkVerwaltungQt-Desktop_Qt_5_15_2_GCC_64bit-Debug/appdir 
#
# [cp -r $HOME/Documents/GitHub/DkVerwaltungQt/guac-gui/root ./]
#
# docker build -t dkverwaltungqt(az) -f $HOME/Documents/GitHub/DkVerwaltungQt/guac-gui/Dockerfile .
#
# docker run --name dkverwaltungqt(az)_container  -p 8080:8080 -p 3389:3389 -v $HOME/Documents/DkVerwaltungQt/guac-daten:/config -d --restart unless-stopped dkverwaltungqt
#
# docker run -d \                                                                                                                                                      
#   --name=dkverwaltungqt(az)_container \
#   -e PUID=1000 \
#   -e PGID=1000 \
#   -e TZ=Europe/London \
#   -e GUAC_USER=abc \
#   -e GUAC_PASS=900150983cd24fb0d6963f7d28e17f72 \
#   -e UMASK_SET=022 \
#   -e CLI_ARGS= \
#   -p 8080:8080 \
#   -p 8081:8081 \
#   -v $HOME/Documents/DkVerwaltungQt/guac-daten:/config \
#   --restart unless-stopped \
#   dkverwaltungqt(az)
#
# deploy to azure
# https://code.visualstudio.com/docs/containers/app-service
# docker tag dkverwaltungqtaz voposcontainerregisry.azurecr.io/dkverwaltungqtaz
# docker push voposcontainerregisry.azurecr.io/dkverwaltungqtaz
# Deploy image to Azure App Service
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

# Azure
# EXPOSE 80:8080/tcp
EXPOSE 8080:8080/tcp
EXPOSE 8081:8081/tcp

# ENV APPNAME="DkVerwaltungQt" UMASK_SET="022"

# RUN echo "**** cleanup ****" && \
# apt-get clean && \
#  rm -rf \
# 	/tmp/* \
# 	/var/lib/apt/lists/* \
# 	/var/tmp/*

# ENV QTWEBENGINE_DISABLE_SANDBOX 1

# add local files
COPY root/ /

RUN mkdir -p /opt/DkVerwaltungQt/
COPY . /opt/DkVerwaltungQt/

# RUN mkdir -p /opt/DkVerwaltungQt/runtime_dir
# ENV XDG_RUNTIME_DIR=/opt/DkVerwaltungQt/runtime_dir

RUN mkdir -p /config/runtime_dir
ENV XDG_RUNTIME_DIR=/config/runtime_dir

RUN ln -s /opt/DkVerwaltungQt/DkVerwaltungQt /usr/bin/DkVerwaltungQt
# RUN chmod ugo+x /usr/bin/DkVerwaltungQt

# WORKDIR /opt/DkVerwaltungQt/
# ENTRYPOINT [ "/opt/DkVerwaltungQt/DkVerwaltungQt" ]

