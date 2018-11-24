/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * ʹ�÷�����
 * ���к�����PL/0Դ�����ļ�?
 * �ش��Ƿ�������������
 * �ش��Ƿ�������ֱ�
 * fa.tmp������������
 * fa1.tmp���Դ�ļ�������ж�Ӧ���׵�ַ
 * fa2.tmp������
 * fas.tmp������ֱ�
 */

#include <stdio.h>
#include "pl0.h"
#include "string.h"

/* ����ִ��ʱʹ�õ�ջ */
#define stacksize 500


int main()
{
	bool nxtlev[symnum];	//define symnum 32

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* �����ļ��� */

	fin = fopen(fname, "r");

	if (fin)
	{
		printf("List object code?(Y/N)");   /* �Ƿ������������� */
		scanf("%s", fname);
		listswitch = (fname[0]=='y' || fname[0]=='Y');

		printf("List symbol table?(Y/N)");  /* �Ƿ�������ֱ� */
		scanf("%s", fname);
		tableswitch = (fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* ��ʼ�� */

		err = 0;		//�������� 
		cc = cx = ll = 0;		/* cc ll getchʹ�õļ�������cc��ʾ��ǰ�ַ�(ch)��λ�� */
								/* cx  ���������ָ��, ȡֵ��Χ[0, vmmax-1]*/
		ch = ' ';				/* ��ȡ�ַ��Ļ�������getch ʹ�� */

		if(-1 != getsym())		//ȷ���ú���������ȷ
		{
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);	//��nxtlev�д�����俪ʼ����������ʼ��
			nxtlev[period] = true;

			if(-1 == block(0, 0, nxtlev))   /* ���ñ������,*/
			//����Ĳ����ֱ����ǰ�ݹ����Ϊ0�����ֱ�βָ��Ϊ0��������ż�Ϊ��俪ʼ���ź�������ʼ���ż���
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
				interpret();    /* ���ý���ִ�г��� */
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
* ��ʼ��
char word[keysize][symmaxlen];         ������ 
enum symbol wsym[keysize];      �����ֶ�Ӧ�ķ���ֵ 
enum symbol ssym[256];       ���ַ��ķ���ֵ 
char mnemonic[fctnum][5];    ���������ָ������ 
bool declbegsys[symnum];     ��ʾ������ʼ�ķ��ż��� 
bool statbegsys[symnum];     ��ʾ��俪ʼ�ķ��ż��� 
bool facbegsys[symnum];      ��ʾ���ӿ�ʼ�ķ��ż���
*/
void init()
{
	int i;

	/* ���õ��ַ����� */
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
	/* ���ñ���������,������ĸ˳�򣬱����۰���� */
	/*�������ַ���word�����У���һά��ÿһ������һ��������*/
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

	/* ���ñ����ַ��� */
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

	/* ����ָ������ */
	/*lit,opr���ѱ��趨Ϊö������*/
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

	/* ���÷��ż� */
	for (i=0; i<symnum; i++)
	{
		declbegsys[i] = false;	/* ��ʾ������ʼ�ķ��ż��� */
		statbegsys[i] = false;	/* ��ʾ��俪ʼ�ķ��ż��� */
		facbegsys[i] = false;	/* ��ʾ���ӿ�ʼ�ķ��ż��� */
	}

	/* ����������ʼ���ż� */
	/*�������У�������ʼ�ķ��Ŷ�Ӧ��λ�ñ���Ϊtrue*/
	/*constsym,varsym,procsym��Ӧ�����֣�Ϊö������*/
	declbegsys[constsym] = true;
	declbegsys[varsym] = true;
	declbegsys[procsym] = true;

	/* ������俪ʼ���ż� */
	/*�������У���俪ʼ�ķ��Ŷ�Ӧ��λ�ñ���Ϊtrue*/
	statbegsys[beginsym] = true;
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;

	/* �������ӿ�ʼ���ż� */
	/*�������У����ӿ�ʼ�ķ��Ŷ�Ӧ��λ�ñ���Ϊtrue*/
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
}

/*
* ������ʵ�ּ��ϵļ�������
* ����s���������eλ�õ�ֵ
*/
int inset(int e, bool* s)
{
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)	//�ڵ�һ�������������д�������������ʼ������俪ʼ��
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
*   ��������ӡ����λ�úʹ������
*/
void error(int n)
{
	char space[81];
	memset(space, 32, 81);	//ȫ����Ϊ�ո�

	space[cc-1]=0; //����ʱ��ǰ�����Ѿ����꣬����cc-1

	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);

	err++;
}

/*
* ©���ո񣬶�ȡһ���ַ���
*
* ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
*
* ������getsym���á�
*/
int getch()
{
	if (cc == ll)	//�����ʵ�����line������ĩβ 
	{
		if (feof(fin))	//�ļ����� 
		{
			printf("program incomplete");	//�������� 
			return -1;
		}
		ll=0;	//ĩβll����Ϊ0 
		cc=0;	//��־��ȡ�ַ���λ�� 
		printf("%d ", cx);
		fprintf(fa1,"%d ", cx);
		ch = ' ';
		while (ch != 10)	//ch�����ڻ���ʱ 
		{
			//fscanf(fin,"%c", &ch)
			//richard
			if (EOF == fscanf(fin,"%c", &ch))	//������ļ��ж���EOFʱ
			{
				line[ll] = 0;	//��line���������һ���ַ���Ϊ0 
				break;			//���� 
			}
			//end richard
			printf("%c", ch);	//��������ַ���� 
			fprintf(fa1, "%c", ch);
			line[ll] = ch;
			ll++;
		}
		//ע�⣺ÿ����һ�а��������Ļ��з�
		printf("\n");		//һ�н������һ�л��� 
		fprintf(fa1, "\n");
	}
	ch = line[cc];
	cc++;
	return 0;
}

/*
* �ʷ���������ȡһ������
*��������������ܹ�������һ���ַ��������京�����sym������
*/
int getsym()
{
	int i,j,k;

	/* the original version lacks "\r", thanks to foolevery */
	while (ch==' ' || ch==10 || ch==13 || ch==9)  /* ���Կո񡢻��С��س���TAB */
	{
		//define  getchdo   if(-1 == getch()) return -1
		/*����getch��ʹch�ַ���line�������л�ȡ�ַ� */
		getchdo;	
	}
	if (ch>='a' && ch<='z')
	{           /* ���ֻ�������a..z��ͷ */
		k = 0;
		do {
			if(k<symmaxlen)		//symmaxlen�Ƿ��ŵ���󳤶� 
			{
				a[k] = ch;		//������ʱ��Ŷ���ķ����ַ��� 
				k++;
			}
			getchdo;		//��ch�ж���һ���ַ�
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
		a[k] = 0;		//����ʱ�����ַ������һ����Ϊ���ַ�
		strcpy(id, a);
		i = 0;
		j = keysize-1;		//�ؼ�����Ŀ-1 
		do {    /* ������ǰ�����Ƿ�Ϊ������ */
			k = (i+j)/2;	//�۰����
			/*��Ϊ�ؼ���Ϊ�ֵ������У�
			���Ե�����ı������ֵ���С�ڵ�ǰ�Ƚϵı�����ʱ��
			��С�İ벿�ֲ��ң�������ڵ�ǰ�Ƚϵı����֣�����İ벿�ֲ���*/ 
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
			sym = wsym[k];/*��ǰö��������Ϊ��Ӧ���Ǹ������ֵ�ö��ֵ*/
		}
		else
		{
			//**********************************************************Ϊʲô������ 
			sym = ident; /* ����ʧ���������ֻ����� */
		}
	}
	else
	{
		if (ch>='0' && ch<='9')
		{           /* ����Ƿ�Ϊ���֣���0..9��ͷ */
			k = 0;
			num = 0;
			sym = number;		//sym������Ϊnumber
			do {
				num = 10*num + ch - '0';
				k++;
				getchdo;
			} while (ch>='0' && ch<='9'); /* ��ȡ���ֵ�ֵ */
			k--;
			if (k > nummaxlen)		//�����������ֳ��������λ�����򱨴�
			{
				error(30);
			}
		}
		else
		{
			if (ch == ':')      /* ��⸳ֵ���� */
			{
				getchdo;
				if (ch == '=')
				{
					sym = becomes;
					getchdo;
				}
				else
				{
					sym = nul;  /* ����ʶ��ķ��� */
				}
			}
			else
			{
				/*
				����Ҫ����������������������ԭ��
				��������������ַ��ģ����Բ���ֱ����sym = ssym[ch];������sym������
				����Ҫ����Щ˫�ַ�����������������������ǵ����ַ������������sym = ssym[ch];
				*/
				if (ch == '<')      /* ���С�ڻ�С�ڵ��ڷ��� */
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
				else if (ch=='>')        /* �����ڻ���ڵ��ڷ��� */
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
					sym = ssym[ch];     /* �����Ų�������������ʱ��ȫ�����յ��ַ����Ŵ��������ַ�������� */
					//getchdo;
					//richard
					if (sym != period)	
					//���������Ų��Ǿ����Ҫ�����ٶ�һ���ַ�����ֹ�´��ٶ�������ַ�
					//����Ǿ�ţ�ֱ�ӷ������ƣ���Ϊ���Ѿ��ǽ�β�ˣ�û����һ���ַ���
					{
						getchdo;
						/***************************************************************************************/
						/*�˴�����ӵȼ��ȳ˵ȳ��ȵ�ʶ��***********************************************************/
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
* �������������
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= vmmax)
	{
		printf("Program too long"); /* ������� */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}


/*
* ���Ե�ǰ�����Ƿ�Ϸ�
*
* ��ĳһ���֣���һ����䣬һ�����ʽ����Ҫ����ʱʱ����ϣ����һ����������ĳ����
* ���ò��ֵĺ�����ţ���test���������⣬���Ҹ��𵱼�ⲻͨ��ʱ�Ĳ��ȴ�ʩ��
* ��������Ҫ���ʱָ����ǰ��Ҫ�ķ��ż��ϺͲ����õļ��ϣ���֮ǰδ��ɲ��ֵĺ��
* ���ţ����Լ���ⲻͨ��ʱ�Ĵ���š�
*
* s1:   ������Ҫ�ķ���
* s2:   �������������Ҫ�ģ�����Ҫһ�������õļ���
* n:    �����
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))		//���������Ų��ں�����ż��У��򱨴�
	{
		error(n);
		/* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ��� */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{
			getsymdo;
		}
	}
	return 0;
}

/*
* �����������
*
* lev:    ��ǰ�ֳ������ڲ�
* tx:     ���ֱ�ǰβָ��
* fsys:   ��ǰģ�������ż���
*/
int block(int lev, int tx, bool* fsys)
{
	int i;
	//**************************��Ӧÿ�����������һ�����ַ��������Ȼ��������������ÿ������ı���������Ե�ַ������
	int dx;                 /* ���ַ��䵽����Ե�ַ */
	int tx0;                /* ������ʼtx */
	int cx0;                /* ������ʼcx */
	bool nxtlev[symnum];    /* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿռ�
							���ݸ��¼�����*/

	dx = 3;					//***************************************************Ϊʲô��Ϊ3
	tx0 = tx;               /* ��¼�������ֵĳ�ʼλ�� */
	table[tx].adr = cx;

	gendo(jmp, 0, 0);		//��������������code[cx]��

	if (lev > levelmax)		//����Ƕ�ײ����򱨴�
	{
		error(32);
	}

	do {

		if (sym == constsym)    /* �յ������������ţ���ʼ���������� */
		{
			getsymdo;		//��ȡconst����ַ����������ȷ��Ӧ���Ƕ�����һ��������

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do { */
			constdeclarationdo(&tx, lev, &dx);  /* dx��ֵ�ᱻconstdeclaration�ı䣬ʹ��ָ�� */
												/*������������������һ�����ֱ��У����ֱ��д������еı����������̵�*/
			while (sym == comma)			//���������һ������
			{
				getsymdo;					//���붺�ź���ַ����������ȷ��Ӧ���Ƕ�����һ��������
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)			//���������һ���ֺ�
			{
				getsymdo;					//�����β�Ļ��з�
			}
			else
			{
				error(5);   /*©���˶��Ż��߷ֺ�*/
			}
			/* } while (sym == ident); */
		}

		if (sym == varsym)      /* �յ������������ţ���ʼ����������� */
		{
			getsymdo;		//����var����ַ���

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do {  */
			vardeclarationdo(&tx, lev, &dx);	//��var��ĵ�һ���ַ��������жϣ������ident(��ʶ��)���������ʶ���������ֱ���
			while (sym == comma)	//������һ�����ţ���������������
			{
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)	//����Ƿֺ���������Ļ��У�������
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
			/* } while (sym == ident);  */
		}

		while (sym == procsym) /* �յ������������ţ���ʼ����������� */
		/*
		����Ϊ��
		�ȶ�һ���ַ���������Ǳ�ʶ�����¼�����������ٶ���һ���ַ���������Ƿֺţ������һ�����С�
		*/
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* ��¼�������� */
				getsymdo;
			}
			else
			{
				error(4);   /* procedure��ӦΪ��ʶ�� */
			}

			if (sym == semicolon)	//������Ƿֺ���֮��Ļ��ж���
			{
				getsymdo;
			}
			else
			{
				error(5);   /* ©���˷ֺ� */
			}

			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev))		//�ݹ����ʱ���ݹ����+1
			{
				return -1;  /* �ݹ���� */
			}

			if(sym == semicolon)	//�����������Ƿֺţ�����ȷ
			{
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
				nxtlev[ident] = true;
				nxtlev[procsym] = true;
				testdo(nxtlev, fsys, 6);
			}
			else
			{
				error(5);   /* ©���˷ֺ� */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   
	/* ֱ��û���������ţ���var,const,procedure�ȣ���ζ�ŵ�ǰ����������Ĺ��̱��������Ѿ���������ʼ�������������� */

	/* ��ʼ���ɵ�ǰ���̴��룬
	��ʱ�Ѿ���������һ�㺯���ж���ı��������͹��̵�ָ�
	��ôcxָ���˵�ǰ����������ÿ�ʼ��ָ��λ�ã����Ե�ַ����*/
	code[table[tx0].adr].a = cx;    
	table[tx0].adr = cx;            /* ����ǰ����������ĵ�һ��ָ��ĵ�ַ�������ֱ��иù��̵�adr */
	table[tx0].size = dx;           /* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
	cx0 = cx;
	gendo(inte, 0, dx);             /* ���ɷ����ڴ���� */

	if (tableswitch)        /* ������ֱ� */
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

	/* ���������Ϊ�ֺŻ�end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* ÿ��������ż��Ͷ������ϲ������ż��ͣ��Ա㲹�� */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);                       /* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
	memset(nxtlev, 0, sizeof(bool)*symnum); /*�ֳ���û�в��ȼ��� */
	testdo(fsys, nxtlev, 8);                /* �����������ȷ�� */
	listcode(cx0);                          /* ������� */
	return 0;
}

/*
* �����ֱ��м���һ��
* ֮�����ͨ���������ֱ�֪�������ڲ�Ρ����ࡢֵ
* k:      ��������const,var or procedure
* ptx:    ���ֱ�βָ���ָ�룬Ϊ�˿��Ըı����ֱ�βָ���ֵ
* lev:    �������ڵĲ��,���Ժ����е�lev��������
* pdx:    dxΪ��ǰӦ����ı�������Ե�ַ�������Ҫ����1������ָ������ֵ��λ��
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;	//��Ϊÿ����һ�����֣����ֱ��ڵ���϶���+1���������ֱ��βָ��+1
	strcpy(table[(*ptx)].name, id); /* ȫ�ֱ���id���Ѵ��е�ǰ���ֵ����� */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:  /* �������� */
		if (num > adressmax)	//�����������adressmax�򱨴�
		{
			error(31);  /* ��Խ�� */
			num = 0;
		}
		table[(*ptx)].val = num;	//const������ֱֵ�Ӵ���val�У���������ͬ
		break;
	case variable:  /* �������� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);	//������ֵ����adr��Ӧ�����λ���ϣ���ͬ��const����ֱ�Ӵ�ֵ
		(*pdx)++;	//���������������ľ�̬����ַ��ƫ�Ƶ�ַ+1
		break;
	case procedur:  /*���������֡�*/
		table[(*ptx)].level = lev;
		break;
	}
}

/*
* �������ֵ�λ��.
* �ҵ��򷵻������ֱ��е�λ��,���򷵻�0.
*
* idt:    Ҫ���ҵ�����
* tx:     ��ǰ���ֱ�βָ��
*/
int position(char* idt, int tx)
{
	int i;
	strcpy(table[0].name, idt);		//�����ڱ������û���ҵ�������֣��򷵻�0
	i = tx;
	while (strcmp(table[i].name, idt) != 0)
	{
		i--;
	}
	return i;
}

/*
* ������������
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)	//������ж�Ϊ������
	{
		getsymdo;
		if (sym==eql || sym==becomes)
		{
			if (sym == becomes)		//����������һ����ֵ�����
				//const����������=��ֵ��������:=
			{
				error(1);   /* ��=д����:= */
			}
			getsymdo;	//������һ���ַ���
			if (sym == number)	//��������������
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* ����˵��=��Ӧ������ */
			}
		}
		else
		{
			error(3);   /* ����˵����ʶ��Ӧ��= */
		}
	}
	else
	{
		error(4);   /* const��Ӧ�Ǳ�ʶ */
	}
	return 0;
}

