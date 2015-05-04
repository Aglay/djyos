//----------------------------------------------------
// Copyright (c) 2014, SHENZHEN PENGRUI SOFT CO LTD. All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. As a constituent part of djyos,do not transplant it to other software
//    without specific prior written permission.

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
// 这份授权条款，在使用者符合以下三条件的情形下，授予使用者使用及再散播本
// 软件包装原始码及二进位可执行形式的权利，无论此包装是否经改作皆然：
//
// 1. 对于本软件源代码的再散播，必须保留上述的版权宣告、本条件列表，以
//    及下述的免责声明。
// 2. 对于本套件二进位可执行形式的再散播，必须连带以文件以及／或者其他附
//    于散播包装中的媒介方式，重制上述之版权宣告、本条件列表，以及下述
//    的免责声明。
// 3. 本软件作为都江堰操作系统的组成部分，未获事前取得的书面许可，不允许移植到非
//    都江堰操作系统环境下运行。

// 免责声明：本软件是本软件版权持有人以及贡献者以现状（"as is"）提供，
// 本软件包装不负任何明示或默示之担保责任，包括但不限于就适售性以及特定目
// 的的适用性为默示性担保。版权持有人及本软件之贡献者，无论任何条件、
// 无论成因或任何责任主义、无论此责任为因合约关系、无过失责任主义或因非违
// 约之侵权（包括过失或其他原因等）而起，对于任何因使用本软件包装所产生的
// 任何直接性、间接性、偶发性、特殊性、惩罚性或任何结果的损害（包括但不限
// 于替代商品或劳务之购用、使用损失、资料损失、利益损失、业务中断等等），
// 不负任何责任，即在该种使用已获事前告知可能会造成此类损害的情形下亦然。
//-----------------------------------------------------------------------------

//所属模块: gui
//作者:  罗侍田.
//版本：V1.0.0
//文件描述: gui kernel主体部分
//其他说明:
//修订历史:
//2. 日期: 2011-07-18
//   作者:  罗侍田.
//   新版本号: V1.1.0
//   修改说明: 窗口数据结构实体已经改为由上层提供，但本文件一直没改，修改之。
//             增加__gk_vmalloc等4个函数，使在显存分配无特殊要求时，driver可以不
//             提供分配内存的函数。
//1. 日期: 2009-11-21
//   作者:  罗侍田.
//   新版本号: V1.0.0
//   修改说明: 原始版本
//------------------------------------------------------
#include "stdint.h"
#include "config-prj.h"
#include "stdlib.h"
#include "stdio.h"
#include "lock.h"
#include "systime.h"
#include "gkernel.h"
#include "align.h"
#include "pool.h"
#include "djyos.h"
#include "..\include\gk_syscall.h"
#include "..\include\gk_usercall.h"
#include "gk_display.h"
#include "..\include\gk_win.h"
#include "..\include\gk_draw.h"
#include "..\include\gk_clip.h"
#include "charset.h"
#include "font.h"
#include "math.h"
#include "string.h"

struct tagGkWinRsc   *g_ptFocusWin;//显示焦点窗口,所有的显示都是对这个窗口的操作
static struct tagRscNode s_tDisplayRootRsc;
static struct tagRscNode s_tWindowRootRsc;
u32 g_u32DefaultBackColor,g_u32DefaultColor;
u16 g_u16GkServerEvtt,g_u16RefreshrEvtt;
struct tagGkChunnel g_tGkChunnel;

extern struct tagMemCellPool *g_ptClipRectPool; //from gk_clip.c

//剪切域数量
#define CN_CLIP_INIT_NUM    100
struct tagClipRect g_tClipRect[CN_CLIP_INIT_NUM];
struct tagGkWinRsc *g_ptZ_Topmost;

struct tagSemaphoreLCB *g_ptUsercallSemp;
//如果调用方希望gui kernel服务完成再返回，使用这个信号量
struct tagSemaphoreLCB *g_ptSyscallSemp;
struct tagMutexLCB *g_ptGkServerSync;
u8 *draw_chunnel_buf;

u32 __ExecOneCommand(u16 DrawCommand,u8 *ParaAddr);

//gkwin在资源树种的位置与z序的联系:
// 1、在资源树中，所有子窗口按z序排列，最前端的是父窗口指向的第一个结点。
// 2、z_prio成员影响win在其兄弟结点中的位置，越小越前端
// 3、z_prio一般设为0，<=0均遮盖父窗口，若>0则被父窗口遮盖


//----初始化gui模块---------------------------------------------------------
//功能: 初始化gui模块，在资源队列中增加必要的资源结点
//参数: 模块初始化函数，没有参数
//返回: 1=成功
//-----------------------------------------------------------------------------
ptu32_t ModuleInstall_GK(ptu32_t para)
{
    draw_chunnel_buf = M_Malloc(gc_u32CfgGuiCmdDeep * 2 + gc_u32CfgGuiStsDeep,0);
    if(draw_chunnel_buf == NULL)
        return 0;
    g_tGkChunnel.syscall_buf = draw_chunnel_buf + gc_u32CfgGuiCmdDeep;
    g_tGkChunnel.usercall_buf = draw_chunnel_buf + 2 * gc_u32CfgGuiCmdDeep;

    Rsc_AddTree(&s_tDisplayRootRsc,sizeof(struct tagRscNode),RSC_RSCNODE,"display");
    Rsc_AddTree(&s_tWindowRootRsc,sizeof(struct tagRscNode),RSC_RSCNODE,"gkwindow");

    Ring_Init(&g_tGkChunnel.ring_syscall,
                        g_tGkChunnel.syscall_buf,gc_u32CfgGuiCmdDeep);
    Ring_Init(&g_tGkChunnel.ring_usercall,
                        g_tGkChunnel.usercall_buf,gc_u32CfgGuiStsDeep);

    g_tGkChunnel.syscall_mutex = Lock_MutexCreate("gui chunnel to gk mutex");
    g_tGkChunnel.syscall_semp = Lock_SempCreate(1,0,CN_SEMP_BLOCK_FIFO,"gui chunnel to gk semp");
    g_tGkChunnel.usercall_semp = Lock_SempCreate(1,0,CN_SEMP_BLOCK_FIFO,"gui chunnel from gk semp");
    g_ptGkServerSync = Lock_MutexCreate("gk server sync");
//    g_ptGkServerSync = Lock_SempCreate(1,0,CN_SEMP_BLOCK_FIFO,"gk server sync");
    g_ptUsercallSemp = Lock_SempCreate(1,0,CN_SEMP_BLOCK_FIFO,"gk wait repaint");
    g_ptSyscallSemp = Lock_SempCreate(1,0,CN_SEMP_BLOCK_FIFO,"gk wait job");
    g_u16GkServerEvtt = Djy_EvttRegist(EN_CORRELATIVE,1,0,0,GK_Server,
                                    NULL,8120,"gui kernel server");
    g_ptClipRectPool = Mb_CreatePool(&g_tClipRect,
                                  CN_CLIP_INIT_NUM,
                                  sizeof(struct tagClipRect),
                                  100,2000, "clip area");

    if(    (g_ptClipRectPool == NULL)
        || (g_u16GkServerEvtt == CN_EVTT_ID_INVALID)
        || (g_ptSyscallSemp == NULL)
        || (g_ptUsercallSemp == NULL)
        || (g_ptGkServerSync == NULL)
        || (g_tGkChunnel.usercall_semp == NULL)
        || (g_tGkChunnel.syscall_semp == NULL)
        || (g_tGkChunnel.syscall_mutex == NULL) )
    {
        goto exit_error;
    }


    Djy_EventPop(g_u16GkServerEvtt,NULL,0,0,0,0);

    return 1;

exit_error:
    Mb_DeletePool(g_ptClipRectPool);
    Djy_EvttUnregist(g_u16GkServerEvtt);
    Lock_SempDelete(g_ptSyscallSemp);
    Lock_SempDelete(g_ptUsercallSemp);
//    Lock_SempDelete(g_ptGkServerSync);
    Lock_MutexDelete(g_ptGkServerSync);
    Lock_SempDelete(g_tGkChunnel.usercall_semp);
    Lock_SempDelete(g_tGkChunnel.syscall_semp);
    Lock_MutexDelete(g_tGkChunnel.syscall_mutex);

    Rsc_DelNode(&s_tDisplayRootRsc);
    Rsc_DelNode(&s_tWindowRootRsc);
    return 0;
}

//根据像素格式，计算显示缓冲所需要的内存尺寸。
u32 __gk_cal_buf_size(u32 xsize,u32 ysize,u32 *linesize,u16 PixelFormat)
{
    u32 size;
    u8 color_bits;
    color_bits = (PixelFormat&CN_PF_BITWIDE_MSK)>>8;
    switch(color_bits)
    {
        case 1:
        {
            *linesize = (xsize + 7)/8;
            size = *linesize*ysize;
        }break;
        case 2:
        {
            *linesize = (xsize + 3)/4;
            size = *linesize*ysize;
        }break;
        case 4:
        {
            *linesize = (xsize + 1)/2;
            size = *linesize*ysize;
        }break;
        case 8:
        {
            *linesize = xsize;
            size = xsize * ysize;
        }break;
        case 16:
        {
            *linesize = 2*xsize;
            size = xsize*ysize*2;
        }break;
        case 24:
        {
            *linesize = 3*xsize;
            size = xsize*ysize*3;
        }break;
        case 32:
        {
            *linesize = 4*xsize;
            size = xsize*ysize*4;
        }break;
        default:
            size = 0;
            *linesize = 0;
            break;
    }
    return size;

}

