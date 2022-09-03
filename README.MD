# SimpleHttpServer

Linux Epoll 多线程简单静态HttpServer

## 描述
一个简单的静态HttpSever，可以处理GET和POST请求

## 技术
- 使用Epoll多路复用IO
- 复用IP以及端口
- 事件循环处理触发事件
- 根据机器CPU核数指定线程池大小
- c14 智能指针, bind等特性
- 多重封装 事件封装
- 连接事件LT 处理事件ET
- one loop one thread 一个线程一个事件循环
- mmap 零拷贝发送文件

## 性能
- CPU: Intel i5-7500 @ 3.4GHz Core 4
- Memory 8G
- webbench 10000 并发连接 10s  
![图片](https://raw.githubusercontent.com/cruegull/SimpleHttpServer/main/doc/webbench.png)
- QPS: 2911

## 使用
- cd bulid
- cmake ..
- make
- ./server -i [ip] -p [port]

## 后续改进
- 可增加日志功能
- 可增加配置功能
