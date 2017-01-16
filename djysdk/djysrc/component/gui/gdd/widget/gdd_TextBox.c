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
// Copyright (c) 2014 ����Ȩ�����������������޹�˾���С�����Ȩ�˱���һ��Ȩ����
//
// �����Ȩ�����ʹ���߷��������������������£�����ʹ����ʹ�ü���ɢ����
// ������װԭʼ�뼰����λ��ִ����ʽ��Ȩ�������۴˰�װ�Ƿ񾭸�����Ȼ��
//
// 1. ���ڱ�����Դ�������ɢ�������뱣�������İ�Ȩ���桢�������б�����
//    ������������������
// 2. ���ڱ��׼�����λ��ִ����ʽ����ɢ���������������ļ��Լ�������������
//    ��ɢ����װ�е�ý�鷽ʽ����������֮��Ȩ���桢�������б����Լ�����
//    ������������
// 3. ��������Ϊ�����߲���ϵͳ����ɲ��֣�δ����ǰȡ�õ��������ɣ���������ֲ����
//    �����߲���ϵͳ���������С�

// �����������������Ǳ�������Ȩ�������Լ�����������״��"as is"���ṩ��
// ��������װ�����κ���ʾ��Ĭʾ֮�������Σ������������ھ��������Լ��ض�Ŀ
// �ĵ�������ΪĬʾ�Ե�������Ȩ�����˼�������֮�����ߣ������κ�������
// ���۳�����κ��������塢���۴�����Ϊ���Լ��ϵ���޹�ʧ������������Υ
// Լ֮��Ȩ��������ʧ������ԭ��ȣ����𣬶����κ���ʹ�ñ�������װ��������
// �κ�ֱ���ԡ�����ԡ�ż���ԡ������ԡ��ͷ��Ի��κν�����𺦣�����������
// �������Ʒ������֮���á�ʹ����ʧ��������ʧ��������ʧ��ҵ���жϵȵȣ���
// �����κ����Σ����ڸ���ʹ���ѻ���ǰ��֪���ܻ���ɴ����𺦵���������Ȼ��
//-----------------------------------------------------------------------------
//����ģ��: GDD
//����:  zhb.
//�汾��V1.0.0
//�ļ�����: ��ť�ؼ�ʵ��
//����˵��:
//�޶���ʷ:
//2. ...
//1. ����: 2016-6-14
//   ����:  zhb.
//   �°汾�ţ�V1.0.0
//   �޸�˵��: ԭʼ�汾
//---------------------------------

#include  "gdd.h"
#include  <gui/gdd/gdd_private.h>
#include  "font.h"
#include  <gdd_widget.h>

extern HWND g_CursorHwnd;         //��괰��

#define CN_CHAR_NUM_MAX                 255
#define CN_CANCLE_KEY                   0xA3

// =============================================================================
// ��������: ��ȡ�ַ��������ֽ���,TextBoxֻ����ʾ������Ϣ�����ȼ���ַ���,�������
//           \n(���з�)���������ȡ���з�֮ǰ���ַ���Ϊ��Ч���ַ���.
// �������: char *str:�ַ���
// �������: �ޡ�
// ����ֵ  :��ȡ����Ч�ַ��ֽ�����
// =============================================================================
static s16 __GetValidStrLen(char *str)
{
    u8 cnt=0;
    char ch;
    s16 str_len;
    if(str==NULL)
        return -1;
    cnt=GetStrLineCount(str);
    if(cnt>1)
    {
        while(1)
        {
            ch=*str;
            if(ch!='\n')
            {
                str_len++;
                str++;
            }
            else
            {
                *(str+1)='\0';  //todo: �����const����ô�죿
                break;
            }
        }
    }
    else
    {
        str_len=strlen(str);
    }
    return str_len;
}

