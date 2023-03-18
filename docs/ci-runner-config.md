# CI-Runner 配置

## 构建 CI 所需的镜像

本仓库中的 Dockerfile 定义了 CI 的环境，使用：

```console
$ docker build .
```

即可构建 docker 镜像。将最新的镜像标记为 `latest`，随后发布、推送到 Docker Hub 或私有的 registry 中。

## 在服务器上启动 CI-Runner

将本仓库中的 `./scripts/launch-ci-runner` 文件拷贝到服务器：

```console
$ scp ./scripts/launch-ci-runner ci-server:
```

然后登录服务器，应该能在家目录下找到 `launch-ci-runner` 文件。随后在环境变量中引入：

- `GITLAB_URL`：本仓库所在 Gitlab 的 URL
- `REGISTRATION_TOKEN`：本仓库注册 CI-Runner 时的 token

最后运行脚本：

```console
$ ~/launch-ci-runner 001
```

即可为本仓库的 CI 创建 id 为 `001` 的 CI-Runner。
