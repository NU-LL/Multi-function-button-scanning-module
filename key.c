/*
 * Copyright (c) 2018 NU-LL<1125934312@qq.com>
 * All rights reserved
 * 
 * data:2018/11/13
 * version:V0.1
 */
#include "key.h"
//用户按键读取电平头文件
// #include "gpio.h"

//组合键判断宏
#define IS_COMBINATION_KEY(PreviousKey,NextKey)								(CONT==(KEY_ON(PreviousKey)+KEY_ON(NextKey))) && (TRI_FLAG==KEY_ON(NextKey))
#define IS_COMBINATION_KEY_PLUS(PreviousKey,MiddleKey,NextKey)				(CONT==(KEY_ON(PreviousKey)+KEY_ON(MiddleKey)+KEY_ON(NextKey))) && (TRI_FLAG==KEY_ON(NextKey))

//全局变量
volatile KeyS_Type           TRI_FLAG=0;    //本次读键时的按键触发状态 在某按键被按下后有且只有一次读取到对应位为1;
volatile KeyS_Type           CONT    =0;    //本次读键时的实时键态 处于按下的对应位为1，处于松开的对应位为0;
volatile unsigned short int  KEYTIME =0;    //本次读键时当前键态持续的时长
volatile static KeyS_Type    KEYSTABLE=0;   //存有稳定(消除抖动后)的键态(读键前)

//本地函数声明
static unsigned char _GetKeyState(unsigned char keynum);


/**************************** 以下为待移植函数 *****************************/

/**
  * @brief  按键初始化函数 可以在别处定义，只要初始化IO即可
  * @param  None
  * @retval None
  */
void KeyInit(void)
{
	
}

/**
  * @brief  硬件按键编码（为应用三行读键程序而准备）
  * 		以战舰版的四键为例（最大暂支持16键，KeyS_Type定义为unsigned int则可支持32键）
  * @param  None
  * @retval 对应键值
  */
static KeyS_Type _GetHalKeyCode(void)
{
	KeyS_Type ktmp=0;
	if(PEin(4) == 0) 	ktmp|=1<<KB_KEY0;//按键0低电平有效
	if(PEin(3) == 0) 	ktmp|=1<<KB_KEY1;//按键1低电平有效
	if(PEin(2) == 0) 	ktmp|=1<<KB_KEY2;//按键2低电平有效
	if(PEin(0) == 1) 	ktmp|=1<<KB_WKUP;////按键 WKUP 高电平有效
	return ktmp;
}

/**
  * @brief  按键判断函数(请根据具体需求修改)
  * 		最终由 _GetAndSaveKey() 在 KeyHandler() 中调用，存入按键队列，主循环调用时请使用 ReadKey()
  * 
  * 		可适应的按键类型如下：
  * 		普通：按下即有效，不用等按键释放
  * 		单击：按下再松开后有效
  * 		双击：快速单击两次（两次单击之间时间不超过SHORT_TICKS）
  * 		长按：按下超过规定的时间LONG_TICKS，超过后可连续输出，通过软件可设置间隔一定时间输出一次键值
  * 		组合：双键组合（其实多键组合也可同理实现）
  * 		
  * 		按键的判断条件设定技巧：
  * 		TRI_FLAG	对应按键的触发状态，在某按键被按下后有且只有一次读取到对应位为1
  * 		CONT		当前按键的状态，处于按下的对应位为1，处于松开的对应位为0
  * 		KEYTIME		当前键态持续的时间
  * @param  None
  * @retval 稳定的键值（直接处理事先定义的键值即可）
  */