/*
* ������������
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	if (sym == ident)
	{
		enter(variable, ptx, lev, pdx); // ��д���ֱ�
		getsymdo;
	}
	else
	{
		error(4);   /* var��Ӧ�Ǳ�ʶ */
	}
	return 0;
}

/*
* ���Ŀ������嵥
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
* ��䴦��0
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];
	if (sym == ident)   /* ׼�����ո�ֵ��䴦�� */
	{
		i = position(id, *ptx);
		enum symbol oprsym;		//���ڸ�����+=,-=���ж���δ���
		if (i == 0)
		{
			error(11);  /* ����δ�ҵ� */
		}
		else
		{
			if(table[i].kind != variable)
			{
				error(12);  /* ��ֵ����ʽ���� */
				i = 0;
			}
			else
			{
				getsymdo;
				/***********************************************************************/
				/*��Ӽӵ��ڼ����ڳ˵��ڳ�����*/
				/***********************************************************************/
				if(sym == becomes ||
				   sym == plusbecomes ||
				   sym == minusbecomes ||
				   sym == timesbecomes ||
				   sym == slashbecomes)
				{
					oprsym = sym;					//���ڱ���������ʽ�����ֲ�����= += -= *= /=�е�һ��
					if (oprsym != becomes)
					{
						gendo(lod, lev - table[i].level, table[i].adr);		//���������ʽ�Ĳ������Ǹ�ֵ���򽫸�ֵ����ջ��������֮������ʽ��ʽ������
					}
					getsymdo;
				}
				else
				{
					error(13);  /* û�м�⵽��ֵ���� */
				}
				/*������űض�Ҫ�޸�*/
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev); /* ����ֵ�����Ҳ���ʽ */
				if(i != 0)
				{
					if (oprsym == becomes)
					{
						/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
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
	else if (sym == readsym) /* ׼������read��䴦�� */
	{
		getsymdo;
		if (sym != lparen)
		{
			error(34);  /* ��ʽ����Ӧ�������� */
		}
		else
		{
			do {
				getsymdo;
				if (sym == ident)
				{
					i = position(id, *ptx); /* ���ֱ����Ҫ���ı��� */
				}
				else
				{
					i = 0;
				}

				if (i == 0)
				{
					error(35);  /* read()��Ӧ���������ı����� */
				}
				else if (table[i].kind != variable)
				{
					error(32);	/* read()������ı�ʶ�����Ǳ���, thanks to amd */
				}
				else
				{
					/***read����ָ��*****************************/
					gendo(opr, 0, 16);  /* ��������ָ���ȡֵ��ջ�� */
					gendo(sto, lev - table[i].level, table[i].adr);   /* ���浽���� */
				}
				getsymdo;

			} while (sym == comma); /* һ��read���ɶ�������� */
		}
		if (sym != rparen)
		{
			error(33);  /* ��ʽ����Ӧ�������� */
			while (!inset(sym, fsys))   /* �����ȣ�ֱ���յ��ϲ㺯���ĺ������ */
			{
				getsymdo;
			}
		}
		else
		{
			getsymdo;
		}
	}
	else if (sym == writesym)    /* ׼������write��䴦����read���� */
	{
		getsymdo;
		if (sym == lparen)
		{
			do {
				getsymdo;
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				nxtlev[rparen] = true;
				nxtlev[comma] = true;       /* write�ĺ������Ϊ) or , */
				expressiondo(nxtlev, ptx, lev); /* ���ñ��ʽ�����˴���read��ͬ��readΪ��������ֵ */
				gendo(opr, 0, 14);  /* �������ָ����ջ����ֵ */
			} while (sym == comma);
			if (sym != rparen)
			{
				error(33);  /* write()��ӦΪ�������ʽ */
			}
			else
			{
				getsymdo;
			}
		}
		gendo(opr, 0, 15);  /* ������� */
	}
	else if (sym == callsym) /* ׼������call��䴦�� */
	{
		getsymdo;
		if (sym != ident)
		{
			error(14);  /* call��ӦΪ��ʶ�� */
		}
		else
		{
			i = position(id, *ptx);
			if (i == 0)
			{
				error(11);  /* ����δ�ҵ� */
			}
			else
			{
				if (table[i].kind == procedur)
				{
					gendo(cal, lev - table[i].level, table[i].adr);   /* ����callָ�� */
				}
				else
				{
					error(15);  /* call���ʶ��ӦΪ���� */
				}
			}
			getsymdo;
		}
	}
	else if (sym == ifsym)   /* ׼������if��䴦�� */
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[thensym] = true;
		nxtlev[dosym] = true;    /* �������Ϊthen��do */
		conditiondo(nxtlev, ptx, lev); /* �������������߼����㣩���� */
		if (sym == thensym)
		{
			getsymdo;
		}
		else
		{
			error(16);  /* ȱ��then */
		}
		cx1 = cx;   /* ���浱ǰָ���ַ */
		gendo(jpc, 0, 0);   /* ����������תָ���ת��ַδ֪����ʱд0 */
		statementdo(fsys, ptx, lev);    /* ����then������ */
		getsymdo;
		if (sym == elsesym)
		{
			int cx2 = cx;
			gendo(jmp, 0, 0);				/*�ȴ���ַ����*/
			code[cx1].a = cx;   /* ��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ */
			getsymdo;
			statementdo(fsys, ptx, lev);
			code[cx2].a = cx;
		}
		else
		{
			code[cx1].a = cx;   /* ��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ */
		}
		
	}
	else if (sym == beginsym)    /* ׼�����ո�����䴦�� */
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[semicolon] = true;
		nxtlev[endsym] = true;  /* �������Ϊ�ֺŻ�end */
		/* ѭ��������䴦������ֱ����һ�����Ų�����俪ʼ���Ż��յ�end */
		statementdo(nxtlev, ptx, lev);

		/*�˴����ѭ�����ڽ������������Ϊ���е�������ʼ���ͷֺţ����Ի᲻�ϵض�ȡ���
		ֱ������end���˳�ѭ��*/
		while (inset(sym, statbegsys) || sym == semicolon)
		{
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(10);  /* ȱ�ٷֺ� */
			}
			statementdo(nxtlev, ptx, lev);
		}


		if (sym == endsym)
		{
			getsymdo;
		}
		else
		{
			error(17);  /* ȱ��end��ֺ� */
		}
	}
	else if (sym == whilesym)    /* ׼������while��䴦�� */
	{
		cx1 = cx;   /* �����ж�����������λ�� */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[dosym] = true;   /* �������Ϊdo */
		conditiondo(nxtlev, ptx, lev);  /* ������������ */
		cx2 = cx;   /* ����ѭ����Ľ�������һ��λ�� */
		gendo(jpc, 0, 0);   /* ����������ת��������ѭ���ĵ�ַδ֪ */
		if (sym == dosym)
		{
			getsymdo;
		}
		else
		{
			error(18);  /* ȱ��do */
		}
		statementdo(fsys, ptx, lev);    /* ѭ���� */
		gendo(jmp, 0, cx1); /* ��ͷ�����ж����� */
		code[cx2].a = cx;   /* ��������ѭ���ĵ�ַ����if���� */
	}
	else
	{
		memset(nxtlev, 0, sizeof(bool)*symnum); /* �������޲��ȼ��� */
		testdo(fsys, nxtlev, 19);   /* �������������ȷ�� */
	}
	return 0;
}

