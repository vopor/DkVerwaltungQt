# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.163.1/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
# ARG VARIANT="buster"
ARG VARIANT="bionic"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

RUN apt-get update
RUN apt-get install software-properties-common -y
RUN apt install mesa-common-dev -y
RUN apt install libglu1-mesa-dev -y
RUN add-apt-repository ppa:beineri/opt-qt-5.15.2-bionic
RUN apt-get update
RUN apt-get install qt515base -y
# RUN apt-get install qt515tools -y
# RUN apt-get install qt515multimedia -y
# RUN apt-get install qt515xmlpatterns -y
RUN apt-get install qt515webengine -y
# RUN apt-get install qt515svg -y
# RUN apt-get install qt515websockets -y
 
RUN wget --no-check-certificate https://github.com/probonopd/linuxdeployqt/releases/download/7/linuxdeployqt-7-x86_64.AppImage -P /opt/linuxdeployqt-7
RUN chmod a+x  /opt/linuxdeployqt-7/linuxdeployqt-7-x86_64.AppImage
WORKDIR /opt/linuxdeployqt-7
RUN /opt/linuxdeployqt-7/linuxdeployqt-7-x86_64.AppImage --appimage-extract

ENV QTDIR=/opt/qt515
ENV QT_PLUGIN_PATH="$QTDIR/plugins"
ENV LD_LIBRARY_PATH="$QTDIR/lib:${LD_LIBRARY_PATH}"
ENV PATH="$QTDIR/bin:${PATH}"
ENV PATH="/opt/linuxdeployqt-7/squashfs-root/usr/bin:${PATH}"

# WORKDIR /opt/appimagetool-12
# RUN wget -c "https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage" -P /opt/appimagetool-12
# RUN chmod a+x  /opt/appimagetool-12/appimagetool-x86_64.AppImage
# RUN /opt/appimagetool-12/appimagetool-x86_64.AppImage  --appimage-extract

# X11
RUN apt-get update
RUN apt-get install x11-apps -y 

# Qt
RUN apt-get install libglu1-mesa -y
RUN apt-get install libharfbuzz0b -y
RUN apt-get install libasound2 -y

RUN mkdir -p $HOME/Documents/DkVerwaltungQt
ENV XDG_RUNTIME_DIR $HOME/Documents/DkVerwaltungQt

# QtWebEngine
ENV QT_OPENGL software
ENV QT_QUICK_BACKEND software
ENV QTWEBENGINE_DISABLE_SANDBOX 1

ENV DISPLAY docker.for.mac.host.internal:0
