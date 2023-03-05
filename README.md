# TODO

Json库
1. 加入异常
2. noexcept
3. append

协程库：
1. coroutine join的方式改进

## 协程库task_coroutine

### 调试开关

文件`define.h`中，打开后，重新编译程序

```c++
// 是否开启debug
#define TASK_COROUTINE_DEBUG 0
```

## 日志logger

### 设计

1. 日志id生成器：为了防止并发问题，id生成器为`thread local`
2. Logger：有一个LogMsgPool和待消费日志队列（由一个线程消费）
3. 发送日志：`LOG_XXXXX`是其他线程向Logger发送日志的接口

可优化点：
1. 多个LogMsgPool，减少多线程竞争
2. 多条待消费日志队列，但需要保证日志时间序

### 配置

<table>
    <tr>
        <th colspan = 2>可配置项</th>
    </tr>
    <tr>
        <td>min_level</td>
        <td>最小日志级别，默认Debug</td>
    </tr>
    <tr>
        <td>capacity</td>
        <td>Logger缓冲区大小，默认1024</td>
    </tr>
    <tr>
        <td>log_directory</td>
        <td>log日志目录，配置绝对路径，<b>必须指定</b></td>
    </tr>
    <tr>
        <td>max_line_count</td>
        <td>每个日志文件最大行数，默认1000</td>
    </tr>
    <tr>
        <th colspan = 2>不可配置项</th>
    </tr>
    <tr>
        <td colspan = 2>每行日志最大长度为65536，超过会截断</td>
    </tr>
</table>

配置示例

```conf
{
    {"min_level", "Debug"},
    {"capacity", "1024"},
    {"log_directory", "/home/hjp/Workspace/coco/test/log"},
    {"max_line_count", "1000"}
}
```

### 使用注意事项

`LOG_XXX`传递`raw`时，如果是`const char*/char*`，只能传递静态文本的
```c++
const char* buf = malloc(...); 
// ...
LOG_DEBUG(log_id, buf); // error
LOG_DEBUG(log_id, std::string(buf)); // right

const char* buf2 = "aaaaa";
LOG_ERROR(log_id, buf2); // right
```

## Json

### clang

```bash
// 下载clang-10
sudo apt install clang-10 --install-suggests
// 输出语法树json格式
clang-10 -Xclang -ast-dump=json -fsyntax-only -Iinclude -x c++ test.h > out.json
```