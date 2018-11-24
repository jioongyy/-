/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件?
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp输出虚拟机代码
 * fa1.tmp输出源文件及其各行对应的首地址
 * fa2.tmp输出结点
 * fas.tmp输出名字表
 */

#include <stdio.h>
#include "pl0.h"
#include "string.h"

/* 解释执行时使用的栈 */
#define stacksize 500


int main()
{
	bool nxtlev[symnum];	//define symnum 32

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* 输入文件名 */

	fin = fopen(fname, "r");

	if (fin)
	{
		printf("List object code?(Y/N)");   /* 是否输出虚拟机代码 */
		scanf("%s", fname);
		listswitch = (fname[0]=='y' || fname[0]=='Y');

		printf("List symbol table?(Y/N)");  /* 是否输出名字表 */
		scanf("%s", fname);
		tableswitch = (fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* 初始化 */

		err = 0;		//错误数量 
		cc = cx = ll = 0;		/* cc ll getch使用的计数器，cc表示当前字符(ch)的位置 */
								/* cx  虚拟机代码指针, 取值范围[0, vmmax-1]*/
		ch = ' ';				/* 获取字符的缓冲区，getch 使用 */

		if(-1 != getsym())		//确保该函数运行正确
		{
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);	//在nxtlev中存入语句开始符和声明开始符
			nxtlev[period] = true;

			if(-1 == block(0, 0, nxtlev))   /* 调用编译程序,*/
			//传入的参数分别代表当前递归深度为0，名字表尾指针为0，后跟符号集为语句开始符号和声明开始符号集合
			{
				fclose(fa);
				fclose(fa1);
				fclose(fas);
				fclose(fin);
				printf("\n");
				return 0;
			}
			fclose(fa);
			fclose(fa1);
			fclose(fas);

			if (sym != period)
			{
				error(9);
			}

			if (err == 0)
			{
				fa2 = fopen("fa2.tmp", "w");
				interpret();    /* 调用解释执行程序 */
				fclose(fa2);
			}
			else
			{
				printf("Errors in pl/0 program");
			}
		}

		fclose(fin);
	}
	else
	{
		printf("Can't open file!\n");
	}

	printf("\n");
	return 0;
}

/*
* 初始化
char word[keysize][symmaxlen];         保留字 
enum symbol wsym[keysize];      保留字对应的符号值 
enum symbol ssym[256];       单字符的符号值 
char mnemonic[fctnum][5];    虚拟机代码指令名称 
bool declbegsys[symnum];     表示声明开始的符号集合 
bool statbegsys[symnum];     表示语句开始的符号集合 
bool facbegsys[symnum];      表示因子开始的符号集合
*/
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i=0; i<=255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;
	ssym['!'] = qvfan;
	ssym['%'] = qvyv;
	/* 设置保留字名字,按照字母顺序，便于折半查找 */
	/*将保留字放在word数组中，第一维的每一个都是一个保留字*/
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
	strcpy(&(word[4][0]), "else");
	strcpy(&(word[5][0]), "end");
	strcpy(&(word[6][0]), "if");
	strcpy(&(word[7][0]), "odd");
	strcpy(&(word[8][0]), "procedure");
	strcpy(&(word[9][0]), "read");
	strcpy(&(word[10][0]), "then");
	strcpy(&(word[11][0]), "var");
	strcpy(&(word[12][0]), "while");
	strcpy(&(word[13][0]), "write");

	/* 设置保留字符号 */
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = constsym;
	wsym[3] = dosym;
	wsym[4] = elsesym;
	wsym[5] = endsym;
	wsym[6] = ifsym;
	wsym[7] = oddsym;
	wsym[8] = procsym;
	wsym[9] = readsym;
	wsym[10] = thensym;
	wsym[11] = varsym;
	wsym[12] = whilesym;
	wsym[13] = writesym;

	/* 设置指令名称 */
	/*lit,opr等已被设定为枚举类型*/
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

	/* 设置符号集 */
	for (i=0; i<symnum; i++)
	{
		declbegsys[i] = false;	/* 表示声明开始的符号集合 */
		statbegsys[i] = false;	/* 表示语句开始的符号集合 */
		facbegsys[i] = false;	/* 表示因子开始的符号集合 */
	}

	/* 设置声明开始符号集 */
	/*该数组中，声明开始的符号对应的位置被设为true*/
	/*constsym,varsym,procsym对应着数字，为枚举类型*/
	declbegsys[constsym] = true;
	declbegsys[varsym] = true;
	declbegsys[procsym] = true;

	/* 设置语句开始符号集 */
	/*该数组中，语句开始的符号对应的位置被设为true*/
	statbegsys[beginsym] = true;
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;

	/* 设置因子开始符号集 */
	/*该数组中，因子开始的符号对应的位置被设为true*/
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
}

