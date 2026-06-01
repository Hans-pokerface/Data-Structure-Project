#pragma once
#include"include.h"

//数据结构：十字链表
typedef struct OLNode
{
	int i, j;//行列值
	int e;//元素值
	OLNode* right, * down;//指向右侧、下方的下一个非零元
}OLNode;

typedef struct
{
	vector<OLNode*> rhead, chead;//储存每一行、每一列第一个非零元的指针
	int mu, nu, tu;//矩阵的行数、列数、非零元个数
}Crosslist;

//基于十字链表的相关函数声明
void CreateOLMatrix(Crosslist* M);

void AddMinusOLMatrix(Crosslist* A, Crosslist* B, int op);

void MultiOLMatrix(Crosslist* A, Crosslist* B, Crosslist* M);

void PrintOLMatrix(Crosslist* M);

void FreeCrosslist(Crosslist* M);