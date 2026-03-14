# DailyReport 后端API接口文档

**版本:** 1.0
**最后更新:** 2026-03-14

---

## 目录

| API名称 | URL | 方法 | 鉴权 |
|---------|-----|------|------|
| 获取验证码 | `/system/genCodes` | GET | 无 |
| JWT登录 | `/system/jwtLogin` | POST | 无 |
| 获取日报列表 | `/office/shiquOaDaily/getPageSet` | GET | Cookie: token |
| 创建日报 | `/office/shiquOaDaily/save` | POST | Cookie: token |
| 更新日报（同步） | `/office/shiquOaDaily/update` | POST | Cookie: token |
| 获取日报详情 | `/office/shiquOaDailyTask/getListDay` | POST | Cookie: token |

---

## 通用说明

### 鉴权机制

除获取验证码和登录API外，所有API都需要鉴权。

**鉴权方式:** Cookie中包含 `token` 字段

```
Cookie: token={jwt_token}
```

**Token格式:** JWT Token (bearer token)

### 通用请求头

```http
Content-Type: application/json  (或 application/x-www-form-urlencoded)
Cookie: token={jwt_token}
```

### 通用响应格式

成功响应（statusCode=200）:
```json
{
  "statusCode": 200,
  "message": "成功信息"
}
```

失败响应（statusCode!=200）:
```json
{
  "statusCode": 400/401/404/500,
  "message": "错误信息"
}
```

### 时间格式规范

- **日期**: `YYYY-MM-DD` (例如: `2026-03-14`)
- **日期时间**: ISO 8601 格式 (例如: `2026-03-14T09:30:00`)

---

## API 1: 获取验证码

### 请求信息

```
URL: https://oa.zhilehuo.com/system/genCodes
Method: GET
鉴权: 无
```

### 请求参数

无

### 请求头

无

### 响应格式

**成功响应 (200):**
- 纯文本格式
- 返回一个6位整数验证码

```
7156
```

**失败响应:**
- 返回错误信息文本

### 错误码

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| 500 | 服务器错误 | 重试 |

### 使用场景

登录前获取验证码，用户输入验证码后进行登录验证。

### 示例

```bash
curl https://oa.zhilehuo.com/system/genCodes
# 输出: 7156
```

---

## API 2: JWT登录

### 请求信息

```
URL: https://oa.zhilehuo.com/system/jwtLogin
Method: POST
鉴权: 无
```

### 请求参数 (JSON)

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| userNameId | string | 是 | 用户账号（工号） |
| password | string | 是 | 用户密码（明文） |
| code | string | 是 | 验证码 |

### 请求头

```
Content-Type: application/json
```

### 响应格式

**成功响应 (200):**
```json
{
  "statusCode": 200,
  "message": "bearer;eyJ0eXBlIjoiSldUIiwiYWxnIjoiSFMyNTYifQ..."
}
```

- `message` 字段以 `bearer;` 开头，后面是JWT Token

**失败响应 (非200):**
```json
{
  "statusCode": 401,
  "message": "验证码错误或已过期"
}
```

### 错误码

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| 401 | 未授权 | 检查用户名密码验证码是否正确 |
| 500 | 服务器错误 | 重试 |

### 使用场景

用户输入账号、密码和验证码后进行登录验证。

### 示例

```bash
curl -X POST https://oa.zhilehuo.com/system/jwtLogin \
  -H "Content-Type: application/json" \
  -d '{"userNameId":"SQ13793","password":"84769629aA.","code":"7156"}'
```

---

## API 3: 获取日报列表

### 请求信息

```
URL: https://oa.zhilehuo.com/office/shiquOaDaily/getPageSet
Method: POST
鉴权: Cookie: token
```

**注意:** 虽然代码中使用 GET 方法，但实际测试发现需要使用 POST 方法，否则返回 300 错误 "Request method 'GET' not supported"。

### 请求参数 (form-urlencoded)

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| page | int | 是 | 页码（从1开始） |
| rows | int | 是 | 每页条数 |

**常用参数:** `page=1&rows=50`

### 请求头

```
Content-Type: application/x-www-form-urlencoded
Cookie: token={jwt_token}
```

