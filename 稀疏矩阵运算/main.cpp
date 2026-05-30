//g++ main.cpp include.h OLMatrix.h RLSMatrix.h -o Matrix
#include"include.h"
#include"OLMatrix.h"
#include"RLSMatrix.h"


int main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	
	while (1)
	{
		int op;//运算类型（1-加；2-减；3-乘）
		cout << "请输入运算类型（1-加；2-减；3-乘；0-退出）：" << endl;
		cin >> op;

		if (op == 1 || op == 2)
		{
			vector<Crosslist*> Minfo;
			int num;
			cout << "请输入矩阵的个数：" << endl;
			cin >> num;

			for (int i = 0; i < num; i++)
			{
				cout << "请输入第" << i+1 << "个矩阵的行数、列数、非零元数：" << endl;
				Crosslist* temp = new Crosslist;
				cin >> temp->mu >> temp->nu >> temp->tu;
				temp->rhead.resize(temp->mu + 1);
				temp->chead.resize(temp->nu + 1);
				Minfo.push_back(temp);

				/*对数据进行检查*/
				if (i > 0 && (Minfo[i - 1]->mu != Minfo[i]->mu || Minfo[i - 1]->nu != Minfo[i]->nu))
				{
					cout << "这组数据不符合要求，请重新输入。";
					i--;
					Minfo.pop_back();
				}

			}

			cout << "以三元组形式输入第一个矩阵的数据,行列下标均从1开始：" << endl;
			CreateOLMatrix(Minfo[0]);

			for (int i = 1; i < num; i++)
			{
				cout << "以三元组形式输入第" << i + 1 << "个矩阵的数据,行列下标均从1开始：" << endl;

				CreateOLMatrix(Minfo[i]);

				AddMinusOLMatrix(Minfo[0], Minfo[i], op);
			}
			PrintOLMatrix(Minfo[0]);	

			/*清理空间*/
			for (auto p : Minfo)
			{
				for (auto node : p->rhead) delete node;
				for (auto node : p->chead) delete node;
				delete p;
			}
			Minfo.clear();
		}

		else if (op == 3)
		{
			vector<RLSMatrix*> Minfo;
			int num;
			cout << "请输入矩阵的个数：" << endl;
			cin >> num;

			for (int i = 0; i < num; i++)
			{
				cout << "请输入第" << i + 1 << "个矩阵的行数、列数、非零元数：" << endl;
				RLSMatrix* temp = new RLSMatrix;
				cin >> temp->mu >> temp->nu >> temp->tu;
				temp->rpos.resize(temp->mu + 2);
				temp->data.resize(temp->tu + 1);
				Minfo.push_back(temp);

				/*对数据进行检查*/
				if (i > 0 && Minfo[i - 1]->nu != Minfo[i]->mu )
				{
					cout << "这组数据不符合要求，请重新输入。";
					i--;
					Minfo.pop_back();
				}

			}

			cout << "以三元组形式输入第一个矩阵的数据,行列下标均从1开始：" << endl;
			InitMatrix(Minfo[0]);

			RLSMatrix* tempSrc = Minfo[0];
			RLSMatrix* tempTar = new RLSMatrix;
			for (int i = 1; i < num; i++)
			{
				cout << "以三元组形式输入第" << i + 1 << "个矩阵的数据,行列下标均从1开始：" << endl;

				InitMatrix(Minfo[i]);

				MultMatrix(tempSrc, Minfo[i], tempTar);
				delete tempSrc;
				tempSrc = tempTar;
				tempTar = new RLSMatrix;
			}
			PrintRLSMatrix(tempSrc);

			/*清理空间*/
			for (auto p : Minfo)
				delete p;
			Minfo.clear();
		}
				
		else if (op == 0)
		{
			return 0;
		}

		else
		{
			cout << "操作类型不合法！" << endl;
			continue;
		}

		cout << endl;
	}
}