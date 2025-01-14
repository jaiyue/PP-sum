# 使用 Ubuntu 22.04 作为基础镜像
FROM ubuntu:22.04

# 设置非交互模式，避免某些安装过程中的提示
ENV DEBIAN_FRONTEND=noninteractive

# 更新包列表并安装 gcc 和 g++
RUN apt-get update && \
    apt-get install -y gcc g++ make && \
    apt-get clean

# 设置工作目录
WORKDIR /workspace

# 将当前目录的所有文件复制到容器中的 /workspace 目录
COPY . /workspace

# 默认启动容器时的命令（可选）
CMD ["bash"]