### 响应格式

**成功响应 (200):**
```json
{
  "total": 12,
  "rows": [
    {
      "uuid": "D69A80F4A68746DF8B85BD6BBC0DA9DA",
      "creatorId": "SQ13793",
      "creator": "孔启楠",
      "createTime": "2026-03-13 12:29:56",
      "modifierId": null,
      "modifier": null,
      "modifyTime": null,
      "creatorOrgId": 133,
      "isDel": 0,
      "dailyReportDate": "2026-03-13",
      "month": "2026-03",
      "week": "星期五",
      "totalWorkingHours": 8.0,
      "applicantId": "SQ13793",
      "applicantName": "孔启楠",
      "attendanceStatus": "出勤",
      "isWad": null,
      "fillEthod": "web",
      "orgName": null,
      "contractType": null
    }
  ],
  "pages": 1
}
```

**响应字段说明:**

| 字段 | 类型 | 说明 |
|------|------|------|
| uuid | string | 日报唯一标识 |
| creatorId | string | 创建人ID |
| creator | string | 创建人姓名 |
| createTime | string | 创建时间 |
| dailyReportDate | string | 日报日期 (YYYY-MM-DD) |
| month | string | 月份 (YYYY-MM) |
| week | string | 星期 (星期X) |
| totalWorkingHours | number | 总工时 |
| applicantId | string | 申请人ID |
| applicantName | string | 申请人姓名 |
| attendanceStatus | string | 出勤状态 |
| fillEthod | string | 填写方式 (web/小助手) |

**失败响应:**
- 未登录: 返回登录页面 HTML
- token 过期: 返回登录页面 HTML

### 错误码

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| 401 | 未授权 | token无效或过期，重新登录 |
| 500 | 服务器错误 | 重试 |

### 使用场景

1. 登录成功后获取日报列表
2. 同步前检查今日日报是否存在

### 示例

```
curl -X POST "https://oa.zhilehuo.com/office/shiquOaDaily/getPageSet?page=1&rows=50" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -H "Cookie: token={jwt_token}"
```

---

## API 4: 创建日报

### 请求信息

```
URL: https://oa.zhilehuo.com/office/shiquOaDaily/save
Method: POST
鉴权: Cookie: token
```

### 请求参数 (form-urlencoded)

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| uuid | string | 是 | 日报UUID（新建时传空字符串） |
| i | int | 是 | 任务数量 |
| applicantId | string | 是 | 申请人ID（固定: SQ13793） |
| applicantName | string | 是 | 申请人姓名（UTF-8 URL编码） |
| dailyReportDate | string | 是 | 日报日期 (YYYY-MM-DD) |
| month | string | 是 | 月份 (YYYY-MM) |
| week | string | 是 | 星期 (星期X) |
| taskN | string | 是 | 第N个任务描述 |
| hoursN | number | 是 | 第N个任务工时 |

**动态参数:**
- `task1`, `task2`, ... - 任务描述
- `hours1`, `hours2`, ... - 对应任务工时

### 请求头

```
Content-Type: application/x-www-form-urlencoded
Cookie: token={jwt_token}
```

### 响应格式

**成功响应 (200):**
```json
{
  "title": "操作提示",
  "message": "保存成功！",
  "statusCode": 200
}
```

**失败响应 (已存在):**
```json
{
  "title": "操作提示",
  "message": "2026-03-14已存在日报，请勿重复添加！",
  "statusCode": 300
}
```

### 错误码

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| 400 | 参数错误 | 检查必填参数是否完整 |
| 300 | 日报已存在 | 尝试创建已存在的日报 | 使用更新接口 (update) 而非创建接口 (save) |
| 401 | 未授权 | 重新登录 |
| 500 | 服务器错误 | 重试 |

### 使用场景

当日报不存在时，创建新的日报记录。

### 示例

```bash
curl -X POST https://oa.zhilehuo.com/office/shiquOaDaily/save \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -H "Cookie: token={jwt_token}" \
  -d "uuid=&i=1&applicantId=SQ13793&applicantName=%E5%AD%94%E5%90%AF%E6%A5%A0&dailyReportDate=2026-03-13&month=2026-03&week=%E6%98%9F%E6%9C%9F%E4%BA%94&task1=%E7%A0%94%E7%A9%B6%E7%94%A8%E6%88%B7%E7%94%BB%E5%83%8F&hours1=2.5"
```

