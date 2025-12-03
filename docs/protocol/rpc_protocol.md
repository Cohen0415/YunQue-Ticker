# RPC 协议文档

> 本文档描述 UI 客户端与 C 服务端之间的通信协议。
> 数据传输格式为 JSON。

---

## 模块：Backlight（屏幕亮度）

| CMD                    | 参数说明                | 返回值说明           | 描述             |
|-----------------------|-----------------------|-------------------|----------------|
| CMD_SET_BRIGHTNESS    | value: int (0~255)     | status: int (0 成功, 其他失败) | 设置屏幕亮度    |
| CMD_GET_BRIGHTNESS    | 无                     | value: int (0~255), status: int | 获取当前亮度值  |

### 示例

**设置亮度**

```json
// 请求
{
  "cmd": "CMD_SET_BRIGHTNESS",
  "params": {
    "value": 200
  }
}

// 响应
{
  "status": 0
}
```

**获取亮度**

```json
// 请求
{
  "cmd": "CMD_GET_BRIGHTNESS",
  "params": {}
}

// 响应
{
  "status": 0,
  "value": 200
}
```