/*
* ���ʽ����
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;  /* ���ڱ��������� */
	bool nxtlev[symnum];

	if(sym==plus || sym==minus) /* ��ͷ�������ţ���ʱ��ǰ���ʽ������һ�����Ļ򸺵��� */
	{
		addop = sym;    /* ���濪ͷ�������� */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* ������ */
		if (addop == minus)
		{
			gendo(opr,0,1); /* �����ͷΪ��������ȡ��ָ�� */
		}
	}
	else    /* ��ʱ���ʽ��������ļӼ� */
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* ������ */
		/*term -> factor ��factor������ָ������������ŵ�ջ����
		Ȼ��term�д���˳����㣬�����ջ��������*/
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* ������ */
		if (addop == plus)
		{
			gendo(opr, 0, 2);   /* ���ɼӷ�ָ�� */
		}
		else
		{
			gendo(opr, 0, 3);   /* ���ɼ���ָ�� */
		}
	}
	return 0;
}

/*
* ���
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;  /* ���ڱ���˳������� */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	nxtlev[qvyv] = true;
	factordo(nxtlev, ptx, lev); /* �������� */
	while(sym==times || sym==slash ||sym==qvyv)		//****************����ȡ�๦��
	{
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if(mulop == times)
		{
			gendo(opr, 0, 4);   /* ���ɳ˷�ָ�� */
		}
		else if(mulop == slash)
		{
			gendo(opr, 0, 5);   /* ���ɳ���ָ�� */
		}
		else if (mulop == qvyv)
		{
			gen(opr, 0, 7);		/*����ȡ��ָ��*/
		}
	}
	return 0;
}

