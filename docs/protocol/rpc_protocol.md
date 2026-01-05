# 本地 RPC 通信协议（基于 Unix Domain Socket）

* 本文档描述 UI 客户端与 C 服务端之间的通信协议。
* 数据传输格式为 JSON。
* 文档版本：v0.2.0
---

## 帧格式

所有请求和响应均采用 固定前缀 + 可变载荷 的二进制帧结构：

```text
+------------------+---------------------+
| 4 字节长度字段    | N 字节 JSON 载荷     |
+------------------+---------------------+
| uint32_t (LE)    | UTF-8 JSON string   |
+------------------+---------------------+
```
长度字段说明：

* 类型：32 位无符号整数（uint32_t）
* 字节序：小端（Little Endian）
* 值含义：后续 JSON 载荷的字节长度（不包含 \0）
* 合法范围：1 ≤ length ≤ 8192（即最大支持 8KB JSON）

## JSON 数据格式模版

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
  "cmd": "brightness.set",
  "status": 0,
  "msg": "ok",
  "data": {}
}
```
响应报文由四个主要字段构成：

`cmd`：

* 返回请求时的命令

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
  | - value：int；0~100；要设置的亮度值
*/
{
  "cmd": "brightness.set",
  "params": {
    "value": 80
  }
}

// 响应
/* 
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - value：int；0~100；当前的亮度值
*/
{
  "cmd": "brightness.set",
  "status": 0,
  "msg": "ok",
  "data": {
    "value": 80
  }
}
```

### 获取亮度（brightness.get）

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
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - value：int；0~100；当前亮度值
*/
{
  "cmd": "brightness.get",
  "status": 0,
  "msg": "ok",
  "data": {
    "value": 80
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
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - time：string；ISO 8601 标准格式
*/
{
  "cmd": "sysinfo.bjtime.get",
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
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - temp：int；45493；除1000就是当前温度，例如45.493℃
*/
{
  "cmd": "sysinfo.temp.get",
  "status": 0,
  "msg": "ok",
  "data": {
    "temp": 45493
  }
}

### 获取系统镜像版本号（sysinfo.version.get）

```json
// 请求
/* 
  cmd：string；sysinfo.version.get；命令名称
  params
  | - NULL
*/
{
  "cmd": "sysinfo.version.get",
  "params": {}
}

// 响应
/* 
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - version：string；v1.0.0
*/
{
  "cmd": "sysinfo.version.get",
  "status": 0,
  "msg": "ok",
  "data": {
    "version": "v1.0.0"
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
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - NULL
*/
{
  "cmd": "audio.play",
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
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - NULL
*/
{
  "cmd": "audio.stop",
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
  | - volume：int；0~100；音量值（0静音，100最大）
*/
{
  "cmd": "audio.volume.set",
  "params": {
    "volume": 100
  }
}

// 响应
/* 
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - volume：int；0~100；当前音量大小
*/
{
  "cmd": "audio.volume.set",
  "status": 0,
  "msg": "ok",
  "data": {
    "volume": 80
  }
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
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - volume：int；0~100；当前音量大小
*/
{
  "cmd": "audio.volume.get",
  "status": 0,
  "msg": "ok",
  "data": {
    "volume": 80
  }
}
```

## 模块：Wi-Fi

### 连接 Wi-Fi（wifi.connect）

```json
// 请求
/* 
  cmd：string；wifi.connect；命令名称
  params
  | - ssid：string；Wi-Fi 名称
  | - password：string；密码（开放网络可为空）
*/
{
  "cmd": "wifi.connect",
  "params": {
    "ssid": "Home_Wifi",
    "password": "12345678"
  }
}

// 响应
/* 
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息，如 "密码错误"、"网络不可用"
  data
  | - ip：string；分配到的 IP，连接失败时为空
*/
{
  "cmd": "wifi.connect",
  "status": 0,
  "msg": "ok",
  "data": {
    "ip": "192.168.1.100"
  }
}
```

###  Wi-Fi（wifi.disconnect）

```json
// 请求
/* 
  cmd：string；wifi.disconnect；命令名称
  params
  | - NULL
*/
{
  "cmd": "wifi.disconnect",
  "params": {}
}

// 响应
/* 
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
*/
{
  "cmd": "wifi.disconnect",
  "status": 0,
  "msg": "ok"
}
```

### 查询当前网络状态（wifi.status.get）

```json
// 请求
/* 
  cmd：string；wifi.status.get；命令名称
  params
  | - NULL
*/
{
  "cmd": "wifi.status.get",
  "params": {}
}

// 响应
/* 
  cmd：string；返回请求时使用的命令
  status：int；0 表示成功，其它表示失败
  msg：string；提示信息
  data
  | - connected：bool；是否已连接
  | - ssid：string；当前连接的 Wi-Fi 名称，无连接时为空
  | - ip：string；当前 IP，无连接时为空
  | - rssi：string；信号强度
*/
{
  "cmd": "wifi.status.get",
  "status": 0,
  "msg": "ok",
  "data": {
    "connected": true,
    "ssid": "Home_Wifi",
    "ip": "192.168.1.100",
    "rssi": "-50"
  }
}
```