//----分配窗口缓存-------------------------------------------------------------
//功能: 分配窗口缓存，包括：1、窗口所需的bitmap,2、如果该窗口有rop4属性，还要分
//      配bitmsk_buf，3、pattern buffer。
//      虽然本函数可能需要分配三部分，但实际上是合并一起从堆中申请，然后再手工
//      分割成几个缓冲区的。因各缓冲区都是byte型指针，故无须担心对齐问题。
//参数: buf，用于返回结果的结构指针
//      xsize、ysize，显示区域尺寸
//      rop_code，窗口的rop_code代码，影响是否分配buf中的rop4_msk成员。
//返回: true=成功，false=失败
//注: 2416的2D加速不支持rop4，无须判断rop_code，只须分配pat和vbuf即可
//-----------------------------------------------------------------------------
bool_t __gk_vmalloc(struct tagDisplayRsc *disp,struct tagGkWinRsc *gkwin,
                    u16 PixelFormat)
{
    u8 *mem;
    u32 buf_size,linesize;// pal_size=0;
    s32 xsize,ysize;
    bool_t result;
    xsize = gkwin->right - gkwin->left;
    ysize = gkwin->bottom - gkwin->top;
    if(PixelFormat == CN_SYS_PF_DISPLAY)
        PixelFormat = disp->pixel_format;
    //计算显示缓冲所需要的内存尺寸。
    buf_size = __gk_cal_buf_size(xsize,ysize,&linesize,PixelFormat);
    buf_size = align_up_sys(buf_size);
    //同时分配bitmap数据结构、图片位图
    mem = (u8*)M_MallocLcHeap(buf_size + sizeof(struct tagRectBitmap),
                            disp->DisplayHeap,0);
    if(mem != NULL)
    {
        gkwin->wm_bitmap = (struct tagRectBitmap*)mem;
        gkwin->wm_bitmap->PixelFormat = PixelFormat;
        gkwin->wm_bitmap->width = xsize;
        gkwin->wm_bitmap->height = ysize;
        gkwin->wm_bitmap->linebytes = linesize;
        mem += sizeof(struct tagRectBitmap);
        gkwin->wm_bitmap->bm_bits = mem;
        result = true;
    }else
    {
        gkwin->wm_bitmap = NULL;
        result = false;
    }
    return result;
}


//----重新分配窗口缓存---------------------------------------------------------
//功能: 当窗口尺寸改变，调用本函数重新分配缓存。逻辑上，本函数的功能="vfree-
//      重新vmalloc"的组合，新设置一个函数，是为了提高效率。因为内存是按块分配
//      的，内存分配是相当耗时的工作，vmalloc分配的窗口缓存，可能比实际的大，当
//      窗口尺寸调整不大的话，可能无需重新分配。而用户通过拖动来改变窗口尺寸时，
//      实际上是一点一点改变的，每次都重新分配的话，系统开销很大，且易产生碎片。
//      vrmalloc函数先检查buf中的缓冲区是否仍然满足需求，若满足，则不重新分配。
//      但可能会跟据xsize和ysize调整指针。
//参数: buf，原窗口缓存指针，也用于返回结果
//      xsize、ysize，显示区域尺寸
//      rop_code，窗口的rop_code代码
//返回: true=成功，false=失败
//-----------------------------------------------------------------------------
bool_t __gk_vrmalloc(struct tagDisplayRsc *disp,struct tagGkWinRsc *gkwin)
{
    u8 *mem;
    u32 buf_size,linesize,oldsize,newsize;//pal_size=0;
    s32 xsize,ysize;
    bool_t remalloc = true,result;
    u16 PixelFormat;
    xsize = gkwin->right - gkwin->left;
    ysize = gkwin->bottom - gkwin->top;
    PixelFormat = gkwin->wm_bitmap->PixelFormat;

    //重新分配显存，若原显存仍然符合要求，可能不会重新分配。
    //若需重新分配，必须采取先分配后释放的办法，因为djyos是多线程环境，先释放后
    //申请的话，虽然能少许增加分配的成功率，但释放后可能因抢占，内存被别的高优先
    //级事件抢去的话，就会造成分配失败。先分配后释放，如果分配失败，至少还可以
    //确保原来的显存不丢失。
    if(PixelFormat == CN_SYS_PF_DISPLAY)
        PixelFormat = disp->pixel_format;
    //计算显示缓冲所需要的内存尺寸。
    buf_size = __gk_cal_buf_size(xsize,ysize,&linesize,PixelFormat);
    buf_size = align_up_sys(buf_size);
    newsize = buf_size + sizeof(struct tagRectBitmap);

    oldsize = M_CheckSize(gkwin->wm_bitmap); //查询原指针实际分配的内存
    if(newsize <= oldsize)
    {
        //新的显存需求在原实际尺寸相差在1/8以内的或者小于最小内存分配尺寸的，
        //不重新分配显存
        if((oldsize - newsize) <= (oldsize>>3))
            remalloc = false;       //无需重新分配
    }
    if(remalloc)
    {
        mem =(u8*) M_MallocLcHeap(newsize,disp->DisplayHeap,0);
        if(mem != NULL)
        {
            gkwin->wm_bitmap = (struct tagRectBitmap*)mem;
            gkwin->wm_bitmap->width = xsize;
            gkwin->wm_bitmap->height = ysize;
            gkwin->wm_bitmap->linebytes = linesize;
            mem += sizeof(struct tagRectBitmap);
            gkwin->wm_bitmap->bm_bits = mem;

            result = true;
        }else
        {
            result = false;   //新分配内存失败，保持原来不变，不释放原内存。
        }
    }else                     //无须重新分配
    {
        result = true;
    }
    return result;
}


//----释放窗口缓存-------------------------------------------------------------
//功能: 释放一个窗口的缓存。
//参数: disp，窗口所属显示器
//      buf，用于返回结果的结构指针
//返回: 无
//-----------------------------------------------------------------------------
void __gk_vfree(struct tagDisplayRsc *disp,struct tagGkWinRsc *gkwin)
{
    //分配内存时，显存和bitmap结构是同时分配的，故只须调用
    //m_free一次，两者均释放
    M_FreeHeap(gkwin->wm_bitmap,disp->DisplayHeap);
}
//----取z轴片段的起始窗口(窗口及其子窗口)--------------------------------------
//功能: 每一个窗口及其子窗口，都在z轴中占据连续的一段，本函数取该段的起始窗口(
//      最后端的窗口)。
//参数: gkwin，窗口指针
//返回: 起始窗口指针
//-----------------------------------------------------------------------------
struct tagGkWinRsc *__GK_GetZsectionStart(struct tagGkWinRsc *gkwin)
{
    struct tagGkWinRsc *temp,*result;
    result = gkwin;
    temp = result;
    while(1)
    {
        temp = (struct tagGkWinRsc *)Rsc_GetSon(&temp->node);
        if(temp == NULL)        //result已经没有子窗口了
            break;
        temp = (struct tagGkWinRsc *)temp->node.previous;
        if(temp ->z_prio <= 0 )  //所有子窗口均在result前端
            break;
        else
        {
            result = temp;
        }
    }
    return result;
}
//----取z轴片段的结束窗口(窗口及其子窗口)--------------------------------------
//功能: 每一个窗口及其子窗口，都在z轴中占据连续的一段，本函数取该段的结束窗口(
//      最前端的窗口)。
//参数: gkwin，窗口指针
//返回: 结束窗口指针
//-----------------------------------------------------------------------------
struct tagGkWinRsc *__GK_GetZsectionEnd(struct tagGkWinRsc *gkwin)
{
    struct tagGkWinRsc *temp,*result;
    result = gkwin;
    temp = result;
    while(1)
    {
        temp = (struct tagGkWinRsc *)Rsc_GetSon(&temp->node);
        if(temp == NULL)        //result已经没有子窗口了
            break;
        else if(temp ->z_prio > 0 )  //所有子窗口均在result后端
            break;
        else
        {
            result = temp;
        }
    }
    return result;
}
//----取z轴片段的起始窗口(窗口及其子孙窗口)------------------------------------
//功能: 每一个窗口及其子孙窗口，都在z轴中占据连续的一段，本函数取该段的起始窗口(
//      最后端的窗口)。
//参数: gkwin，窗口指针
//返回: 起始窗口指针
//-----------------------------------------------------------------------------
struct tagGkWinRsc *__GK_GetZsectionAllStart(struct tagGkWinRsc *gkwin)
{
    struct tagGkWinRsc *temp,*result;
    result = gkwin;
    temp = result;
    while(1)
    {
        temp = (struct tagGkWinRsc *)Rsc_GetSon(&temp->node);
        if(temp == NULL)        //result已经没有子窗口了
            break;
        temp = (struct tagGkWinRsc *)Rsc_GetNext(&temp->node);
        if(temp ->z_prio <= 0 )  //所有子窗口均在result前端
            break;
        else
        {
            result = temp;
        }
    }
    return result;
}

//----取z轴片段的结束窗口(窗口及其子窗口)--------------------------------------
//功能: 每一个窗口及其子孙窗口，都在z轴中占据连续的一段，本函数取该段的结束窗口(
//      最前端的窗口)。
//参数: gkwin，窗口指针
//返回: 结束窗口指针
//-----------------------------------------------------------------------------
struct tagGkWinRsc *__GK_GetZsectionAllEnd(struct tagGkWinRsc *gkwin)
{
    struct tagGkWinRsc *temp,*result;
    result = gkwin;
    temp = result;
    while(1)
    {
        temp = (struct tagGkWinRsc *)Rsc_GetSon(&temp->node);
        if(temp == NULL)        //result已经没有子窗口了
            break;
        else if(temp ->z_prio > 0 )  //所有子窗口均在result后端
            break;
        else
        {
            result = temp;
        }
    }
    return result;
}


