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
 
RUN wget --no-check-certificate https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -P /opt
RUN chmod +x /opt/linuxdeploy-x86_64.AppImage
