# 构建 CI 所需的镜像

本仓库中的 Dockerfile 定义了 CI 的环境，使用：

```console
$ docker build .
```

即可构建 docker 镜像。将最新的镜像标记为 `latest`，随后发布、推送到 Docker Hub 或私有的 registry 中。本仓库使用的 Github Actions 的配置文件中，使用了 [coekjan/jrinx-devel:latest](https://hub.docker.com/r/coekjan/jrinx-devel) 镜像。
