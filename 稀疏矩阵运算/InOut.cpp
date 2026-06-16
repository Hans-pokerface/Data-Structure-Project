#include"include.h"
#include"OLMatrix.h"

//CreateOLMatrix():根据三元组的输入初始化十字链表
//要求以行序输入三元组，时间复杂度为O(t)
void CreateOLMatrix(Crosslist* M)
{
	int row = 0;
	OLNode* currentRow = NULL;//行的当前指针
	vector<OLNode*> currentCol(M->nu + 1, NULL);//每一列的当前指针

	for (int k = 0; k < M->tu; k++)
	{
		int icur, jcur, ecur;
		cin >> icur >> jcur >> ecur;
		OLNode* p = new OLNode;
		p->i = icur; p->j = jcur; p->e = ecur;
		p->down = NULL;	p->right = NULL;

		if (icur != row)//换行后更新rhead
		{
			row = icur;
			M->rhead[icur] = p;			
		}
		else
		{
			currentRow->right = p;
		}
		currentRow = p;//更新行指针
		
		if (currentCol[jcur] == NULL)//该列还没有非零元，更新chead
		{
			M->chead[jcur] = p;			
		}
		else
		{
			currentCol[jcur]->down = p;
		}
		currentCol[jcur] = p;//更新列指针
	}
}

//PrintOLMatrix():根据十字链表生成二维数组并打印
void PrintOLMatrix(Crosslist* M)
{
	//结果暂存于一个二维数组
	vector<vector<int>> res(M->mu + 1);
	for (int i = 1; i <= M->mu; i++)
	{
		res[i].resize(M->nu + 1, 0);
		OLNode* p = M->rhead[i];
		while (p)
		{
			res[i][p->j] = p->e;
			p = p->right;
		}
	}

	cout << "计算结果如下：" << endl;
	for (int i = 1; i <= M->mu; i++)
	{
		for (int j = 1; j <= M->nu; j++)
		{
			cout << res[i][j] << ' ';
		}
		cout << endl;
	}
}


//FreeCrosslist():把一个十字链表中的各个OLNode结点都清除
//通过rhead数组遍历各个节点
void FreeCrosslist(Crosslist* M)
{
	if (!M)	return;

	for (int i = 1; i <= M->mu; i++)
	{
		OLNode* p = M->rhead[i];
		while (p)
		{
			OLNode* temp = p->right;
			delete p;
			p = temp;
		}
	}
	delete M;
}