// =============================================================================
// ��������: ��ȡ�ַ�����ָ����ŵ��ַ�����Ŵ�1,2,3......��ʼ.
// �������: str:�ַ���ָ��
//           idx:�ַ����
// �������:
// ����ֵ  :ucs4�ַ�
// =============================================================================
static u32 __GetCharByIndex(char *str,u8 idx)
{
    s16 str_len=0,len=0;
    struct FontRsc* cur_font;
    struct Charset* cur_enc;
    u32 wc=0;
    u8 cnt;
    if(str==NULL)
        return false;
    str_len=__GetValidStrLen(str);
    if(str_len==-1)
        return false;
     //�����ַ������ַ���
    cur_font = Font_GetCurFont();
    cur_enc = Charset_NlsGetCurCharset();
    for(; str_len > 0;)
    {
       len= cur_enc->MbToUcs4(&wc, str, -1);
       if(len == -1)
       {  // ��Ч�ַ�
           str_len--;
           str++;
       }
       else if(len == 0)
       {
            break;
       }
       else
       {    // ��Ч�ַ�
           str += len;
           str_len -= len;
           cnt++;
           if(cnt==idx)
           {
               return wc;
           }
        }
    }
    return wc;

}
// =============================================================================
// ��������: ��ȡ�ַ������ַ������ܵ��ַ���ռ��������
// �������: str:�ַ���ָ��
//          pChunm:�ַ���ָ��,�������ΪNULLʱ�򲻼����ַ�����
//          pChWidthSum:�ַ���ռ��������ָ�룬�������ΪNULLʱ�򲻼����ַ���ռ�����ؿ���.
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static bool_t __GetValidStrInfo(char *str,u16 *pChunm,u16 *pChWidthSum)
{
    u16 cnt=0,chwidthsum=0;
    s16 str_len=0;
    s32 len,chwidth=0;
    struct FontRsc* cur_font;
    struct Charset* cur_enc;
    u32 wc;
    if(str==NULL)
        return false;
    str_len=__GetValidStrLen(str);
    if(str_len==-1)
        return false;
     //�����ַ������ַ���
     cur_font = Font_GetCurFont();
     cur_enc = Charset_NlsGetCurCharset();
     for(; str_len > 0;)
     {
        len= cur_enc->MbToUcs4(&wc, str, -1);
        if(len == -1)
        {   // ��Ч�ַ�
            str_len--;
            str++;
        }
        else if(len == 0)
        {
             break;
        }
        else
        {          // ��Ч�ַ�
            str += len;
            str_len -= len;
            if(pChunm!=NULL)
            {
              cnt++;
            }
            if(pChWidthSum!=NULL)
            {
              chwidth=cur_font->GetCharWidth(wc);
              chwidthsum+=chwidth;
            }
            continue;
        }
     }

     *pChWidthSum=chwidthsum;
     *pChunm=cnt;
     return true;
}

// =============================================================================
// ��������: ��ȡ�ַ�����ָ������ַ���ռ�����ؿ���.
// �������: str:�ַ���ָ��
//          idx:�ַ����.
// �������: �ޡ�
// ����ֵ  :�ַ������ܿ���.
// =============================================================================
static s16 __GetStrWidth(char *str,u16 idx)
{
    struct FontRsc* cur_font;
    struct Charset* cur_enc;
    u32 wc;
    s32 len,chwidth=0;
    u16 cnt=0,chwidthsum=0;
    s16 str_len=0;
    if(str==NULL)
         return -1;
    str_len=__GetValidStrLen(str);
    if(str_len==-1)
         return -1;
    if(idx==0)
        return 0;
    //�����ַ������ַ���
    cur_font = Font_GetCurFont();
    cur_enc = Charset_NlsGetCurCharset();
    for(; str_len > 0;)
    {
        len= cur_enc->MbToUcs4(&wc, str, -1);
        if(len == -1)
        {   // ��Ч�ַ�
            str_len--;
            str++;
        }
        else if(len == 0)
             break;
        else
        {          // ��Ч�ַ�
            str += len;
            str_len -= len;
            cnt++;
            chwidth=cur_font->GetCharWidth(wc);
            chwidthsum+=chwidth;
            if(cnt==idx)
                break;
            else
               continue;
        }
    }

     return chwidthsum;
}