//----创建桌面-----------------------------------------------------------------
//功能: 桌面是所有显示器的第一个窗口；所有的显示器，必须先创建桌面才能创建窗口，
//      桌面的尺寸不能小于显示器的尺寸，但可以大于显示器尺寸。桌面在刷新screen
//      的过程中和普通窗口是等同的。
//参数: para，参数数据结构，其成员含义见结构定义处。
//返回: 新窗口句柄(指针)
//-----------------------------------------------------------------------------
struct tagGkWinRsc *GK_CreateDesktop(struct tagGkscParaCreateDesktop *para)
{
    struct tagGkWinRsc *desktop;
    struct tagDisplayRsc *display;
    struct tagRscNode *rsc;
    struct tagClipRect *clip;
    bool_t BufM;
    s32 desktop_x,desktop_y;
    u32 msk_size;
    struct tagGkscParaFillWin para_fill;
    if(para->display == NULL)
    {
        return NULL;
    }
    display = para->display;
    if(para->width >= display->width)
        desktop_x = para->width;            //桌面可以大于显示器尺寸
    else
        desktop_x = display->width;   //桌面不能小于显示器尺寸
    if(para->height >= display->height)
        desktop_y = para->height;
    else
        desktop_y = display->height;
    if(display->frame_buffer == NULL)
    {
        if(para->buf_mode == CN_WINBUF_BUF)
        {
            return NULL;        //无帧缓冲的显示器，不允许创建缓冲桌面
        }
        BufM = false;
    }
    else
    {
        if(para->buf_mode == CN_WINBUF_NONE)
            BufM = false;
        else
            BufM = true;
    }
    desktop = para->desktop;    //para->desktop由调用者提供内存，传指针过来
    //下面初始化桌面，桌面也是按窗口管理的
    desktop->z_back = desktop;
    desktop->z_top = desktop;
    strcpy(desktop->win_name,"desktop");
    desktop->redraw_clip = NULL;
    desktop->changed_clip = NULL;
    desktop->disp = display;            //所属显示器
    desktop->z_prio = 0;                //桌面的优先级
    desktop->RopCode = 0;               //桌面只能是0
    desktop->dest_blend = false;        //本参数对桌面无效
    desktop->change_flag = CN_GKWIN_CHANGE_NONE; //清屏后才显示
    desktop->absx0 = 0;
    desktop->absy0 = 0;
    desktop->bound_limit = true;
    desktop->left = 0;
    desktop->top = 0;
    desktop->right = desktop_x;
    desktop->bottom = desktop_y;
    desktop->limit_left = 0;
    desktop->limit_top = 0;
    desktop->limit_right = display->width;
    desktop->limit_bottom = display->height;

    display->z_topmost = desktop;
    display->desktop = desktop;
    clip = (struct tagClipRect*)Mb_Malloc(g_ptClipRectPool,0);
    if(clip == NULL)
        return NULL;
    if(BufM)      //缓冲模式，为桌面准备缓冲区
    {
        if( __gk_vmalloc(display,desktop,para->PixelFormat) )//分配显存
        {
            msk_size = ((desktop_x +7)/8+7)/8*((desktop_y +7)/8);
            //存在frame buffer时才需要changed_msk
            desktop->changed_msk.bm_bits = M_MallocHeap(msk_size,
                                            display->DisplayHeap,0);
            if( desktop->changed_msk.bm_bits == NULL)   //分配changed_msk内存失败
            {
                display->z_topmost = NULL;
                display->desktop = NULL;
                M_FreeHeap(desktop->changed_msk.bm_bits,display->DisplayHeap);
                Mb_Free(g_ptClipRectPool,clip);
                __gk_vfree(display,desktop);
                Djy_SaveLastError(EN_GK_NO_MEMORY);
                printf("显存不足\n\r");
                return NULL;
            }
            else
            {//分配changed_msk内存成功，将显存再分配
                memset(desktop->changed_msk.bm_bits,0,msk_size);
                desktop->changed_msk.PixelFormat = CN_SYS_PF_GRAY1;
                desktop->changed_msk.height = desktop_y/8;
                desktop->changed_msk.width = desktop_x/8;
                desktop->changed_msk.linebytes = ((desktop_x+7)/8+7)/8;
                desktop->KeyColor = 0;

                desktop->wm_bitmap->ExColor = para->BaseColor;
            }
        }
        else               //分配显存失败，
        {
            display->z_topmost = NULL;
            display->desktop = NULL;
            Djy_SaveLastError(EN_GK_NO_MEMORY);
            printf("显存不足\n\r");
            return NULL;
        }

    }
    else                   //非缓冲模式，窗口没有缓冲区
    {
        desktop->wm_bitmap = NULL;
        desktop->changed_msk.bm_bits = NULL;///没有这一句会有不敢想象的bug
    }
    //桌面可视域，可视域大小与显示器尺寸大小一致
    clip->next = clip;
    clip->previous = clip;
    clip->rect.left = 0;
    clip->rect.top = 0;
    clip->rect.right = display->width;
    clip->rect.bottom = display->height;
    desktop->visible_clip = clip;
    desktop->visible_bak = NULL;

    rsc = Rsc_SearchTree("gkwindow");
    Rsc_AddSon(rsc,&desktop->node,sizeof(struct tagGkWinRsc),RSC_GKWIN,desktop->win_name);

    //用给定的颜色填充桌面
    para_fill.gkwin = desktop;
    para_fill.color = para->color;
    GK_FillWin(&para_fill);

    //返回新窗口指针
    return desktop;
}


//----创建窗口-----------------------------------------------------------------
//功能: 创建一个窗口，新窗口的优先级为0，位于所有优先级=0的窗口的最前端。
//参数: para，参数数据结构，其成员含义见结构定义处。
//返回: 新窗口句柄(指针)
//-----------------------------------------------------------------------------
struct tagGkWinRsc *GK_CreateWin(struct tagGkscParaCreateGkwin *para)
{
    u32 size,RopCode;
    s32 width,height;
    bool_t BufM;
    struct tagGkWinRsc *target_section;
    struct tagGkWinRsc *parent;                       //gkwin的父窗口
    struct tagDisplayRsc *display;
    //z轴中被移动的窗口段最后端的窗口
    struct tagGkWinRsc *move_start;
    //z轴中被移动的窗口段最前端的窗口
    struct tagGkWinRsc *move_end;
    struct tagGkWinRsc *gkwin;
    struct tagGkscParaFillWin para_fill;

    width = para->right - para->left;
    height = para->bottom - para->top;
    display = para->parent_gkwin->disp;
    gkwin = para->gkwin;    //para->gkwin由调用者提供内存，传指针过来
    parent = para->parent_gkwin;
    if((parent == NULL)||(width < 0) || (height < 0))
    {
        return NULL;
    }
    if(display->frame_buffer == NULL)
    {
        if(para->buf_mode == CN_WINBUF_BUF)
        {
            return NULL;        //无帧缓冲的显示器，不允许创建缓冲桌面
        }
        BufM = false;
    }
    else
    {
        if(para->buf_mode == CN_WINBUF_NONE)
            BufM = false;
        else
            BufM = true;
    }

    RopCode = para->RopCode;
    if((RopCode & (CN_ROP_ROP2_EN + CN_ALPHA_MODE_NEED_AD)) != 0)
        return NULL;   //窗口属性不支持rop2和需要Dst Alpha参与的alpha混合

    *gkwin = *parent;               //新窗口拷贝父窗口的属性

    size = strnlen(para->name,CN_GKWIN_NAME_LIMIT);
    memcpy(gkwin->win_name,para->name,size);
    gkwin->win_name[size] = '\0';       //串结束符\0。

    //给定的新窗口的坐标是相对其父窗口的
    gkwin->left = para->left;
    gkwin->top = para->top;
    gkwin->right = para->right;
    gkwin->bottom = para->bottom;
    //窗口的绝对位置，相对于所属screen的桌面原点
    gkwin->absx0 = para->left + para->parent_gkwin->absx0;
    gkwin->absy0 = para->top + para->parent_gkwin->absy0;
    gkwin->z_prio = 0;                //新建窗口默认为0
    display->reset_clip = false;

    gkwin->dest_blend = __GK_RopIsNeedDestination(RopCode);
    gkwin->RopCode = RopCode;

    if(BufM)              //缓冲模式，为窗口准备缓冲区
    {
        if( __gk_vmalloc(display,gkwin,para->PixelFormat) )  //分配显存
        {
            size = (((width +7)/8+7)/8 *((height +7)/8));
            //存在frame buffer时才需要changed_msk
            gkwin->changed_msk.bm_bits = M_MallocHeap(size,
                                                    display->DisplayHeap,0);
            if(gkwin->changed_msk.bm_bits == NULL)   //分配修改掩码内存失败
            {
                __gk_vfree(display,gkwin);
                Djy_SaveLastError(EN_GK_NO_MEMORY);
                printf("显存不足\n\r");
                return NULL;
            }
            else
            {
                memset(gkwin->changed_msk.bm_bits,0,size);
                gkwin->changed_msk.PixelFormat = CN_SYS_PF_GRAY1 ;
                gkwin->changed_msk.height = (height + 7)/8;
                gkwin->changed_msk.width = (width + 7)/8;
                gkwin->changed_msk.linebytes = ((width+7)/8+7)/8;
                gkwin->KeyColor = para->KeyColor;
                gkwin->wm_bitmap->ExColor = para->BaseColor;
            }
        }
        else               //分配显存失败，
        {
            Djy_SaveLastError(EN_GK_NO_MEMORY);
            printf("显存不足\n\r");
            return NULL;
        }
    }
    else                   //非缓冲模式，窗口没有缓冲区
    {
        gkwin->wm_bitmap = NULL;
        gkwin->changed_msk.bm_bits = NULL;///没有这一句会有不敢想象的bug
    }

