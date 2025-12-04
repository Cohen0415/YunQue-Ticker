v0.1.0 (2025-12-05)
- 实现 UNIX Socket RPC 服务端基础功能
- 支持命令注册与查找
- 支持 JSON 解析及处理回调映射
- 引入日志库（支持颜色、函数追踪、可选输出文件）
- 引入队列库，实现异步收发模型
- 实现 recv/process/send 三线程架构
- 实现 request -> handler -> response 流程闭环