static unsigned char _GetKey(void)
{
	unsigned char i=0;
	unsigned char keyp=0;

	//以下是按键判断，用户可根据需要随意添加或删改（注释掉的部分也可根据需要作为参考语句使用）
    //注意：排在前面的判断条件具有高的优先级，一旦条件满足即刻返回，不再执行后续语句。
    if(IS_COMBINATION_KEY(KB_WKUP,KB_KEY0))
    { //WKUP+KEY0组合按键（KEY0必须最后按下）
		_GetKeyState(KB_CLR); //复位状态机，防止本按键对其干扰(本按键与状态机有冲突时请调用此句)
		return WKUP_PLUSKEY0_PRES;
    }
	if(IS_COMBINATION_KEY_PLUS(KB_KEY0,KB_KEY1,KB_KEY2))
    { //KEY0+KEY1+KEY2组合按键（KEY2必须最后按下）
		_GetKeyState(KB_CLR); //复位状态机，防止本按键对其干扰(本按键与状态机有冲突时请调用此句)
		return KEY0_KEY1_KEY2_PRES;
    }

    //以下是使用状态机得到判断单击、双击、长按、保持等键码
	//如需修改触发状态种类，请修改 _GetKeyState 状态机（如：增加三连击、四连击等）
	for(i=0;i<KEYNUMMAX;i++)
    {
        keyp=_GetKeyState(i);
        if(keyp)
            return keyp;
    }
	return keyp;
}

/**
  * @brief  按键预处理程序:  允许对有强实时性需要的紧要按键无须排队优先执行，其效果类似回调函数
  * 		本函数根据实际项目需求可以有三种具体实现模式选择：
  * 		模式一（需使能宏 REALTIMEKEY_ENABLE ）：如果按键处理占用时间短（无延时语句、长循环等），按键要求强实时处理，则可以把所有的按键处理都放在这里
  * 		模式二：对按键处理实时性要求不高，能够忍受一定程序的按键响应时延，可以把所有按键处理放在主循环中查询响应
  * 		模式三（需使能宏 REALTIMEKEY_ENABLE ）：强实时性需要紧急处理的键，直接在这里写执行代码，其它允许延时的键留待主循环查询处理
  * @param  None
  * @retval None
  */
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
			// LED0=!LED0;
			// LED1=!LED1;
        break;
        default:
			ret=newkeytmp;
	}
	return ret;
#else
    return _GetKey();
#endif
}




//**********************  以下为通用函数，一般不要修改  ****************************
volatile static unsigned char KEYBUFF[KEY_BUFFSIZE];//按键缓冲区
volatile static unsigned char KEYIDX=0;//按键缓冲区当前数据位置

/**
  * @brief  后台读键，如果有键值则存入按键缓冲区（由KeyHandler调用）
  * @param  None
  * @retval None
  */
static void _GetAndSaveKey(void)
{
	unsigned char newkeytmp;
	//按键已经全部释放
	if((KEYTIME>=LONG_TICKS) && (CONT==0))//简化的条件
	// if((KEYTIME>=LONG_TICKS) && (CONT==0 && TRI_FLAG==0))//严格的条件
    {//键盘长时间闲置，直接返回（绝大部分时间基本都是这种状态，此举将大大节省CPU资源）
		if(KEYTIME > 65530)//此句防止KEYTIME溢出(KEYTIME由扫键程序累增)
			KEYTIME=LONG_TICKS;
        return;
    }
	TRI_FLAG=KEYSTABLE & (KEYSTABLE ^ CONT); //调用三行读键方法,其实核心只有此行，使得TRI_FLAG在某按键被按下后有且只有一次读取到对应位为1;
	CONT=KEYSTABLE;
	newkeytmp=_KeyPrePro();//从键预处理程序中读键值
	if(newkeytmp)//如果有新的键值
	{
		KEYBUFF[KEYIDX++]=newkeytmp;//存入按键缓冲区(KEYIDX永远指向下一空位置)
		if(KEYIDX==KEY_BUFFSIZE)
            KEYIDX=0;//按键缓冲区循环使用
	}
}

/**
  * @brief  从按键缓存中读取按键值（由主循环调用）
  * @param  None
  * @retval 0	: 	无缓存，读取失败 
  * 		其他:	读取成功
  */
