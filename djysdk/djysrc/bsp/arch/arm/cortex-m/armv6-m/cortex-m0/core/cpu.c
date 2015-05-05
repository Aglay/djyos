//----------------------------------------------------
// Copyright (c) 2014, SHENZHEN PENGRUI SOFT CO LTD. All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------
// Copyright (c) 2014 著作权由深圳鹏瑞软件有限公司所有。著作权人保留一切权利。
//
// 这份授权条款，在使用者符合下列条件的情形下，授予使用者使用及再散播本
// 软件包装原始码及二进位可执行形式的权利，无论此包装是否经改作皆然：
//
// 1. 对于本软件源代码的再散播，必须保留上述的版权宣告、本条件列表，以
//    及下述的免责声明。
// 2. 对于本套件二进位可执行形式的再散播，必须连带以文件以及／或者其他附
//    于散播包装中的媒介方式，重制上述之版权宣告、本条件列表，以及下述
//    的免责声明。

// 免责声明：本软件是本软件版权持有人以及贡献者以现状（"as is"）提供，
// 本软件包装不负任何明示或默示之担保责任，包括但不限于就适售性以及特定目
// 的的适用性为默示性担保。版权持有人及本软件之贡献者，无论任何条件、
// 无论成因或任何责任主义、无论此责任为因合约关系、无过失责任主义或因非违
// 约之侵权（包括过失或其他原因等）而起，对于任何因使用本软件包装所产生的
// 任何直接性、间接性、偶发性、特殊性、惩罚性或任何结果的损害（包括但不限
// 于替代商品或劳务之购用、使用损失、资料损失、利益损失、业务中断等等），
// 不负任何责任，即在该种使用已获事前告知可能会造成此类损害的情形下亦然。
//-----------------------------------------------------------------------------
//所属模块: 调度器
//作者：lst
//版本：V1.0.1
//文件描述: 调度器中与CPU直接相关的代码。
//其他说明:
//修订历史:
//2. 日期: 2009-04-24
//   作者: lst
//   新版本号: V1.0.1
//   修改说明: 删除了一些为dlsp版本准备的东西
//1. 日期: 2009-01-04
//   作者: lst
//   新版本号: V1.0.0
//   修改说明: 原始版本
//------------------------------------------------------
#include "stdint.h"
#include "core-cfg.h"
#include "stdlib.h"
#include "cortexm0.h"
#include "exception.h"

void __start_systick(void);
struct tagSystickReg volatile * const pg_systick_reg
                        = (struct tagSystickReg *)0xE000E010;

struct scb_reg volatile * const pg_scb_reg
                        = (struct scb_reg *)0xe000ed00;

struct nvic_reg volatile * const pg_nvic_reg
                        = (struct nvic_reg *)0xe000e100;

extern s64  g_s64OsTicks;             //操作系统运行ticks数
u32 g_u32CycleSpeed; //for(i=j;i>0;i--);每循环纳秒数*1.024

//----创建线程-----------------------------------------------------------------
//功能：为事件类型创建线程，初始化上下文环境，安装执行函数，构成完整线程
//参数：evtt_id，待创建的线程所服务的事件类型id
//返回：新创建的线程指针
//注: 移植敏感函数
//-----------------------------------------------------------------------------
struct  tagThreadVm *__CreateThread(struct  tagEventType *evtt,u32 *stack_size)
{
    struct  tagThreadVm  *result;
    ptu32_t  len;

    //计算线程栈:线程+最大单个api需求的栈
    len = evtt->stack_size+CN_KERNEL_STACK+sizeof(struct  tagThreadVm);
    //栈顶需要对齐，malloc函数能保证栈底是对齐的，对齐长度可以使栈顶对齐
    len = align_up_sys(len);
    result=(struct  tagThreadVm  *)__MallocStack(len);
    *stack_size = len;
    if(result==NULL)
    {
        Djy_SaveLastError(EN_MEM_TRIED);   //内存不足，返回错误
        return result;
    }
#if CN_CFG_STACK_FILL != 0
    len = M_CheckSize(result);
    memset(result,CN_CFG_STACK_FILL,len);
#endif

