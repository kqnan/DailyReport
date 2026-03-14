# API接口文档编写计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 编写标准的后端API接口文档，包含所有API的详细规格（请求参数、响应格式、错误码、签名说明）

**Architecture:**
- 创建Markdown格式文档（人类阅读）+ OpenAPI/Swagger YAML格式（机器可读）
- 文档基于现有代码分析的6个API端点
- 每个API包含：URL、方法、鉴权方式、请求参数、响应格式、错误码、使用场景

**Tech Stack:** Qt 6.6, C++20, OpenAPI 3.0

---
## 文件结构

| 文件 | 类型 | 用途 |
|------|------|------|
| `docs/API接口文档.md` | 新建 | Markdown格式的人类可读文档 |
| `docs/api.yaml` | 新建 | OpenAPI 3.0格式的机器可读定义 |

---

### Task 1: 创建Markdown格式API文档

**Files:**
- Create: `docs/API接口文档.md`

- [ ] **Step 1: 创建Markdown文档框架**

编写文档-header，包含：
- 文档标题和版本信息
- 目录结构
- API概览表格（6个API的名称、URL、用途、鉴权方式）

- [ ] **Step 2: 编写通用说明章节**

包含：
- 鉴权机制（Cookie中的token）
- 通用请求头要求
- 通用响应格式说明
- 时间格式规范（ISO 8601）

- [ ] **Step 3: 编写API 1 - 获取验证码**

包含：
- URL: `https://oa.zhilehuo.com/system/genCodes`
- 方法: GET
- 请求参数: 无
- 请求头: 无
- 响应格式: 纯文本整数
- 错误码: N/A
- 使用场景: 登录前获取验证码

- [ ] **Step 4: 编写API 2 - JWT登录**

包含：
- URL: `https://oa.zhilehuo.com/system/jwtLogin`
- 方法: POST
- 请求参数（JSON）: userNameId, password, code
- 请求头: Content-Type: application/json
- 响应格式（成功）: statusCode=200, message=bearer;{token}
- 响应格式（失败）: statusCode!=200, message=错误信息
- 错误码: 401未授权等
- 使用场景: 登录验证

- [ ] **Step 5: 编写API 3 - 获取日报列表**

包含：
- URL: `https://oa.zhilehuo.com/office/shiquOaDaily/getPageSet`
- 方法: GET
- 请求参数: page, rows
- 请求头: Cookie (必须包含token)
- 响应格式（完整JSON）: total, rows数组
- rows字段详细说明（uuid, creator, dailyReportDate, month, week等）
- 错误码: 401未登录, 500服务器错误
- 使用场景: 登录后获取日报列表，同步前检查

- [ ] **Step 6: 编写API 4 - 创建日报**

包含：
- URL: `https://oa.zhilehuo.com/office/shiquOaDaily/save`
- 方法: POST
- 请求参数（form-urlencoded）: uuid, i, applicantId, applicantName, dailyReportDate, month, week, taskN, hoursN
- 请求头: Content-Type: application/x-www-form-urlencoded, Cookie
- 响应格式（成功）: statusCode=200, message=保存成功
- 响应格式（失败）: statusCode!=200, message=错误信息
- 任务参数动态说明（task1/hours1, task2/hours2...）
- 错误码: 400参数错误, 409日报已存在
- 使用场景: 今日日报不存在时创建

- [ ] **Step 7: 编写API 5 - 更新日报（同步）**

包含：
- URL: `https://oa.zhilehuo.com/office/shiquOaDaily/update`
- 方法: POST
- 请求参数（form-urlencoded）: uuid, i, applicantId, applicantName, dailyReportDate, month, week, taskN, hoursN
- 请求头: Content-Type: application/x-www-form-urlencoded, Cookie
- 响应格式（成功）: statusCode=200, message=更新成功
- 响应格式（失败）: statusCode!=200, message=错误信息
- uuid字段必填说明
- 错误码: 404日报不存在, 409重复更新
- 使用场景: 同步本地会话到云端

- [ ] **Step 8: 编写API 6 - 获取日报详情**

包含：
- URL: `https://oa.zhilehuo.com/office/shiquOaDailyTask/getListDay`
- 方法: POST
- 请求参数: dailyReportDate, applicantId
- 请求头: Cookie
- 响应格式（JSON数组）: uuid, creator, taskDescription, workingHours, puuid等
- 每个字段类型和含义说明
- 错误码: 404日报不存在, 401未登录
- 使用场景: 加载历史日报数据

- [ ] **Step 9: 添加错误码汇总表**

包含所有API中出现的错误码：
| 错误码 | 含义 | 可能原因 | 处理建议 |
|--------|------|----------|----------|
| 401 | 未授权 | token无效或过期 | 重新登录 |
| 404 | 资源不存在 | 日报不存在 | 检查日期或创建日报 |
| 409 | 冲突 | 日报已存在 | 检查是否重复创建 |

- [ ] **Step 10: 添加使用流程图**

包含两个主要流程：
- 登录流程：获取验证码 → 登录
- 同步流程：检查日报UUID → 获取列表/创建日报 → 同步会话

---

### Task 2: 创建OpenAPI/Swagger YAML文档

**Files:**
- Create: `docs/api.yaml`

- [ ] **Step 1: 创建OpenAPI文档头部**

