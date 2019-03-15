# 裸机多功能按键扫描模块

## 简介
本按键模块主要是参考ShuifaHe的STM32项目（github地址: https://github.com/ShuifaHe/STM32 ）修改而来，并改进诸多细节以及长时间等待后KEYTIME未判断是否溢出的BUG，使之更加方便移植到别的嵌入式系统中去

## 移植
1.根据配置文件key_config.h中的注释修改对应的宏，需注意KEYNUMMAX和KEY_COMBINATION_MAX宏必须为对应的最大值

2.移植IO口初始化函数**void KeyInit(void)** ，也可直接调用系统中已经写好的按键初始化IO口函数，不做强制性要求

3.移植硬件按键编码函数**static KeyS_Type _GetHalKeyCode(void))** ，此函数主要为确定IO口的有效触发电平并进行相应的编码（即确定唯一的键值），可以按照如下方式进行编写：
```c
static KeyS_Type _GetHalKeyCode(void)
{
	KeyS_Type ktmp=0;
	if(PEin(4) == 0) 	ktmp|=1<<KB_KEY0;//按键0低电平有效
	if(PEin(3) == 0) 	ktmp|=1<<KB_KEY1;//按键1低电平有效
	if(PEin(2) == 0) 	ktmp|=1<<KB_KEY2;//按键2低电平有效
	if(PEin(0) == 1) 	ktmp|=1<<KB_WKUP;////按键 WKUP 高电平有效
	return ktmp;
}
```
4.可选择是否修改按键判断函数**static unsigned char _GetKey(void)** ，此函数主要为定义系统中起作用的按键事件，可以增加定义一些组合键事件。所有按键的单击、双击、长按、保持等键值输出已经实现，如果有需要则在该函数中增加组合键等键值。具体修改请参考注释

5.当有必要处理一些紧急按键响应的时候（按键处理占用时间短（无延时语句、长循环等），按键要求强实时处理），可以在key_config.h中开启REALTIMEKEY_ENABLE宏（注：实时性受INTERRUPT_TICKS宏的影响），此时就需要修改key.c中的**static unsigned char _KeyPrePro(void)** 函数，强实时性的事件均应该位于此处。

## 使用
1.主循环中使用读取键值函数**ReadKey()** ，获取当前的键值，再通过宏**KEY_EVENT(keynum,event)** 判断按键事件
```c
while(1)
{
	key=ReadKey();	//得到键值
	switch(key)
	{				 
		//常规一般按键测试（按下键就起作用）：
		case KEY_EVENT(KB_KEY2,PRESS_DOWN):			//KEY2按下即有效，控制LED0翻转	
			LED0=!LED0;
			break;
		case KEY_EVENT(KB_KEY0,PRESS_DOWN):			//KEY0按下即有效，控制LED1翻转	 
			LED1=!LED1;
			break;
		
		//下面可自由增加其它按键测试，比如（仅举数例）：
		case KEY_EVENT(KB_KEY1,SINGLE_CLICK):		//KEY1键单击（快速按下再松开）	 
			LED0=!LED0;
			break;
		case KEY_EVENT(KB_WKUP,DOUBLE_CLICK):		//WKUP键双击（连续单击两次）
			BEEP=!BEEP;
			break;
		case WKUP_PLUSKEY0_PRES:					//WKUP+KEY0组合按键（先按下WKUP再按下KEY0）
			LED0=!LED0;
			LED1=!LED1;
			break;
		case KEY_EVENT(KB_WKUP,LONG_RRESS_START):	//长按键WKUP
			LED1=!LED1;
			break;
		case KEY_EVENT(KB_KEY1,LONG_PRESS_HOLD):	//长按键KEY1
			LED0=!LED0;
			break;
	}
	delay_ms(50);//模拟CPU干其它工作的时间
}
```
2.在key_config.h中开启REALTIMEKEY_ENABLE宏后（注：实时性受INTERRUPT_TICKS宏的影响），部分要求强实时性的紧急按键事件应该在key.c中的**static unsigned char _KeyPrePro(void)** 函数处理
```c
static unsigned char _KeyPrePro(void)
{
#ifdef REALTIMEKEY_ENABLE
	unsigned char ret=0;
	unsigned char newkeytmp=0;
	newkeytmp=_GetKey();

    //部分要求强实时性的紧急按键处理；
	switch(newkeytmp)
	{
		case KEY_EVENT(KB_KEY1,DOUBLE_CLICK)://KEY1双击，执行两灯同时翻转（仅作为示例）
			LED0=!LED0;
			LED1=!LED1;
        break;
        default:
			ret=newkeytmp;
	}
	return ret;
#else
    return _GetKey();
#endif
}
```