/*
* 用数组实现集合的集合运算
* 返回s数组中这个e位置的值
*/
int inset(int e, bool* s)
{
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)	//在第一个参数的数组中存入所有声明开始符和语句开始符
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]||s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&(!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&s2[i];
	}
	return 0;
}

/*
*   出错处理，打印出错位置和错误编码
*/
void error(int n)
{
	char space[81];
	memset(space, 32, 81);	//全部设为空格

	space[cc-1]=0; //出错时当前符号已经读完，所以cc-1

	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);

	err++;
}

/*
* 漏掉空格，读取一个字符。
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
*/
int getch()
{
	if (cc == ll)	//当访问到整个line缓冲行末尾 
	{
		if (feof(fin))	//文件结束 
		{
			printf("program incomplete");	//程序不完整 
			return -1;
		}
		ll=0;	//末尾ll设置为0 
		cc=0;	//标志读取字符的位置 
		printf("%d ", cx);
		fprintf(fa1,"%d ", cx);
		ch = ' ';
		while (ch != 10)	//ch不等于换行时 
		{
			//fscanf(fin,"%c", &ch)
			//richard
			if (EOF == fscanf(fin,"%c", &ch))	//如果从文件中读到EOF时
			{
				line[ll] = 0;	//将line缓冲行最后一个字符置为0 
				break;			//跳出 
			}
			//end richard
			printf("%c", ch);	//将读入的字符输出 
			fprintf(fa1, "%c", ch);
			line[ll] = ch;
			ll++;
		}
		//注意：每读入一行包括了最后的换行符
		printf("\n");		//一行结束后多一行换行 
		fprintf(fa1, "\n");
	}
	ch = line[cc];
	cc++;
	return 0;
}

