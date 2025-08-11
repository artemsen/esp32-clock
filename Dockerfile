FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade --yes && apt install --no-install-recommends --yes \
    build-essential \
    ca-certificates \
    cmake \
    libusb-1.0-0 \
    python3 \
    python3-venv

# default user
ENV USER=ubuntu
ENV HOME=/home/ubuntu
USER ubuntu
