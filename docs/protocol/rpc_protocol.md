# RPC 协议文档

* 本文档描述 UI 客户端与 C 服务端之间的通信协议。
* 数据传输格式为 JSON。
* 文档版本：v0.1.0
---

## 协议格式模版

### 请求示例

```json
// 请求
{
  "cmd": "brightness.set",
  "params": {
    "value": 200
  }
}
```
请求报文由两个主要字段构成：

`cmd`：
* 字符串类型，表示具体的命令或接口标识，例如 brightness.set 表示设置亮度。
* 命令统一采用小写字母和点分隔符（.）风格，便于分类和管理。

`params`：
* 对象类型，包含该命令所需的参数，每个接口的参数字段根据功能不同而不同。
* 如果命令不需要参数，params 可为空对象 {}。

注意：客户端发送的请求必须严格遵循 JSON 格式，命令名称区分大小写。

### 响应示例

```json
// 响应
{
  "status": 0,
  "msg": "ok",
  "data": {}
}
```
响应报文由三个主要字段构成：

`status`：

* 整数类型，表示命令执行的结果状态。
* 0 表示成功；非零值表示错误或异常，具体数值可结合接口文档说明。

`msg`：

* 字符串类型，对 status 的文字描述或提示信息。
* 例如 "ok" 表示成功，"fail" 表示失败。即便 status 已经提供状态码，msg 也可用于显示给用户或调试。

`data`：

* 对象类型，包含该命令返回的具体数据。
* 不同命令返回的数据结构不同，例如亮度值接口返回 { "value": 200 }，系统信息接口返回 { "version": "v0.1.0" }。

如果命令没有返回数据，可使用空对象 {} 占位。

## 模块：Backlight（屏幕亮度）

### 设置亮度（brightness.set）

```json
// 请求
/* 
  cmd：string；brightness.set；命令名称
  params
  | - value：int；0~255；要设置的亮度值
*/
{
  "cmd": "brightness.set",
  "params": {
    "value": 200
  }
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - NULL
*/
{
  "status": 0,
  "msg": "ok",
  "data": {}
}
```

### 获取亮度（brightness.set）

```json
// 请求
/* 
  cmd：string；brightness.get；命令名称
  params：
  | - NULL
*/
{
  "cmd": "brightness.get",
  "params": {}
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - value：int；0~255；当前亮度值
*/
{
  "status": 0,
  "msg": "ok",
  "data": {
    "value": 200
  }
}
```

## 模块：Sysinfo（系统信息）

### 获取北京时间（sysinfo.bjtime.get）

```json
// 请求
/* 
  cmd：string；sysinfo.bjtime.get；命令名称
  params
  | - NULL
*/
{
  "cmd": "sysinfo.bjtime.get",
  "params": {}
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - time：string；ISO 8601 标准格式
*/
{
  "status": 0,
  "msg": "ok",
  "data": {
    "time": "2025-12-05T09:26:25+08:00"
  }
}
```

### 获取 CPU 温度（sysinfo.temp.get）

```json
// 请求
/* 
  cmd：string；sysinfo.temp.get；命令名称
  params
  | - NULL
*/
{
  "cmd": "sysinfo.temp.get",
  "params": {}
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - temp：int；45493；除100就是当前温度，例如45.493℃
*/
{
  "status": 0,
  "msg": "ok",
  "data": {
    "temp": 45493
  }
}
```

## 模块：Audio（音频）

### 播放指定音频文件（audio.play）

```json
// 请求
/* 
  cmd：string；audio.play；命令名称
  params
  | - name：string；音频文件的绝对路径
*/
{
  "cmd": "audio.play",
  "params": {
    "name": "happy.wav"
  }
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - NULL
*/
{
  "status": 0,
  "msg": "ok",
  "data": {}
}
```

### 停止播放（audio.stop）

```json
// 请求
/* 
  cmd：string；audio.stop；命令名称
  params
  | - NULL
*/
{
  "cmd": "audio.stop",
  "params": {}
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - NULL
*/
{
  "status": 0,
  "msg": "ok",
  "data": {}
}
```

### 设置音量（audio.volume.set）

```json
// 请求
/* 
  cmd：string；audio.volume.set；命令名称
  params
  | - volume：int；0~255；音量值（0静音，255最大）
*/
{
  "cmd": "audio.volume.set",
  "params": {
    "volume": 200
  }
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - NULL
*/
{
  "status": 0,
  "msg": "ok",
  "data": {}
}
```

### 获取音量（audio.volume.get）

```json
// 请求
/* 
  cmd：string；audio.volume.get；命令名称
  params
  | - NULL
*/
{
  "cmd": "audio.volume.get",
  "params": {}
}

// 响应
/* 
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - volume：int；0~255；当前音量大小
*/
{
  "status": 0,
  "msg": "ok",
  "data": {
    "volume": 200
  }
}
```