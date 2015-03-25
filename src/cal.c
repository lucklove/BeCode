/*****************************************************************************
   	简易计算器   周帅   2013－10－24   gnu.crazier@gmail.com	   
*****************************************************************************/

#include	<stdio.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<setjmp.h>
#include	<ctype.h>
#include	<errno.h>
#include	<string.h>

#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define NOERROR 1
#define ERROR   2

jmp_buf rerun;
double number_value;
char string_value[20];			//假设用户输入的变量名长度小于20

struct Table;
typedef struct Table* PtrTable;

struct Table
{
	char 		name[20];
	double		value;
	PtrTable	next;
};

PtrTable head = NULL;			//链表的头
			
typedef enum					//用户可能的输入
{
	NAME,	NUMBER,	END,
	PLUS = '+',	MINUS = '-',	MUL = '*', DIV = '/',
	PRINT = ';', 	ASSIGN = '=', 	LP = '(',	RP = ')'
} INPUT;
INPUT cur_input;

void help_msg();				//帮助信息
void exit_sig( int );				//用于处理终止请求
void restart( int );				//使用longjmp重启程序
void deal_str( char * );			//处理交互命令
void init();					//程序初始化
double expr( bool );				//处理加减
double term( bool );
double prim( bool );
INPUT get_token();				//用于判断用户输入的是神马东东
void input_clear();				//清空输入缓存

PtrTable table_locate( char * );		//定位用户变量
double	table_get( char * );			//获取变量的值
PtrTable table_add( char *, double );		//加入一个用户变量
PtrTable table( char *, double );		//改变或生成用户变量
void table_init();				
void table_free( PtrTable );
void table_print();

void db( char * p );

int
main( int argc, char *argv[] )
{
	int jmpret;
	init();
	jmpret = setjmp( rerun );			//从这里重启程序
	switch( jmpret )
	{	
		case 0:
		case NOERROR:
			break;
		case ERROR:
			input_clear();			//出错则清空输入缓存
			break;
		default:
			raise( SIGINT );
	}
	while( true )
	{	
		printf( ">>>" );
		get_token();
		if( cur_input == END )
			raise(SIGINT);
		if( cur_input == PRINT )
			continue;
		printf( "%lf\n", expr(false) );
	}
	return 0;
} 	

void
input_clear()
{
	
	while( getchar() != '\n' );
}

void
init()
{
	table_init();					//初始化变量空间
	cur_input = PRINT;
	signal( SIGINT, exit_sig );
	signal( SIGFPE, SIG_DFL );
	printf("控制台计算器0.1\n当你不知所措时，请键入help,帮助无处不在！\n\n");
}
void
db( char *p)
{
	printf( "调试信息, 当前位置：%s\n",  p );
}
void 
exit_sig( int sig )
{
	table_free( head );				//释放变量空间	
	printf( "see you ......\n" ); 
	exit( 0 );
}


void
help_msg()
{
	printf("－－－－－－－简易计算器－－－－－－\n");
	printf("————————————————————————————————————————————————————————\n");
	printf("语法说明：\n");
	
	printf("program:\n");
	printf("\tEND\n");
	printf("\texpr_list END\n\n");

	printf("expr_list:\n");
	printf("\texpression PRINT\n");
	printf("\texpression PRINT expr_list\n\n");

	printf("expression:\n");
	printf("\texpression + term\n");
	printf("\texpression - term\n");
	printf("\tterm\n\n");

	printf("term:\n");
	printf("\tterm / primary\n");
	printf("\tterm * primary\n");
	printf("\tprimary\n\n");

	printf("primary\n");
	printf("\tNUMBER\n");
	printf("\tNAME\n");
	printf("\tNAME = expression\n");
	printf("\t-primary\n");
	printf("\t(expression)\n\n");
	
	printf("————————————————————————————————————————————————————————\n");
	printf("命令选项：\n");
	printf("help\t--显示帮助信息\n");
	printf("ls\t--列出现有变量\n");
	printf("exit\t--退出程序\n");
        
	printf("————————————————————————————————————————————————————————\n");
	printf("特别注意：如果您需要自定义变量，请将变量与其它字符分开写，如：pi = 3.14为正确的写法，pi=3.14为错误的写法.不要使用与命令相同的变量。\n\n");
	printf("————————————————————————————————————————————————————————\n");
}

double
expr( bool get )
{
	double left = term( get );
	while( true )
	{
		switch( cur_input )
		{
			case PLUS:
				left += term( true );
				break;
			case MINUS:
				left -= term( true );
				break;
			default:
				return left;
		}
	}
}

