/*
 * Copyright (c) 2018 NU-LL<1125934312@qq.com>
 * All rights reserved
 * 
 * data:2018/11/13
 * version:V0.1
 */
#ifndef __KEY_CONFIG_H
#define __KEY_CONFIG_H

//键态字类型定义
//（根据按键总数定义足够长度的数据类型，本例程只有4个键，用unsigned char足矣，但为扩充方便这里用了unsigned short int，最大可满足16键,大于16键时请定义为unsigned int）
typedef	unsigned short int 	KeyS_Type;

//根据需求可修改定义如下参数.
#define REALTIMEKEY_ENABLE                                  //紧急按键响应开启，按键处理占用时间短，要求强实时处理时开启此宏，在key.c的 _KeyPrePro() 中编写紧急按键响应事件


#define TICKS_INTERVAL    	        5				        //后台调用间隔时间（以KeyHandler调用次数为单位（默认5ms））
#define DEBOUNCE_TICKS    	        4				        //消除抖动次数，即时长至少为TICKS_INTERVAL*DEBOUNCE_TICKS（默认5ms*4）
#define NORMAL_SCAN_FREQ            6                       //正常情况下扫键频率因子，稳定后扫键周期为NORMAL_SCAN_FREQ*TICKS_INTERVAL（默认为6，即稳定后扫键周期为6*5＝30ms，最大40即120ms扫描一次均可）
#define SHORT_TICKS       	        (300 /TICKS_INTERVAL)   //短按时间定义（默认300ms）
#define LONG_TICKS        	        (1200 /TICKS_INTERVAL)  //长按时间定义（默认1200ms）
#define KEY_BUFFSIZE		        16                      //按键缓存FIFO深度（默认定义保存16个键值）
#define INTERRUPT_TICKS		        TICKS_INTERVAL*12       //按键分析时间间隔 _KeyPrePro() 中的紧急按键响应时间和 ReadKey() 中的读取键值都依赖此（默认5*12=60ms一次）

//硬件实体按键编号，键态字 KeyS_Type 依此顺序按位组合，每BIT位对应一个实体按键
#define KB_KEY0                     0
#define KB_KEY1  		            1
#define KB_KEY2  		            2 
#define KB_WKUP 		            3
#define KEYNUMMAX		            4			            //硬件实体按键数量


//这里可以定义一些特殊键码（如组合键等）
#define WKUP_PLUSKEY0_PRES          1                       //示例：WKUP+KEY0组合按键（先按下WKUP再按下KEY0）
#define KEY0_KEY1_KEY2_PRES         2                       //示例：KEY0+KEY1+KEY2组合按键（KEY2必须最后按下）
#define KEY_COMBINATION_MAX         3                       //最大组合键数目

#endif