/*
* ���Ӵ���
*/
int factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);    /* ������ӵĿ�ʼ���� */
	/* while(inset(sym, facbegsys)) */  /* ѭ��ֱ���������ӿ�ʼ���� */
	if(inset(sym,facbegsys))    /* BUG: ԭ���ķ���var1(var2+var3)�ᱻ����ʶ��Ϊ���� */
	{
		if(sym == ident)    /* ����Ϊ��������� */
		{
			i = position(id, *ptx); /* �������� */
			if (i == 0)
			{
				error(11);  /* ��ʶ��δ���� */
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:  /* ����Ϊ���� */
					gendo(lit, 0, table[i].val);    /* ֱ�Ӱѳ�����ֵ��ջ */
					break;
				case variable:  /* ����Ϊ���� */
					gendo(lod, lev-table[i].level, table[i].adr);   /* �ҵ�������ַ������ֵ��ջ */
					break;
				case procedur:  /* ����Ϊ���� */
					error(21);  /* ����Ϊ���� */
					break;
				}
			}
			getsymdo;
		}
		else if (sym == number)   /* ����Ϊ�� */
		{
			if (num > adressmax)
			{
				error(31);
				num = 0;
			}
			gendo(lit, 0, num);
			getsymdo;
		}
		else if (sym == lparen)  /* ����Ϊ���ʽ */
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
				error(22);  /* ȱ�������� */
			}
		}
		testdo(fsys, facbegsys, 23);    /* ���Ӻ��зǷ����� */
	}
	return 0;
}