    //以下把新窗口连接到资源队列中，并插入到z轴中
    if(Rsc_GetSon(&para->parent_gkwin->node) == NULL)  //父窗口无子窗口
    {
        Rsc_AddSon(&parent->node,&gkwin->node,
                     sizeof(struct tagGkWinRsc),RSC_GKWIN,gkwin->win_name);
        //以下4句在z轴中把win插入父节点的前端
        gkwin->z_back = parent;
        gkwin->z_top = parent->z_top;
        parent->z_top->z_back = gkwin;
        parent->z_top = gkwin;
        if(display->z_topmost == parent)
            display->z_topmost = gkwin;
    }
    else
    {//父窗口有子窗口
        //取z轴中被移动的与gkwin同级的窗口段开始和结束窗口段
        move_end = (struct tagGkWinRsc*)Rsc_GetSon(&parent->node);
        move_start = (struct tagGkWinRsc*)Rsc_GetPrevious(&move_end->node);
        target_section = move_end;
        while(1)
        {//扫描同级窗口中和目标窗口优先级相同的窗口，扫描以z轴为对象
            //扫描顺序:从z轴段前端向后端进行扫描，只扫兄弟窗口，不扫子窗口
            if(target_section->z_prio >= 0)
                break;
            //扫描目标窗口所在z轴段最后一窗口，终止跳出
            if(target_section == move_start)
                break;
            target_section = (struct tagGkWinRsc*)Rsc_GetNext(&target_section->node);
        }
        if(target_section->z_prio == 0)
        {//同级窗口中存在和gkwin的prio相等的窗口
            Rsc_AddToPrevious(&target_section->node,&gkwin->node,
                        sizeof(struct tagGkWinRsc),RSC_GKWIN,gkwin->win_name);
            if(Rsc_IsHead(&target_section->node))
                Rsc_RoundPrevious(&parent->node);
            //获取target_section和其子窗口所在z轴段的结束窗口(最前端)
            //新窗口插入到它的前端
            move_end = __GK_GetZsectionAllEnd(target_section);
            if(move_end == display->z_topmost)
                display->z_topmost = gkwin;
            //插到目标窗口前端
            gkwin->z_back = move_end;
            gkwin->z_top = move_end->z_top;
            move_end->z_top->z_back = gkwin;
            move_end->z_top = gkwin;
        }
        else //兄弟窗口中没有优先级=0的窗口
        {
            //同级窗口中有优先级大于0的，资源队列中新窗口插入其前端
            if(target_section->z_prio > 0)
            {
                Rsc_AddToPrevious(&target_section->node,&gkwin->node,
                            sizeof(struct tagGkWinRsc),RSC_GKWIN,gkwin->win_name);
            }
            else    //同级窗口优先级均高于新窗口(prio = 0)
            {
                //新窗口在资源队列中处于同级窗口最后端
                Rsc_AddToNext(&target_section->node,&gkwin->node,
                            sizeof(struct tagGkWinRsc),RSC_GKWIN,gkwin->win_name);
            }
            //新创建的窗口gkwin优先级为0，无论同级窗口优先级均如何分布，新
            //窗口肯定在父窗口前端
            if(parent == display->z_topmost)
                display->z_topmost = para->gkwin;
            //插到目标窗口前端
            gkwin->z_back = parent;
            gkwin->z_top = parent->z_top;
            parent->z_top->z_back = gkwin;
            parent->z_top = gkwin;
        }
    }
    //设置窗口的可见边界，不受窗口互相遮挡影响，必须设置资源队列后才能调用
    __GK_SetBound(gkwin);

    gkwin->visible_clip = NULL;
    gkwin->visible_bak = NULL;
    display->reset_clip = true;
    para_fill.gkwin = gkwin;
    para_fill.color = para->color;
    GK_FillWin(&para_fill); //对于无缓冲的窗口，因visible_clip空，本句实际无效。

    return gkwin;
}

//----窗口改名-----------------------------------------------------------------
//功能: 修改窗口名字，窗口最多127字符，超过部分将截断。
//参数: gcwin，被修改的窗口句柄(指针)
//      name，新的名字
//返回: 无
//-----------------------------------------------------------------------------
void GK_SetName(struct tagGkWinRsc *gkwin,char *name)
{
    s32 size;
    size = strnlen(name,CN_GKWIN_NAME_LIMIT+1);
    if(size > CN_GKWIN_NAME_LIMIT)     //名字长度超过限制
        size = CN_GKWIN_NAME_LIMIT;   //强制切掉超长部分
    memcpy(gkwin->win_name,name,size);//copy名字，因可能超长，故未copy串结束符\0。
    gkwin->win_name[size] = '\0';     //串封口(加结束符)
}

//----解锁窗口-----------------------------------------------------------------
//功能: 解除锁定窗口，需要重新输出本窗口修改过的部分。锁定状态下，窗口显示在屏幕
//      上，但不内容不会改变。
//参数: gcwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
//void GK_SetUnlock(struct tagGkWinRsc *gkwin)
//{
//    if(gkwin == gkwin->disp->desktop)   //桌面不可锁定
//        return;
//    if(gkwin->locked == false)      //本来就没有锁定，无操作
//        return;
//    gkwin->locked = false;
//    if(gkwin->hided == false)       //若窗口可视，则要重新扫描可视域
//    {
//        gkwin->disp->reset_clip = true;
//    }
//    return;
//}

//----锁定窗口-----------------------------------------------------------------
//功能: 锁定窗口，需要重新输出本窗口修改过的部分
//参数: gcwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
//void GK_SetLock(struct tagGkWinRsc *gkwin)
//{
//    if(gkwin == gkwin->disp->desktop)   //桌面不可锁定
//        return;
//    if(gkwin->locked == true)       //本来就已锁定，无操作
//        return;
//    gkwin->locked = true;
//}

//----隐藏窗体-----------------------------------------------------------------
//功能: 把窗口设置为隐藏，会引起重新扫描可视域，重新输出其他窗口被自己遮盖部分。
//      不影响子窗口，若要同时设置子窗口，应在gui windows中完成
//参数: gcwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
//void GK_SetHide(struct tagGkWinRsc *gkwin)
//{
//    if(gkwin == gkwin->disp->desktop)   //桌面不可隐藏
//        return;
//    if(gkwin->hided == true)                    //原来就是隐藏的，无操作
//        return;
//    gkwin->hided = true;
//    gkwin->disp->reset_clip = true;
//}

//----显示窗口-----------------------------------------------------------------
//功能: 把窗口设置为可视，会引起重新扫描可视域，重新输出本窗口。不影响子窗口，
//      若要同时设置子窗口，应在gui windows中完成
//参数: gcwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
//void GK_SetShow(struct tagGkWinRsc *gkwin)
//{
//    if(gkwin == gkwin->disp->desktop)   //桌面不可隐藏
//        return;
//    if(gkwin->hided == false)   //原来就是可视的，无操作
//        return;
//    gkwin->hided = false;
//    gkwin->disp->reset_clip = true;
// }

//----改变窗口尺寸和位置-------------------------------------------------------
//功能: 改变一个窗口的尺寸以及位置，如果没改尺寸，实际上就是一个gk_move_win。
//参数: gcwin，目标窗口
//      new_area，新的窗口尺寸和位置
//      mode，改尺寸后，窗口图像处理模式，0=保持不变，1=拉伸或压缩，
//            缓冲窗口才有效
//返回: true = 成功，false = 失败(一般是内存分配导致)
//注: 有缓冲区的情况下，如果只是移动了位置，不需要重新绘制。
//    如果修改了尺寸，应用程序应该重绘，直接重绘即可，无须等待重绘消息。
//-----------------------------------------------------------------------------
bool_t GK_ChangeWinArea(struct tagGkscParaChangeWinArea *para)
{
    s32 left,top,right,bottom;
    s32 delta_top,delta_left;
    struct tagGkWinRsc *current,*cwawin,bak;
    struct tagDisplayRsc *disp;
    struct tagGkscParaMoveWin movwin_para;

    cwawin = para->gkwin;
    disp = cwawin->disp;
    left = para->left;
    top = para->top;
    right = para->right;
    bottom = para->bottom;
    if(cwawin == disp->desktop)
    {
        if(((right - left) < disp->width) || ((bottom - top) < disp->height))
            return false;                   //桌面尺寸不能小于显示器尺寸
    }
    if((right < left) || (bottom < top))    //尺寸数据不合法
        return false;

    if(((right-left) == (cwawin->right-cwawin->left))
            && ((bottom-top) == (cwawin->bottom - cwawin->top)))
    {
        movwin_para.gkwin = cwawin;
        movwin_para.left = left;
        movwin_para.top = top;
        GK_MoveWin(&movwin_para);        //只是移动了窗口，尺寸未变
        return true;
    }
    if(cwawin->wm_bitmap != NULL)
    {
        bak = *cwawin;
        if( !__gk_vrmalloc(disp,&bak))
        {
            Djy_SaveLastError(EN_GK_NO_MEMORY);
            printf("显存不足\n\r");
            return false;   //分配显存失败，窗口尺寸未改变(仍然有效)
        }else
        {
            if(bak.wm_bitmap != cwawin->wm_bitmap)
                M_FreeHeap(cwawin->wm_bitmap,disp->DisplayHeap);
            *cwawin = bak;
        }
    }
    if(cwawin == disp->desktop)
    {   //目标窗口是桌面，有额外限制:桌面必须覆盖显示器区域
        s32 dtop,dleft;
        if(para->left > 0)
            dleft = 0;    //桌面左边界必须覆盖显示器
        else if((cwawin->wm_bitmap->width+para->left) < disp->width)
            //右边界必须覆盖显示器
            dleft = disp->width - cwawin->wm_bitmap->width;
        else
            dleft = para->left;
        if(para->top > 0)
            dtop = 0;     //桌面上边界必须覆盖显示器
        else if((cwawin->wm_bitmap->height+para->top) < disp->height)
            //下边界必须覆盖显示器
            dtop =disp->height - cwawin->wm_bitmap->height;
        else
        dtop = para->top;
        delta_left = dleft - cwawin->left;
        delta_top = dtop - cwawin->top;
        cwawin->absx0 += delta_left;//修改窗口绝对坐标
        cwawin->absy0 += delta_top;

        cwawin->top = dtop;
        cwawin->left = dleft;
        cwawin->right = dleft + right - left;
        cwawin->bottom = dtop + bottom - top;

        cwawin->limit_left = dleft;
        cwawin->limit_top = dtop;
        cwawin->limit_right = dleft + disp->width;
        cwawin->limit_bottom = dtop + disp->height;
    }else
    {
        delta_left = para->left-cwawin->left;
        delta_top = para->top-cwawin->top;
        cwawin->absx0 += delta_left;//修改窗口绝对坐标
        cwawin->absy0 += delta_top;

        cwawin->left = left;                      //修改窗口相对坐标
        cwawin->top = top;
        cwawin->right = right;
        cwawin->bottom = bottom;
        __GK_SetBound(cwawin);                        //设置可显示边界
    }

    current = cwawin;
    cwawin->change_flag = CN_GKWIN_CHANGE_ALL;
    //遍历gkwin的所有子窗口,遍历完成后，current就=NULL了
    while((current = (struct tagGkWinRsc*)
           Rsc_TraveScion(&(cwawin->node),&(current->node)))
            != NULL)
    {
        current->absx0 += delta_left; //修改窗口绝对坐标
        current->absy0 += delta_top;
        __GK_SetBound(current);
        current->change_flag = CN_GKWIN_CHANGE_ALL;
    }
    cwawin->disp->reset_clip = true;
    return true;
}