unsigned char ReadKey(void)
{
	volatile static unsigned char readkeyidx=0;//读键位置
	if(readkeyidx==KEY_BUFFSIZE)
        readkeyidx=0;//按键缓冲区循环使用
	if(readkeyidx==KEYIDX)
        return 0;//键已经取尽，返回0
	return KEYBUFF[readkeyidx++];
}

/**
  * @brief  按键扫描函数：一般由Systick中断服务程序以5ms一次的时间节拍调用此函数
  * 		采用了键盘自适应变频扫描措施，在键盘正常稳定期间（非消抖期间）扫描频率降低以减少CPU资源占用
  * 		该函数将影响全局变量：消除抖动后的稳定键态值 KEYSTABLE 及累计时长 KEYTIME
  * @param  None
  * @retval None
  */
static void _KeyScanStick(void)
{
	volatile KeyS_Type keyvaltemp;//临时获取的新键值
	volatile static KeyS_Type keyvaltempold=0;//保存本地的旧键值
	volatile static unsigned short int debounce_cnt=0;//消抖时间计数器
	volatile static unsigned short int debouncing=0;//消抖标志位
	
	KEYTIME++;//在稳定键态（包括无键）状态下，全局变量KEYTIME是持续增加的
	if((!debouncing) && (KEYTIME%NORMAL_SCAN_FREQ))//非消抖期间且累计计时不是NORMAL_SCAN_FREQ的倍数(即TICKS_INTERVAL*NORMAL_SCAN_FREQ(默认30ms)才扫描一次)
		return;	//不扫描键盘直接返回
	
	keyvaltemp=_GetHalKeyCode();//扫描键盘，得到实时键值（合并），可存16个键值（按下相应位为1松开为0）;
	
	if(keyvaltemp!=KEYSTABLE) //如果当前值不等于旧存值，即键值有变化
	{
		debouncing=1;//标示为消抖期
		if(!(keyvaltemp^keyvaltempold))//如果临时值不稳定（即新键值有变化）
		{
			debounce_cnt=0;
			keyvaltempold=keyvaltemp;
		}
		else//临时值稳定
		{
		 if(++debounce_cnt >= DEBOUNCE_TICKS) 
		 {
			KEYSTABLE = keyvaltemp;//键值更新为当前值.
			debounce_cnt = 0;//并复位消抖计数器.
			KEYTIME=1; //新键值累计时长复位为1个时间单位
			debouncing=0;//消抖期结束
		 }
	  } 
	} 
	else //如果键值仍等于旧存值：
	{ //则复位消抖计数器（注意：只要消抖中途读到一次键值等于旧存值，消抖计数器均从0开始重新计数）.
		debounce_cnt = 0;
		keyvaltempold=keyvaltemp;
	}
}

 /**
  * @brief  多功能按键状态机
  * @param  keynum: 实体按键编号（参数为KEYCLR用于复位状态机）
  * @retval 键值（按键事件值）＝(keynum+2)*10+键事件值; 其它返回0.
  */
