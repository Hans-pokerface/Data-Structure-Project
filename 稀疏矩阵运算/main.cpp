//g++ main.cpp include.h OLMatrix.h RLSMatrix.h -o Matrix
#include"include.h"
#include"OLMatrix.h"

int main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	
	while (1)
	{
		cout << "请输入第1个矩阵的行数、列数、非零元数：" << endl;
		Crosslist* Src = new Crosslist;
		cin >> Src->mu >> Src->nu >> Src->tu;
		Src->rhead.resize(Src->mu + 1, NULL);
		Src->chead.resize(Src->nu + 1, NULL);

		cout << "以三元组形式输入第1个矩阵的数据(行序)：" << endl;
		CreateOLMatrix(Src);

		Crosslist* Tar;
		int num = 1;

		while (1)
		{
			int op;//运算类型（1-加；2-减；3-乘）
			cout << "请输入运算类型（1-加；2-减；3-乘；0-退出运算）：" << endl;
			cin >> op;

			num++;
			cout << "请输入第" << num << "个矩阵的行数、列数、非零元数：" << endl;
			Tar = new Crosslist;
			cin >> Tar->mu >> Tar->nu >> Tar->tu;
			Tar->rhead.resize(Tar->mu + 1, NULL);
			Tar->chead.resize(Tar->nu + 1, NULL);

			if (op == 1 || op == 2)
			{
				/*对数据进行检查*/
				if (Tar->mu != Src->mu || Tar->nu != Src->nu)
				{
					cout << "这组数据不符合要求，请重新输入。";
					num--;
					delete Tar;
					continue;
				}

				cout << "以三元组形式输入第" << num << "个矩阵的数据(行序)：" << endl;
				CreateOLMatrix(Tar);
				AddMinusOLMatrix(Src, Tar, op);				
				PrintOLMatrix(Src);

				FreeCrosslist(Tar);
			}

			else if (op == 3)
			{
				/*对数据进行检查*/
				if (Tar->mu != Src->nu)
				{
					cout << "这组数据不符合要求，请重新输入。";
					num--;
					delete Tar;
					continue;
				}

				cout << "以三元组形式输入第" << num << "个矩阵的数据(行序)：" << endl;
				CreateOLMatrix(Tar);

				Crosslist* Res = new Crosslist;

				MultiOLMatrix(Src, Tar, Res);
				PrintOLMatrix(Res);

				FreeCrosslist(Src);
				FreeCrosslist(Tar);
				Src = Res;
			}

			else if (op == 0)
			{
				break;
			}

			else
			{
				cout << "操作类型不合法！" << endl;
				continue;
			}

			cout << endl;
		}

		FreeCrosslist(Src);
	}
}