# Allocation-and-Reclaim
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FKwanhooo%2FAllocation-and-Reclaim.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2FKwanhooo%2FAllocation-and-Reclaim?ref=badge_shield)

### 模拟器 - 主存储器空间的分配和回收

#### 内容

  在可变分区管理方式下，采用最先适应算法实现主存空间的分配和回收   

####  提示 

 1、自行假设主存空间大小，预设操作系统所占大小并构造未分分区表；

  表目内容：起址、长度、状态（未分/空表目）

2、结合实验一，PCB增加为：

  { PID，要求运行时间，优先权，状态，所需主存大小，主存起始位置，PCB指针 }

3、采用最先适应算法分配主存空间；

4、进程完成后，回收主存，并与相邻空闲分区合并。


## License
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FKwanhooo%2FAllocation-and-Reclaim.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2FKwanhooo%2FAllocation-and-Reclaim?ref=badge_large)