// =============================================================================
// ��������: �ƶ����.
// �������: hwnd:�ı��򴰿ھ��;
//          idx:�ı����ַ����.
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static bool_t __MoveCursor(HWND hwnd,u8 idx)
{
    //�ڲ����ã�������в������
    TextBox *pTB;
    char *str;
    RECT rc;
    u16 width;
    s32 x,y;
    pTB=(TextBox *)GetWindowPrivateData(hwnd);
    if(pTB==NULL)
        return false;
    GetClientRectToScreen(hwnd,&rc);
    str=hwnd->Text;
    if(str==NULL)
        return false;
    if(idx==0)
    {
        x=rc.left+1;
    }
    else
    {
        width=__GetStrWidth(str,idx);
        x=rc.left+width+1;
    }
    y=rc.top;
    MoveWindow(g_CursorHwnd,x,y);
    UpdateDisplay(CN_TIMEOUT_FOREVER);
    return true;
}
// =============================================================================
// ��������: ���ݴ����������ȡ�ı������ַ����
// �������: hwnd:�ı��򴰿ھ��
//           x:������x��������
//           y:������y��������
// �������: �ޡ�
// ����ֵ  :�ı������ַ����.
// =============================================================================
static s16 __FindIdx(HWND hwnd,u16 x,u16 y)
{
    char *str;
    TextBox *pTB;
    RECT rc;
    s16 tmp,val1,val2;
    u16 chnum,idx,width=0;
    str=hwnd->Text;
    if(str==NULL)
        return -1;
    pTB=(TextBox *)GetWindowPrivateData(hwnd);
    if(pTB==NULL)
        return -1;
    chnum=pTB->ChNum;
    GetClientRectToScreen(hwnd,&rc);
    for(idx=1;idx<chnum;idx++)
    {
        width=__GetStrWidth(str, idx);
        tmp=rc.left+width;
        if(tmp>x)
        {
           break;
        }
    }
    val1=tmp-x;
    width=__GetStrWidth(str, idx-1);
    tmp=rc.left+width;
    val2=x-tmp;
    if(val1>val2)
    {
        return idx-1;
    }
    else
    {
        return idx;
    }
}
// =============================================================================
// ��������: ��ȡ�ַ����дӿ�ʼ��ָ����ŵ��ַ��ܵ��ֽ���.
// �������: str:�ַ���ָ��
//           idx:�ַ�����ָ���ַ���ţ���1��ʼ��������������.
// �������: ��
// ����ֵ  :ָ���ַ���ַ.
// =============================================================================
static s16 __CharToBytes(char *str,u8 num)
{
     u16 bytes=0;
     s16 str_len=0;
     u8 cnt=0;
     u32 wc;
     s32 len;
     struct FontRsc* cur_font;
     struct Charset* cur_enc;
     if(str==NULL)
          return -1;
     str_len=__GetValidStrLen(str);
     if(str_len==-1)
        return -1;
     if(num==0)
         return -1;
     cur_font = Font_GetCurFont();
     cur_enc = Charset_NlsGetCurCharset();
     for(; str_len > 0;)
     {
        len= cur_enc->MbToUcs4(&wc, str, -1);
        if(len == -1)
        {     // ��Ч�ַ�
            str_len--;
            str++;
            bytes++;
        }
        else if(len == 0)
        {
            break;
        }
        else
        {          // ��Ч�ַ�
             str += len;
             str_len -= len;
             bytes+=len;
             cnt++;
         if(cnt==num)
         {
             return bytes;
         }
              continue;
         }
     }
     return bytes;
}

// =============================================================================
// ��������:��TextBox�������ַ���
// �������: hwnd:�ı�������
//          str:�ַ���ָ��
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static bool_t TextBox_AddChar(HWND hwnd,char *str )
{
     char *text;
     s16 len=0,str_len=0;
     u16 cnt,num=0;
     s16 bytes;
     u8 i;
     TextBox *pTB;
     bool_t ret;
     if(hwnd==NULL)
        return false;
     if(str==NULL)
        return false;
     pTB=(TextBox *)GetWindowPrivateData(hwnd);
     if(pTB==NULL)
        return false;
    //����ı���༭����
     if(pTB->EditProperty==EN_R_O)
        return false;
     text=hwnd->Text;
     len=__GetValidStrLen(text);
     if(len==-1)
          return false;
     //���һ��str�ĺϷ���
      str_len=__GetValidStrLen(str);
      if(str_len==-1)
          return false;
      ret=__GetValidStrInfo(str,&num,NULL);
      if(!ret)
          return false;
      //�ַ���ԭ�е��ַ���
      cnt=pTB->ChNum;
      if(cnt+num>CN_CHAR_NUM_MAX)
      {
           num=CN_CHAR_NUM_MAX-cnt;
      }
      bytes=__CharToBytes(str, num);
      //���ַ�������ԭ���ַ�������
      for(i=0;i<bytes;i++)
      {
          *(text+len+i)=*(str+i);
      }
      *(text+len+bytes)='\0';
      return true;
}