/*
* 词法分析，获取一个符号
*调用这个函数后，能够读入下一个字符，并将其含义放入sym变量中
*/
int getsym()
{
	int i,j,k;

	/* the original version lacks "\r", thanks to foolevery */
	while (ch==' ' || ch==10 || ch==13 || ch==9)  /* 忽略空格、换行、回车和TAB */
	{
		//define  getchdo   if(-1 == getch()) return -1
		/*调用getch可使ch字符从line缓冲区中获取字符 */
		getchdo;	
	}
	if (ch>='a' && ch<='z')
	{           /* 名字或保留字以a..z开头 */
		k = 0;
		do {
			if(k<symmaxlen)		//symmaxlen是符号的最大长度 
			{
				a[k] = ch;		//用于临时存放读入的符号字符串 
				k++;
			}
			getchdo;		//向ch中读入一个字符
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
		a[k] = 0;		//该临时符号字符串最后一个置为空字符
		strcpy(id, a);
		i = 0;
		j = keysize-1;		//关键字数目-1 
		do {    /* 搜索当前符号是否为保留字 */
			k = (i+j)/2;	//折半查找
			/*因为关键词为字典序排列，
			所以当读入的保留字字典序小于当前比较的保留字时，
			到小的半部分查找，如果大于当前比较的保留字，到大的半部分查找*/ 
			if (strcmp(id,word[k]) <= 0)	 
			{
				j = k - 1;
			}
			if (strcmp(id,word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i-1 > j)
		{
			sym = wsym[k];/*当前枚举类型置为对应的那个保留字的枚举值*/
		}
		else
		{
			//**********************************************************为什么是数字 
			sym = ident; /* 搜索失败则，是名字或数字 */
		}
	}
	else
	{
		if (ch>='0' && ch<='9')
		{           /* 检测是否为数字：以0..9开头 */
			k = 0;
			num = 0;
			sym = number;		//sym类型置为number
			do {
				num = 10*num + ch - '0';
				k++;
				getchdo;
			} while (ch>='0' && ch<='9'); /* 获取数字的值 */
			k--;
			if (k > nummaxlen)		//如果读入的数字超出了最大位数，则报错
			{
				error(30);
			}
		}
		else
		{
			if (ch == ':')      /* 检测赋值符号 */
			{
				getchdo;
				if (ch == '=')
				{
					sym = becomes;
					getchdo;
				}
				else
				{
					sym = nul;  /* 不能识别的符号 */
				}
			}
			else
			{
				/*
				这里要单独把运算符提出来分析的原因：
				在运算符中有两字符的，所以不能直接用sym = ssym[ch];来定义sym的意义
				而需要对这些双字符的运算符逐个分析，如果不是单个字符的运算符则用sym = ssym[ch];
				*/
				if (ch == '<')      /* 检测小于或小于等于符号 */
				{
					getchdo;
					if (ch == '=')
					{
						sym = leq;		//<=
						getchdo;
					}
					else
					{
						sym = lss;		//<
					}
				}
				else if (ch=='>')        /* 检测大于或大于等于符号 */
				{
					getchdo;
					if (ch == '=')
					{
						sym = geq;		//>=
						getchdo;
					}
					else
					{
						sym = gtr;		//>
					}
				}
				else
				{
					sym = ssym[ch];     /* 当符号不满足上述条件时，全部按照单字符符号处理，即单字符的运算符 */
					//getchdo;
					//richard
					if (sym != period)	
					//如果这个符号不是句号则要向下再读一个字符，防止下次再读入这个字符
					//如果是句号，直接放弃治疗，因为这已经是结尾了，没有下一个字符了
					{
						getchdo;
						/***************************************************************************************/
						/*此处加入加等减等乘等除等的识别***********************************************************/
						/***************************************************************************************/
						if (sym == plus)
						{
							if (ch == '=')
							{
								sym = plusbecomes;
								getchdo;
							}
							else
							{
								
							}
						}
						else if (sym == minus)
						{
							if (ch == '=')
							{
								sym = minusbecomes;
								getchdo;
							}
							else
							{
								
							}
						}
						else if (sym == times)
						{
							if (ch == '=')
							{
								sym = timesbecomes;
								getchdo;
							}
							else
							{
								
							}
						}
						else if (sym == slash)
						{
							if (ch == '=')
							{
								sym = slashbecomes;
								getchdo;
							}
							else if (ch == '*')
							{
								getch();
								while (true)
								{
									while (ch != '*')
									{
										getch();
									}
									getch();
									if (ch == '/')
									{
										break;
									}
								}
								getch();
								getsym();
							}
							else if (ch == '/')
							{
								cc = ll;
								ch = ' ';
								getsym();
							}
							else
							{
								
							}
						}
					}
					//end richard
				}
			}
		}
	}
	return 0;
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= vmmax)
	{
		printf("Program too long"); /* 程序过长 */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}


/*
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集合
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集合
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))		//如果后跟符号不在后跟符号集中，则报错
	{
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{
			getsymdo;
		}
	}
	return 0;
}

/*
* 编译程序主体
*
* lev:    当前分程序所在层
* tx:     名字表当前尾指针
* fsys:   当前模块后跟符号集合
*/
int block(int lev, int tx, bool* fsys)
{
	int i;
	//**************************对应每个虚拟机都有一个名字分配的区域？然后相对于这块区域每个保存的变量有其相对地址？？？
	int dx;                 /* 名字分配到的相对地址 */
	int tx0;                /* 保留初始tx */
	int cx0;                /* 保留初始cx */
	bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空间
							传递给下级函数*/

	dx = 3;					//***************************************************为什么置为3
	tx0 = tx;               /* 记录本层名字的初始位置 */
	table[tx].adr = cx;

	gendo(jmp, 0, 0);		//将虚拟机代码存在code[cx]中

	if (lev > levelmax)		//超过嵌套层数则报错
	{
		error(32);
	}

	do {

		if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
		{
			getsymdo;		//读取const后的字符串，如果正确，应该是读入了一个变量名

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do { */
			constdeclarationdo(&tx, lev, &dx);  /* dx的值会被constdeclaration改变，使用指针 */
												/*将读入的这个常量存在一个名字表中，名字表中存有所有的变量常量过程等*/
			while (sym == comma)			//如果读入了一个逗号
			{
				getsymdo;					//读入逗号后的字符串，如果正确，应该是读入了一个变量名
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)			//如果读入了一个分号
			{
				getsymdo;					//读入结尾的换行符
			}
			else
			{
				error(5);   /*漏掉了逗号或者分号*/
			}
			/* } while (sym == ident); */
		}

		if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
		{
			getsymdo;		//读入var后的字符串

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do {  */
			vardeclarationdo(&tx, lev, &dx);	//将var后的第一个字符串进行判断，如果是ident(标识符)，则将这个标识符存在名字表中
			while (sym == comma)	//如果后跟一个逗号，则继续读入变量名
			{
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)	//如果是分号则读入后跟的换行，并结束
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
			/* } while (sym == ident);  */
		}

		while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
		/*
		过程为：
		先读一个字符串，如果是标识符则记录，并继续。再读入一个字符串，如果是分号，则读入一个换行。
		*/
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
				getsymdo;
			}
			else
			{
				error(4);   /* procedure后应为标识符 */
			}

			if (sym == semicolon)	//读入的是分号则将之后的换行读入
			{
				getsymdo;
			}
			else
			{
				error(5);   /* 漏掉了分号 */
			}

			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev))		//递归调用时，递归深度+1
			{
				return -1;  /* 递归调用 */
			}

			if(sym == semicolon)	//最后读入的如果是分号，则正确
			{
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
				nxtlev[ident] = true;
				nxtlev[procsym] = true;
				testdo(nxtlev, fsys, 6);
			}
			else
			{
				error(5);   /* 漏掉了分号 */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   
	/* 直到没有声明符号，即var,const,procedure等，意味着当前层次下声明的过程变量常量已经结束，开始本层主函数分析 */

	/* 开始生成当前过程代码，
	此时已经生成完这一层函数中定义的变量常量和过程的指令，
	那么cx指向了当前层次主函数该开始的指令位置，所以地址反填*/
	code[table[tx0].adr].a = cx;    
	table[tx0].adr = cx;            /* 将当前层次主函数的第一个指令的地址传给名字表中该过程的adr */
	table[tx0].size = dx;           /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gendo(inte, 0, dx);             /* 生成分配内存代码 */

	if (tableswitch)        /* 输出名字表 */
	{
		printf("TABLE:\n");
		if (tx0+1 > tx)
		{
			printf("    NULL\n");
		}
		for (i=tx0+1; i<=tx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				printf("    %d const %s ", i, table[i].name);
				printf("val=%d\n", table[i].val);
				fprintf(fas, "    %d const %s ", i, table[i].name);
				fprintf(fas, "val=%d\n", table[i].val);
				break;
			case variable:
				printf("    %d var   %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(fas, "    %d var   %s ", i, table[i].name);
				fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedur:
				printf("    %d proc  %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d proc  %s ", i, table[i].name);
				fprintf(fas,"lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			}
		}
		printf("\n");
	}

	/* 语句后跟符号为分号或end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);                       /* 每个过程出口都要使用的释放数据段指令 */
	memset(nxtlev, 0, sizeof(bool)*symnum); /*分程序没有补救集合 */
	testdo(fsys, nxtlev, 8);                /* 检测后跟符号正确性 */
	listcode(cx0);                          /* 输出代码 */
	return 0;
}

/*
* 在名字表中加入一项
* 之后可以通过访问名字表知道其所在层次、种类、值
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1，用于指定变量值的位置
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;	//因为每插入一个名字，名字表内的项肯定会+1，所以名字表表尾指针+1
	strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:  /* 常量名字 */
		if (num > adressmax)	//数字如果大于adressmax则报错
		{
			error(31);  /* 数越界 */
			num = 0;
		}
		table[(*ptx)].val = num;	//const常量的值直接存在val中，而变量不同
		break;
	case variable:  /* 变量名字 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);	//变量的值存在adr对应的相对位置上，不同于const常量直接存值
		(*pdx)++;	//这个变量相对于它的静态链地址的偏移地址+1
		break;
	case procedur:  /*　过程名字　*/
		table[(*ptx)].level = lev;
		break;
	}
}

/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
*/
int position(char* idt, int tx)
{
	int i;
	strcpy(table[0].name, idt);		//设置哨兵，如果没有找到这个名字，则返回0
	i = tx;
	while (strcmp(table[i].name, idt) != 0)
	{
		i--;
	}
	return i;
}

/*
* 常量声明处理
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)	//如果被判断为变量名
	{
		getsymdo;
		if (sym==eql || sym==becomes)
		{
			if (sym == becomes)		//如果读入的是一个赋值运算符
				//const常量定义用=赋值，而不是:=
			{
				error(1);   /* 把=写成了:= */
			}
			getsymdo;	//读入下一个字符串
			if (sym == number)	//如果读入的是数字
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* 常量说明=后应是数字 */
			}
		}
		else
		{
			error(3);   /* 常量说明标识后应是= */
		}
	}
	else
	{
		error(4);   /* const后应是标识 */
	}
	return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	if (sym == ident)
	{
		enter(variable, ptx, lev, pdx); // 填写名字表
		getsymdo;
	}
	else
	{
		error(4);   /* var后应是标识 */
	}
	return 0;
}

