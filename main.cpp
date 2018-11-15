#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MaxBuf 1024                    //���������Ϊ1K
#define MaxStringLength 100            //��ʶ����󳤶�100
#define NumKeyword 37                  //C�ؼ��ָ���

//�Ǻ����
#define ID 0                           //�û�����ı�ʶ��
#define KEYWORD 1                      //�ؼ���
#define CONSTANT 2                     //����
#define RELATIONALOPERATOR 3           //��ϵ�����
#define ARITHMETICOPERATOR 4           //���������
#define ASSIGNMENTOPERATOR 5           //��ֵ�����
#define INCREMENTOPERATOR 6            //++--
#define COMPAREOPERATOR 7              //�Ƚ������
#define LOCATEOPERATOR 8               //��ַ��λ�����
#define LOGICALOPERATOR 9              //�߼������
#define BOOLOPERATOR 10                //����λ�����
#define SHIFTOPERATOR 11               //��λ�����


typedef struct identifier {            //�û�����ı�ʶ����
	char ch[MaxStringLength];
	struct identifier * next;          //����ṹʵ�ֱ�ʶ����
}Id;


typedef struct keyword {               //�ؼ��ֱ�
	char ch[MaxStringLength];
}Keyword;

//ȫ�ֱ���
char buffer[MaxBuf + 2];                     //������,��СΪ1K������λ������eof��Ϊ���
int state = 0;                               //��ǰ״ָ̬ʾ
int isKey = 0;                               //-1��ʾʶ����ĵ��ʲ��ǹؼ��֣�����Ϊ�ؼ���
char C;                                      //��ǰ������ַ�
char token[MaxStringLength];                 //��ŵ�ǰʶ��ĵ����ַ���
//char * lexemebegin = NULL;                   //ָ�����뻺�����е�ǰ���ʵĿ�ʼλ��
char * forward = NULL;                       //��ǰָ��
FILE * fp = NULL;                            //Դ�ļ���ȡָ��
FILE * write_back_fp = NULL;                 //д���ļ�ָ��
Keyword kword[NumKeyword];                   //C�ؼ��ֱ�
Id * id_sheet = NULL;                        //�û��Զ����ʶ����
int num_Line = 1;                            //Դ��������
int num_char = 0;                            //Դ�����ַ�����

void reload(int start)
//��startλ��������仺����
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

