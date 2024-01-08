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

u8 code smgduan[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};//������ַ���������
u8 code TAB[8]={0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};//8*8LED����ɨ������

u8 FRESH[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//8*8LED����ˢ������
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
};//̰ʳ�������¼����

u8 random(u16 seed,u8 MIN,u8 MAX)//��������ɺ���
{
	srand(seed);//�������ʼ��
	return rand()%(MAX-MIN+1)+MIN;
}

void delay(u16 i)//�ӳٺ���
{
	while(i--);
}

void Hc595SendByte(u8 dat)//8*8LED�������Ժ���
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

u8 KeyDown(void)//�������ɨ�躯��
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

u8 next(u8 direct,position)//̰������ͷǰ��������һ������㺯��,����Խ�߽�
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

u8 main()//������
{ 	
	u16 seed;//���������
	u8 i,j,direct,food,length,flag;
	seed=0xffff;//��������Ӹ�ֵ
	i=0;
	j=0;
	direct=0;
	food=0;
	length=0;
	flag=0;
	while(1)//��ѭ��
	{
		if(RANDOM_CREATE==0)//���¶�������K1
		{
			delay(1000);
			if(RANDOM_CREATE==0)//ȷ��K1���º�LEDģ�������
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
					seed--;//��K1���ְ���ʱ���ݰ��µ�ʱ��������������
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
		if(RANDOM_EXIT==0 && seed!=0xffff)//���¶�������K2
		{
			delay(1000);
			if(RANDOM_EXIT==0)//ȷ��K2���º�LEDģ�����Բ�������Ϸ
			{
				LED=0xff;
				break;
			}	
		}
	}
	direct=random(seed--,0,3);//���̰������ʼ����
	switch(direct)//�޸ķ���Ϊ������ֵ
	{
		case(0X00):direct=0x06;break;//right
		case(0X01):direct=0x01;break;//up
		case(0X02):direct=0x04;break;//left
		case(0X03):direct=0x09;break;//down
	}
	i=random(seed--,0,63);//�����ͷ��ʼ����
	food=random(seed--,0,63);//�����һ��ʳ������
	while(i==food || i%8==food%8 || i/8==food/8)
	{
		food=random(seed--,0,63);//�����ͷ��ʼ����͵�һ��ʳ������һ�»�����ͷ��ʼ����������ϴ��ڵ�һ��ʳ�����������
	}
	SNAKE[0]=i;//��ͷ��ʼ������̰���������¼����
	length=0x01;//��ʼ̰���߳���Ϊ1
	LED=0xff^length;//̰���߳���ֵȡ��������LEDģ��
	flag=0x00;//������Ϸʱ��ʶΪ0
	FRESH[i/8]=_crol_(0x01,i%8);//��ͷ��ʼ������8*8LED����ˢ������
	FRESH[food/8]=_crol_(0x01,food%8);//��һ��ʳ��������8*8LED����ˢ������
	while(1)//��Ϸѭ��
	{
		j=KeyDown();
		if(length==1)//��̰���߳���Ϊ1ʱ�жϾ�����̰��·���
		{
			if(j==0x01 || j==0x04 || j==0x06 || j==0x09)
			{
				direct=j;
			}
		}
		else//��̰���߳��Ȳ�Ϊ1ʱ�жϾ�����̰��·���(̰���߳��Ȳ�Ϊ1ʱ������ԭ�ص�ͷ)
		{
			if((j==0x01 && direct!=0x09)||(j==0x04 && direct!=0x06)||(j==0x06 && direct!=0x04)||(j==0x09 && direct!=0x01))
			{
				direct=j;
			}
		}
		j=next(direct,SNAKE[0]);//������ͷǰ������һ������
		if(flag==0x00)//��Ϸ����״̬
		{
			for(i=1;i<length;i++)
			{
				if(SNAKE[0]==SNAKE[i])break;//�ж���ͷ�Ƿ�ײ������
			}
			if(i<length)
			{
				flag=0x01;//��ͷײ������������״̬
			}
		}
		if(flag==0x01)//��Ϸ����
		{
			flag=length%0x0a;//����̰���߳���BCD���λ
			length/=0x0a;//����̰���߳���BCD��ʮλ
			LSB=0;
			LSC=0;
			while(1)
			{
				LSA=0;
				P0=smgduan[flag];//���Զ�̬����ܸ�λ
				delay(500);
				LSA=1;
				P0=smgduan[length];//���Զ�̬�����ʮλ
				delay(500);
				if(RESET==0)//���¶�������K4
				{
					delay(1000);
					if(RESET==0)//ȷ��K4���º�����������,�����¿�ʼ��Ϸ
					{
						return 0;
					}	
				}
			}
		}
		if(j!=food)//�����ͷ����һ�����겻��ʳ��
		{
			for(i=length-1;i>0;i--)
			{
				SNAKE[i]=SNAKE[i-1];//����̰������������
			}
			SNAKE[0]=j;//������ͷ����
		}
		else//�����ͷ����һ��������ʳ��
		{
			length++;//̰���߳���+1
			LED=0xff^length;//̰���߳���ֵȡ��������LEDģ��
			for(i=length-1;i>0;i--)
			{
				SNAKE[i]=SNAKE[i-1];//����̰������������
			}
			SNAKE[0]=j;//������ͷ����
			if(length!=0x40)//���̰���߳���û��ռ������8*8LED����,���пռ��������ʳ��
			{
				while(1)
				{
					food=random(seed--,0,63);//���ʳ������
					for(i=0;i<length;i++)
					{
						if(food==SNAKE[i])break;//���̰����ȫ�������ʳ������һ�����������
					}
					if(i>=length)break;
				}
			}
			else//���̰���߳���ռ������8*8LED����,��Ϸ����
			{
				flag=length%0x0a;//����̰���߳���BCD���λ
				length/=0x0a;//����̰���߳���BCD��ʮλ
				LSB=0;
				LSC=0;
				while(1)
				{
					LSA=0;
					P0=smgduan[flag];//���Զ�̬����ܸ�λ
					delay(500);
					LSA=1;
					P0=smgduan[length];//���Զ�̬�����ʮλ
					delay(500);
					if(RESET==0)//���¶�������K4
					{
						delay(1000);
						if(RESET==0)//ȷ��K4���º�����������,�����¿�ʼ��Ϸ
						{
							return 0;
						}	
					}
				}
			}
		}
		for(i=0;i<8;i++)//��λ��Ļˢ����������
		{
			FRESH[i]=0x00;
		}
		for(i=0;i<length;i++)
		{
			FRESH[SNAKE[i]/8]|=_crol_(0x01,SNAKE[i]%8);//̰����ȫ��������8*8LED����ˢ������
		}
		FRESH[food/8]|=_crol_(0x01,food%8);//ʳ��������8*8LED����ˢ������
		for(i=0;i<8;i++)//ˢ��8*8LED����
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