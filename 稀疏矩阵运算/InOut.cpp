#include"include.h"
#include"OLMatrix.h"

void CreateOLMatrix(Crosslist* M)
{
	int row = 0;
	OLNode* currentRow = NULL;
	vector<OLNode*> currentCol(M->nu + 1, NULL);

	for (int k = 0; k < M->tu; k++)
	{
		int icur, jcur, ecur;
		cin >> icur >> jcur >> ecur;
		OLNode* p = new OLNode;
		p->i = icur; p->j = jcur; p->e = ecur;
		p->down = NULL;	p->right = NULL;

		if (icur != row)
		{
			row = icur;
			M->rhead[icur] = p;
			currentRow = p;
		}
		else
		{
			currentRow->right = p;
			currentRow = p;
		}
		
		if (currentCol[jcur] == NULL)
		{
			M->chead[jcur] = p;
			currentCol[jcur] = p;
		}
		else
		{
			currentCol[jcur]->down = p;
			currentCol[jcur] = p;
		}
	}

}

void PrintOLMatrix(Crosslist* M)
{
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