//----移动窗口-----------------------------------------------------------------
//功能: 改变窗口在父窗口内的相对位置，由于子窗口的坐标是相对于父窗口的，故移动窗
//      口时，连子窗口一起移动。本函数是gk_change_win_area函数的一个特例。
//参数: gcwin，目标窗口
//      left、top，新的左上角坐标，相对于父窗口
//返回: 无
//-----------------------------------------------------------------------------
void GK_MoveWin(struct tagGkscParaMoveWin *para)
{
    s32 delta_top,delta_left;
    struct tagGkWinRsc *current,*movewin;
    struct tagDisplayRsc *disp;
    movewin = para->gkwin;
    if((para->left == movewin->left) && (para->top == movewin->top))
        return;                                 //位置没有修改，直接返回。
    disp = movewin->disp;
    if(movewin == disp->desktop)
    {   //目标窗口是桌面，有额外限制:桌面必须覆盖显示器区域
        s32 dtop,dleft;
        if(para->left > 0)
            dleft = 0;    //桌面左边界必须覆盖显示器
        else if((movewin->wm_bitmap->width+para->left) < disp->width)
            //右边界必须覆盖显示器
            dleft = disp->width - movewin->wm_bitmap->width;
        else
            dleft = para->left;
        if(para->top > 0)
            dtop = 0;     //桌面上边界必须覆盖显示器
        else if((movewin->wm_bitmap->height+para->top) < disp->height)
            //下边界必须覆盖显示器
            dtop =disp->height - movewin->wm_bitmap->height;
        else
            dtop = para->top;
        if((dtop == movewin->top) || (dleft == movewin->left))
            return;         //桌面位置实际上未发生改变
        else
        {
            delta_left = dleft - movewin->left;
            delta_top = dtop - movewin->top;
            movewin->absx0 += delta_left;//修改窗口绝对坐标
            movewin->absy0 += delta_top;

            movewin->top = dtop;
            movewin->left = dleft;
            movewin->right += delta_left;
            movewin->bottom += delta_top;

            movewin->limit_left = 0-dleft;
            movewin->limit_top = 0-dtop;
            movewin->limit_right = movewin->limit_left + disp->width;
            movewin->limit_bottom = movewin->limit_top + disp->height;
        }
    }else
    {
        delta_left = para->left-movewin->left;
        delta_top = para->top-movewin->top;
        movewin->absx0 += delta_left;//修改窗口绝对坐标
        movewin->absy0 += delta_top;

        movewin->left = para->left;                      //修改窗口相对坐标
        movewin->top = para->top;
        movewin->right += delta_left;
        movewin->bottom += delta_top;
        __GK_SetBound(movewin);                        //设置可显示边界
    }
    current = movewin;
    current->disp->reset_clip = true;
    movewin->change_flag = CN_GKWIN_CHANGE_ALL;
    //遍历gkwin的所有子窗口
    while((current =
             (struct tagGkWinRsc*)Rsc_TraveScion(&(movewin->node),&(current->node)))
             != NULL)
    {
        current->absx0 += delta_left;        //修改窗口绝对坐标
        current->absy0 += delta_top;
        __GK_SetBound(current);
        current->change_flag = CN_GKWIN_CHANGE_ALL;
    }
}

//----设置可显示边界-----------------------------------------------------------
//功能: 任何窗口，如果即使不考虑被z轴前端窗口剪切，其窗口可显示范围也是有限的，
//      受bound_limit=true的、世代最近的祖先窗口的可显示边界限制。窗口位置改变
//      后，需要重置可显示边界。
//      gkwin，被重置的窗口
//返回: 无
//-----------------------------------------------------------------------------
void __GK_SetBound(struct tagGkWinRsc *gkwin)
{
    struct tagGkWinRsc *ancestor;
    s32 gkwin_absx,gkwin_absy,ancestor_absx,ancestor_absy;
    ancestor = gkwin;
    //获取祖先窗口中第一个边界受父窗口限制的第一个窗口，gkwin的边界受该窗口限定
    while(ancestor->bound_limit == false)
    {
        ancestor = (struct tagGkWinRsc*)Rsc_GetParent(&ancestor->node);
    }
    ancestor = (struct tagGkWinRsc*)Rsc_GetParent(&ancestor->node);
    //由于left和limit_left都是相对于父窗口的，所以要转换成绝对坐标才能比较
    gkwin_absx = gkwin->absx0;  //取本窗口的绝对坐标，也即gkwin->left的绝对坐标
    gkwin_absy = gkwin->absy0;
    //取边界受限窗口的可见边界原点绝对坐标
    ancestor_absx = ancestor->absx0 +ancestor->limit_left - ancestor->left;
    ancestor_absy = ancestor->absy0 +ancestor->limit_top - ancestor->top;

    //
    if(gkwin_absx < ancestor_absx)
       gkwin->limit_left = gkwin->left + ancestor_absx - gkwin_absx;
    else
        gkwin->limit_left = gkwin->left;

    if(gkwin_absy < ancestor_absy)
       gkwin->limit_top = gkwin->top + ancestor_absy - gkwin_absy;
    else
        gkwin->limit_top = gkwin->top;

    if((gkwin_absx+gkwin->right-gkwin->left)
            < (ancestor_absx+ancestor->limit_right-ancestor->limit_left))
        gkwin->limit_right = gkwin->right;
    else
        gkwin->limit_right = gkwin->right -
                    (  gkwin_absx+gkwin->right-gkwin->left
                   -(ancestor_absx+ancestor->limit_right-ancestor->limit_left));

    if((gkwin_absy+gkwin->bottom-gkwin->top)
            < (ancestor_absy+ancestor->limit_bottom-ancestor->limit_top))
        gkwin->limit_bottom = gkwin->bottom;
    else
        gkwin->limit_bottom = gkwin->bottom -
                    (  gkwin_absy+gkwin->bottom-gkwin->top
                   -(ancestor_absy+ancestor->limit_bottom-ancestor->limit_top));

}
//----设置边界模式-------------------------------------------------------------
//功能: 设定窗口的显示边界是否受父窗口限制，限制后，子窗口超出父窗口的部分将不予
//      显示，desktop的直接子窗口默认受限，不能更改。
//参数: gkwin，目标窗口
//      mode，true为受限，false为不受限
//返回: 无
//-----------------------------------------------------------------------------
void GK_SetBoundMode(struct tagGkscParaSetBoundMode *para)
{
    struct tagGkWinRsc *current;
    if(para->gkwin == NULL)
        return;
    if(para->gkwin == para->gkwin->disp->desktop)   //桌面不可修改边界模式
        return;
    if(para->gkwin->bound_limit == para->mode)      //模式未改变
        return;
    if(&para->gkwin->disp->desktop->node == Rsc_GetParent(&para->gkwin->node))
        return;                         //直接放在桌面的子窗口，不许改变
    //目标窗口边界模式改变，窗口属性改变
    para->gkwin->bound_limit = para->mode;
    __GK_SetBound(para->gkwin);
    current = para->gkwin;
    //遍历gkwin的所有子窗口
    while( ( current = (struct tagGkWinRsc*)Rsc_TraveScion
                                    (&para->gkwin->node,&current->node)))
    {
        __GK_SetBound(current);
    }
    para->gkwin->disp->reset_clip = true;
}