// =============================================================================
// ��������:��TextBoxָ��λ�ÿ�ʼɾ���ַ���
// �������: hwnd,�ı���������
//          idx,�ַ���ţ�
//          count,�ַ���.
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static bool_t TextBox_DeleteChar(HWND hwnd,u8 idx,u8 count)
{
     char *text;
     char temp_str[2*CN_CHAR_NUM_MAX+1];
     u8 i,cnt,k,tmp;
     u16 f_len,len,last_len=0;
     s16 str_len=0;
     bool_t ret;
     TextBox *pTB;
     if(hwnd==NULL)
         return false;
     pTB=(TextBox *)GetWindowPrivateData(hwnd);
     if(pTB==NULL)
            return false;
     if(pTB->EditProperty==EN_R_O)
           return false;
     text=hwnd->Text;
     cnt=pTB->ChNum;
     str_len=__GetValidStrLen(text);
     if(str_len==-1)
         return false;
     if(count>idx)
         return false;
     tmp=idx-count;
     if(tmp==0)
     {
         k=cnt-idx;
         if(k==0)
         {
            *text='\0';
            return true;
         }
         else
         {
             len=__CharToBytes(text,idx);
             last_len=str_len-len;
             for(i=0;i<last_len;i++)
             {
                temp_str[i]=*(text+len+i);
             }
             temp_str[last_len]='\0';
             str_len=last_len+1;
             memcpy(text,temp_str,str_len);
             return true;
         }
     }

     f_len=__CharToBytes(text,tmp);
     for(i=0;i<f_len;i++)
     {
        temp_str[i]=*(text+i);
     }
     if(idx!=cnt)
     {
        len=__CharToBytes(text,idx);
        last_len=str_len-len;
        for(i=0;i<last_len;i++)
        {
            temp_str[f_len+i]=*(text+len+i);
        }
     }
     temp_str[f_len+last_len]='\0';
     str_len=f_len+last_len+1;
     memcpy(text,temp_str,str_len);
     return true;
}

// =============================================================================
// ��������:��TextBoxָ���ַ�����ʼ�����ַ���
// �������:  hwnd,�ı���������
//           idx,�ַ���ţ�
//           str,�ַ���ָ��.
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static bool_t TextBox_InsertChar(HWND hwnd,u8 idx,char *str)
{
     char temp_str[2*CN_CHAR_NUM_MAX+1];
     char *text;
     u8 i;
     u16 f_len,last_len;
     u16 num=0,cnt;
     s16 bytes,str_len=0;
     TextBox *pTB;
     bool_t ret;
     if(hwnd==NULL)
        return false;
     if(str==NULL)
        return false;
     pTB=(TextBox *)GetWindowPrivateData(hwnd);
     if(pTB==NULL)
        return false;
     //����ı���༭����
     if(pTB->EditProperty==EN_R_O)
            return false;
     text=hwnd->Text;
     str_len=__GetValidStrLen(text);
     if(str_len==-1)
         return false;
     ret=__GetValidStrInfo(str,&num,NULL);
     if(!ret)
         return false;
    //�ַ���ԭ�е��ַ���
     cnt=pTB->ChNum;
     if(cnt+num>CN_CHAR_NUM_MAX)
     {
          num=CN_CHAR_NUM_MAX-cnt;
     }
     bytes=__CharToBytes(str, num);
     f_len=__CharToBytes(text,idx);

     for(i=0;i<f_len;i++)
     {
         temp_str[i]=*(text+i);
     }
     //Insert
     for(i=0;i<bytes;i++)
     {
         temp_str[f_len+i]=*(str+i);
     }
     last_len=str_len-f_len;
     for(i=0;i<last_len;i++)
     {
         temp_str[f_len+bytes+i]=*(text+f_len+i);
     }
     temp_str[f_len+bytes+last_len]='\0';
     str_len=f_len+bytes+last_len+1;
     memcpy(text,temp_str,str_len);

     return true;
}

