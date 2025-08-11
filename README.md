# 简介
GcRpc是Linux系统下的一个简易rpc框架，基于etcd实现服务发现、负载均衡等功能。基于protobuf进行数据的序列化与反序列化。框架分为客户端和服务端两部分。

# 服务端
服务端采用Proactor模式。网络IO操作`accept`、`read`、`write`、`close`等由`io_uring`异步完成，具体的服务实现由若干工作线程来完成，主线程负责事件循环，根据不同的事件状态进行异步调用：

![事件循环](md_resource/事件循环.svg "事件循环")

## 细节
### 1. io_uring的异步回调设计
`io_uring`异步操作完成后返回的`io_uring_cqe`对象本身是不含有**异步操作类型**和**异步操作对象**的信息的，
只有当提交`sqe`时，在它的`user_data`里面设置一个64位的数据后，拿到的`cqe`才能通过`cqe->user_data`拿到异步操作的信息。

`user_data`一般而言会选择去设置一个类，类里面包含异步对象操作的`fd`和`event_type`，或者将它们结合起来，组成一个64位的数，比如`fd << 32 | event_type`

在`GcRpc`当中，每个fd对应一个`TcpConnection`，因此框架内一般不会直接对`fd`进行操作，此时这个类里面需要具备`TcpConnection *`和`event_type`。考虑到`event_type`实际上一个字节（事实上`3 bit`）就能完成，而64位系统下，字节对齐一般是8字节，因此这样的一个类会出现几乎8字节的空间浪费。此外，`user_data`需要保证其生命周期覆盖整个异步执行期间，因此类要么用`new`创建（系统调用开销），要么预先去制作一个`user_data pool`，用池化技术提前创建若干`user_data`类，也颇为麻烦。

`GcRpc`的思路是：利用`x86-64`和`arm64`中，用户空间的实际地址一般只低用`48 bit`的特性，将`TcpConnection *`与`event_type`组合起来：`user_data = (uintptr_t)(TcpConnection *) | (event_type << 48 )`

### 2. 动态连接池的实现
在使用`io_uring`的场景下，Tcp连接并不需要由事件循环进行管理，因为可以利用`io_uring`的`user_data`实现异步回调。
为了保证主线程一直都能调用`accept`接受新的连接，EventLoop的每次循环都需要根据当前的连接情况获得一些`TcpConnection`对象，并让它异步执行`accept`。
如果每次都选择去`new`若干个`TcpConnection`对象，那么每次需要进行系统调用，造成性能开销，并且`TcpConnection`的生命周期并不好控制。

`GcRpc`选择创建一个`ConnectionManager`类来管理`TcpConnection`，它来管理何时要向内核申请对象，来管理对象的析构，来回应EventLoop对新的Tcp连接的需求。

### 3. 自定义的etcd API
`etcd-cpp-apiv3`是`etcd`官方的API库，但是在实际使用时，我发现它有一些问题，特别是当使用该库做服务发现。它直接基于`grpc`来与`etcd server`进行通信，因为`grpc`调用时本身就会创建若干工作线程来实现`io`，这意味着如果仍旧使用该库，将浪费大量的线程在`grpc`的io操作上，得不偿失。因此`GcRpc`选择自己去写`etcd API`，并能够基于`io_uring`实现与`TcpConnection`相同的异步操作，从而实现自主可控的`etcd`调用。

# 客户端
客户端服务调用使用`RpcCaller`类，该类维护多个线程，每个线程具有自己的任务队列。

在`RpcCaller`类中，客户端会根据服务名，从etcd中获取服务节点列表，然后根据负载均衡算法，选择一个节点，将请求发送到该节点。这里支持的负载均衡算法有：
- 轮询，配置名称为`robin`
- 随机，配置名称为`random`
- 最佳分数，配置名称为`score`，它是基于服务端的当前连接数和最大连接数


客户端支持支持同步调用和异步调用：
- 同步调用，调用`call`方法
- 异步调用，调用`asyncCall`方法，它基于future/promise实现，返回一个`std::future`对象，调用`get`方法等待结果。



# 安装
项目还不是很完善，TODO

# 特性
- 服务发现，基于etcd实现
- 负载均衡，支持轮询、随机、最佳分数等算法
- 服务调用，支持同步调用和异步调用
- 服务注册，服务端自动注册到etcd中