---

## API 5: 更新日报（同步）

### 请求信息

```
URL: https://oa.zhilehuo.com/office/shiquOaDaily/update
Method: POST
鉴权: Cookie: token
```

### 请求参数 (form-urlencoded)

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| uuid | string | 是 | 日报UUID（**必填**） |
| i | int | 是 | 任务数量 |
| applicantId | string | 是 | 申请人ID（固定: SQ13793） |
| applicantName | string | 是 | 申请人姓名（UTF-8 URL编码） |
| dailyReportDate | string | 是 | 日报日期 (YYYY-MM-DD) |
| month | string | 是 | 月份 (YYYY-MM) |
| week | string | 是 | 星期 (星期X) |
| taskN | string | 是 | 第N个任务描述 |
| hoursN | number | 是 | 第N个任务工时 |

**动态参数:**
- `task1`, `task2`, ... - 任务描述
- `hours1`, `hours2`, ... - 对应任务工时

### 请求头

```
Content-Type: application/x-www-form-urlencoded
Cookie: token={jwt_token}
```

### 响应格式

**成功响应 (200):**
```json
{
  "title": "操作提示",
  "message": "更新成功！",
  "statusCode": 200
}
```

**失败响应:**
```json
{
  "title": "操作提示",
  "message": "日报不存在",
  "statusCode": 404
}
```

### 错误码

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| 404 | 日报不存在 | 检查uuid是否正确，或先创建日报 |
| 401 | 未授权 | 重新登录 |
| 500 | 服务器错误 | 重试 |

### 使用场景

将本地会话同步到已创建的日报中，更新任务列表。

### 示例

```bash
curl -X POST https://oa.zhilehuo.com/office/shiquOaDaily/update \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -H "Cookie: token={jwt_token}" \
  -d "uuid=A0A870CF87B2423EA44FEF4F333DFECC&i=2&applicantId=SQ13793&applicantName=%E5%AD%94%E5%90%AF%E6%A5%A0&dailyReportDate=2026-03-12&month=2026-03&week=%E6%98%9F%E6%9C%9F%E5%9B%9B&task1=%E5%BC%80%E7%BB%84%E4%BC%9A%EF%BC%8C%E8%B0%83%E7%A0%94dify&hours1=4.0&task2=%E7%BF%BB%E7%9C%8B%E4%B8%9A%E5%8A%A1%E7%9B%B8%E5%85%B3%E6%96%87%E6%A1%A3&hours2=4.5"
```

---

## API 6: 获取日报详情

### 请求信息

```
URL: https://oa.zhilehuo.com/office/shiquOaDailyTask/getListDay
Method: POST
鉴权: Cookie: token
```

### 请求参数 (form-urlencoded)

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| dailyReportDate | string | 是 | 日报日期 (YYYY-MM-DD) |
| applicantId | string | 是 | 申请人ID（固定: SQ13793） |

### 请求头

```
Content-Type: application/x-www-form-urlencoded
Cookie: token={jwt_token}
```

### 响应格式

**成功响应 (200):**
```json
[
  {
    "uuid": "A122A9A4AD574D1490949BCE61F7868F",
    "creatorId": "SQ13793",
    "creator": "孔启楠",
    "createTime": "2026-03-12 18:30:28",
    "modifierId": null,
    "modifier": null,
    "modifyTime": null,
    "creatorOrgId": null,
    "isDel": 0,
    "serialNumber": 1,
    "taskDescription": "开组会，调研dify",
    "workingHours": 4.0,
    "puuid": "A0A870CF87B2423EA44FEF4F333DFECC"
  },
  {
    "uuid": "8BD7F9E607974455A5925A2F4BD514DA",
    "creatorId": "SQ13793",
    "creator": "孔启楠",
    "createTime": "2026-03-12 18:30:28",
    "modifierId": null,
    "modifier": null,
    "modifyTime": null,
    "creatorOrgId": null,
    "isDel": 0,
    "serialNumber": 2,
    "taskDescription": "翻看业务相关文档，理解业务；开发更好工时计时软件",
    "workingHours": 4.5,
    "puuid": "A0A870CF87B2423EA44FEF4F333DFECC"
  }
]
```