//----设置窗口显示优先级-------------------------------------------------------
//功能: 设置一个窗口的优先级，优先级决定在同级窗口中的z轴相对位置，数字越小，在
//      z轴中越靠前，优先级<=0将覆盖父窗口，反之被父窗口覆盖。窗口改变优先级后，
//      它在Z轴中的顺序可能会改变，屏幕内容也可能会改变。
//      由于被改变优先级的窗口可能还有子窗口，所以，在z轴中被移动的，不是一个窗
//      口，而是连续的一段窗口。
//参数: gkwin，目标窗口
//      prio，新优先级
//返回: 无
//-----------------------------------------------------------------------------
void GK_SetPrio(struct tagGkscParaSetPrio *para)
{
    struct tagGkWinRsc *target_section,*gkwin;
    struct tagDisplayRsc *display;
    struct tagGkWinRsc *parent;       //gkwin的父窗口
    //z轴中被移动的窗口段最后端的窗口
    struct tagGkWinRsc *section_start;
    //z轴中被移动的窗口段最前端的窗口
    struct tagGkWinRsc *section_end,*target_win;

    gkwin = para->gkwin;
    display = para->gkwin->disp;
    if(gkwin == display->desktop)  //桌面窗口的优先级不可改变
        return;


    parent = (struct tagGkWinRsc *)Rsc_GetParent(&gkwin->node);
    //取z轴中被移动的窗口段最前端的窗口段
    section_end = (struct tagGkWinRsc*)Rsc_GetSon(&parent->node);
    section_start = (struct tagGkWinRsc*)Rsc_GetPrevious(&section_end->node);
    target_section = section_end;
    while(1)
    {//查找同级窗口中和优先级等于para->prio的的窗口，扫描以z轴为对象
        //扫描顺序:从z轴段前端向后端进行扫描，只扫兄弟窗口，不扫子窗口
        if(target_section->z_prio >= para->prio)
            break;
        //扫描目标窗口所在z轴段最后一窗口，终止跳出
        if(target_section == section_start)
            break;
        target_section = (struct tagGkWinRsc*)Rsc_GetNext(&target_section->node);
    }
    if(target_section->z_prio >= para->prio)
    {// 找到gkwin同级窗口中优先级低于或等于新prio的窗口
        //在资源队列中无须移动，但优先级队列可能要移动
        if((target_section == gkwin)
                || (target_section == (struct tagGkWinRsc*)Rsc_GetNext(&gkwin->node)))
        {
            if((gkwin->z_prio <= 0) && (para->prio > 0))    //移到父窗口后端
            {
                //获取gkwin及其子孙窗口所在z轴段的开始和结束窗口
                section_start = __GK_GetZsectionAllStart(gkwin);
                section_end = __GK_GetZsectionAllEnd(gkwin);
                //考虑gkwin要插入的窗口是否是最前端窗口,
                //是,则将gkwin所在Z轴段最前端的窗口调整为整个Z轴的最前端窗口
                if(section_end == display->z_topmost)
                    display->z_topmost = parent;
                //将gkwin所在z轴段从z轴取出来
                section_start->z_back->z_top = section_end->z_top;
                section_end->z_top->z_back = section_start->z_back;

                section_start->z_top = parent;
                section_end->z_back = parent->z_back;
                parent->z_back->z_top = section_start;
                parent->z_back = section_end;

                gkwin->z_prio = para->prio;
                display->reset_clip = true;
            }
            else if((gkwin->z_prio > 0) && (para->prio <= 0))//移到父窗口前端
            {
                //获取gkwin及其子孙窗口所在z轴段的开始和结束窗口
                section_start = __GK_GetZsectionAllStart(gkwin);
                section_end = __GK_GetZsectionAllEnd(gkwin);
                //考虑gkwin要插入的窗口是否是最前端窗口,
                //是,则将gkwin所在Z轴段最前端的窗口调整为整个Z轴的最前端窗口
                if(parent == display->z_topmost)
                    display->z_topmost = section_end;
                //将gkwin所在z轴段从z轴取出来
                section_start->z_back->z_top = section_end->z_top;
                section_end->z_top->z_back = section_start->z_back;

                section_start->z_back = parent;
                section_end->z_top = parent->z_top;
                parent->z_top->z_back = section_end;
                parent->z_top = section_start;

                gkwin->z_prio = para->prio;
                display->reset_clip = true;
            }
            else    //其他情况，无须移动z轴
            {
                gkwin->z_prio = para->prio;
            }
        }
        else    //资源队列须移动
        {
            Rsc_MoveToPrevious(&target_section->node,&gkwin->node);

            if((para->prio <= 0) && (target_section->z_prio > 0))
                target_win = parent;
            else
                target_win = __GK_GetZsectionAllEnd(target_section);
            //获取gkwin及其子孙窗口所在z轴段的开始和结束窗口
            section_start = __GK_GetZsectionAllStart(gkwin);
            section_end = __GK_GetZsectionAllEnd(gkwin);
            //考虑gkwin要插入的窗口是否是最前端窗口,
            //是,则将gkwin所在Z轴段最前端的窗口调整为整个Z轴的最前端窗口
            if(target_win == display->z_topmost)
                display->z_topmost = section_end;
            //将gkwin所在z轴段从z轴取出来
            section_start->z_back->z_top = section_end->z_top;
            section_end->z_top->z_back = section_start->z_back;

            section_start->z_back = target_win;
            section_end->z_top = target_win->z_top;
            target_win->z_top->z_back = section_end;
            target_win->z_top = section_start;

            gkwin->z_prio = para->prio;
            display->reset_clip = true;
        }
    }
    else
    {//同级窗口优先级均高于新prio
        Rsc_MoveToNext(&target_section->node,&gkwin->node);

        if((para->prio > 0) && (target_section->z_prio <=0))
        {
            target_win = parent;
        }
        else
        {
            //找到new_location及其子孙组成的z轴段中最后端的窗口
            target_win = __GK_GetZsectionAllStart(target_section);
        }
        //以下把gkwin取出并插入move_end_temp后端
        //获取gkwin及其子孙窗口所在z轴段的开始和结束窗口
        section_start = __GK_GetZsectionAllStart(gkwin);
        section_end = __GK_GetZsectionAllEnd(gkwin);

        //将gkwin所在z轴段从z轴取出来
        section_start->z_back->z_top = section_end->z_top;
        section_end->z_top->z_back = section_start->z_back;

        section_start->z_top = target_win;
        section_end->z_back = target_win->z_back;
        target_win->z_back->z_top = section_start;
        target_win->z_back = section_end;

        gkwin->z_prio = para->prio;
        display->reset_clip = true;
    }
}

//----设置光栅操作码---------------------------------------------------------
//功能: 改变窗口的光栅操作码。非缓冲窗口和桌面只支持cn_r3_srccopy，
//        不可修改光栅操作码。
//参数: para，参数
//返回: true=成功，false=失败
//-----------------------------------------------------------------------------
bool_t GK_SetRopCode(struct tagGkscParaSetRopCode *para)
{
    struct tagGkWinRsc *mygkwin;
    struct tagDisplayRsc *mydisplay;

    mygkwin = para->gkwin;
    mydisplay = mygkwin->disp;
    //非缓冲窗口和桌面只支持cn_r3_srccopy，不可修改rop_code
    if((mygkwin->wm_bitmap == NULL)
        || (mydisplay->desktop == mygkwin)
        || (para->RopCode == mygkwin->RopCode))
    {
        return false;
    }
    else
    {
        if(para->RopCode != mygkwin->RopCode)
        {
            mygkwin->dest_blend = __GK_RopIsNeedDestination(para->RopCode);
            mygkwin->RopCode = para->RopCode;
            mygkwin->RopCode &= ~CN_ROP_ROP2_MSK;   //窗口属性不支持rop2
        }
        return true;
    }
}

//----设置窗口透明色-----------------------------------------------------------
//功能: 改变窗口透明色
//参数: gkwin，目标窗口
//      transparentcolor，设置的透明色,颜色格式是真彩色
//返回: true=成功，false=失败
//-----------------------------------------------------------------------------
bool_t GK_SetTransparentColor(struct tagGkscParaSetTransparentColor *para)
{
    //桌面不需要KeyColor
    if(para->gkwin->disp->desktop == para->gkwin)
        return false;
    if(para->gkwin->wm_bitmap == NULL)
        return false;
    para->gkwin->KeyColor = para->KeyColor;

    //下面的程序用于窗口光栅操作属性为透明，只改变当前透明色的情况
    if(para->gkwin->RopCode & CN_ROP_KEYCOLOR_EN)
    {
        para->gkwin->change_flag = CN_GKWIN_CHANGE_ALL;
    }
    return true;
}

//----设置窗口直接写屏属性-----------------------------------------------------
//功能: 设置窗口直接写屏属性。
//参数: gkwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
void GK_SetDirectScreen(struct tagGkscParaSetDirectScreen *para)
{
    if(para->gkwin->direct_screen == true)
        return;
    else
        para->gkwin->direct_screen = true;
}
//----撤销窗口直接写屏属性-----------------------------------------------------
//功能: 撤销窗口直接写屏属性。
//参数: gkwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
void GK_UnSetDirectScreen(struct tagGkscParaUnsetDirectScreen *para)
{
    if(para->gkwin->direct_screen == false)
        return;
    else
        para->gkwin->direct_screen = false;
}
//----销毁窗口(子窗口)---------------------------------------------------------
//功能: 释放窗口所占用的资源。
//参数: gkwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
void __gk_destroy_win(struct tagGkWinRsc *gkwin)
{
    if(gkwin->disp->z_topmost == gkwin)
        gkwin->disp->z_topmost = gkwin->z_back;
    gkwin->z_back->z_top = gkwin->z_top;
    gkwin->z_top->z_back = gkwin->z_back;
    Rsc_DelNode(&gkwin->node);
    if(gkwin->visible_clip != NULL)
        gkwin->disp->reset_clip = true;
    if(gkwin->disp->frame_buffer != NULL)
    {
        if(gkwin->wm_bitmap != NULL)//有帧缓冲在缓冲模式下
        {
            __gk_vfree(gkwin->disp,gkwin);
            M_FreeHeap(gkwin->changed_msk.bm_bits,gkwin->disp->DisplayHeap);
            gkwin->visible_clip =
                            __GK_FreeClipQueue(gkwin->visible_clip);
            return;
        }
        else
            return;
    }
    else
        return;
}
//----销毁窗口-------------------------------------------------------------
//功能: 释放窗口所占用的资源。销毁窗口，同时销毁窗口的子孙窗口。
//参数: gkwin，目标窗口
//返回: 无
//-----------------------------------------------------------------------------
void GK_DestroyWin(struct tagGkscParaDestroyWin *para)
{
    struct tagGkWinRsc *gkwin;

    while((gkwin = (struct tagGkWinRsc *)Rsc_GetTwig(&para->gkwin->node))!= NULL)
    {
        __gk_destroy_win(gkwin);
    }
    __gk_destroy_win(para->gkwin);

}

