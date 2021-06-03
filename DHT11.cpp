DHT11头文件：
#ifndef _DHT11_H_
#define _DHT11_H_

#include "stm32f10x.h"
#include "bitband_cm3.h" // 位带操作头文件

#define DHT11_REV_DATA  PBin(6)
#define DHT11_SEND_DATA PBout(6)

void DHT11_Init(void);
int DHT11_Read_Data(uint8_t *Temp_H,uint8_t* Temp_L,uint8_t* RH_H,uint8_t* RH_L);



#endif 



DHT11源文件：
#include "dht11.h"
#include "systick.h"
#if 0  //0---库函数  1---寄存器
// DTH11  ---> PB6
// 输出模式 
void DHT11_OUT(void) 
{
  GPIOB->CRL &=~ (0xf<<24);
        GPIOB->CRL |=  (3<<24);//通用推挽输出模式 50M
}

// 输入模式
void DHT11_IN(void)  
{
        GPIOB->CRL &=~ (0xf<<24);
        GPIOB->CRL |=  (4<<24);//配置浮空输入(外接上拉电阻)
}

//DHT11初始化
void DHT11_Init(void) 
{
        RCC->APB2ENR |= (1<<3); // 开启PB端口时钟
        DHT11_IN( );// DHT11 的DATA 引脚处于输入状态，时刻检测外部信号
        delay_ms(500);
        delay_ms(500);//DHT11 上电后要等待 1S 以越过不稳定状态在此期间不能发送任何指令
}
#else
// DTH11  ---> PB6
// 输出模式 
void DHT11_OUT(void) 
{
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;
        GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;//通用推挽输出
        GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;//50M
        GPIO_Init(GPIOB, &GPIO_InitStruct);//PB6
}

// 输入模式
void DHT11_IN(void)  
{
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;
        GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//配置浮空输入(外接上拉电阻)
        GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;//50M
        GPIO_Init(GPIOB, &GPIO_InitStruct);//PB6
}

//DHT11初始化
void DHT11_Init(void) 
{
        RCC->APB2ENR |= (1<<3); // 开启PB端口时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开启PB端口时钟.
        DHT11_IN( );// DHT11 的DATA 引脚处于输入状态，时刻检测外部信号
        delay_ms(500);
        delay_ms(500);//DHT11 上电后要等待 1S 以越过不稳定状态在此期间不能发送任何指令
}
#endif
//起始信号
void DHT11_Start(void)
{
        DHT11_SEND_DATA=0;
        delay_ms(18);
        DHT11_SEND_DATA=1;
        delay_us(20);//释放总线
}

//应答信号
//返回值 0--无应答  1--有应答
u8 DHT11_Ack(void)
{
        if(DHT11_REV_DATA!=0)
        {
                return 0;
        }
        while(DHT11_REV_DATA==0);//等待80us低电平应答结束
        
        while(DHT11_REV_DATA==1);//等待80us高电平应答结束
        
        return 1;
}

//读取一个字节数据
u8 DHT11_Read_Byte(void)
{
        u8 data=0;
        u8 i;
        //接受数据,每个数据以50us低电平开始
        for(i=0;i<8;i++)
        {
                data <<= 1;
                while(!DHT11_REV_DATA);//等待发送数据的时序变为高电平 
                delay_us(40);//23~27u为数据0，68~74us为u数据1
                if(DHT11_REV_DATA==1)
                {
                        data |=1;//数据1
                        while(DHT11_REV_DATA);//等待高电平应答结束
                }
        }
        return data;
}

//读取温湿度
int DHT11_Read_Data(u8 *Temp_H,u8* Temp_L,u8* RH_H,u8* RH_L)
{
        u8 check;
        DHT11_OUT( );
        DHT11_Start( );
        DHT11_IN( );//切换为输入
        if( DHT11_Ack( )==0 )
        {
                return -1;
        }
        
        //读取数据
        *RH_H=DHT11_Read_Byte( );
        *RH_L=DHT11_Read_Byte( );
        *Temp_H=DHT11_Read_Byte( );
        *Temp_L=DHT11_Read_Byte( );
        check=DHT11_Read_Byte( );        
        
        //结束读取
        delay_ms(50);
        DHT11_OUT( );//切换为输出
        DHT11_SEND_DATA=1;//释放总线
        
        //校验数据
        if(check !=(*Temp_H+*Temp_L+*RH_H+*RH_L) )
        {
                return -2;
        }
        return 0;
}