static unsigned char _GetKeyState(unsigned char keynum)
{
	//按键记忆状态(每字节低四位存state，高4位存repeat)
	volatile static unsigned char KeyState[KEYNUMMAX];
	
	KeyS_Type KeyOnCode=0;
	unsigned char i=0;
	unsigned char state=0;
	unsigned char repeat=0;
	unsigned char event=0;
    
	if(keynum==KB_CLR) //参数为KB_CLR时，则消除所有按键记忆状态
	{
		for(i=0;i<KEYNUMMAX;i++)
			KeyState[i]=0;
		return 0;
	}
	KeyOnCode=(KeyS_Type)1<<keynum;
	state=KeyState[keynum]&0x0f; //取相应的记忆状态值
	repeat=(KeyState[keynum]>>4)&0x0f;
	
	if(TRI_FLAG && (TRI_FLAG!=KeyOnCode))
		state=0; //出现其它键，则状态清0
	
	switch (state) 
	{
		case 0://状态0：键完全松开
			if((TRI_FLAG & KeyOnCode) && (CONT & KeyOnCode))//初次按键触发并有效
			{
				event = (unsigned char)PRESS_DOWN;
				repeat = 1;
				state = 1;//初次按键有效，变成状态1
			} 
			else //无效电平，即按键为松开状态
				event = (unsigned char)NONE_PRESS;
		break;

		case 1://状态1：初次按键触发并有效
			if(!(CONT & KeyOnCode))//检测到按键松开
			{
				event = (unsigned char)PRESS_UP;
				state = 2;//按键按下后松开，变成状态2
			}
			else if(KEYTIME > LONG_TICKS)
			{//按键未松开，且持续时间已经超过LONG_TICKS
				event = (unsigned char)LONG_RRESS_START;
				state = 5;//即长按触发启动，变成状态5
			}
		break;

		case 2://状态2：按键按下后已松开
			if((TRI_FLAG & KeyOnCode) && (CONT & KeyOnCode))//再次检测到按下
			{
				event = (unsigned char)PRESS_DOWN;
				repeat++;//重按次数累计
				if(repeat == 2) state = 3;//如果重按次数等于2,则变成状态3
			} 
			else //持续松开
			{
			if(KEYTIME > SHORT_TICKS)  
				{//如果松开时间超过SHORT_TICKS，即一次按键结束
					state = 0;//因按键松开时间超过SHORT_TICKS，则复位成状态0	
					if(repeat==1) event=(unsigned char)SINGLE_CLICK;//次数为1的情况下触发单击事件
					else if(repeat==2) event=(unsigned char)DOUBLE_CLICK;//重按次数为2的情况下触发双击事件
				}
			} //隐含：如果松开时间还没有超过SHORT_TICKS，仍然维持状态2，有待后续判断		
		break;

		case 3://状态3：按下、松开、又按下（即第二次按下）				
			if(!(CONT & KeyOnCode))  //检测到按键松开
			{
				event = (unsigned char)PRESS_UP;
				if(KEYTIME < SHORT_TICKS) state = 2; //松开时间小于SHORT_TICKS，回到状态2 
				else state = 0;//松开时间大于SHORT_TICKS，则变成状态0
			}//隐含：如果仍按下则停留在状态3等待松开（第二次按下没有长按之说）
		break;

		case 5://状态5：长按触发已经启动
			if(CONT & KeyOnCode)  //如果按键仍持续按下
				event = (unsigned char)LONG_PRESS_HOLD;//长按并保持按键事件成立
			else
			{ //如果按键松开
				event = (unsigned char)PRESS_UP;
				state = 0; //则恢复到状态0
			}
		break;
	}
	KeyState[keynum]=state; //保存相应的记忆状态值
	KeyState[keynum]+= repeat<<4;
	if(event>=(unsigned char)PRESS_DOWN) //设定只输出特殊功能键（修改此处可输出按下/松开等一般事件）
//	if(event) //输出所有事件
		return KEY_EVENT(keynum,event);
	else
		return 0;
}

/**
  * @brief  中断或者while循环中调用（推荐1ms一次）
  * @param  None
  * @retval None
  */
void KeyHandler(void)
{
    static unsigned int nTicks = 0;

    nTicks++;

	if (( nTicks % TICKS_INTERVAL) == 0 )
	 {
		_KeyScanStick(); //每TICKS_INTERVAL扫键一次 在后台扫描按键获取消除抖动后的稳定键值
		if ( nTicks % INTERRUPT_TICKS == 0 )
		    _GetAndSaveKey();//每TICKS_INTERVAL*12分析一次键值 在后台读键，如果有键值则存入按键缓冲区
	 }
}