// =============================================================================
// ��������: �ı���ؼ�Create��Ϣ��������.
// �������: pMsg,������Ϣָ��.
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static bool_t TextBox_Create(struct WindowMsg *pMsg)
{
	HWND hwnd;
	TextBox *pTB;
	if(pMsg==NULL)
		return false;
	hwnd =pMsg->hwnd;
	if(hwnd==NULL)
		return false;
	if(pMsg->Param1==0)
	{
		pTB=(TextBox *)malloc(sizeof(TextBox));
		if(pTB==NULL)
			return false;
		pTB->ChNum=0;
		pTB->CharNumLimit=CN_CHAR_NUM_MAX;
		pTB->EditProperty=EN_R_W;
		pTB->Visible=true;
		pTB->CursorLoc=0;
        pTB->IsMultiLines=false;
        pTB->MaxLines=1;
        pTB->CharWidthSum=0;
        SetWindowPrivateData(hwnd,(void *)pTB);
    }

    return true;
}
// =============================================================================
// ��������: �жϰ������µ��ַ��Ƿ�Ϊ���ֻ���С�����ַ�.
// �������: keyval:�ַ�ASCII��.
// �������: �ޡ�
// ����ֵ  :���򷵻�true,���򷵻�false.
// =============================================================================
static bool_t __IsValidInputKey(u8 keyval)
{
    bool_t ret=true;
    if(keyval<VK_NUM_0-1)
    {
        if(keyval!=VK_DECIMAL_POINT)
            return false;
    }
    if(keyval>VK_NUM_9+1)
        return false;
    return ret;
}
// =============================================================================
// ��������: �ı���ؼ����ƺ���
// �������: pMsg,������Ϣָ��
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
static  bool_t TextBox_Paint(struct WindowMsg *pMsg)
{
    HWND hwnd;
    HDC hdc;
    RECT rc;
    u8 linecount;
    char *str;
    u16 count;
    u32 flag;
    TextBox *pTB;
    bool_t ret;
    if(pMsg==NULL)
        return false;
    hwnd =pMsg->hwnd;
    if(hwnd==NULL)
        return false;
    pTB=(TextBox *)GetWindowPrivateData(hwnd);
    if(pTB==NULL)
        return false;
    hdc =BeginPaint(hwnd);
    if(NULL!=hdc)
    {
        //����TextBoxֻ����ʾ������Ϣ,������ж�һ���ַ����Ƿ��������,��Ϊ����,��
        //ֻ��ʾ��һ����Ϣ,��������Ϣֱ�Ӻ���.
        if(!pTB->IsMultiLines)
        {
            str=hwnd->Text;
            ret=__GetValidStrInfo(str,&count,NULL);
            if(!ret)
                return false;
       }
       GetClientRect(hwnd,&rc);
       SetFillColor(hdc,RGB(255,0,0));
       FillRect(hdc,&rc);
       if(hwnd->Style&WS_BORDER)
       {
          if(hwnd->Style&BORDER_FIXED3D)
          {
             SetDrawColor(hdc,RGB(173,173,173));
             DrawLine(hdc,0,0,0,RectH(&rc)-1); //L
             SetDrawColor(hdc,RGB(234,234,234));
             DrawLine(hdc,0,0,RectW(&rc)-1,0);   //U
             DrawLine(hdc,RectW(&rc)-1,0,RectW(&rc)-1,RectH(&rc)-1); //R
             DrawLine(hdc,0,RectH(&rc)-1,RectW(&rc)-1,RectH(&rc)-1); //D
          }
          else
          {
             SetDrawColor(hdc,RGB(169,169,169));
             DrawLine(hdc,0,0,0,RectH(&rc)-1); //L
             DrawLine(hdc,0,0,RectW(&rc)-1,0);   //U
             DrawLine(hdc,RectW(&rc)-1,0,RectW(&rc)-1,RectH(&rc)-1); //R
             DrawLine(hdc,0,RectH(&rc)-1,RectW(&rc)-1,RectH(&rc)-1); //D
          }
        }

        SetTextColor(hdc,RGB(1,1,1));
        DrawText(hdc,str,count,&rc,DT_VCENTER|DT_LEFT);
        EndPaint(hwnd,hdc);
      }
    return true;
}