void get_char()                       //�ӻ�������ȡ��һ���ַ�
{
	if (*forward == EOF)              //forwardָ�뵱ǰָ��������EOF
	{
		if (forward - buffer == MaxBuf+1) //forwardָ�����Ұ����յ�
		{
			reload(0);
			forward = buffer;
			C = *forward;
			forward++;

		}
		else if (forward - buffer == MaxBuf / 2) //fowardָ����������յ�
		{
			reload(MaxBuf / 2 + 1);
			forward++;
			C = *forward;
			forward++;
		}
		else                              //Դ����������
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

void get_nbc()                         //���C�е��ַ��Ƿ�Ϊ�ո����򷴸�����
{
	//return C==' ';
	while (C == ' '|| C == '\n'||C=='\t')                  //CΪ�ո�ʱ�ظ�����
	{
		get_char();
		if (C == '\n') { num_Line++; }
	}
}

void _cat()                           //��C�е��ַ����ӵ�token���ַ�����
{
	int i = 0;
	for(i = 0;token[i]!='\0';i++){}
	token[i] = C;
} 

bool isLetter()                       //�ж�C�е������Ƿ�Ϊ��ĸ
{
	return (C<='Z'&&C>='A')||(C>='a'&&C<='z');
}

bool isDigit()                        //�ж�C�е������Ƿ�Ϊ����
{
	return C>='0'&&C<='9';
}

bool is_()                            //�ж�C���ַ��Ƿ����»���
{
	return C == '_';
}

void retract()                        //��ǰָ�����
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

int reserve()                         //����token�еĵ��ʲ�ѯ�ؼ��ֱ�
{
	for (int i = 0; i < NumKeyword; i++)
	{
		if (strcmp(token, kword[i].ch) == 0)
		{
			return 1;                //token�еĵ����ǹؼ��֣�����1
		}
	}
	return 0;                        //token�еĵ��ʲ��ǹؼ��֣�����0
}

int reserve_ID()                  //����token�еĵ��ʲ�ѯ���б�ʶ����
{
	Id * ptr = id_sheet;
	while (ptr != NULL)
	{
		if (strcmp(ptr->ch, token) == 0)
			return 1;                       //�ñ�ʶ���Ѽ����ʶ����
		ptr = ptr->next;
	}
	return 0;                               //�ñ�ʶ��δ�����ʶ����
}

void load_keyword()                   //��C�Ĺؼ��ּ��ص��ؼ��ֱ�kword��
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

void table_insert()                   //���û��Զ���ı�ʶ��������ű�
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

void write_back(char ch[100],int token_class) //�������ĵ��ʵļǺŷ��ظ�����
{
	if (token_class == ID)                    //�û��Զ���ı�ʶ��
	{
		fprintf(write_back_fp, "<%s , ��ʶ��>\n",ch);
	}
	else if (token_class == KEYWORD)          //�ؼ���
	{
		fprintf(write_back_fp, "<%s , �ؼ���>\n",ch);
	}
	else if (token_class == CONSTANT)         //����
	{
		fprintf(write_back_fp, "<%s , ����>\n",ch);
	}
	else if (token_class == RELATIONALOPERATOR)//��ϵ�����
	{
		fprintf(write_back_fp, "<%s , ��ϵ�����>\n",ch);
	}
	else if(token_class == ARITHMETICOPERATOR) //���������
	{
		fprintf(write_back_fp, "<%s , ���������>\n", ch);
	}
	else if (token_class == ASSIGNMENTOPERATOR) //��ֵ�����
	{
		fprintf(write_back_fp, "<%s , ��ֵ�����>\n", ch);
	}
	else if (token_class == INCREMENTOPERATOR) //���������
	{
		fprintf(write_back_fp, "<%s , ���������>\n", ch);
	}
	else if (token_class == COMPAREOPERATOR) //�Ƚ������
	{
		fprintf(write_back_fp, "<%s , �Ƚ������>\n", ch);
	}
	else if (token_class == LOCATEOPERATOR) //��ַ��λ�����
	{
		fprintf(write_back_fp, "<%s , ��ַ��λ�����>\n", ch);
	}
	else if (token_class == LOGICALOPERATOR)
	{
		fprintf(write_back_fp, "<%s , �߼������>\n", ch);
	}
	else if (token_class == SHIFTOPERATOR)
	{
		fprintf(write_back_fp, "<%s , ��λ�����>\n", ch);
	}
	else
	{
		printf("Error: undefined class!\n");
		exit(3);
	}
}

void Init()                       //��ʼ����������Դ�����ĵ�������ĵ�
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
	reload(0);                            //��ʼ��ʱ��仺�����������
	load_keyword();                       //���عؼ���
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

void error()                //����
{
	fprintf(write_back_fp, "Դ�����%d�е�%s���ʲ��Ϸ�\n",num_Line,token);
}

void AnalyseProc()                                   //��������
{
	state = 0;
	//int num_Line = 0;
	while (C != EOF)
	{
		switch (state)
		{
			case 0:
				clear();            //���token�е�����
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
							fprintf(write_back_fp, "<%s ,  ����>\n",token);
							state = 0;
							break;
						case ',':
							_cat();
							fprintf(write_back_fp, "<%s ,  ˳������>\n",token);
							state = 0;
							break;
						case ';':
							_cat();
							fprintf(write_back_fp, "<%s ,  �ֺ�>\n", token);
							state = 0;
							break;
						case'{':
							_cat();
							fprintf(write_back_fp, "<%s  , �������>\n", token);
							state = 0;
							break;
						case '}':
							_cat();
							fprintf(write_back_fp, "<%s  , �Ҵ�����>\n", token);
							state = 0;
							break;
						case'(':
							_cat();
							fprintf(write_back_fp, "<%s  , ������>\n", token);
							state = 0;
							break;
						case')':
							_cat();
							fprintf(write_back_fp, "<%s  , �Ҵ�����>\n", token);
							state = 0;
							break;
						case'[':
							_cat();
							fprintf(write_back_fp, "<%s  , ��������>\n", token);
							state = 0;
							break;
						case']':
							_cat();
							fprintf(write_back_fp, "<%s  , ��������>\n", token);
							state = 0;
							break;
						case '"':
							_cat();
							fprintf(write_back_fp, "<%s  , ˫����>\n", token);
							state = 0;
							break;
						case '\\' :
							_cat();
							fprintf(write_back_fp, "<%s  , ��б��>\n", token);
							state = 0;
							break;
						default: state = 0;  break;
					}
				}
				break;
			case 1:                                  //��ʶ��
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
					if (isKey == 1)                     //ʶ��ı�ʶ���ǹؼ���
					{
						write_back(token, KEYWORD);
					}
					else
					{
						if(reserve_ID()==0)             //�Ǳ�ı�ʶ��δ�����ʶ����
							table_insert();
						write_back(token, ID);
					}
				}
				break;
			case 2:                                    //����
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
	fprintf(write_back_fp, "Դ����һ��%d�У�%d���ַ�\n", num_Line, num_char);
	fclose(fp);
	fclose(write_back_fp);

}

int main(void)
{
	Init();
	AnalyseProc();
	printf("������");
	End();
	return 0;
}