/*
* ��������
*/
int condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if(sym == oddsym)   /* ׼������odd���㴦�� */
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);   /* ����oddָ�� */
	}
	else
	{
		/* �߼����ʽ���� */
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
* ���ͳ���
*/
void interpret()
{
	int oprPoint, oprBaseAdr, topPoint;    /* ָ��ָ�룬ָ���ַ��ջ��ָ�� */
	struct instruction nowOpr;   /* ��ŵ�ǰָ�� */
	int storestack[stacksize];   /* ջ */

	printf("start pl0\n");
	topPoint = 0;
	oprBaseAdr = 0;
	oprPoint = 0;
	storestack[0] = storestack[1] = storestack[2] = 0;
	do {
		nowOpr = code[oprPoint];    /* ����ǰָ�� */
		oprPoint++;
		switch (nowOpr.f)
		{
		case lit:   /* ��a��ֵȡ��ջ�� */
			storestack[topPoint] = nowOpr.a;
			topPoint++;
			break;
		case opr:   /* ��ѧ���߼����� */
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
			case 7:					//����һ��ȡ��ָ��
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
		case lod:   /* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			storestack[topPoint] = storestack[base(nowOpr.l,storestack,oprBaseAdr)+nowOpr.a];
			topPoint++;
			break;
		case sto:   /* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			topPoint--;
			storestack[base(nowOpr.l, storestack, oprBaseAdr) + nowOpr.a] = storestack[topPoint];
			break;
		case cal:   /* �����ӹ��� */
			storestack[topPoint] = base(nowOpr.l, storestack, oprBaseAdr); /* �������̻���ַ��ջ */
			storestack[topPoint+1] = oprBaseAdr; /* �������̻���ַ��ջ������������base���� */
			storestack[topPoint+2] = oprPoint; /* ����ǰָ��ָ����ջ */
			oprBaseAdr = topPoint;  /* �ı����ַָ��ֵΪ�¹��̵Ļ���ַ */
			oprPoint = nowOpr.a;    /* ��ת */
			break;
		case inte:  /* �����ڴ� */
			topPoint += nowOpr.a;
			break;
		case jmp:   /* ֱ����ת */
			oprPoint = nowOpr.a;
			break;
		case jpc:   /* ������ת */
			topPoint--;
			if (storestack[topPoint] == 0)
			{
				oprPoint = nowOpr.a;
			}
			break;
		}
	} while (oprPoint != 0);
}

/* ͨ�����̻�ַ����l����̵Ļ�ַ */
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

