# Soft-UE ns-3 模块集成测试 - 完整任务描述

## 项目背景

成功将 `/home/makai/Soft-UE` 原型系统的所有功能完整迁移到 `/home/makai/Soft-UE-ns3/src/soft-ue/model` 离散事件仿真网络环境中，正在进行集成测试。

继续优化/home/makai/Soft-UE-ns3/scratch/Soft-UE/Soft-UE.cc，/home/makai/Soft-UE-ns3/src/soft-ue/model。善用git，但是禁止提交远程。

如需生成文档，放在/home/makai/Soft-UE-ns3/docs/cccc下，禁止生成过多文档，不必要的文档及时清理。

## 优化点

1. 检查是否将所有 `src/soft-ue/model` 模块串联，检查是否实现了`/home/makai/Soft-UE` 原型系统的所有功能。

2. 完整的1对1通信场景：
在完整的1对1通信场景中可以进行正常通信

3. 详细统计信息

4. 全部ns-3化，务必使用ns-3的思想，如队列全部替换成ns3:Queue。

5. 日志优化，当前的日志log/soft-ue.log 太丑了，可读性极差，尽可能优化日志，方便排查错误。

6. 如果发生了改变，请同步更新
docs/cccc/01-dataflow.md
docs/cccc/02-problems-solutions.md
docs/cccc/03-diagrams.md

7. 继续优化/home/makai/Soft-UE-ns3/scratch/Soft-UE/Soft-UE.cc，/home/makai/Soft-UE-ns3/src/soft-ue/model。
如需新增文件，请放在/home/makai/Soft-UE-ns3/src/soft-ue/model，其他位置不要生成代码文件。


## 使用方法
### 编译

./ns3 build

### 运行
```bash
# 可以通过命令行参数调整
./ns3 run "Soft-UE --packetSize=256 --numPackets=5"
```

