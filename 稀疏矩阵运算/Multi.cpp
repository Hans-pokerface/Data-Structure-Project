#include"include.h"
#include"OLMatrix.h"

void MultiOLMatrix(Crosslist* A, Crosslist* B, Crosslist* M)
{
	M->mu = A->mu;	M->nu = B->nu;	M->tu = 0;
	M->rhead.resize(M->mu + 1, NULL);
	M->chead.resize(M->nu + 1, NULL);

	int row = 0;
	OLNode* currentRow = NULL;
	vector<OLNode*> currentCol(M->nu + 1, NULL);

	for (int i = 1; i <= M->mu; i++)
	{
		vector<int> ctemp(M->nu + 1, 0);
		OLNode* Atemp = A->rhead[i];
		while (Atemp)
		{
			int Adata = Atemp->e;
			OLNode* Btemp = B->rhead[Atemp->j];
			while (Btemp)
			{
				int Bdata = Btemp->e;
				ctemp[Btemp->j] += Adata * Bdata;
				Btemp = Btemp->right;
			}
			Atemp = Atemp->right;
		}

		for (int k = 1; k <= M->nu; k++)
		{
			if (ctemp[k])
			{
				M->tu++;
				int icur, jcur, ecur;
				icur = i;	jcur = k;	ecur = ctemp[k];
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
	}

}