/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		for (i=cx0; i<cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fa,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* 语句处理0
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];
	if (sym == ident)   /* 准备按照赋值语句处理 */
	{
		i = position(id, *ptx);
		enum symbol oprsym;		//用于根据是+=,-=等判断如何存入
		if (i == 0)
		{
			error(11);  /* 变量未找到 */
		}
		else
		{
			if(table[i].kind != variable)
			{
				error(12);  /* 赋值语句格式错误 */
				i = 0;
			}
			else
			{
				getsymdo;
				/***********************************************************************/
				/*添加加等于减等于乘等于除等于*/
				/***********************************************************************/
				if(sym == becomes ||
				   sym == plusbecomes ||
				   sym == minusbecomes ||
				   sym == timesbecomes ||
				   sym == slashbecomes)
				{
					oprsym = sym;					//用于保存这个表达式是哪种操作，= += -= *= /=中的一种
					if (oprsym != becomes)
					{
						gendo(lod, lev - table[i].level, table[i].adr);		//如果这个表达式的操作不是赋值，则将该值放于栈顶，用于之后与表达式右式再运算
					}
					getsymdo;
				}
				else
				{
					error(13);  /* 没有检测到赋值符号 */
				}
				/*后跟符号必定要修改*/
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
				if(i != 0)
				{
					if (oprsym == becomes)
					{
						/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					else if (oprsym == plusbecomes)
					{
						gendo(opr, 0, 2);
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					else if (oprsym == minusbecomes)
					{
						gendo(opr, 0, 3);
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					else if (oprsym == timesbecomes)
					{
						gendo(opr, 0, 4);
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					else if (oprsym == slashbecomes)
					{
						gendo(opr, 0, 5);
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					
				}
			}
		}//if (i == 0)
	}
	else if (sym == readsym) /* 准备按照read语句处理 */
	{
		getsymdo;
		if (sym != lparen)
		{
			error(34);  /* 格式错误，应是左括号 */
		}
		else
		{
			do {
				getsymdo;
				if (sym == ident)
				{
					i = position(id, *ptx); /* 名字表查找要读的变量 */
				}
				else
				{
					i = 0;
				}

				if (i == 0)
				{
					error(35);  /* read()中应是声明过的变量名 */
				}
				else if (table[i].kind != variable)
				{
					error(32);	/* read()参数表的标识符不是变量, thanks to amd */
				}
				else
				{
					/***read生成指令*****************************/
					gendo(opr, 0, 16);  /* 生成输入指令，读取值到栈顶 */
					gendo(sto, lev - table[i].level, table[i].adr);   /* 储存到变量 */
				}
				getsymdo;

			} while (sym == comma); /* 一条read语句可读多个变量 */
		}
		if (sym != rparen)
		{
			error(33);  /* 格式错误，应是右括号 */
			while (!inset(sym, fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
			{
				getsymdo;
			}
		}
		else
		{
			getsymdo;
		}
	}
	else if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
	{
		getsymdo;
		if (sym == lparen)
		{
			do {
				getsymdo;
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				nxtlev[rparen] = true;
				nxtlev[comma] = true;       /* write的后跟符号为) or , */
				expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
				gendo(opr, 0, 14);  /* 生成输出指令，输出栈顶的值 */
			} while (sym == comma);
			if (sym != rparen)
			{
				error(33);  /* write()中应为完整表达式 */
			}
			else
			{
				getsymdo;
			}
		}
		gendo(opr, 0, 15);  /* 输出换行 */
	}
	else if (sym == callsym) /* 准备按照call语句处理 */
	{
		getsymdo;
		if (sym != ident)
		{
			error(14);  /* call后应为标识符 */
		}
		else
		{
			i = position(id, *ptx);
			if (i == 0)
			{
				error(11);  /* 过程未找到 */
			}
			else
			{
				if (table[i].kind == procedur)
				{
					gendo(cal, lev - table[i].level, table[i].adr);   /* 生成call指令 */
				}
				else
				{
					error(15);  /* call后标识符应为过程 */
				}
			}
			getsymdo;
		}
	}
	else if (sym == ifsym)   /* 准备按照if语句处理 */
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[thensym] = true;
		nxtlev[dosym] = true;    /* 后跟符号为then或do */
		conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
		if (sym == thensym)
		{
			getsymdo;
		}
		else
		{
			error(16);  /* 缺少then */
		}
		cx1 = cx;   /* 保存当前指令地址 */
		gendo(jpc, 0, 0);   /* 生成条件跳转指令，跳转地址未知，暂时写0 */
		statementdo(fsys, ptx, lev);    /* 处理then后的语句 */
		getsymdo;
		if (sym == elsesym)
		{
			int cx2 = cx;
			gendo(jmp, 0, 0);				/*等待地址反填*/
			code[cx1].a = cx;   /* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址 */
			getsymdo;
			statementdo(fsys, ptx, lev);
			code[cx2].a = cx;
		}
		else
		{
			code[cx1].a = cx;   /* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址 */
		}
		
	}
	else if (sym == beginsym)    /* 准备按照复合语句处理 */
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[semicolon] = true;
		nxtlev[endsym] = true;  /* 后跟符号为分号或end */
		/* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
		statementdo(nxtlev, ptx, lev);

		/*此处这个循环由于将后跟符号设置为所有的声明开始符和分号，所以会不断地读取语句
		直到遇到end则退出循环*/
		while (inset(sym, statbegsys) || sym == semicolon)
		{
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(10);  /* 缺少分号 */
			}
			statementdo(nxtlev, ptx, lev);
		}


		if (sym == endsym)
		{
			getsymdo;
		}
		else
		{
			error(17);  /* 缺少end或分号 */
		}
	}
	else if (sym == whilesym)    /* 准备按照while语句处理 */
	{
		cx1 = cx;   /* 保存判断条件操作的位置 */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[dosym] = true;   /* 后跟符号为do */
		conditiondo(nxtlev, ptx, lev);  /* 调用条件处理 */
		cx2 = cx;   /* 保存循环体的结束的下一个位置 */
		gendo(jpc, 0, 0);   /* 生成条件跳转，但跳出循环的地址未知 */
		if (sym == dosym)
		{
			getsymdo;
		}
		else
		{
			error(18);  /* 缺少do */
		}
		statementdo(fsys, ptx, lev);    /* 循环体 */
		gendo(jmp, 0, cx1); /* 回头重新判断条件 */
		code[cx2].a = cx;   /* 反填跳出循环的地址，与if类似 */
	}
	else
	{
		memset(nxtlev, 0, sizeof(bool)*symnum); /* 语句结束无补救集合 */
		testdo(fsys, nxtlev, 19);   /* 检测语句结束的正确性 */
	}
	return 0;
}

/*
* 表达式处理
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;  /* 用于保存正负号 */
	bool nxtlev[symnum];

	if(sym==plus || sym==minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
	{
		addop = sym;    /* 保存开头的正负号 */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		if (addop == minus)
		{
			gendo(opr,0,1); /* 如果开头为负号生成取负指令 */
		}
	}
	else    /* 此时表达式被看作项的加减 */
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		/*term -> factor 在factor处生成指令，将变量或常量放到栈顶，
		然后term中处理乘除运算，最后在栈顶留下项*/
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		if (addop == plus)
		{
			gendo(opr, 0, 2);   /* 生成加法指令 */
		}
		else
		{
			gendo(opr, 0, 3);   /* 生成减法指令 */
		}
	}
	return 0;
}

/*
* 项处理
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;  /* 用于保存乘除法符号 */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	nxtlev[qvyv] = true;
	factordo(nxtlev, ptx, lev); /* 处理因子 */
	while(sym==times || sym==slash ||sym==qvyv)		//****************加入取余功能
	{
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if(mulop == times)
		{
			gendo(opr, 0, 4);   /* 生成乘法指令 */
		}
		else if(mulop == slash)
		{
			gendo(opr, 0, 5);   /* 生成除法指令 */
		}
		else if (mulop == qvyv)
		{
			gen(opr, 0, 7);		/*生成取余指令*/
		}
	}
	return 0;
}

/*
* 因子处理
*/
int factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);    /* 检测因子的开始符号 */
	/* while(inset(sym, facbegsys)) */  /* 循环直到不是因子开始符号 */
	if(inset(sym,facbegsys))    /* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
	{
		if(sym == ident)    /* 因子为常量或变量 */
		{
			i = position(id, *ptx); /* 查找名字 */
			if (i == 0)
			{
				error(11);  /* 标识符未声明 */
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:  /* 名字为常量 */
					gendo(lit, 0, table[i].val);    /* 直接把常量的值入栈 */
					break;
				case variable:  /* 名字为变量 */
					gendo(lod, lev-table[i].level, table[i].adr);   /* 找到变量地址并将其值入栈 */
					break;
				case procedur:  /* 名字为过程 */
					error(21);  /* 不能为过程 */
					break;
				}
			}
			getsymdo;
		}
		else if (sym == number)   /* 因子为数 */
		{
			if (num > adressmax)
			{
				error(31);
				num = 0;
			}
			gendo(lit, 0, num);
			getsymdo;
		}
		else if (sym == lparen)  /* 因子为表达式 */
		{
			getsymdo;
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[rparen] = true;
			expressiondo(nxtlev, ptx, lev);
			if (sym == rparen)
			{
				getsymdo;
			}
			else
			{
				error(22);  /* 缺少右括号 */
			}
		}
		testdo(fsys, facbegsys, 23);    /* 因子后有非法符号 */
	}
	return 0;
}

