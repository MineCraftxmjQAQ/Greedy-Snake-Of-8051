#include<reg51.h>
#include<stdlib.h>
#include<intrins.h>

#define GPIO_DIG P0
#define GPIO_KEY P1
#define LED P2

sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;
sbit RANDOM_EXIT=P3^0;
sbit RANDOM_CREATE=P3^1;
sbit RESET=P3^3;
sbit SER=P3^4;
sbit RCLK=P3^5;
sbit SRCLK=P3^6;

typedef unsigned char u8;
typedef unsigned int u16;

u8 code smgduan[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};//数码管字符定义数组
u8 code TAB[8]={0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};//8*8LED点阵扫描数组

u8 FRESH[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//8*8LED点阵刷新数组
u8 SNAKE[64]=
{
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};//贪食蛇坐标记录数组

u8 random(u16 seed,u8 MIN,u8 MAX)//随机数生成函数
{
	srand(seed);//随机数初始化
	return rand()%(MAX-MIN+1)+MIN;
}

void delay(u16 i)//延迟函数
{
	while(i--);
}

void Hc595SendByte(u8 dat)//8*8LED点阵送显函数
{
	u8 a;
	SRCLK=0;							
	RCLK=0;                
	for(a=0;a<8;a++) 
	{
		SER=dat>>7;
		dat<<=1; 
		SRCLK=1;
		_nop_();
		_nop_();
		SRCLK=0;	     
	}
	RCLK=1;
	_nop_();
	_nop_();
	RCLK=0;     
}

u8 KeyDown(void)//矩阵键盘扫描函数
{
	u8 KeyValue=0x00;
	GPIO_KEY=0x0f;
	if(GPIO_KEY!=0x0f)
	{
		delay(1000);
		if(GPIO_KEY!=0x0f)
		{
			GPIO_KEY=0X0f;
			switch(GPIO_KEY)
			{
				case(0X07):KeyValue=0x00;break;
				case(0X0b):KeyValue=0x01;break;
				case(0X0d):KeyValue=0x02;break;
				case(0X0e):KeyValue=0x03;break;
			}
			GPIO_KEY=0Xf0;
			switch(GPIO_KEY)
			{
				case(0X70):break;
				case(0Xb0):KeyValue+=0x04;break;
				case(0Xd0):KeyValue+=0x08;break;
				case(0Xe0):KeyValue+=0x0c;break;
			}
		}
	}
	return KeyValue;
}

u8 next(u8 direct,position)//贪吃蛇蛇头前进方向下一坐标计算函数,允许穿越边界
{
	u8 temp;
	if(direct==0x04)//left
	{
		position-=0x08;
		if(position<0)position+=0x40;
	}
	else if(direct==0x09)//down
	{
		if(position==0)
		{
			position=0x07;
		}
		else
		{
			temp=position/8;
			position-=0x01;
			if(temp!=position/8)position+=8;
		}
	}
	else if(direct==0x06)//right
	{
		position+=0x08;
		if(position>0x3f)position-=0x40;
	}
	else if(direct==0x01)//up
	{
		temp=position/8;
		position+=0x01;
		if(temp!=position/8)position-=8;
	}
	else
	{
		position=0xff;
	}
	return position;
}

u8 main()//主函数
{ 	
	u16 seed;//随机数种子
	u8 i,j,direct,food,length,flag;
	seed=0xffff;//随机数种子赋值
	i=0;
	j=0;
	direct=0;
	food=0;
	length=0;
	flag=0;
	while(1)//主循环
	{
		if(RANDOM_CREATE==0)//按下独立按键K1
		{
			delay(1000);
			if(RANDOM_CREATE==0)//确认K1按下后LED模块跑马灯
			{
				direct=0xfe;
				LED=direct;
				delay(5000);
				for(i=0;i<4;i++)
				{
					direct=direct<<1;
					LED=direct;
					delay(5000);
				}
				while(RANDOM_CREATE==0)
				{
					seed--;//当K1保持按下时根据按下的时间更新随机数种子
					j=direct&0x80;
					j=j>>7;
					direct=direct<<1;
					direct=direct&0xfe;
					direct=direct|j;
					LED=direct;
					delay(5000);
				}	
			}
		}
		if(RANDOM_EXIT==0 && seed!=0xffff)//按下独立按键K2
		{
			delay(1000);
			if(RANDOM_EXIT==0)//确认K2按下后LED模块消显并进入游戏
			{
				LED=0xff;
				break;
			}	
		}
	}
	direct=random(seed--,0,3);//随机贪吃蛇起始方向
	switch(direct)//修改方向为方向函数值
	{
		case(0X00):direct=0x06;break;//right
		case(0X01):direct=0x01;break;//up
		case(0X02):direct=0x04;break;//left
		case(0X03):direct=0x09;break;//down
	}
	i=random(seed--,0,63);//随机蛇头初始坐标
	food=random(seed--,0,63);//随机第一个食物坐标
	while(i==food || i%8==food%8 || i/8==food/8)
	{
		food=random(seed--,0,63);//如果蛇头初始坐标和第一个食物坐标一致或者蛇头初始坐标的行列上存在第一个食物则重新随机
	}
	SNAKE[0]=i;//蛇头初始坐标入贪吃蛇坐标记录数组
	length=0x01;//初始贪吃蛇长度为1
	LED=0xff^length;//贪吃蛇长度值取反后送显LED模块
	flag=0x00;//正常游戏时标识为0
	FRESH[i/8]=_crol_(0x01,i%8);//蛇头初始坐标送8*8LED点阵刷新数组
	FRESH[food/8]=_crol_(0x01,food%8);//第一个食物坐标送8*8LED点阵刷新数组
	while(1)//游戏循环
	{
		j=KeyDown();
		if(length==1)//当贪吃蛇长度为1时判断矩阵键盘按下方向
		{
			if(j==0x01 || j==0x04 || j==0x06 || j==0x09)
			{
				direct=j;
			}
		}
		else//当贪吃蛇长度不为1时判断矩阵键盘按下方向(贪吃蛇长度不为1时不允许原地掉头)
		{
			if((j==0x01 && direct!=0x09)||(j==0x04 && direct!=0x06)||(j==0x06 && direct!=0x04)||(j==0x09 && direct!=0x01))
			{
				direct=j;
			}
		}
		j=next(direct,SNAKE[0]);//计算蛇头前进的下一个坐标
		if(flag==0x00)//游戏正常状态
		{
			for(i=1;i<length;i++)
			{
				if(SNAKE[0]==SNAKE[i])break;//判断蛇头是否撞上蛇身
			}
			if(i<length)
			{
				flag=0x01;//蛇头撞上蛇身进入结束状态
			}
		}
		if(flag==0x01)//游戏结束
		{
			flag=length%0x0a;//计算贪吃蛇长度BCD码个位
			length/=0x0a;//计算贪吃蛇长度BCD码十位
			LSB=0;
			LSC=0;
			while(1)
			{
				LSA=0;
				P0=smgduan[flag];//送显动态数码管个位
				delay(500);
				LSA=1;
				P0=smgduan[length];//送显动态数码管十位
				delay(500);
				if(RESET==0)//按下独立按键K4
				{
					delay(1000);
					if(RESET==0)//确认K4按下后主函数结束,可重新开始游戏
					{
						return 0;
					}	
				}
			}
		}
		if(j!=food)//如果蛇头的下一个坐标不是食物
		{
			for(i=length-1;i>0;i--)
			{
				SNAKE[i]=SNAKE[i-1];//更新贪吃蛇蛇身坐标
			}
			SNAKE[0]=j;//更新蛇头坐标
		}
		else//如果蛇头的下一个坐标是食物
		{
			length++;//贪吃蛇长度+1
			LED=0xff^length;//贪吃蛇长度值取反后送显LED模块
			for(i=length-1;i>0;i--)
			{
				SNAKE[i]=SNAKE[i-1];//更新贪吃蛇蛇身坐标
			}
			SNAKE[0]=j;//更新蛇头坐标
			if(length!=0x40)//如果贪吃蛇长度没有占满整个8*8LED点阵,则还有空间可以生成食物
			{
				while(1)
				{
					food=random(seed--,0,63);//随机食物坐标
					for(i=0;i<length;i++)
					{
						if(food==SNAKE[i])break;//如果贪吃蛇全体坐标和食物坐标一致则重新随机
					}
					if(i>=length)break;
				}
			}
			else//如果贪吃蛇长度占满整个8*8LED点阵,游戏结束
			{
				flag=length%0x0a;//计算贪吃蛇长度BCD码个位
				length/=0x0a;//计算贪吃蛇长度BCD码十位
				LSB=0;
				LSC=0;
				while(1)
				{
					LSA=0;
					P0=smgduan[flag];//送显动态数码管个位
					delay(500);
					LSA=1;
					P0=smgduan[length];//送显动态数码管十位
					delay(500);
					if(RESET==0)//按下独立按键K4
					{
						delay(1000);
						if(RESET==0)//确认K4按下后主函数结束,可重新开始游戏
						{
							return 0;
						}	
					}
				}
			}
		}
		for(i=0;i<8;i++)//复位屏幕刷新送显数组
		{
			FRESH[i]=0x00;
		}
		for(i=0;i<length;i++)
		{
			FRESH[SNAKE[i]/8]|=_crol_(0x01,SNAKE[i]%8);//贪吃蛇全体坐标送8*8LED点阵刷新数组
		}
		FRESH[food/8]|=_crol_(0x01,food%8);//食物坐标送8*8LED点阵刷新数组
		for(i=0;i<8;i++)//刷新8*8LED点阵
		{
			P0=TAB[i];
			Hc595SendByte(FRESH[i]);
			delay(100);
			P0=0xff;
			Hc595SendByte(0x00);
		}
		delay(2000);
	}
}