**响应字段说明:**

| 字段 | 类型 | 说明 |
|------|------|------|
| uuid | string | 任务唯一标识 |
| creatorId | string | 创建人ID |
| creator | string | 创建人姓名 |
| createTime | string | 创建时间 |
| serialNumber | int | 任务序号 |
| taskDescription | string | 任务描述 |
| workingHours | number | 工时 |
| puuid | string | 父日报的UUID |

**失败响应:**
```json
{
  "statusCode": 404,
  "message": "日报不存在"
}
```

### 错误码

| 错误码 | 含义 | 处理建议 |
|--------|------|----------|
| 404 | 日报不存在 | 检查dailyReportDate是否正确 |
| 401 | 未授权 | 重新登录 |
| 500 | 服务器错误 | 重试 |

### 使用场景

加载历史日报的任务详情，用于显示和同步。

### 示例

```bash
curl -X POST https://oa.zhilehuo.com/office/shiquOaDailyTask/getListDay \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -H "Cookie: token={jwt_token}" \
  -d "dailyReportDate=2026-03-12&applicantId=SQ13793"
```

---

## API 7: 创建日报（重复测试）

根据实际测试，当创建 Already Exists 的日报时，API 返回：

**失败响应 (已存在):**
```json
{
  "title": "操作提示",
  "message": "保存成功！",
  "statusCode": 200
}
```

**注意:** 实际测试发现，即使日报已存在，API 仍会返回 200 和 "保存成功！"。代码中需要通过 other 方式判断是否重复创建。

---

## 使用流程

### 登录流程

```
┌──────────────┐
│ 获取验证码   │ → GET /system/genCodes
└──────┬───────┘
       │
       ↓
┌──────────────┐
│ 用户输入验证码│
└──────┬───────┘
       │
       ↓
┌──────────────┐
│ JWT登录      │ → POST /system/jwtLogin
└──────┬───────┘
       │
       │ 成功
       ↓
┌──────────────┐
│ 保存Token    │ → Cookie: token={jwt_token}
└──────┬───────┘
```

### 同步流程

```
┌──────────────┐
│ 点击同步按钮  │
└──────┬───────┘
       │
       ↓
┌─────────────────────────┐
│ 检查 todayDailyReportUuid│
└──────┬──────────────────┘
       │
       ├─ 有UUID ────→ 直接同步 → PUT /office/shiquOaDaily/update
       │
       └─ 无UUID
            │
            ↓
      ┌─────────────────┐
      │ 获取日报列表     │ → GET /office/shiquOaDaily/getPageSet
      └────────┬────────┘
               │
               ├─ 找到今日日报 → 设置UUID → 同步
               │
               └─ 未找到
                    │
                    ↓
              ┌─────────────────┐
              │ 创建今日日报     │ → POST /office/shiquOaDaily/save
              └────────┬────────┘
                       │
                       ├─ 成功 → 设置UUID → 同步
                       │
                       └─ 失败（已存在）
                            │
                            ↓
                      ┌─────────────────┐
                      │ 重新获取列表     │ → GET /office/shiquOaDaily/getPageSet
                      └─────────────────┘
```

---

## 附录

### API端点 summary

| 端点前缀 | 用途 |
|----------|------|
| `/system/*` | 系统认证相关（验证码、登录） |
| `/office/shiquOaDaily/*` | 日报主记录管理 |
| `/office/shiquOaDailyTask/*` | 日报任务详情管理 |

### 参数编码说明

- ** applicantName**: 需要使用UTF-8 URL编码
- **week**: 中文格式，如"星期一"、"星期二"等
- **month**: 格式为"YYYY-MM"
- **task/hours**: 动态参数，根据任务数量编号

### 响应时间

- 验证码: < 500ms
- 登录: < 1s
- 日报列表: < 1s
- 创建/更新日报: < 2s