// =============================================================================
// ��������: TextBox�ؼ�KEY_DOWN_MSG��Ϣ��Ӧ����
// �������: pMsg,������Ϣ�ṹ��ָ��
// �������: �ޡ�
// ����ֵ  :�ɹ�����true,ʧ���򷵻�false��
// =============================================================================
static bool_t TextBox_KeyDown(struct WindowMsg *pMsg)
{
    HWND hwnd;
    u8 cursorloc,chnum,chnummax,keyval;
    TextBox *pTB;
    char tmpbuf[2];
    bool_t ret;
    struct FontRsc* cur_font;
    struct Charset* cur_enc;
    u32 wc;
    s32 chwidth,width=0;
    u32 ch;
    char *str;
    RECT rc;
    if(pMsg==NULL)
        return false;
    hwnd =pMsg->hwnd;
    if(hwnd==NULL)
        return false;
    ret=IsFocusWindow(hwnd);
    if(!ret)
        return false;
    keyval=(u8)pMsg->Param1;
    tmpbuf[0]=(char)keyval;
    tmpbuf[1]='\0';
    pTB=(TextBox *)GetWindowPrivateData(hwnd);
    cursorloc=pTB->CursorLoc;
    chnum=pTB->ChNum;
    chnummax=pTB->CharNumLimit;
    ret=__IsValidInputKey( keyval);
    if(ret)
    {
         if(chnum!=chnummax)
         {
            cur_font = Font_GetCurFont();
            cur_enc = Charset_NlsGetCurCharset();
            cur_enc->MbToUcs4(&wc, tmpbuf, -1);
            chwidth=cur_font->GetCharWidth(wc);
            GetClientRect(hwnd,&rc);
            width=RectW(&rc);
            if(width>=pTB->CharWidthSum+chwidth)
            {
                if(chnum!=cursorloc)
                {
                    TextBox_InsertChar(hwnd, cursorloc, tmpbuf);
                }
                else
                {
                    TextBox_AddChar(hwnd, tmpbuf);
                }
                cur_font = Font_GetCurFont();
                cur_enc = Charset_NlsGetCurCharset();
                cur_enc->MbToUcs4(&wc, tmpbuf, -1);
                chwidth=cur_font->GetCharWidth(wc);
                pTB->ChNum++;
                pTB->CursorLoc++;
                pTB->CharWidthSum+=chwidth;
                InvalidateWindow( hwnd, true);
                __MoveCursor(hwnd,pTB->CursorLoc);
             }
         }
    }
    else
    {
         switch (keyval)
         {
               case VK_RETURN:
                //todo
                break;
               case VK_DEL:
               case CN_CANCLE_KEY:
                   if(pTB->ChNum>=1)
                   {
                      if(cursorloc!=0)
                      {
                         str=hwnd->Text;
                         ch=__GetCharByIndex(str,pTB->CursorLoc);
                         cur_font = Font_GetCurFont();
                         chwidth=cur_font->GetCharWidth(ch);
                         TextBox_DeleteChar(hwnd, cursorloc,1);
                         pTB->ChNum--;
                         pTB->CursorLoc--;
                         pTB->CharWidthSum-=chwidth;
                      }
                      InvalidateWindow( hwnd, true);
                      __MoveCursor(hwnd,pTB->CursorLoc);
                   }
                break;

           default:
            break;

         }
    }

    SetWindowPrivateData( hwnd, (void *)pTB);
    return true;
}

// =============================================================================
// ��������: TextBox�ؼ�KEY_UP_MSG��Ϣ��Ӧ����
// �������: pMsg,������Ϣ�ṹ��ָ��
// �������: �ޡ�
// ����ֵ  :�ޡ�
// =============================================================================
static bool_t TextBox_KeyUp(struct WindowMsg *pMsg)
{

}
// =============================================================================
// ��������: TextBox�ؼ�KEY_PRESS_MAG��Ϣ���ƺ���
// �������: pMsg,������Ϣ�ṹ��ָ��
// �������: �ޡ�
// ����ֵ  :�ޡ�
// =============================================================================
static bool_t TextBox_KeyPress(struct WindowMsg *pMsg)
{
    return true;
}



static bool_t TextBoxL_Down(struct WindowMsg *pMsg)
{
    return true;
}

static bool_t TextBoxL_Up(struct WindowMsg *pMsg)
{
   return true;
}


static void TextBox_TouchUp(struct WindowMsg *pMsg)
{

}

