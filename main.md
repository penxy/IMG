# 4.系统实现
## 4.1 系统架构
> 系统架构图
![0f3690e29aeb0ee906e57c3637e51ea2.jpg](https://www.helloimg.com/i/2025/03/12/67d0e1596437a.jpg)
> 下面是系统架构的详细说明
### 4.1.1 云盘挂载至本地
- 在本地创建一个文件夹，用于挂载云盘, 在该文件夹的读写, 会自动同步到云盘
- 所用技术: 
1. Alist
> Alist, 是一个开源的网盘挂载工具, 可以将网盘挂载到本地, 支持多种网盘, 如阿里云盘, 百度网盘等
2. mount.davfs
> mount.davfs 是一个用于挂载WebDAV协议的文件系统工具, 可以将WebDAV协议的文件系统挂载到本地
3. webdav
> webdav是一种基于HTTP协议的文件共享协议, 可以用于在网络上共享文件,
4. samba
> samba 是一个用于在Linux和Windows之间共享文件和打印机的软件, 可以用于在网络上共享文件
5. fuse
> fuse 是用户空间文件系统, 可以将用户空间的应用程序挂载到文件系统上, 可以用于在本地挂载云盘
6. cifs
> cifs 是一种功能强大且广泛使用的文件共享协议，可以用于在网络上共享文件和文件夹。

- 实现步骤:
1. 下载alist, 根据不同的操作系统, 下载对应的alist版本, [下载地址](https://github.com/AlistGo/alist/releases)
2. 解压安装后, ```Linux```和```Window```都是执行```./alist server```启动alist
3. 浏览器访问当前服务器的ip地址+端口号(默认为5244), 如http://172.16.10.247:5244, 进入alist的web界面
4. 在alist的web界面中, 点击添加存储, 选择需要挂载的网盘, 如阿里云盘, 百度网盘等, 以阿里云盘为例, 如下图所示
![Aliyun_device.png](https://www.helloimg.com/i/2025/03/12/67d100a5b6828.png)
填写对应的令牌, [阿里网盘token获取链接](https://easy-token.cooluc.com/)
![Alist_Aliyun_token.png](https://www.helloimg.com/i/2025/03/12/67d0e18214b7e.png)
点击保存之后, 即可完成挂载, 状态如下:
![Alist_Mount_List.png](https://www.helloimg.com/i/2025/03/12/67d1010799ed1.png)
此时即可使用mount.davfs将alist挂载到本地
Linux执行以下命令:
```bash
sudo mount -t davfs http://172.16.10.247:5244/dav/aliyun /mnt/aliyun 
```
Window则启动程序```RaiDrive```
![Window_Mount_Webdav.png](https://www.helloimg.com/i/2025/03/12/67d1022b59184.png)
此时即可在本地访问阿里云盘中的文件, 包括上传和下载等操作
5. 挂载情况
![mount_status.png](https://www.helloimg.com/i/2025/03/12/67d0e1cc93d21.png)
6. 一些问题
Alist需要开启webdav权限, 否则无法挂载
![chmod.png](https://www.helloimg.com/i/2025/03/12/67d0e1ccc70ad.png)

7. 上传下载速度
> 以阿里云盘为例
- 上传速度
![aliy_upload_speed.png](https://www.helloimg.com/i/2025/03/12/67d0e1cc68d21.png)
- 下载速度
![aliy_download_speed.png](https://www.helloimg.com/i/2025/03/12/67d0e18257ca7.png)
> 上传速度为6M/s左右, 具体和网络情况有关
> 下载速度受限于阿里云盘官方的下载速度, 仅有0.6M/s

8. 资源占用
![Alist_upload.png](https://www.helloimg.com/i/2025/03/12/67d0e12fd7cfa.png)
- 上传/下载文件时, CPU占用大概在40%-50%左右, 不过, 如果是下载时被限速, 比如阿里云盘, 则占用在10%以下
- 内存占用在60-180MB之间


### 4.1.2 挂载Webdav到本地
- 可以用[caddy](https://github.com/caddyserver/caddy), nginx搭建webdav服务, 或者直接使用开源项目[webdav服务](https://github.com/hacdias/webdav)
- 这里以caddy为例, caddy配置如下:
```text
{
    order webdav before file_server
}
:8080 {
    root * /dav
    basicauth {
        pxy $2a$14$h2ygVzUbX4.pocuM4y2HL.cC4RgFcoUuCGxr53nXXserxJHBS15E2
    }
    webdav
}
```
- 随后即可挂载到本地, Linux:
```bash
sudo mount -t davfs http://172.16.10.247:8080/ /mnt/webdav
```
- Windows还是用```RaiDrive```
- 效果与Alist挂载到本地一样
- 不过存在性能上的差异, 毕竟Alist需要中转, 而webdav服务直接访问
![diff_webdav_alist.png](https://www.helloimg.com/i/2025/03/12/67d111c33d85f.png)
- 但是其速度几乎一致, 只是CPU占用存在差异, 因此直接使用webdav服务即可

### 4.1.3 挂载Samba到本地
- 挂载方式:

<br>
Linux:<br> 

```bash
sudo mount -t cifs //172.16.0.19/home3_pengxy /mnt/samba -o username=pxy,password=123456
```
Windows:
> 可以直接访问地址栏,输入```\\172.16.0.19\home3_pengxy```, 然后输入用户名和密码即可
- 效果与Alist挂载到本地一样, 但是比Alist挂载到本地快很多, 因为不需要中转, 而且Alist挂载后的文件系统为```fuse```, 
而samba的为```cifs```, ```cifs```的性能比```fuse```好很多
![Alist_filesystem.png](https://www.helloimg.com/i/2025/03/12/67d0e183261c2.png)
- Alist挂载的文件系统
![diff_samba_mount.png](https://www.helloimg.com/i/2025/03/12/67d0e1cc9958c.png)
- cifs挂载的文件系统
![cifs_samba.png](https://www.helloimg.com/i/2025/03/12/67d0e1cc9f6a8.png)






