#include"include.h"
#include"OLMatrix.h"


void AddMinusOLMatrix(Crosslist* A, Crosslist* B, int op)
{
	vector<OLNode*> hl(A->nu + 1, NULL);
	for (int col = 1; col <= A->nu; col++)
		hl[col] = A->chead[col];
	//辅助指针hl，类似与行的前驱pre

	for (int row = 1; row <= A->mu; row++)
	{
		OLNode* pa = A->rhead[row], * pb = B->rhead[row], * pre = NULL;//辅助指针pre
		while (pb != NULL)
		{
			if (pa == NULL || pa->j > pb->j)//表示A中要新增一个来自B的结点
			{
				OLNode* p = new OLNode;
				p->right = NULL; p->down = NULL;
				p->i = pb->i;
				p->j = pb->j;
				if (op == 1)
					p->e = pb->e;
				else
					p->e = -pb->e;

				if (pre == NULL)//p将成为本行第一个非零元
					A->rhead[row] = p;
				else
					pre->right = p;
				p->right = pa;
				pre = p;

				if (!A->chead[p->j] || A->chead[p->j]->i > p->i)
				{
					//p将成为本列第一个非零元，chead也要随之更改
					p->down = A->chead[p->j];
					A->chead[p->j] = p;
				}
				else
				{
					//在hl的基础上再向下找
					while (hl[p->j]->down && hl[p->j]->down->i < p->i)
						hl[p->j] = hl[p->j]->down;
					p->down = hl[p->j]->down;
					hl[p->j]->down = p;
				}
				hl[p->j] = p;
				pb = pb->right;
			}
			else if (pa->j < pb->j)
			{
				//沿用A中的结点，但是要更新pre和pa
				pre = pa;
				pa = pa->right;
			}
			else
			{
				//此时A、B在此位置均有非零元，要进行计算
				if (op == 1)
					pa->e += pb->e;
				else
					pa->e -= pb->e;
				pb = pb->right;
				if (pa->e == 0)
				{
					if (pre == NULL)//删除的是本行原来的第一个非零元 
						A->rhead[row] = pa->right;
					else
						pre->right = pa->right;
					OLNode* p = pa;
					pa = pa->right;

					if (A->chead[p->j] == p)
					{
						//删除的是本列第一个非零元
						A->chead[p->j] = p->down;
						hl[p->j] = p->down;
					}
					else
					{
						//通过hl向下找到被删节点的前驱
						while (hl[p->j]->down && hl[p->j]->down->i < p->i)
							hl[p->j] = hl[p->j]->down;
						hl[p->j]->down = p->down;
					}
					free(p);
				}
				else
				{
					pre = pa;//重要，记得更新
					pa = pa->right;
				}
			}
		}
	}
}