// =============================================================================
// ��������: TextBox�ؼ�������������Ӧ������
// �������: pMsg,������Ϣ�ṹ��ָ��
// �������: �ޡ�
// ����ֵ  :�ɹ�����true,ʧ���򷵻�false��
// =============================================================================
static bool_t TextBox_TouchDown(struct WindowMsg *pMsg)
{
    HWND hwnd;
    u32 loc;;
    u16 chnum,idx,CharWidth,x,y;
    TextBox *pTB;
    char *str;
    s32 tmp;
    RECT rc;
    bool_t ret;
    if(pMsg==NULL)
        return false;
    hwnd =pMsg->hwnd;
    if(hwnd==NULL)
        return false;
    pTB=(TextBox *)GetWindowPrivateData(hwnd);
    if(pTB==NULL)
        return false;
    //����ǰTextBox�ؼ�����Ϊ���㴰��
    SetFocusWindow(hwnd);
    //���ı�����
    loc=pMsg->Param2;   //��ȡ��������Ϣ
    x=loc;
    y=loc>>16;
    chnum=pTB->ChNum;
    str=hwnd->Text;
    if(str==NULL)
    {
         pTB->CursorLoc=0;
        __MoveCursor(hwnd,0);
    }
    else
    {
        ret=__GetValidStrInfo(str,NULL,&CharWidth);
        GetWindowRect(hwnd,&rc);
        tmp=rc.left+CharWidth;
        if(x>tmp)
        {
            pTB->CursorLoc=chnum;
            __MoveCursor(hwnd,chnum);
        }
        else
        {
           idx=__FindIdx(hwnd,x,y);
           pTB->CursorLoc=idx;
           __MoveCursor(hwnd,idx);
        }
    }
    SetWindowPrivateData(hwnd,(void *)pTB);
    return true;
}

// =============================================================================
// ��������: TextBox�ؼ���ý�����Ϣ��Ӧ������
// �������: pMsg,������Ϣָ��
// �������: �ޡ�
// ����ֵ  :�ɹ�����true,ʧ���򷵻�false��
// ======================================================================
static bool_t TextBox_SetFocus(struct WindowMsg *pMsg)
{
    HWND hwnd,Tmrhwnd;
    if(pMsg==NULL)
        return false;
    hwnd =pMsg->hwnd;
    if(hwnd==NULL)
        return false;
    Tmrhwnd=GetDesktopWindow();
    if(Tmrhwnd!=NULL)
    {
       PostMessage(Tmrhwnd,MSG_TIMER_START,CN_CURSOR_TIMER_ID,(ptu32_t)hwnd);
    }
    return true;
}
// =============================================================================
// ��������: TextBox�ؼ�ʧȥ������Ϣ��Ӧ������
// �������: pMsg,������Ϣָ��
// �������: �ޡ�
// ����ֵ  :�ɹ�����true,ʧ���򷵻�false��
// ======================================================================
static bool_t TextBox_KillFocus(struct WindowMsg *pMsg)
{
     HWND hwnd,Tmrhwnd;
     if(pMsg==NULL)
          return false;
     hwnd =pMsg->hwnd;
     if(hwnd==NULL)
          return false;
     Tmrhwnd=GetDesktopWindow();
     if(Tmrhwnd!=NULL)
     {
         PostMessage(Tmrhwnd,MSG_TIMER_STOP,CN_CURSOR_TIMER_ID,(ptu32_t)hwnd);
     }
     return true;
}

// =============================================================================
// ��������: TextBox�ؼ���ȡ�ı����ݺ�����
// �������: hwnd,�ı��򴰿ھ��.
// �������: �ޡ�
// ����ֵ  :�ޡ�
// =============================================================================
static void __TextBox_GetText(HWND hwnd,char *text)
{
     TextBox *pTB;
     u16 len;
     char *str;
     pTB=GetWindowPrivateData(hwnd);
     if(pTB==NULL)
         return ;
     str=hwnd->Text;
     len=strlen(str);
     memcpy(text,str,len);
     *(text+len)='\0';
}

// =============================================================================
// ��������: TextBox�ؼ������ı�Text������
// �������: hwnd,�ı��򴰿ھ��.
// �������: �ޡ�
// ����ֵ  :�ޡ�
// =============================================================================
static void __TextBox_SetText(HWND hwnd,char *str)
{
     u16 len;
     char *dst;
     dst=hwnd->Text;
     len=strlen(str);
     memcpy(dst,str,len);
     *(dst+len)='\0';
     InvalidateWindow(hwnd, true);
}

