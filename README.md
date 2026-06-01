# 数据结构课程大作业

## 一、稀疏矩阵运算器（十字链表实现）
### 1. 项目文件结构
- include.h  
  - 包含项目所需公共头文件与命名空间声明

- OLMatrix.h  
  - 十字链表结点（OLNode）与矩阵结构（Crosslist）定义  
  - 所有运算函数声明

- InOut.cpp  
  - CreateOLMatrix()：创建稀疏矩阵十字链表  
  - PrintOLMatrix()：按矩阵格式打印结果  
  - FreeCrosslist()：释放十字链表内存

- AddMinus.cpp  
  - AddMinusOLMatrix()：稀疏矩阵加法/减法

- Multi.cpp  
  - MultiOLMatrix()：稀疏矩阵乘法

- main.cpp  
  - 主函数：交互菜单、输入矩阵、调用运算、输出结果

### 2. 功能说明
- 支持稀疏矩阵的**创建、显示、销毁**
- 支持**矩阵加法、减法、乘法**
- 采用**十字链表**存储，节省空间、提高非零元操作效率

---

## 二、电梯模拟系统