//----输出窗口redraw部分-------------------------------------------------------
//功能: 把所有窗口有修改的可视域输出至显示器，假设可视域已经准备好。
//参数: display，被扫描的显示器
//返回: 无
//-----------------------------------------------------------------------------
void __GK_OutputRedraw(struct tagDisplayRsc *display)
{
    s32 x,y,x_src,y_src,x_dst,y_dst;
    u32 color;
    struct tagClipRect *clip;
    struct tagDisplayRsc *mirror,*current;
    struct tagGkWinRsc *gkwin,*desktop_gkwin,*topwin,*frame_buf;
    struct tagGkucParaRepaint repaint;
    struct tagRectBitmap *src,*dst;
    struct tagRectangle *rect,SrcRect;
    struct tagRectBitmap *bitmap;

    frame_buf = display->frame_buffer;          //取帧缓冲窗口
    desktop_gkwin = display->desktop;
    topwin = display->z_topmost;
    if(frame_buf == NULL)   //无帧缓冲，无帧缓冲的显示器，必然没有窗口缓冲
    {
        //在没有缓冲区的情况下，绘制操作是不会生成redraw_clip的，只有窗口管理
        //操作才会要求重绘。
        gkwin = desktop_gkwin;
        do{
            clip = gkwin->redraw_clip;
            if(clip != NULL)
            {
                repaint.win = (void*)gkwin;
                repaint.redraw_clip = clip;
                GK_UsercallChunnel(CN_GKUC_REPAINT,g_ptUsercallSemp,NULL,
                                  &repaint,sizeof(struct tagGkucParaRepaint));
                //等待用户完成重绘操作，超时(1秒)不候
                Lock_SempPend(g_ptUsercallSemp,0);
                gkwin->redraw_clip = __GK_FreeClipQueue(gkwin->redraw_clip);
            }
            gkwin = gkwin->z_top;
        }while(gkwin != topwin->z_top);
    }else                                           //有帧缓冲
    {
        gkwin = desktop_gkwin;
        do{
            clip = gkwin->redraw_clip;
            if(clip != NULL)
            {
                if(gkwin->wm_bitmap == NULL)   //无窗口缓冲
                {
                    repaint.win = (void*)gkwin;
                    repaint.redraw_clip = clip;
                    GK_UsercallChunnel(CN_GKUC_REPAINT,g_ptUsercallSemp,NULL,
                                    &repaint,sizeof(struct tagGkucParaRepaint));
                    //等待用户完成重绘操作，超时(1秒)不候
                    Lock_SempPend(g_ptUsercallSemp,0);
                }else                                   //有窗口缓冲
                {
                    do
                    {
                        SrcRect.left = clip->rect.left-gkwin->absx0;
                        SrcRect.top = clip->rect.top-gkwin->absy0;
                        SrcRect.right = clip->rect.right -gkwin->absx0;
                        SrcRect.bottom = clip->rect.bottom -gkwin->absy0;
                        if(!display->draw.blt_bm_to_bm(
                                            frame_buf->wm_bitmap,
                                            &clip->rect,
                                            gkwin->wm_bitmap,
                                            &SrcRect,
                                            gkwin->RopCode,
                                            gkwin->KeyColor))
                        {
                            dst = frame_buf->wm_bitmap;
                            src = gkwin->wm_bitmap;

                            y_dst = clip->rect.top;
                            if(gkwin->RopCode == 0)
                            {
                                for(y_src = clip->rect.top-gkwin->absy0;
                                    y_src < clip->rect.bottom-gkwin->absy0;
                                    y_src++)
                                {
                                    x_dst = clip->rect.left;
                                    for(x_src = clip->rect.left-gkwin->absx0;
                                        x_src < clip->rect.right-gkwin->absx0;
                                        x_src++)
                                    {
                                        __GK_CopyPixelBm(dst,src,x_dst,y_dst,
                                                            x_src,y_src);
                                        x_dst ++;
                                    }
                                    y_dst ++;
                                 }
                            }
                            else
                            {
                                for(y_src = clip->rect.top-gkwin->absy0;
                                    y_src < clip->rect.bottom-gkwin->absy0;
                                    y_src++)
                                {
                                    x_dst = clip->rect.left;
                                    for(x_src = clip->rect.left-gkwin->absx0;
                                        x_src < clip->rect.right-gkwin->absx0;
                                        x_src++)
                                    {
                                        __GK_CopyPixelRopBm(dst,src,
                                                x_dst,y_dst,x_src,y_src,
                                                gkwin->RopCode,gkwin->KeyColor);
                                        x_dst ++;
                                    }
                                    y_dst ++;
                                }
                            }
                        }
                        __GK_ShadingRect(frame_buf,&clip->rect);
                        clip = clip->next;
                    }while(clip != gkwin->redraw_clip);
                }
                gkwin->redraw_clip = __GK_FreeClipQueue(gkwin->redraw_clip);
            }
            gkwin = gkwin->z_top;
        }while(gkwin != topwin->z_top);
        //以下把帧缓冲区输出到显示器
        clip = __GK_GetChangedClip(frame_buf);  //获取帧缓冲中被修改的可视域
        if(clip != NULL)
        {
            __GK_ShadingClear(frame_buf);
            frame_buf->redraw_clip = clip;
            mirror = (struct tagDisplayRsc *)gkwin->disp->node.child;//镜像显示器
            do{
                if(display->framebuf_direct == false)
                {
                    //输出到screen
                    if(display->draw.bm_to_screen(&clip->rect,
                                                frame_buf->wm_bitmap,
                                                clip->rect.left,clip->rect.top))
                    {
                    }
                    else
                    {
                        rect = &clip->rect;
                        bitmap = frame_buf->wm_bitmap;
                        for(y = rect->top;y < rect->bottom;y++)
                        {
                            for(x = rect->left;x < rect->right;x++)
                            {
                                color = GK_GetPixelBm(bitmap,x,y);
                                color =GK_ConvertColorToRGB24(bitmap->PixelFormat,
                                                         color,bitmap->ExColor);
                                display->draw.set_pixel_screen(x,y,color,
                                                                CN_R2_COPYPEN);
                            }
                        }
                    }
                }
                current = mirror;

                while(current != NULL)  //遍历全部镜像窗口
                {
                    current->draw.bm_to_screen(&clip->rect,
                                                frame_buf->wm_bitmap,
                                                clip->rect.left,clip->rect.top);
                    current = (struct tagDisplayRsc *)Rsc_TraveScion(
                                                &mirror->node,&current->node);
                }
                clip = clip->next;
            }while(clip != frame_buf->redraw_clip);
            frame_buf->redraw_clip =
                            __GK_FreeClipQueue(frame_buf->redraw_clip);
        }
    }
}

//----显示内容输出到显示器-----------------------------------------------------
//功能: 把所有显示器中需要输出的部分输出到显示器。
//参数: 无
//返回: 无
//-----------------------------------------------------------------------------
void __gk_redraw_all(void)
{
    struct tagDisplayRsc *display,*display_next;
    display = (struct tagDisplayRsc *)Rsc_GetSon(&s_tDisplayRootRsc);
    display_next = display;
    if(display != NULL)
    {
        do{
            __GK_GetRedrawClipAll(display_next);//扫描取得新的需重绘的区域
            __GK_OutputRedraw(display_next);    //重绘
            display_next = (struct tagDisplayRsc *)
                                    Rsc_GetNext(&display_next->node);
        }while(display != display_next);  //扫描所有显示器
    }
}
//----系统调用管道-------------------------------------------------------------
//功能: gui api与gui kernel的接口函数，把api所请求的绘制要求传递到gui kernel，在
//      mp版本中，可以考虑使用软中断实现。
//参数: param，参数地址
//      size，参数长度
//返回: 实际写入管道的数据量，0或者size。
//-----------------------------------------------------------------------------
u16 GK_SyscallChunnel(u16 command,u32 sync_time,void *param1,u16 size1,
                                                void *param2,u16 size2)
{
    u16 completed = 0;
    u8 buf[2];
    Lock_MutexPend(g_tGkChunnel.syscall_mutex,1000*mS);   //管道访问互斥
    while(1)
    {
        if((Ring_Capacity(&g_tGkChunnel.ring_syscall)
                    - Ring_Check(&g_tGkChunnel.ring_syscall)) <(u32)(size1+size2+2))
        {       //管道空闲容量不够
            if(!Lock_SempPend(g_tGkChunnel.syscall_semp,1000*mS))//等待管道释放空间
                break;                                  //等待管道失败
            else
                continue;                               //再次检查容量
        }
        buf[0] = (u8)command;
        buf[1] = (u8)(command>>8);
        completed = Ring_Write(&g_tGkChunnel.ring_syscall,buf,2);
        if(size1 != 0)
            completed += Ring_Write(&g_tGkChunnel.ring_syscall,param1,size1);
        if(size2 != 0)
            completed += Ring_Write(&g_tGkChunnel.ring_syscall,param2,size2);
        break;
    }
    Lock_MutexPost(g_tGkChunnel.syscall_mutex);    //管道访问互斥解除
    if(Djy_IsMultiEventStarted())
    {
        if(Lock_SempQueryFree(g_ptSyscallSemp) == 0)
            Lock_SempPost(g_ptSyscallSemp);
        if(0 != sync_time)      //设定了等待时间
        {
            Lock_MutexPend(g_ptGkServerSync,sync_time);
            Lock_MutexPost(g_ptGkServerSync);
        }
    }
    else
    {
        Ring_Read(&g_tGkChunnel.ring_syscall,(u8*)draw_chunnel_buf,
                                        gc_u32CfgGuiCmdDeep);
        __ExecOneCommand(command,draw_chunnel_buf+2);
    }
    return completed;
}


