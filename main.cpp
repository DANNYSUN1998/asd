#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MaxBuf 1024                    //缓冲区最大为1K
#define MaxStringLength 100            //标识符最大长度100
#define NumKeyword 37                  //C关键字个数

//记号类别
#define ID 0                           //用户定义的标识符
#define KEYWORD 1                      //关键字
#define CONSTANT 2                     //常数
#define RELATIONALOPERATOR 3           //关系运算符
#define ARITHMETICOPERATOR 4           //算术运算符
#define ASSIGNMENTOPERATOR 5           //赋值运算符
#define INCREMENTOPERATOR 6            //++--
#define COMPAREOPERATOR 7              //比较运算符
#define LOCATEOPERATOR 8               //地址定位运算符
#define LOGICALOPERATOR 9              //逻辑运算符
#define BOOLOPERATOR 10                //布尔位运算符
#define SHIFTOPERATOR 11               //移位运算符


typedef struct identifier {            //用户定义的标识符表
	char ch[MaxStringLength];
	struct identifier * next;          //链表结构实现标识符表
}Id;


typedef struct keyword {               //关键字表
	char ch[MaxStringLength];
}Keyword;

//全局变量
char buffer[MaxBuf + 2];                     //缓冲区,大小为1K，两个位置留给eof作为标记
int state = 0;                               //当前状态指示
int isKey = 0;                               //-1表示识别出的单词不是关键字，否则为关键字
char C;                                      //当前读入的字符
char token[MaxStringLength];                 //存放当前识别的单词字符串
//char * lexemebegin = NULL;                   //指向输入缓冲区中当前单词的开始位置
char * forward = NULL;                       //向前指针
FILE * fp = NULL;                            //源文件读取指针
FILE * write_back_fp = NULL;                 //写回文件指针
Keyword kword[NumKeyword];                   //C关键字表
Id * id_sheet = NULL;                        //用户自定义标识符表
int num_Line = 1;                            //源程序行数
int num_char = 0;                            //源程序字符个数

void reload(int start)
//从start位置重新填充缓冲区
{
	char ch;
	int i = start,a = 0;
	while (a<MaxBuf/2)
	{
		ch = fgetc(fp);
		if (ch == EOF)
		{
			buffer[i] = ch;
			break;
		}
		else
		{
			buffer[i] = ch;
			i++, a++;
		}
	}
}

void get_char()                       //从缓冲区获取下一个字符
{
	if (*forward == EOF)              //forward指针当前指向内容是EOF
	{
		if (forward - buffer == MaxBuf+1) //forward指针在右半区终点
		{
			reload(0);
			forward = buffer;
			C = *forward;
			forward++;

		}
		else if (forward - buffer == MaxBuf / 2) //foward指针在左半区终点
		{
			reload(MaxBuf / 2 + 1);
			forward++;
			C = *forward;
			forward++;
		}
		else                              //源程序结束标记
		{
			C = EOF;
			return;
		}
	}
	else
	{
		C = *forward;
		forward++;
	}
	num_char++;
}

void get_nbc()                         //检查C中的字符是否为空格，是则反复调用
{
	//return C==' ';
	while (C == ' '|| C == '\n'||C=='\t')                  //C为空格时重复调用
	{
		get_char();
		if (C == '\n') { num_Line++; }
	}
}

void _cat()                           //将C中的字符连接到token的字符串后
{
	int i = 0;
	for(i = 0;token[i]!='\0';i++){}
	token[i] = C;
} 

bool isLetter()                       //判断C中的内容是否为字母
{
	return (C<='Z'&&C>='A')||(C>='a'&&C<='z');
}

bool isDigit()                        //判断C中的内容是否为数字
{
	return C>='0'&&C<='9';
}

bool is_()                            //判断C中字符是否是下划线
{
	return C == '_';
}

void retract()                        //向前指针回退
{
	if (forward == buffer)
	{
		forward = buffer + MaxBuf;
	}
	else if (forward == buffer + MaxBuf / 2 + 1)
	{
		forward -= 2;
	}
	else
	{
		forward--;
	}
}

int reserve()                         //根据token中的单词查询关键字表
{
	for (int i = 0; i < NumKeyword; i++)
	{
		if (strcmp(token, kword[i].ch) == 0)
		{
			return 1;                //token中的单词是关键字，返回1
		}
	}
	return 0;                        //token中的单词不是关键字，返回0
}

int reserve_ID()                  //根据token中的单词查询已有标识符表
{
	Id * ptr = id_sheet;
	while (ptr != NULL)
	{
		if (strcmp(ptr->ch, token) == 0)
			return 1;                       //该标识符已加入标识符表
		ptr = ptr->next;
	}
	return 0;                               //该标识符未加入标识符表
}

