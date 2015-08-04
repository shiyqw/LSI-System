/*
	LSI Word Searching System by 1200012741
	    --Version 1.0

	Program Features:
	   1, All calculation packed in C program. You don't have to preprocess it in Matlab.
	   2, Comprise both poem searching system and synonym searching system.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "engine.h"


#define WORDNUM 4862
#define POEMSNUM 542
#define LINEMAX 100
#define QUERYMAX 300
//#define RELATIONNUM 22109

FILE * inword, * inpoems;
Engine *ep;

double vquery[WORDNUM]; 
int vresult[POEMSNUM];
double rvresult[POEMSNUM];
int vsyn[WORDNUM];
double rvsyn[WORDNUM];
double vrelation[POEMSNUM];
double srelation[WORDNUM];

void Welcome_LSI();  
int Initialize_LSI();  
int Connect_Engine_LSI(); 
int Get_Data_LSI();
int Get_Query_LSI(); 
void Work_LSI(); 
void Search_LSI();  
void Show_Synonym_LSI(); 
void Search_Synonym_LSI();
void Show_Word_LSI(int);
int Feedback_LSI(int); 
void Show_Poem_LSI(int);
void Show_Result_LSI(int * , int);

void Welcome_LSI()
{
	printf("LSI Word Searching System -- Version 1.0\n\n");
	printf("Please be sure that you have set the environment with VS and Matlab well\n");
	printf("Please put the data.xlsx into your Matlab catalog.\n");
	printf("Press Enter to start.\n");
	getchar();
}

int Connect_Engine_LSI()
{
	if (!(ep = engOpen(NULL)))  
	{   
		return 0;  
	}
	engEvalString(ep, "X=xlsread('data.xlsx');"); 
	engEvalString(ep, "Y=sparse(X(:,2),X(:,1),X(:,3));");   
	engEvalString(ep, "[S,U,V]=svds(Y,100);");  
	return 1;
}

int Get_Data_LSI()
{
	inword = fopen("id_word.txt","r");
	inpoems = fopen("id_poems.txt","r");
	if (!inword || !inpoems) return 0;
	return 1;
}

int Initialize_LSI()
{
	printf("Initializing...\n");
	printf("Connecting with math engine...");
	if ( Connect_Engine_LSI() ) printf("OK\n");
	else 
	{
		printf("ERROR\n");
		return 0;
	}
	printf("Getting Data...");
	if ( Get_Data_LSI() ) printf("OK\n");
	else 
	{
		printf("ERROR\n");
		return 0;
	}
	return 1;
}

void Work_LSI()
{
	memset(vquery , 0 , sizeof(vquery));
	memset(vresult , 0 , sizeof(vresult));
	memset(rvresult , 0 , sizeof(rvresult));
	memset(vsyn , 0 , sizeof(vsyn));
	memset(rvsyn , 0 , sizeof(rvsyn));
	memset(vrelation , 0 , sizeof(vrelation));
	memset(srelation , 0 , sizeof(srelation));
	if (!Get_Query_LSI()) 
	{
		printf ("Invalid input!\n");
		Work_LSI();
	}
	else 
	{
		Search_LSI();
		for (int i=0 ; i <= POEMSNUM - 1 ; ++i)
		{
			vresult[i]= (int) rvresult[i];
		}
		Show_Result_LSI(vresult, 0);
		if (Feedback_LSI(0)) Work_LSI();
	}
}


int Get_Query_LSI()
{
	int bo = 0;
	int i, j;
	char line[LINEMAX+1];
	char qword[5], query[QUERYMAX];
	printf("input your query:\n");
	gets(query);
	while (query[0] == '\0') gets(query);
	memset(qword, 0, sizeof(qword)); j = 0;
	for (i = 0;i <= strlen(query); ++i)
	{
		if (query[i]==' ' || query[i]=='\n' || query[i]=='\0') 
		{
			rewind (inword);
			while (fgets(line,LINEMAX,inword) != NULL) 
			{
				char * ptr;
				ptr = strstr(line, qword);
				if (!ptr || ptr[strlen(qword)] != ',' || ptr != line) continue;
				else
				{
					ptr = ptr + strlen(qword) + 1;
					int word_id = 0 , i;
					for ( i=0 ; i <= strlen(ptr)-1 ; ++i )
					{
						if (ptr[i] >= '0' && ptr[i] <= '9') word_id = word_id*10 + ptr[i] - '0';
						else break;
					}
					vquery[word_id - 1] = 1;
					bo = 1;
					break;
				}
			}
			memset(qword, 0, sizeof(qword));
			j=0;
		}
		else 
		{
			qword[j++] = query[i];
		}
	}
	return bo;
}



void Search_LSI()
{
	mxArray * vquery_m = mxCreateDoubleMatrix(1,WORDNUM, mxREAL);  
	memcpy(mxGetPr(vquery_m), vquery, WORDNUM * sizeof(double));  
	engPutVariable(ep, "Q",vquery_m);
	engEvalString(ep, "B=zeros(542,1);");
	engEvalString(ep, "A = Q*(S)*(inv(U));");
	engEvalString(ep, "k = 1;");
	for (int i=1 ;i<= WORDNUM ; ++i)
	{
		engEvalString(ep, "B(k,1) = dot(A,V(k,:)) / (norm(A)*norm(V(k,:)) ) ;");
		engEvalString(ep, "k = k + 1;");
	}
	engEvalString(ep, "[L,IND]=sort(B,'descend');" );	
    mxArray * vresult_m = engGetVariable(ep, "IND" );
	mxArray * vrelation_m = engGetVariable(ep, "L") ;
	memcpy(rvresult, mxGetPr(vresult_m) , POEMSNUM * sizeof(double));
	memcpy(vrelation, mxGetPr(vrelation_m) , POEMSNUM * sizeof(double));
	mxDestroyArray(vquery_m);
}

int Feedback_LSI(int x)
{
	printf("\n");
	printf("Press 1 to see next ten result.\n");
	printf("Press 2 to see the word that may help you.\n");
	printf("Press 3 to input a new query.\n");
	printf("Press 0 to EXIT.\n");
	int k;
	scanf("%d",&k);
	printf("\n");
	switch (k)
	{
	case 0: 
		return 0;
	case 1: 
		{
			Show_Result_LSI(vresult, x+10);
			return (Feedback_LSI(x + 10));
		}
	case 2:
		{
			Search_Synonym_LSI();
			Show_Synonym_LSI();
			printf("Press 3 to input a new query.\n");
			printf("Press 0 to EXIT.\n");
			int kk;
			scanf("%d",&kk);
			if (kk == 0) return 0;
			if (kk == 3) 
			{
				return 1;
			}
		}
	case 3:
		{
			return 1;
		}
	default:
		return 0;
	} 
}


void Search_Synonym_LSI()
{
	double rvpoems[POEMSNUM];
	memset(rvpoems , 0 , sizeof(rvpoems));
	for (int i = 0 ; i <= 19 ; ++i)
	{
		rvpoems[ vresult[i] -1] = 1;
	}
	mxArray * vpoems_m = mxCreateDoubleMatrix(1,POEMSNUM, mxREAL);  
	memcpy(mxGetPr(vpoems_m), rvpoems, POEMSNUM * sizeof(double));  
	engPutVariable(ep, "R",vpoems_m);
	engEvalString(ep, "BS=zeros(4862,1);");
	engEvalString(ep, "AS = R*(V)*(inv(U));");
	engEvalString(ep, "k = 1;");
	for (int i=1 ;i<= WORDNUM ; ++i)
	{
		engEvalString(ep, "BS(k,1) = dot(AS,S(k,:)) / (norm(AS)*norm(S(k,:)) );");
		engEvalString(ep, "k = k + 1;");
	}
	engEvalString(ep, "[LS,INDS]=sort(BS,'descend');" );	
	mxArray * vsyn_m = engGetVariable(ep, "INDS" );
	mxArray * srelation_m = engGetVariable(ep , "LS");
	memcpy(rvsyn, mxGetPr(vsyn_m) , WORDNUM * sizeof(double));
	memcpy(srelation , mxGetPr(srelation_m) , WORDNUM * sizeof(double));
	mxDestroyArray(vpoems_m);
	mxDestroyArray(vsyn_m);
}

void Show_Synonym_LSI()
{
	for (int i=0 ; i <= WORDNUM - 1 ; ++i)
	{
		vsyn[i]= (int) rvsyn[i];
	}
	printf("These word may help you:\n");
	for (int i = 0 , k = 0; i <= 9 + k ; ++i)
	{
		if (vquery[vsyn[i] - 1] == 0) 
		{
			Show_Word_LSI(vsyn[i]);
			printf("similarity: %lf\n",srelation[i]);
		}
		else
		{
			++ k;
		}
	//	else --i;
	}
	printf("\n");
}

void Show_Word_LSI(int n)
{
	int l=0;
	char st[5];
	memset(st , 0 , sizeof(st));
	for (int e = 1 ; e <= 10000 ; e *= 10)
	{
		if (e <= n) ++l;
		else break;
	}
	for (int i = 0; i <= l-1 ; ++i)
	{
		st[l - 1 - i] = n % 10 + '0';
		n = n / 10;
	}
	char line[LINEMAX + 1];
	rewind(inword);
	while (fgets(line,LINEMAX,inword) != NULL)
	{
		int loc = 0;
		int ll = strlen(line);
		if (line[ll - 1] == '\n') line[ll - 1] = 0;
		char * ptr = line;
		while (*ptr != ',') { ++ptr; ++loc;}
		if (strcmp(st,ptr + 1) != 0) continue;
		else
		{
			for (int j = 0 ; j <= loc-1 ; ++j) printf("%c",line[j]);
			break;
		}
	}
}


void Show_Result_LSI(int *p, int x)
{
	int i;
	for (i = x;i <= x + 9;++i)
	{
		if (i>=POEMSNUM-1)
		{
			printf("No More Result.\n");
			break;
		}
		else 
		{
			printf("No. %d \n", i+1);
			Show_Poem_LSI(p[i]);
			printf("Relation: %lf\n",vrelation[i]);
			printf("\n");
		}
	}
}

void Show_Poem_LSI(int n)
{
	int i;
	int dig[4];
	memset(dig, 0, sizeof(dig));
	for (i=3; i>=0; --i)
	{
		dig[i] = n % 10;
		n = n / 10;
	}
	char line[LINEMAX + 1];
	rewind(inpoems);
	while (fgets(line,LINEMAX,inpoems) != NULL)
	{
		int val = 0, j = 0;
		for (i = 0; i <= 3; ++i)
		{
			if (dig[i] == 0 && val == 0) continue;
			else 
			{
				val = 1;
				if (line[j] == dig[i] + '0')
				{
					++j;
				}
				else break;
			}
		}
		if ( i == 4 && val == 1)
		{
			if (line[j] == ',') 
			{
				int k = j;
				while (( line[k] >= '0' &&  line[k] <= '9') || ( line[k] == ',') 
					|| (  line[k] == '$') || ( line[k] == '#') || line[k] == '-' || line[k] == '%') ++k;
				if (k < strlen(line) && k >= 1)
				{
					if (line[k-1] == '#') printf("Title:   ");
					if (line[k-1] == '$') printf("Author:  ");
					if (line[k-1] == '%') printf("Prologue:  ");
					printf("%s",line+k);
				}
			}
		}
	}
}

int main()
{
	 Welcome_LSI();
	 if (Initialize_LSI()) 
	 {
		 printf("Initialization Completed\n\n");
	 }
	 else
	 {
		 printf("Initialization Failed\n");
		 return 0;
	 }
//	 for (int i = 0; i <= 10 ; ++i) rvsyn[i] = 100*i+77;
//	 Show_Synonym_LSI();
//	 Show_Word_LSI(777);
	 Work_LSI();
	 printf("Thanks for your searching! See you next time!\n");
	 fclose(inword);
	 fclose(inpoems);
	 engClose(ep);
	 return 0;
}



