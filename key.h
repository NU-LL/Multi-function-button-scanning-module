/*
 * Copyright (c) 2018 NU-LL<1125934312@qq.com>
 * All rights reserved
 * 
 * data:2018/11/13
 * version:V0.1
 */
//////////////////////////////////////////////////////////////////////////////////
//本扫描键盘模块的特点：
//一、使用灵活：一体实现按键的普通、单击、双击、长按、保持以及组合等功能，无须事前为每个按键每种键值逐一进行宏定义，也无须逐一编写各事件的条件判断，                     
//                     只须为需要的按键事件编写相应的响应代码即可，同时留有特殊键组合等的扩展接口；
//                     可以选择每一按键事件的处理实时性，从而能够使强实时性的紧急按键优先得到处理，可自由选择中断处理及查询处理或二者混合的处理方式，
//                     灵活适配使应用项目能够兼备按键的强实时性要求以及超长（主循环执行一遍的时间长达1秒以上的）程序的适应性。
//二、注重通用：模块设计时注重通用性，按键事件（键值）依简单易懂的标准事件格式编写；除能满足几乎所有按键应用需求外，在按键数量上，
//                     从少到2-4个按键直到最大32个按键（包括端口直联、行列式矩阵、矩阵加直联混合）都可适用。
//三、稳定可靠：后台智能抖动消除、按键干扰杂波滤除措施有力，获取按键稳定可靠，不会产生重复按键，即使在CPU非常繁忙时也不会漏失按键。           
//四、移植简便：所有可调整参数（数量不多）均以宏定义列出，除与硬件相关（按键个数及连接端口）的部分须根据具体系统修改外，其它均无须变化，很容易移植。
//                     程序可读性强，注释详尽丰富，其中包括函数调用关系及详细运用修改说明，如有未尽事宜，可提出探讨，本人尽量解答修改。
//五、高效节能：消抖无须延时等待，同时采取自适应变频扫键、键盘闲置检测、消抖读键双进程周期差异等多项智能措施尽量减少占用CPU的计算资源。
//////////////////////////////////////////////////////////////////////////////////

#ifndef __KEY_H
#define __KEY_H
#include "key_config.h"


//功能键值输出：
//有了下述宏定义后，无须再为各个按键单独写宏定义，使用 KEY_EVENT(键编号,键值事件) 就可以代表特定按键事件了。
//例如：用 KEY_EVENT(KB_WKUP,DOUBLE_CLICK) 就表示了 WKUP 键双击的键值（或称事件值）
#define KEY_EVENT(keynum,event)			(unsigned char)(KEY_COMBINATION_MAX+keynum*MAX_EVENT+event)//按键事件(即键值)宏定义
//宏定义：按键未释放值 便于组合键判断 _GetKey()函数中
#define KEY_ON(keynum)                  (0x0001<<keynum)


//状态机键值事件宏定义如下：
#define NONE_PRESS                      0
#define PRESS_UP					    1
#define PRESS_DOWN				        2
#define SINGLE_CLICK			        3
#define DOUBLE_CLICK			        4
#define LONG_RRESS_START                5
#define LONG_PRESS_HOLD                 6
#define MAX_EVENT                       7
//定义一个特殊值用于复位状态机
#define KB_CLR                          44

/**************数据和函数接口声明*******************/
// extern KeyS_Type            TRI_FLAG;
// extern KeyS_Type            CONT;
// extern unsigned short int   KEYTIME;


void            KeyInit(void);      //键硬件IO端口初始化，由主函数调用
unsigned char   ReadKey(void);      //读取按键值：由系统主循环调用。
void            KeyHandler(void);   //中断或者while循环中调用

//函数的调用关系：由 KeyHandler() 自动启动两个进程：
//进程一： KeyHandler() ->自动调用 _KeyScanStick() ；该进程比较简单，扫描按键获取消除抖动后的稳定键值，无须修改。（默认5ms一次）
//进程二： KeyHandler() ->自动调用 _GetAndSaveKey() ->调用 _KeyPrePro() ->调用 _GetKey() ->调用 _GetKeyState() （默认60ms一次）
//       该进程中间的两个函数，可根据具体需求进行修改实现：
// 1是 _GetKey() 函数，定义系统中起作用的按键事件，可增加定义一些组合键事件；
// 2是 _KeyPrePro() 函数，按键预处理，实质是截获部分要求强实时性的紧急按键优先处理；剩下的键则由 _GetAndSaveKey() 自动存入 FIFO 队列待主循环查询处理。
//
//系统主循环 ->调用 ReadKey() ：系统主循环中通过调用 ReadKey() ，对 FIFO 队列中的按键进行查询处理。
//总之，运用本代码模块的主要工作是三件事：
//一：硬件接口初始化：包括必要的 GPIO 初始化，实体硬按键的排序等。
//二：键值（或称事件）生成：对 _GetKey() 函数进行改写，使之能输出想要的键值。所有按键的单击、双击、长按、保持等键值输出已经实现，如果有需要则在该函数中增加组合键等键值。
//三：键值（或称事件）处理：对键值的具体响应处理可在两个地方实现。如有强实时性的紧急按键需要优先处理的键值请在 _KeyPrePro() 函数编写代码，
//                        其它按键的响应请在系统主循环调用 ReadKey() 得到键值后编写代码。
//                        根据具体项目需求，也可全部响应代码都在主循环中编写，也可以全部响应代码都在 _KeyPrePro() 函数中编写。
//                        如果全部响应代码都在 _KeyPrePro() 函数中编写，则主循环中无须再处理按键（也无须调用 ReadKey() 函数）。

#endif