/*
* 条件处理
*/
int condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if(sym == oddsym)   /* 准备按照odd运算处理 */
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);   /* 生成odd指令 */
	}
	else
	{
		/* 逻辑表达式处理 */
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsymdo;
			expressiondo(fsys, ptx, lev);
			switch (relop)
			{
			case eql:
				gendo(opr, 0, 8);
				break;
			case neq:
				gendo(opr, 0, 9);
				break;
			case lss:
				gendo(opr, 0, 10);
				break;
			case geq:
				gendo(opr, 0, 11);
				break;
			case gtr:
				gendo(opr, 0, 12);
				break;
			case leq:
				gendo(opr, 0, 13);
				break;
			}
		}
	}
	return 0;
}

/*
* 解释程序
*/
void interpret()
{
	int oprPoint, oprBaseAdr, topPoint;    /* 指令指针，指令基址，栈顶指针 */
	struct instruction nowOpr;   /* 存放当前指令 */
	int storestack[stacksize];   /* 栈 */

	printf("start pl0\n");
	topPoint = 0;
	oprBaseAdr = 0;
	oprPoint = 0;
	storestack[0] = storestack[1] = storestack[2] = 0;
	do {
		nowOpr = code[oprPoint];    /* 读当前指令 */
		oprPoint++;
		switch (nowOpr.f)
		{
		case lit:   /* 将a的值取到栈顶 */
			storestack[topPoint] = nowOpr.a;
			topPoint++;
			break;
		case opr:   /* 数学、逻辑运算 */
			switch (nowOpr.a)
			{
			case 0:
				topPoint = oprBaseAdr;
				oprPoint = storestack[topPoint+2];
				oprBaseAdr = storestack[topPoint+1];
				break;
			case 1:
				storestack[topPoint-1] = -storestack[topPoint-1];
				break;
			case 2:
				topPoint--;
				storestack[topPoint-1] = storestack[topPoint-1]+storestack[topPoint];
				break;
			case 3:
				topPoint--;
				storestack[topPoint-1] = storestack[topPoint-1]-storestack[topPoint];
				break;
			case 4:
				topPoint--;
				storestack[topPoint-1] = storestack[topPoint-1]*storestack[topPoint];
				break;
			case 5:
				topPoint--;
				storestack[topPoint-1] = storestack[topPoint-1]/storestack[topPoint];
				break;
			case 6:
				storestack[topPoint-1] = storestack[topPoint-1]%2;
				break;
			case 7:					//增加一个取余指令
				topPoint--;
				storestack[topPoint-1] = storestack[topPoint-1]% storestack[topPoint];
				break;
			case 8:
				topPoint--;
				storestack[topPoint-1] = (storestack[topPoint-1] == storestack[topPoint]);
				break;
			case 9:
				topPoint--;
				storestack[topPoint-1] = (storestack[topPoint-1] != storestack[topPoint]);
				break;
			case 10:
				topPoint--;
				storestack[topPoint-1] = (storestack[topPoint-1] < storestack[topPoint]);
				break;
			case 11:
				topPoint--;
				storestack[topPoint-1] = (storestack[topPoint-1] >= storestack[topPoint]);
				break;
			case 12:
				topPoint--;
				storestack[topPoint-1] = (storestack[topPoint-1] > storestack[topPoint]);
				break;
			case 13:
				topPoint--;
				storestack[topPoint-1] = (storestack[topPoint-1] <= storestack[topPoint]);
				break;
			case 14:
				printf("%d", storestack[topPoint-1]);
				fprintf(fa2, "%d", storestack[topPoint-1]);
				topPoint--;
				break;
			case 15:
				printf("\n");
				fprintf(fa2,"\n");
				break;
			case 16:
				printf("?");
				fprintf(fa2, "?");
				scanf("%d", &(storestack[topPoint]));
				fprintf(fa2, "%d\n", storestack[topPoint]);
				topPoint++;
				break;
			}
			break;
		case lod:   /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			storestack[topPoint] = storestack[base(nowOpr.l,storestack,oprBaseAdr)+nowOpr.a];
			topPoint++;
			break;
		case sto:   /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			topPoint--;
			storestack[base(nowOpr.l, storestack, oprBaseAdr) + nowOpr.a] = storestack[topPoint];
			break;
		case cal:   /* 调用子过程 */
			storestack[topPoint] = base(nowOpr.l, storestack, oprBaseAdr); /* 将父过程基地址入栈 */
			storestack[topPoint+1] = oprBaseAdr; /* 将本过程基地址入栈，此两项用于base函数 */
			storestack[topPoint+2] = oprPoint; /* 将当前指令指针入栈 */
			oprBaseAdr = topPoint;  /* 改变基地址指针值为新过程的基地址 */
			oprPoint = nowOpr.a;    /* 跳转 */
			break;
		case inte:  /* 分配内存 */
			topPoint += nowOpr.a;
			break;
		case jmp:   /* 直接跳转 */
			oprPoint = nowOpr.a;
			break;
		case jpc:   /* 条件跳转 */
			topPoint--;
			if (storestack[topPoint] == 0)
			{
				oprPoint = nowOpr.a;
			}
			break;
		}
	} while (oprPoint != 0);
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, int* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}

