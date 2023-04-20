ARG BASE=ubuntu:22.04
FROM ${BASE}

ARG MIRROR=mirrors.tuna.tsinghua.edu.cn
ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i "s/archive.ubuntu.com/${MIRROR}/g" /etc/apt/sources.list; \
    sed -i "s/security.ubuntu.com/${MIRROR}/g" /etc/apt/sources.list; \
    sed -i "s/deb.debian.org/${MIRROR}/g" /etc/apt/sources.list; \
    sed -i "s/deb.debian.org/${MIRROR}/g" /etc/apt/sources.list.d/debian.sources; \
    apt-get update && \
    apt-get -y upgrade && \
    apt-get -y install tzdata locales && \
    { apt-get -y install language-pack-zh-hans; true; }

RUN ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo -e "\nen_US.UTF-8 UTF-8\nzh_CN.UTF-8 UTF-8" > /etc/locale.gen && \
    dpkg-reconfigure -f noninteractive tzdata locales && \
    update-locale LANG=zh_CN.UTF-8 LANGUAGE=zh_CN:zh:en_US:en LC_ALL=zh_CN.UTF-8

ENV LANG=zh_CN.UTF-8 LANGUAGE=zh_CN:zh:en_US:en LC_ALL=zh_CN.UTF-8

# Basic Toolchains
RUN apt-get -y install git \
                       build-essential \
                       gcc-riscv64-linux-gnu \
                       qemu-system-misc

ARG LLVM_VERSION=15
ARG LLVM_MIRROR=https://${MIRROR}/llvm-apt

# Dependencies of llvm.sh
RUN apt-get -y install wget \
                       lsb-release \
                       software-properties-common \
                       gnupg \
                       python3-pip \
                       zstd

# Let llvm.sh only prepare the source, since we just want clang-format.
RUN wget -O /tmp/llvm.sh ${LLVM_MIRROR}/llvm.sh && \
    chmod +x /tmp/llvm.sh && \
    sed -i 's/apt-get install/echo packages:/g' /tmp/llvm.sh && \
    /tmp/llvm.sh ${LLVM_VERSION} all -m ${LLVM_MIRROR} && \
    apt-get -y install clang-format-${LLVM_VERSION} && \
    ln -s clang-format-${LLVM_VERSION} /usr/bin/clang-format && \
    rm /tmp/llvm.sh

# Fetch ec from an Arch repo for better connectivity than GitHub.
ARG EC_REPO=https://${MIRROR}/archlinux/community/os/x86_64/
RUN file=$(wget -qO - ${EC_REPO} | grep -om 1 'editorconfig-checker-[^"<]*' | head -n 1) && \
    wget -O /tmp/"$file" "${EC_REPO}$file" && \
    tar -xvf /tmp/"$file" -C / usr/bin/editorconfig-checker usr/bin/ec && \
    rm /tmp/"$file"

# Extra Toolchains
RUN apt-get -y install gawk \
                       shellcheck \
                       python3 \
                       python3-autopep8 \
                       python3-psutil \
                       python3-yaml