void load_keyword()                   //将C的关键字加载到关键字表kword中
{
	FILE * k_fp = NULL;
	if ((k_fp = fopen("keyword.txt", "r")) == NULL)
	{
		printf("Failed open keyword.txt\n");
		exit(1);
	}
	int i = 0;
	
	while (i < NumKeyword)
	{
		fgets(kword[i].ch, 14, k_fp);
		//printf("%d %s\n", i, kword[i].ch);
		i++;
	}
	
	fclose(k_fp);
	for (int i = 0; i < NumKeyword-1; i++)
	{
		int j = 0;
		for (j = 0; kword[i].ch[j] != '\n'; j++) {}
		kword[i].ch[j] = '\0';
	}
}

void table_insert()                   //将用户自定义的标识符加入符号表
{
	Id * tmp = (Id*)malloc(sizeof(Id));
	if (tmp == NULL)
	{
		printf("Failed create new Id!\n");
		exit(4);
	}
	strcpy(tmp->ch, token);
	tmp->next = NULL;
	Id * ptr = id_sheet;
	while (ptr->next != NULL)
	{
		ptr = ptr->next;
	}
	ptr->next = tmp;
	//Id * newId = (Id*)malloc(sizeof(struct identifier));


}

void write_back(char ch[100],int token_class) //将读到的单词的记号返回给程序
{
	if (token_class == ID)                    //用户自定义的标识符
	{
		fprintf(write_back_fp, "<%s , 标识符>\n",ch);
	}
	else if (token_class == KEYWORD)          //关键字
	{
		fprintf(write_back_fp, "<%s , 关键字>\n",ch);
	}
	else if (token_class == CONSTANT)         //常数
	{
		fprintf(write_back_fp, "<%s , 常数>\n",ch);
	}
	else if (token_class == RELATIONALOPERATOR)//关系运算符
	{
		fprintf(write_back_fp, "<%s , 关系运算符>\n",ch);
	}
	else if(token_class == ARITHMETICOPERATOR) //算数运算符
	{
		fprintf(write_back_fp, "<%s , 算术运算符>\n", ch);
	}
	else if (token_class == ASSIGNMENTOPERATOR) //赋值运算符
	{
		fprintf(write_back_fp, "<%s , 赋值运算符>\n", ch);
	}
	else if (token_class == INCREMENTOPERATOR) //自增运算符
	{
		fprintf(write_back_fp, "<%s , 自增运算符>\n", ch);
	}
	else if (token_class == COMPAREOPERATOR) //比较运算符
	{
		fprintf(write_back_fp, "<%s , 比较运算符>\n", ch);
	}
	else if (token_class == LOCATEOPERATOR) //地址定位运算符
	{
		fprintf(write_back_fp, "<%s , 地址定位运算符>\n", ch);
	}
	else if (token_class == LOGICALOPERATOR)
	{
		fprintf(write_back_fp, "<%s , 逻辑运算符>\n", ch);
	}
	else if (token_class == SHIFTOPERATOR)
	{
		fprintf(write_back_fp, "<%s , 移位运算符>\n", ch);
	}
	else
	{
		printf("Error: undefined class!\n");
		exit(3);
	}
}

void Init()                       //初始化缓冲区及源代码文档与输出文档
{
	if ((fp = fopen("source.txt", "r")) == NULL)
	{
		printf("Failed open source.txt.\n");
		exit(0);
	}
	if ((write_back_fp = fopen("writeback.txt","w")) == NULL)
	{
		printf("Failed open writeback.txt.\n");
		exit(2);
	}
	buffer[MaxBuf / 2] = EOF,buffer[MaxBuf+1] = EOF;
	reload(0);                            //初始化时填充缓冲区的左半区
	load_keyword();                       //加载关键字
	forward = buffer;
	//lexemebegin = buffer;
	state = 0;
	id_sheet = (Id*)malloc(sizeof(Id));
	id_sheet->ch[0] = '\0';
	id_sheet->next = NULL;
}

void clear()
{
	for (int i = 0; i < MaxStringLength&&token[i] != '\0'; i++)
		token[i] = '\0';
}

void error()                //报错
{
	fprintf(write_back_fp, "源程序第%d行的%s构词不合法\n",num_Line,token);
}

