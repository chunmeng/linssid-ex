FROM debian:buster

ENV DEBIAN_FRONTEND noninteractive
ENV TERM=xterm

# Base
RUN apt-get update -y && apt-get install -y \
	build-essential \
	libboost-regex-dev \
	qt5-default \
	qt5-qmake \
	libqt5svg5-dev \
	libqt5opengl5-dev \
	libqwt-qt5-6 \
	libqwt-headers \
	&& apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /src
CMD /bin/bash
