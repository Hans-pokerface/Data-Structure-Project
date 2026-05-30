#pragma once
#include"include.h"

typedef struct
{
	int i, j;
	int e;
}Triple;

typedef struct
{
	vector<Triple> data;
	vector<int> rpos;//每行第一个元素所在的位置
	int mu, nu, tu;
}RLSMatrix;

void InitMatrix(RLSMatrix* A)
{
	for (int row = 1; row <= A->mu + 1; row++)
		A->rpos[row] = 0;

	for (int k = 1; k <= A->tu; k++)
	{
		cin >> A->data[k].i;
		A->rpos[A->data[k].i + 1]++;
		cin >> A->data[k].j;
		cin >> A->data[k].e;
	}
	A->rpos[1] = 1;
	for (int row = 2; row <= A->mu + 1; row++)
		A->rpos[row] += A->rpos[row - 1];
}

void MultMatrix(RLSMatrix* A, RLSMatrix* B, RLSMatrix* P)
{
	P->mu = A->mu, P->nu = B->nu, P->tu = 0;
	P->rpos.resize(A->mu + 2);
	P->data.push_back({ 0,0,0 });
	if (A->tu * B->tu != 0)
	{
		vector<int> ctemp(A->nu+1);
		int arow;//对A的每一行进行计算
		for (arow = 1; arow <= A->mu; arow++)
		{
			for (int col = 1; col <= A->nu; col++)
				ctemp[col] = 0;
			//暂存这一行每个位置的计算结果

			int aindex;
			for (aindex = A->rpos[arow]; aindex < A->rpos[arow + 1]; aindex++)
			{
				int brow = A->data[aindex].j;//通过A中元素的列找到B中元素的行
				int bindex;
				for (bindex = B->rpos[brow]; bindex < B->rpos[brow + 1]; bindex++)
					ctemp[B->data[bindex].j] += A->data[aindex].e * B->data[bindex].e;
				//相当于每次取A中的一个元素，乘B中的一行，一次性把A中那个元素的贡献算完                
			}


			for (int col = 1; col <= A->nu; col++)
			{
				if (ctemp[col] != 0)
				{
					P->tu++;
					P->data.push_back({ arow,col,ctemp[col] });
				}
			}
		}
	}
}

void PrintRLSMatrix(RLSMatrix* M)
{
	vector<vector<int>> res(M->mu + 1);
	for (int i = 1; i <= M->mu; i++)
	{
		res[i].resize(M->nu + 1, 0);
	}

	for (int i = 1; i <= M->tu; i++)
	{
		res[M->data[i].i][M->data[i].j] = M->data[i].e;
	}

	cout << "结果如下：" << endl;
	for (int i = 1; i <= M->mu; i++)
	{
		for (int j = 1; j <= M->nu; j++)
		{
			cout << res[i][j] << ' ';
		}
		cout << endl;
	}
}

