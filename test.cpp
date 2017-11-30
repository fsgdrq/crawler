#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <queue>
#include <iostream>
using namespace std;
queue<char*> asd;
void func(char* asdd);
int main()
{
	char* aaa = (char*)malloc(100*sizeof(char));
	strcpy(aaa,"1232323");
	func(aaa);
	char* sdf = asd.front();
	asd.pop();
	cout<<sdf;
	free(sdf);
}

void func(char* asdd)
{
	char* ab = (char*)malloc(100*sizeof(char));
	strcpy(ab,"qweqweqwe");
	asd.push(ab);
	return;
}