double
term( bool get )
{
	double left = prim( get );
//db("term.head");
	while( true )
	{	
//db("term.while");
		switch( cur_input )
		{	
			case MUL:
				left *= prim( true );
				break;
			case DIV:
			{
				double d;
				d = prim( true );  
				left /= d;
			}	
			default:
				return left;
		}
	}
}

INPUT
get_token()
{
	char ch = 0;
	
	do
	{
		ch = getchar();
		
		if( ch == EOF )
			return cur_input = END;
	}
	while( ch != '\n' && isspace( ch ) );

	switch( ch )
	{	
		case ';':
		case '\n':
			return cur_input = PRINT;
		case 0:
			return cur_input = END;
		case ':':
		case '*':
		case '/':
		case '+':
		case '-':
		case '(':
		case ')':
		case '=':
//db("gettock.swich.case;*/+...");
			return cur_input = (INPUT) ch;
		case '0':
		case '1':
		case '2':
		case '3':	
		case '4':	
		case '5':	
		case '6':	
		case '7':	
		case '8':	
		case '9':	
		case '.':
			ungetc( ch, stdin );
			scanf( "%lf", &number_value );
			return cur_input = NUMBER;
		default:
			if(isalpha( ch ) )
			{
				ungetc( ch, stdin );
				scanf("%s", string_value);
				return cur_input = NAME;
			}
			printf( "读入出错！" );
			return cur_input = PRINT;
	}
}

double
prim( bool get )
{
	if( get )
		get_token();
	switch( cur_input )
	{
		case NUMBER:
		{
			double v = number_value;
			get_token();
			return v;
		}
		case NAME:
		{ 
			double v;
			if( get_token() == ASSIGN )
			{  
				char tmp[20];				//expr运行过程中会改变string_value,用此变量先保存string_value
				strcpy( tmp, string_value ); 
				v = expr( true );
				table( tmp, v );
			}
			else
				v = table_get( string_value );
			return v;
		}
		case MINUS:					//一元
			return -prim( true );
		case LP:
		{
			double e = expr( true );
			if( cur_input != RP )
			{
				printf("丢失)");
				exit(1);
			}
			get_token();
			return e;
		}
		default:
			printf("primary need!(参见语法帮助[help]）\n");
			restart( ERROR );
			return 0.0;				//不会返回
	}
}

PtrTable
table_locate( char *str )
{
	PtrTable p = head;
	
	while( p != NULL && strcmp( p->name, str ) != 0 )
		p = p->next;
	return p;
}

double
table_get( char *str )
{
	PtrTable p = table_locate( str );
	
	if( p != NULL)
		return p->value;
	deal_str( str );
	printf( "未定义的变量：%s\n按下enter程序继续：", str );
	restart( ERROR );
	return 0.0;						//永远不会返回
}

PtrTable
table_add( char *str, double value )
{
	PtrTable p = head;
	
	while( p->next != NULL )
		p = p->next;
	p->next = ( PtrTable ) malloc( sizeof( struct Table ) );
	p = p->next;
	strcpy( p->name, str );
	p->value = value;
	p->next = NULL;	
	
	return p;
}

PtrTable
table( char *str, double value )
{
	PtrTable p;
	if( ( p = table_locate( str ) ) != NULL )
	{
		p->value = value;
		return p;
	}
	p = table_add( str, value );
	p->next = NULL;
	return p;
}

void
table_init()
{
	head = ( PtrTable ) malloc( sizeof( struct Table ) );
	strcpy( head->name, "e" );
	head->value = 2.71828;
	head->next = NULL;
	table_add( "pi", 3.14159 );
}

void
table_free( PtrTable p )
{
	if( p == NULL )
		return;
	if( p->next == NULL )
		free( p );
	if( p->next->next == NULL );
	{
		free( p->next );
		p->next = NULL;
	}
	table_free( p->next );
}

void
restart( int ret )
{
	longjmp( rerun, ret );
}

void
deal_str( char *p )
{
	if( strcmp( p, "help" ) == 0 )
	{
		help_msg();
		restart( NOERROR );
	}
	if( strcmp( p, "ls" ) == 0 )
	{
		table_print();
		restart( NOERROR );
	}
	if( strcmp( p, "exit" ) == 0 )
	{
		raise( SIGINT );
	}
}

void
table_print()
{
	PtrTable p = head;
	while( p != NULL )
	{
		printf("\t%s\t=\t%lf\n", p->name, p->value );
		p = p->next;
	}
}	