void AnalyseProc()                                   //分析过程
{
	state = 0;
	//int num_Line = 0;
	while (C != EOF)
	{
		switch (state)
		{
			case 0:
				clear();            //清空token中的内容
				get_char();
				if (C == '\n') { num_Line++; }
				get_nbc();
				if (isLetter() || is_())
					state = 1;
				else if (isDigit())
					state = 2;
				else
				{
					switch (C)
					{
						case '=': state = 13; break;
						case '+': state = 14; break;
						case '-': state = 15; break;
						case '*': state = 16; break;
						case '/': state = 8; break;
						case '<': state = 17; break;
						case '>': state = 19; break;
						case '|': state = 21; break;
						case '!': state = 22; break;
						case '&': state = 23; break;
						case '^': state = 24; break;
						case '%': state = 25; break;
						case '.':
							_cat();
							write_back(token, LOCATEOPERATOR);
							state = 0;
							break;
						case '~': 
							_cat();
							write_back(token, BOOLOPERATOR);
							state = 0;
							break;
						case '#':
							_cat();
							fprintf(write_back_fp, "<%s ,  井号>\n",token);
							state = 0;
							break;
						case ',':
							_cat();
							fprintf(write_back_fp, "<%s ,  顺序计算符>\n",token);
							state = 0;
							break;
						case ';':
							_cat();
							fprintf(write_back_fp, "<%s ,  分号>\n", token);
							state = 0;
							break;
						case'{':
							_cat();
							fprintf(write_back_fp, "<%s  , 左大括号>\n", token);
							state = 0;
							break;
						case '}':
							_cat();
							fprintf(write_back_fp, "<%s  , 右大括号>\n", token);
							state = 0;
							break;
						case'(':
							_cat();
							fprintf(write_back_fp, "<%s  , 左括号>\n", token);
							state = 0;
							break;
						case')':
							_cat();
							fprintf(write_back_fp, "<%s  , 右大括号>\n", token);
							state = 0;
							break;
						case'[':
							_cat();
							fprintf(write_back_fp, "<%s  , 左中括号>\n", token);
							state = 0;
							break;
						case']':
							_cat();
							fprintf(write_back_fp, "<%s  , 右中括号>\n", token);
							state = 0;
							break;
						case '"':
							_cat();
							fprintf(write_back_fp, "<%s  , 双引号>\n", token);
							state = 0;
							break;
						case '\\' :
							_cat();
							fprintf(write_back_fp, "<%s  , 反斜杠>\n", token);
							state = 0;
							break;
						default: state = 0;  break;
					}
				}
				break;
			case 1:                                  //标识符
				_cat();
				get_char();
				if (isLetter() || isDigit() || is_())
				{
					state = 1;
				}
				else
				{
					retract();
					state = 0;
					isKey = reserve();
					if (isKey == 1)                     //识别的标识符是关键字
					{
						write_back(token, KEYWORD);
					}
					else
					{
						if(reserve_ID()==0)             //是别的标识符未加入标识符表
							table_insert();
						write_back(token, ID);
					}
				}
				break;
			case 2:                                    //数字
				_cat();
				get_char();
				if (isDigit())
				{
					state = 2;
				}
				else if (C == 'E' || C == 'e')
				{
					state = 5;
				}
				else if (C == '.')
				{
					state = 3;
				}
				else if (C == '\n' || C == '\t' || C == ' ')
				{
					retract();
					state = 0;
					write_back(token, CONSTANT);
				}
				else if ((C != 'E'&&C != 'e'&&isLetter()) || is_())
				{
					error();
					retract();
					state = 0;
				}
				else
				{
					retract();
					state = 0;
				}
				break;
		
			case 3:
				_cat();
				get_char();
				if (isDigit())
				{
					state = 4;
				}
				else 
				{
					error();
					state = 0;
				}
					
				break;
			case 4:
				_cat();
				get_char();
				if (isDigit())
				{
					state = 4;
				}
				else if (C == 'E'||C=='e')
				{
					state = 5;
				}
				else if(C == ' ' || C == '\n' || C == '\t')
				{
					retract();
					state = 0;
					write_back(token, CONSTANT);
				}
				else if ((C != 'E'&&C != 'e'&&isLetter()) || is_())
				{
					error();
					retract();
					state = 0;
				}
				else
				{
					retract();
					state = 0;
				}
				break;
			case 5:
				_cat();
				get_char();
				if (C == '+'||C=='-')
				{
					state = 6;
				}
				else if (isDigit())
				{
					state = 7;
				}
				else
				{
					error();
					state = 0;
				}
				break;
			case 6:
				_cat();
				get_char();
				if (isDigit())
				{
					state = 7;
				}
				else
				{
					error();
					state = 0;
				}
				break;
			case 7:
				_cat();
				get_char();
				if (isDigit())
				{
					state = 7;
				}
				else
				{
					retract();
					write_back(token, CONSTANT);
					state = 0;
				}
				break;
			case 8:
				_cat();
				get_char();
				if (C == '/')
				{
					state = 9;
				}
				else if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);// /=
					state = 0;
				}
				else if (C == '*')
				{
					state = 11;
				}
				else
				{
					retract();
					write_back(token, ARITHMETICOPERATOR);//      /
					state = 0;
				}
				break;
			case 9:
				//_cat();
				get_char();
				if (C == '\n')
				{
					num_Line++;
					state = 10;
				}
				else
				{
					state = 9;
				}
				break;
			case 10:
				state = 0;
				break;
			case 11:
				//_cat();
				get_char();
				if (C == '*')
					state = 12;
				else {
					if (C == '\n')
						num_Line++;
					state = 11;
				}
				break;
			case 12:
				//_cat();
				get_char();
				if (C == '*')
					state = 12;
				else if (C == '/')
					state = 10;
				else
					state = 11;
				break;
			case 13:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, COMPAREOPERATOR);//==
					state = 0;
				}
				else
				{
					retract();
					write_back(token, ASSIGNMENTOPERATOR);   // =
					state = 0;
				}
				break;
			case 14:
				_cat();
				get_char();
				if (C == '+')
				{
					_cat();
					write_back(token, INCREMENTOPERATOR);//      ++
					state = 0;
				}
				else if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);  //+=
					state = 0;

				}
				else
				{
					retract();
					write_back(token, ARITHMETICOPERATOR);  // +
					state = 0;
				}
				break;
			case 15:
				_cat();
				get_char();
				if (C == '-')
				{
					_cat();
					write_back(token, INCREMENTOPERATOR);//     --
					state = 0;
				}
				else if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);//-=
					state = 0;
				}
				else if (C == '>')
				{
					_cat();
					write_back(token, LOCATEOPERATOR);//->
					state = 0;
				}
				else
				{
					retract();
					write_back(token, ARITHMETICOPERATOR);// -
					state = 0;
				}
				break;
			case 16:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);// *=
					state = 0;
				}
				else
				{
					retract();
					write_back(token, ARITHMETICOPERATOR); //*
					state = 0;
				}
				break;
			case 17:
				_cat();
				get_char();
				if (C == '<')
					state = 18;
				else if (C == '=')
				{
					_cat();
					write_back(token, COMPAREOPERATOR);//   <=
					state = 0;
				}
				else
				{
					retract();
					write_back(token, COMPAREOPERATOR);//<
					state = 0;
				}
				break;
			case 18:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);//<<=
					state = 0;
				}
				else
				{
					retract();
					write_back(token, SHIFTOPERATOR);//   <<
				}
				break;
			case 19:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, COMPAREOPERATOR);//>=
					state = 0;
				}
				else if (C == '>')
				{
					state = 20;
				}
				else
				{
					retract();
					write_back(token, COMPAREOPERATOR);// >
					state = 0;
				}
				break;
			case 20:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);//  >>=
					state = 0;
				}
				else
				{
					retract();
					write_back(token, SHIFTOPERATOR);// >>
					state = 0;
				}
				break;
			case 21:
				_cat();
				get_char();
				if (C == '|')
				{
					_cat();
					write_back(token, LOGICALOPERATOR);      //   ||
					state = 0;
				}
				else if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);    //    |=   
					state = 0;
				}
				else
				{
					retract();
					write_back(token, BOOLOPERATOR);        //   |
					state = 0;
				}
				break;
			case 22:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, COMPAREOPERATOR);    //  !=
					state = 0;
				}
				else
				{
					retract();
					write_back(token, LOGICALOPERATOR);      //   !
					state = 0;
				}
				break;
			case 23:
				_cat();
				get_char();
				if (C == '&')
				{
					_cat();
					write_back(token, LOGICALOPERATOR);     //   &&
					state = 0;
				}
				else if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);    //   &=
					state = 0;
				}
				else {
					retract();
					write_back(token, LOCATEOPERATOR);     //   &
					state = 0;
				}
				break;
			case 24:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);   //   ^=
					state = 0;
				}
				else
				{
					retract();
					write_back(token, BOOLOPERATOR);
					state = 0;
				}
				break;
			case 25:
				_cat();
				get_char();
				if (C == '=')
				{
					_cat();
					write_back(token, ASSIGNMENTOPERATOR);   //%=
					state = 0;

				}
				else
				{
					retract();
					write_back(token, ARITHMETICOPERATOR);      //%
					state = 0;
				}
				break;
			default:
				error();
				state = 0;
				break;
		}
	}
}

void End()
{
	fprintf(write_back_fp, "源程序一共%d行，%d个字符\n", num_Line, num_char);
	fclose(fp);
	fclose(write_back_fp);

}

int main(void)
{
	Init();
	AnalyseProc();
	printf("结束！");
	End();
	return 0;
}