    //看实际分配了多少内存，djyos内存分配使用块相联策略，如果分配的内存量大于
    //申请量，可以保证其实际尺寸是对齐的。之所以注释掉，是因为当len大于申请量时，
    //对齐只是实际结果，而不是内存管理的规定动作，如果不注释掉，就要求内存管理
    //模块必须提供对齐的结果，对模块独立性是不利的。
//    len = M_CheckSize(result);
    result->stack_top = (u32*)((ptu32_t)result+len); //栈顶地址，移植敏感
    result->next = NULL;
    result->stack_size = len - sizeof(struct tagThreadVm); //保存栈深度
    result->host_vm = NULL;
    //复位线程并重置线程
    __asm_reset_thread(evtt->thread_routine,result);
    return result;
}

//----测量指令指令延时常数-----------------------------------------------------
//功能: 设置指令延时常数,使不管用何种编译器和编译优化选项y_delay_us函数准确延时，
//参数：无
//返回: 无
//备注: 本函数移植敏感
//-----------------------------------------------------------------------------
void __DjySetDelay(void)
{
    u32 counter,u32_fors=10000,clksum;
    volatile u32 u32loops;


    clksum = CN_CFG_FCLK/50;              //减计数,计满是100mS
    while(1)  //测量32位变量循环时间(nS)
    {
        pg_systick_reg->reload = clksum;
        pg_systick_reg->current = 0;
        pg_systick_reg->ctrl =   (1<<bo_systick_ctrl_enable)    //使能
                                |(0<<bo_systick_ctrl_tickint)   //允许产生中断
                                |(1<<bo_systick_ctrl_clksource);//用内核时钟
        for(u32loops=u32_fors;u32loops>0;u32loops--);       //循环u32_fors次
        //停止定时器
        pg_systick_reg->ctrl =   (0<<bo_systick_ctrl_enable)    //不使能
                                |(0<<bo_systick_ctrl_tickint)   //允许产生中断
                                |(1<<bo_systick_ctrl_clksource);//用内核时钟
        counter = pg_systick_reg->current;      //读取循环u32_fors次所需时间
        if((pg_systick_reg->ctrl & bo_systick_ctrl_countflag) != 0)
        {
            u32_fors = u32_fors>>1;     //循环次数减半
        }else
            break;      //循环过程中没有发生中断，满足要求。
    };
    counter = clksum - counter;    //取实际脉冲数。
    g_u32CycleSpeed = (uint64_t)counter * 10E8/CN_CFG_FCLK/u32_fors;//防溢出，用64位
    g_u32CycleSpeed = (g_u32CycleSpeed << 10) / 1000;     //扩大1.024倍

}

//----初始化tick---------------------------------------------------------------
//功能: 初始化定时器,并连接tick中断函数,启动定时器.
//参数: 无
//返回: 无
//备注: 本函数是移植敏感函数.
//-----------------------------------------------------------------------------
void __DjyInitTick(void)
{
    Exp_ConnectSystick(Djy_IsrTick);
    pg_systick_reg->reload = CN_CFG_FCLK/CN_CFG_TICK_HZ;
    pg_systick_reg->ctrl =   (1<<bo_systick_ctrl_enable)    //使能
                            |(1<<bo_systick_ctrl_tickint)   //允许产生中断
                            |(1<<bo_systick_ctrl_clksource);//用内核时钟
}

//----读取当前时间(uS)---------------------------------------------------------
//功能：读取当前时间(uS),从计算机启动以来经历的us数，64位，默认不会溢出
//      g_s64OsTicks 为64位变量，非64位系统中，读取 g_s64OsTicks 需要超过1个
//      周期,需要使用原子操作。
//参数：无
//返回：当前时钟
//说明: 这是一个桩函数,被rtc.c文件的 DjyGetTime 函数调用
//-----------------------------------------------------------------------------
s64 __DjyGetTime(void)
{
    u32 time1;
    s64 time2;
    atom_low_t atom_low;
    atom_low = Int_LowAtomStart();

    time1 = pg_systick_reg->current;
    if(pg_systick_reg->ctrl & bm_systick_ctrl_countflag)
        time2 = (g_s64OsTicks+1)*CN_CFG_TICK_US;
    else
        time2 = g_s64OsTicks*CN_CFG_TICK_US
                    + ((pg_systick_reg->reload-time1)*CN_CFG_FINE_US >>16);

    Int_LowAtomEnd(atom_low);
    return time2;
}