//----用户调用管道-------------------------------------------------------------
//功能: 本函数是gk_syscall_chunnle函数的逆操作函数，当gui kernel需要上层应用程序
//      完成某些操作时，调用本函数，把操作要求通过管道传送上去。
//参数: param，参数地址
//      size，参数长度
//返回: 实际写入管道的数据量，0或者size。
//注: usercall_buf只有gkwin会往里写东西，故无需互斥量保护
//-----------------------------------------------------------------------------
u16 GK_UsercallChunnel(u16 usercall_id,struct tagSemaphoreLCB *semp,
                        void **result,void *param,u16 size)
{
    u16 completed = 0,loop;
    u8 buf[100];    //不会有超过100字节的命令的。
    while(1)
    {
        if((Ring_Capacity(&g_tGkChunnel.ring_usercall)
                    - Ring_Check(&g_tGkChunnel.ring_usercall)) < size)
        {       //管道空闲容量不够
            if(!Lock_SempPend(g_tGkChunnel.usercall_semp,0))//等待管道充足空间
                break;                                  //等待管道失败
            else
                continue;                               //再次检查容量
        }
        buf[0] = (u8)usercall_id;
        buf[1] = (u8)(usercall_id>>8);
        for(loop = 0; loop < sizeof(void*); loop++)
        {
            buf[2+loop] = ((u8*)&semp)[loop] ;
        }
        for(loop = 0; loop < sizeof(void*); loop++)
        {
            buf[2 + sizeof(void*) +loop] = ((u8*)&result)[loop] ;
        }
        for(loop = 0; loop < size-2+2*sizeof(void*); loop++)
        {
            buf[2+2*sizeof(void*)+loop] = ((u8*)param)[loop] ;
        }
        completed = Ring_Write(&g_tGkChunnel.ring_usercall,buf,
                                size+2+sizeof(void*));
    }
    return completed;
}

//返回: 命令长度(字节数)
u32 __ExecOneCommand(u16 DrawCommand,u8 *ParaAddr)
{
    u32 result = 0;
    switch(DrawCommand)
    {
        case CN_GKSC_CREAT_GKWIN:
        {
            struct tagGkscParaCreateGkwin para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaCreateGkwin));
            *(para.result) = (void*)GK_CreateWin(&para);
            result = sizeof(struct tagGkscParaCreateGkwin);
        } break;
        case CN_GKSC_SET_PIXEL:
        {
            struct tagGkscParaSetPixel para;
            memcpy(&para,ParaAddr,sizeof(struct tagGkscParaSetPixel));
            GK_SetPixel(&para);
            result = sizeof(struct tagGkscParaSetPixel);
        } break;
        case CN_GKSC_LINETO:
        {
            struct tagGkscParaLineto para;
            memcpy(&para,ParaAddr,sizeof(struct tagGkscParaLineto));
            GK_Lineto(&para);
            result = sizeof(struct tagGkscParaLineto);
        } break;
        case CN_GKSC_LINETO_INC_END:
        {
            struct tagGkscParaLineto para;
            memcpy(&para,ParaAddr,sizeof(struct tagGkscParaLineto));
            GK_LinetoIe(&para);
            result = sizeof(struct tagGkscParaLineto);
        } break;
        case CN_GKSC_DRAW_BITMAP_ROP:
        {
            struct tagGkscParaDrawBitmapRop para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaDrawBitmapRop));
            GK_DrawBitMap(&para);
            result = sizeof(struct tagGkscParaDrawBitmapRop);
        } break;
        case CN_GKSC_FILL_WIN:
        {
            struct tagGkscParaFillWin para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaFillWin));
            GK_FillWin(&para);
            result = sizeof(struct tagGkscParaFillWin);
        } break;
#if 0
        case CN_GKSC_FILL_PART_WIN:
        {
            struct tagGkscParaFillPartWin para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaFillPartWin));
            GK_FillPartWin(&para);
            result = sizeof(struct tagGkscParaFillPartWin);
        } break;
#endif
        case CN_GKSC_FILL_RECT:
        {
            struct tagGkscParaGradientFillWin para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaGradientFillWin));
            GK_GradientFillRect(&para);
            result = sizeof(struct tagGkscParaGradientFillWin);
        } break;
        case CN_GKSC_SET_ROP_CODE:
        {
            struct tagGkscParaSetRopCode para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaSetRopCode));
            GK_SetRopCode(&para);
            result = sizeof(struct tagGkscParaSetRopCode);
        } break;
        case CN_GKSC_DRAW_TEXT:
        {
            struct tagGkscParaDrawText para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaDrawText));
            result = sizeof(struct tagGkscParaDrawText);
            GK_DrawText(&para,(char*)ParaAddr+result);
            result += para.count;
        }break;
        case CN_GKSC_SET_TRANSPARENTCOLOR:
        {
            struct tagGkscParaSetTransparentColor para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaSetTransparentColor));
            GK_SetTransparentColor(&para);
            result = sizeof(struct tagGkscParaSetTransparentColor);
        } break;
        case CN_GKSC_DESTROY_WIN:
        {
            struct tagGkscParaDestroyWin para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaDestroyWin));
            GK_DestroyWin(&para);
            result = sizeof(struct tagGkscParaDestroyWin);
        } break;
        case CN_GKSC_SET_PRIO:
        {
            struct tagGkscParaSetPrio para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaSetPrio));
            GK_SetPrio(&para);
            result = sizeof(struct tagGkscParaSetPrio);
        } break;
        case CN_GKSC_SET_BOUND_MODE:
        {
            struct tagGkscParaSetBoundMode para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaSetBoundMode));
            GK_SetBoundMode(&para);
            result = sizeof(struct tagGkscParaSetBoundMode);
        }break;
        case CN_GKSC_MOVE_WIN:
        {
            struct tagGkscParaMoveWin para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaMoveWin));
            GK_MoveWin(&para);
            result = sizeof(struct tagGkscParaMoveWin);
        }break;
        case CN_GKSC_CHANGE_WIN_AREA:
        {
            struct tagGkscParaChangeWinArea para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaChangeWinArea));
            GK_ChangeWinArea(&para);
            result = sizeof(struct tagGkscParaChangeWinArea);
        }break;
        case CN_GKSC_DRAW_CIRCLE:
        {
            struct tagGkscParaDrawCircle para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaDrawCircle));
            GK_DrawCircle(&para);
            result = sizeof(struct tagGkscParaDrawCircle);
        } break;
        case CN_GKSC_BEZIER:
        {
            struct tagGkscParaBezier para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaBezier));
            GK_Bezier(&para);
            result = sizeof(struct tagGkscParaBezier);
        } break;
        case CN_GKSC_SET_DIRECT_SCREEN:
        {
            struct tagGkscParaSetDirectScreen para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaSetDirectScreen));
            GK_SetDirectScreen(&para);
            result = sizeof(struct tagGkscParaSetDirectScreen);
        } break;
        case CN_GKSC_UNSET_DIRECT_SCREEN:
        {
            struct tagGkscParaUnsetDirectScreen para;
            memcpy(&para,ParaAddr,
                    sizeof(struct tagGkscParaUnsetDirectScreen));
            GK_UnSetDirectScreen(&para);
            result = sizeof(struct tagGkscParaUnsetDirectScreen);
        } break;
        case CN_GKSC_SYNC_SHOW:
        {
            __gk_redraw_all();
        } break;
        default: break;
    }       //for switch(command)
    return result;
}

//----gui kernel服务器---------------------------------------------------------
//功能: 从缓冲区取出显示请求，并处理之。
//参数: 无
//返回: 无
//-----------------------------------------------------------------------------
ptu32_t GK_Server(void)
{
    u16 command;
    u32 num,offset;
    Lock_MutexPend(g_ptGkServerSync,CN_TIMEOUT_FOREVER);
    Djy_SetEventPrio(248);
    while(1)
    {
        //一次读取全部命令，因为发送时有互斥量保护，所以管道中的数据肯定是完整的
        //命令，不存在半条命令的可能。
        num = Ring_Read(&g_tGkChunnel.ring_syscall,(u8*)draw_chunnel_buf,
                                        gc_u32CfgGuiCmdDeep);

        if(num == 0)
        {
            //所有命令均执行完后，检查有没有win buffer需要刷到screen上
//            __gk_redraw_all();

            Djy_SetEventPrio(1);
            Lock_MutexPost(g_ptGkServerSync);
            Lock_MutexPend(g_ptGkServerSync,CN_TIMEOUT_FOREVER);
            Djy_SetEventPrio(248);
            Lock_SempPend(g_ptSyscallSemp,CN_TIMEOUT_FOREVER);
            continue;
        }
        if(Lock_SempQueryFree(g_tGkChunnel.syscall_semp) == 0)
            Lock_SempPost(g_tGkChunnel.syscall_semp);    //释放管道
        offset = 0;
        while(num > offset)
        {
            //由于管道中的数据可能不对齐，故必须把数据copy出来，不能直接用指针
            //指向或强制类型转换
            command = draw_chunnel_buf[offset] + ((u16)draw_chunnel_buf[offset+1]<<8);
            //sizeof(u16)不可用2替代，在cn_byte_bits>=16的机器上，sizeof(u16)=1.
            offset += sizeof(u16);
//            sync_draw = draw_chunnel_buf[offset];
//            offset += 1;
            offset += __ExecOneCommand(command,draw_chunnel_buf + offset);
        }   //for while(num != offset)
    }   //for while(1)

}