```yaml
openapi: 3.0.0
info:
  title: DailyReport API Documentation
  version: 1.0.0
  description: 工时记录系统的后端API接口定义
servers:
  - url: https://oa.zhilehuo.com
```

- [ ] **Step 2: 定义通用组件**

包含：
- schemas: Error, DailyReport, DailyReportTask, LoginResponse
- parameters: PageParams, DateParam
- securitySchemes: TokenAuth (Cookie)

- [ ] **Step 3: 定义API路径 - /system/genCodes**

```yaml
get:
  summary: 获取验证码
  operationId: getVerificationCode
  responses:
    '200':
      description: 验证码（整数）
      content:
        text/plain:
          schema:
            type: integer
```

- [ ] **Step 4: 定义API路径 - /system/jwtLogin**

```yaml
post:
  summary: JWT登录
  operationId: login
  requestBody:
    content:
      application/json:
        schema:
          $ref: '#/components/schemas/LoginRequest'
  responses:
    '200':
      description: 登录成功
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/LoginResponse'
```

- [ ] **Step 5: 定义API路径 - /office/shiquOaDaily/getPageSet**

```yaml
get:
  summary: 获取日报列表
  operationId: getDailyReportList
  parameters:
    - name: page
      in: query
      schema: { type: integer }
    - name: rows
      in: query
      schema: { type: integer }
  Security:
    - TokenAuth: []
  responses:
    '200':
      description: 日报列表
      content:
        application/json:
          schema:
            type: object
            properties:
              total: { type: integer }
              rows:
                type: array
                items:
                  $ref: '#/components/schemas/DailyReport'
```

- [ ] **Step 6: 定义API路径 - /office/shiquOaDaily/save**

```yaml
post:
  summary: 创建日报
  operationId: createDailyReport
  requestBody:
    content:
      application/x-www-form-urlencoded:
        schema:
          type: object
          properties:
            uuid: { type: string }
            i: { type: integer }
            applicantId: { type: string }
            applicantName: { type: string }
            dailyReportDate: { type: string, format: date }
            month: { type: string, pattern: '^\\d{4}-\\d{2}$' }
            week: { type: string }
            task1: { type: string }
            hours1: { type: number }
            # 动态任务参数...
  responses:
    '200':
      description: 创建成功
```

- [ ] **Step 7: 定义API路径 - /office/shiquOaDaily/update**

```yaml
post:
  summary: 更新日报（同步）
  operationId: syncDailyReport
  requestBody:
    content:
      application/x-www-form-urlencoded:
        schema:
          type: object
          properties:
            uuid: { type: string, description: "日报UUID，必填" }
            # ... 其他参数
  responses:
    '200':
      description: 更新成功
```

- [ ] **Step 8: 定义API路径 - /office/shiquOaDailyTask/getListDay**

```yaml
post:
  summary: 获取日报详情
  operationId: getDailyReportDetails
  requestBody:
    content:
      application/x-www-form-urlencoded:
        schema:
          type: object
          properties:
            dailyReportDate: { type: string, format: date }
            applicantId: { type: string }
  responses:
    '200':
      description: 任务列表
      content:
        application/json:
          schema:
            type: array
            items:
              $ref: '#/components/schemas/DailyReportTask'
```

- [ ] **Step 9: 添加错误响应定义**

在每个API中添加常见的error responses：
```yaml
responses:
  '401':
    description: 未授权
  '404':
    description: 资源不存在
  '500':
    description: 服务器错误
```

- [ ] **Step 10: 验证YAML格式**

运行验证命令：
```bash
# 使用python验证
python -c "import yaml; yaml.safe_load(open('docs/api.yaml'))"
# 或使用在线验证器 https://apigee.com/spec/tools

Expected: No errors
```

---

### Task 3: 验证文档完整性

**Files:**
- Test: `docs/API接口文档.md`
- Test: `docs/api.yaml`

- [ ] **Step 1: 检查Markdown文档**

验证清单：
- [ ] 所有6个API都有完整章节
- [ ] 每个API包含请求方法、URL、参数、响应、错误码
- [ ] 有通用说明章节
- [ ] 有错误码汇总表
- [ ] 有使用流程图
- [ ] 字段类型和含义清晰

- [ ] **Step 2: 检查OpenAPI文档**

验证清单：
- [ ] OpenAPI版本正确 (3.0.0)
- [ ] servers配置正确
- [ ] 所有6个API路径定义完整
- [ ] schemas定义完整（LoginRequest, LoginResponse, DailyReport, DailyReportTask, Error）
- [ ] parameters定义完整
- [ ] securitySchemes定义正确
- [ ] 无YAML语法错误

- [ ] **Step 3: 运行YAML语法验证**

```bash
python -c "import yaml; yaml.safe_load(open('docs/api.yaml'))"
```

Expected output: No errors

- [ ] **Step 4: 提交文档**

```bash
git add docs/API接口文档.md docs/api.yaml
git commit -m "docs: 添加标准API接口文档"
```

Expected: Commit successful

---

## 执行说明

完成所有任务后，文档应包含：
1. **docs/API接口文档.md** - 完整的人类可读API文档
2. **docs/api.yaml** - 有效的OpenAPI 3.0定义文件

文档应覆盖当前代码中使用的6个API端点的所有细节。