// =============================================================================
// ��������: TextBox�ؼ�ɾ���ı�������
// �������: hwnd,�ı��򴰿ھ��.
// �������: �ޡ�
// ����ֵ  :�ޡ�
// =============================================================================
static void __TextBox_DeleteText(HWND hwnd)
{
     hwnd->Text='\0';
     InvalidateWindow(hwnd, true);
}
// =============================================================================
// ��������: TextBox�ؼ���ʾ���ݿ��ƺ���
// �������: hwnd,TextBox�ؼ����ھ��;
//          ctrlcmd,����μ�enum TextCtrlCmd
//          para1:����EN_GET_TEXT��EN_SET_TEXT��EN_DELETE_TEXT�ò�����Ч,��ֱ����0��
//          para2:����EN_DELETE_TEXT�ò�����Ч,��ֱ����0,����EN_GET_TEXT��EN_SET_TEXT
//          �����ַ���ָ��.
// ����ֵ  :�ɹ��򷵻�true��ʧ���򷵻�false.
// =============================================================================
bool_t TextBox_TextCtrl(HWND hwnd,u8 ctrlcmd,u32 para1,ptu32_t para2)
{
    if(hwnd==NULL)
        return false;
    switch(ctrlcmd)
    {
        case EN_GET_TEXT:
            __TextBox_GetText(hwnd,(char *)para2);
            break;
        case EN_SET_TEXT:
            __TextBox_SetText(hwnd,(char *)para2);
            break;
        case EN_DELETE_TEXT:
            __TextBox_DeleteText(hwnd);
            break;
        default:
            break;
    }
    return true;
}


//Ĭ�ϰ�ť��Ϣ�����������������û���������û�д�������Ϣ��
static struct MsgProcTable s_gTextBoxMsgProcTable[] =
{
    {MSG_KEY_DOWN,TextBox_KeyDown},
    {MSG_KEY_UP,TextBox_KeyUp},
    {MSG_PAINT,TextBox_Paint},
    {MSG_CREATE,TextBox_Create},
    {MSG_TOUCH_DOWN,TextBox_TouchDown},
    {MSG_TOUCH_UP,TextBox_TouchUp},
    {MSG_SETFOCUS,TextBox_SetFocus},
    {MSG_KILLFOCUS,TextBox_KillFocus}
};

static struct MsgTableLink  s_gTextBoxMsgLink;
// =============================================================================
// ��������: TextBox�ؼ�����������
// �������: Text:�ı��򴰿�Text;
//           Style:�ı����񣬲μ�gdd.h;
//           x:�ı�����ʼλ��x��������(��λ������);
//           y:�ı�����ʼλ��y��������(��λ������);
//           w:�ı������(��λ������);
//           h:�ı���߶�(��λ������);
//           hParent:�ı��򸸴��ھ��;
//           WinId:�ı���ؼ�Id;
//           pdata:�ı���ؼ�˽�����ݽṹ;
//           UserMsgTableLink:�ı���ؼ��û���Ϣ�б��ṹָ�롣
// �������: �ޡ�
// ����ֵ  :�ɹ��򷵻��ı�������ʧ���򷵻�NULL��
// =============================================================================
HWND CreateTextBox(const char *Text,u32 Style,
                    s32 x,s32 y,s32 w,s32 h,
                    HWND hParent,u32 WinId,void *pdata,
                    struct MsgTableLink *UserMsgTableLink)
{
    WINDOW *pGddWin=NULL;
    struct MsgTableLink *Current;
    if(UserMsgTableLink != NULL)
    {
        Current = UserMsgTableLink;
        while(Current->LinkNext != NULL)
            Current = Current->LinkNext;
        Current->LinkNext = &s_gTextBoxMsgLink;
        Current = UserMsgTableLink;
    }
    else
        Current = &s_gTextBoxMsgLink;
    s_gTextBoxMsgLink.LinkNext = NULL;
    s_gTextBoxMsgLink.MsgNum = sizeof(s_gTextBoxMsgProcTable) / sizeof(struct MsgProcTable);
    s_gTextBoxMsgLink.myTable = (struct MsgProcTable *)&s_gTextBoxMsgProcTable;
    pGddWin=CreateWindow(Text,WS_CHILD|Style|TEXTBOX,x,y,w,h,hParent,WinId,CN_WINBUF_PARENT,pdata,Current);
    return pGddWin;
}

