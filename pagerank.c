
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
int main(int argc, char* argv[]) 
//int main()
{
	// char* argv[3]={0};
	// int argc=3;
	// argv[1]="URL.txt";
	// argv[2]="top10.txt";
	if (argc != 3) 
	{
		printf("input error\n输入格式./pagerank url.txt top10.txt\n");
		return 0;
	}

	//统计url个数，链接个数
	FILE *fileRead, *fileWrite;
	if ((fileRead = fopen(argv[1], "r")) == NULL) 
	{
		fprintf(stderr, "open url.txt wrong .");
		return 0;
	}
	char str[256];
	int nNodeNum = 0;
	int nEdgeNum = 0;
	int nLine = 0;
	while (!feof(fileRead)) 
	{
		fgets(str, 256 - 1, fileRead);
		if (str[0] == 'h') 
		{
			nNodeNum++;
		} 
		else 
		{
			if (str[0] >= '0' && str[0] <= '9') 
			{
				nEdgeNum++;
			}
		}
		nLine++;
	}
	fclose(fileRead);

	if ((fileRead = fopen(argv[1], "r")) == NULL) 
	{
		fprintf(stderr, "[Error] Cannot open the file");
		exit(1);
	}

	if ((fileWrite = fopen(argv[2], "w+")) == NULL) 
	{
		fprintf(stderr, "[Error] Cannot open the file");
		exit(1);
	}

	//使用CSR压缩方式
	double *val = (double*)calloc(nEdgeNum, sizeof(double));
	int *col = (int*)calloc(nEdgeNum, sizeof(int));
	int *row = (int*)calloc(nNodeNum, sizeof(int));
	row[0] = 0;

	int nFromNode, nToNode;
	int nCurRow = 0;
	int i = 0;
	int j = 0;
	int elrow = 0;
	int curel = 0;
	char url[256];
	char **index = (char**)malloc(sizeof(*index) * (nLine-nEdgeNum));
	for (int i = 0; i < (nLine-nEdgeNum); i++) 
	{
		index[i] = (char*) malloc(sizeof(url));
	}
	//保存url和id对应关系
	int count =0;
	int id;
	int count2 = 0;
	while (!feof(fileRead)) 
	{
		fgets(str, 256 - 1, fileRead);
		sscanf(str, "%s %d", url,&id);
		if (url[0] == 'h')
		{
			if(strlen(url)>=256)
			{
				printf("wrong?:%d\n",id);
				break;
			}
			strcpy(index[id], url);
		} 
		else 
		{
			if( url[0] >=0 && url[0] <= 9 )
			{
				break; 
			}
			else
			{
				//something wrong in url.txt
				char* surl = strstr(url,"http");
				if(surl!=NULL)
				{
					strcpy(index[id],surl);
				}
			}
		}
		count++;
		if(count>=(nLine-nEdgeNum))
		{
			break;
		}
	}
	printf("count: %d\n",count);
    //读入所有链接，保存到压缩矩阵中
	while (!feof(fileRead)) 
	{
		fscanf(fileRead, "%d-%d", &nFromNode, &nToNode);
		if(nFromNode<=0||nToNode<=0||i>=nEdgeNum)
		{
			printf("read links wrong!\n");
			break;
		}
		if (nFromNode > nCurRow) 
		{ 
			curel = curel + elrow;
			for (int k = nCurRow + 1; k <= nFromNode; k++) 
			{
				row[k] = curel;
			}
			elrow = 0;
			nCurRow = nFromNode;
		}
		val[i] = 1.0;
		col[i] = nToNode;
		elrow++;
		i++;
	}
	row[nCurRow + 1] = curel + elrow - 1;
	int out_link[nNodeNum];
	for (i = 0; i < nNodeNum; i++) 
	{
		out_link[i] = 0;
	}
	int rowel = 0;
	//统计每个url出度
	for (i = 0; i < nNodeNum; i++)
	 {
		if (row[i + 1] != 0) 
		{
			rowel = row[i + 1] - row[i];
			out_link[i] = rowel;
		}
	}
	int curcol = 0;
	for (i = 0; i < nNodeNum; i++) 
	{
		rowel = row[i + 1] - row[i];
		for (j = 0; j < rowel; j++) 
		{
			val[curcol] = val[curcol] / out_link[i];
			curcol++;
		}
	}

	//设置阻尼因子
	double d = 0.15;

	double p[nNodeNum];
	for (i = 0; i < nNodeNum; i++) 
	{
		p[i] = 1.0 / nNodeNum;
	}
	int nLoopFlag = 1;
	int k = 0;
	double p_new[nNodeNum];

	//开始迭代计算Ap = p  幂迭代法
	while (nLoopFlag) 
	{
		for (i = 0; i < nNodeNum; i++) 
		{
			p_new[i] = 0.0;
		}

		int rowel = 0;
		int curcol = 0;

		for (i = 0; i < nNodeNum; i++) 
		{
			rowel = row[i + 1] - row[i];
			for (j = 0; j < rowel; j++) 
			{
				p_new[col[curcol]] = p_new[col[curcol]]	+ val[curcol] * p[i];
				curcol++;
			}
		}

		for (i = 0; i < nNodeNum; i++) 
		{
			p_new[i] = (1.0-d) * p_new[i] + (d/nNodeNum);
		}

		double error = 0.0;
		for (i = 0; i < nNodeNum; i++) 
		{
			error = error + fabs(p_new[i] - p[i]);
		}
		//设置迭代终止条件
		if (error < 0.000001) 
		{
			nLoopFlag = 0;
		}
		for (i = 0; i < nNodeNum; i++)
		{
			p[i] = p_new[i];
		}
		//迭代次数
		k = k + 1;
	}

#define TOPNUM 10
	double max_pr[TOPNUM];
	int max_index[TOPNUM];
	for (i = 0; i < TOPNUM; i++) 
	{
		max_pr[i] = 0.0;
		max_index[i] = 0;
	}
	for (i = 0; i < nNodeNum; i++) 
	{
		if (p[i] > max_pr[TOPNUM - 1]) 
		{
			max_pr[TOPNUM - 1] = p[i];
			max_index[TOPNUM - 1] = i;
		}
		//冒泡排序，每次排序10大小数组
		for (int p = 0; p < TOPNUM - 1; p++)
		{
			for (int q = 0; q < TOPNUM - p - 1; q++) 
			{
				if (max_pr[q] < max_pr[q + 1]) 
				{
					double temp = max_pr[q];
					max_pr[q] = max_pr[q + 1];
					max_pr[q + 1] = temp;

					int temp2 = max_index[q];
					max_index[q] = max_index[q + 1];
					max_index[q + 1] = temp2;
				}
			}
		}
	}

	for (i = 0; i < TOPNUM; i++) 
	{
		fprintf(fileWrite, "%s\t%.9lf\n", index[max_index[i]], max_pr[i]);
	}

	fclose(fileRead);
	fclose(fileWrite);
	printf("nodes = %d, edges = %d\n", nNodeNum, nEdgeNum);
	return 0;
}

