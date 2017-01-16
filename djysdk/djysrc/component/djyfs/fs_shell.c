//-----------------------------------------------------------------------------
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <djyfs/file.h>
#include <djyfs/iofile.h>
#include <djyfs/mount.h>
#include <shell.h>
#include <driver.h>
#include <stdlib.h>

//
//
//
static bool_t SH_Format(const char *Param);
static bool_t SH_PrintWorkingPath(const char *Param);
static bool_t SH_MkDir(const char *Param);
static bool_t SH_List(const char *Param);
static bool_t SH_ChangeDir(const char *Param);
static bool_t SH_Remove(const char *Param);
static bool_t SH_Cat(const char *Param);
//
//
//
static struct ShellCmdRsc *s_pCmdRsc;
char *g_pWokingPath;
static struct ShellCmdTab const FsCmdTable[] =
{
    {
        "format",
        SH_Format,
        "�ļ�ϵͳ��ʽ��",
        "��ʽ: format ���ص�Ŀ¼(·��)"
    },
    {
        "pwd",
        SH_PrintWorkingPath,
        "��ʾ�û���ǰ����·��",
        "\r\n"
    },
    {
        "mkdir",
        SH_MkDir,
        "����Ŀ¼",
        "\r\n"
    },
    {
        "ls",
        SH_List,
        "��ʾĿ¼��",
        "\r\n"
    },
    {
        "cd",
        SH_ChangeDir,
        "���ĵ�ǰ����·��",
        "\r\n"
    },
    {
        "rm",
        SH_Remove,
        "ɾ���ļ�����Ŀ¼",
        "\r\n"
    },
    {
        "cat",
        SH_Cat,
        "���ļ�����ӡ",
        "\r\n"
    },
#if 0
    {
        "mount",
        Sh_Mount,
        "�����ļ�ϵͳ",
        "todo: "
    },
    {
        "fstell",
        SH_FsTell,
        "�Ѱ�װ�ļ�ϵͳ��Ϣ",
        "todo:"
    },
#endif
};

//-----------------------------------------------------------------------------
//����:
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
const char *s_cREG = "�ļ�";
const char *s_cLINK = "����";
const char *s_cDIR = "Ŀ¼";
const char *s_cUNKNOW = "δ֪";
static const char *PropertyToStr(u32 Mode)
{
    if(((S_ISREG(Mode)) && (S_ISLNK(Mode))) ||
        ((S_ISREG(Mode)) && (S_ISDIR(Mode))) ||
        ((S_ISDIR(Mode)) && (S_ISLNK(Mode))))
        return (s_cUNKNOW);

    if(S_ISREG(Mode))
        return (s_cREG);

    if(S_ISLNK(Mode))
        return (s_cLINK);

    if(S_ISDIR(Mode))
        return (s_cDIR);

    return (s_cUNKNOW);
}

//-----------------------------------------------------------------------------
//����: �ļ�ϵͳ��ʽ��
//����:
//����:
//��ע: todo: �߼�������
//-----------------------------------------------------------------------------
static bool_t SH_Format(const char *Param)
{
    if(NULL == Param)
        return (FALSE);

    printf("�ļ�ϵͳ��ʽ�� ----");
    if(Format(Param))
        printf("ʧ��!");
    else
        printf("�ɹ�!");

    printf("\r\n");
    return (TRUE);
}
//-----------------------------------------------------------------------------
//����: ��ʾ�û���ǰ����·��
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
static bool_t SH_PrintWorkingPath(const char *Param)
{
    s32 Len;
    bool_t Ret;
    char *PWD;

    Len = CurWorkPathLen();
    if(0 == Len)
    {
        printf("�û���ǰ����·��δ����\r\n");
        return (TRUE);
    }

    PWD = malloc(Len);
    if(NULL == PWD)
    {
        return (FALSE);
    }

    if(0 == CurWorkPath(PWD, Len))
    {
        printf("�û���ǰ����·����\"%s\"", PWD);
        Ret = TRUE;
    }
    else
    {
        printf("δ֪����\r\n");
        Ret = TRUE;
    }

    free(PWD);
    return (Ret);

}
//-----------------------------------------------------------------------------
//����: ����Ŀ¼
//����: -p -- ��ͼ����·�������в����ڵ�Ŀ¼��
//����:
//��ע:
//-----------------------------------------------------------------------------
static bool_t  SH_MkDir(const char *Param)
{
    char *Temp, *SlashPos, *Path, *FullPath;
    int Fd;
    u32 PathLen;
    u8 Changed;
    u8 JustLast = 0;

    if(NULL == Param)
        return (FALSE);

    Temp = strstr(Param, "-p ");
    if(Temp)
        Path = (char*)Temp + 3; // ȥ��"-p"������
    else
    {
        Path = (char*)Param;
        JustLast = 1;
    }

    while(*Path == ' ')
        Path = Path + 1; // ȥ������Ŀո��,�����ļ�ϵͳ·������,�ǲ������Կո�ʼ��

    FullPath = malloc(strlen(Path)+1);
    strcpy(FullPath, Path);
    if(JustLast)
    {
        Fd = open(FullPath, O_DIRECTORY | O_CREAT | O_EXCL |O_RDWR, 0);
        if(-1 != Fd)
            close(Fd);
    }
    else
    {
        // ��ȡ·����ÿ���ڵ�,����/aa/bb/cc��·�����ֱ���ȡ/aa��/aa/bb��/aa//bb/cc����ͼ����
        SlashPos = FullPath;
        do
        {
            while(*SlashPos == '/')
                SlashPos = SlashPos + 1; // ȥ������'/'

            while((*SlashPos != '/') && (*SlashPos != '\0'))
                SlashPos = SlashPos + 1; // ��ȡһ��segment

            PathLen = SlashPos - FullPath;
            if('/' == FullPath[PathLen])
            {
                FullPath[PathLen] = '\0';
                Changed = 1;
            }

            Fd = open(FullPath, O_DIRECTORY | O_CREAT | O_EXCL | O_RDWR, 0);
            if(-1 != Fd)
                close(Fd);

            if(Changed)
                FullPath[PathLen] = '/';
        }while(*SlashPos != '\0');
    }

    if(-1 == Fd)
        printf("����ʧ�ܻ�Ŀ¼�Ѵ���\r\n");

    free(FullPath);
    return (TRUE);
}
//-----------------------------------------------------------------------------
//����: ��ʾĿ¼��
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
static bool_t SH_List(const char *Param)
{
    DIR *Dir;
    s32 Len;
    char *CWP;
    struct dirent *DirentInfo;
    struct stat DirentStat;
    u8 Flags = 0;

    if(Param && strstr(Param, "-l"))
        Flags = 1; // ��ʾ������Ϣ

    Len = CurWorkPathLen();
    if(0 == Len)
    {
        printf("�û���ǰ����·��δ����\r\n");
        return (TRUE);
    }

    CWP = malloc(257);
    if(NULL == CWP)
    {
        printf("�ڴ治��\r\n");
        return (TRUE);
    }

    if(0 == CurWorkPath(CWP, Len))
    {
        Dir = opendir(CWP);
        if(Dir)
        {
            CWP[Len-1] = '/';
            while((DirentInfo = readdir(Dir)) != NULL)
            {
                if(1 == Flags)
                {
                    strcpy(CWP+Len, DirentInfo->d_name);
                    if(stat(CWP, &DirentStat))
                        printf("%s -- ��Ŀ¼���޷�����\r\n", DirentInfo->d_name);
                    else
                    {
                        printf("%s", PropertyToStr(DirentStat.st_mode));
                        printf("\t");
                        printf("%d", (u32)(DirentStat.st_size));// todo: 64λ�߼����ڴ�����
                        printf("\t");
                        printf("%s", DirentInfo->d_name);
                        printf("\r\n");
                    }
                }
                else
                {
                    printf("%s", DirentInfo->d_name);
                    printf("    ");
                }
            }

            if(0 == Flags)
                printf("\r\n");

            closedir(Dir);
        }
    }

    free(CWP);
    return (TRUE);
}
//-----------------------------------------------------------------------------
//����: ���ĵ�ǰ����·��
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
static bool_t SH_ChangeDir(const char *Param)
{
    s32 Ret;
    char *Path = (char *)Param;

    if(NULL == Path)
        return (FALSE);

    while(*Path == ' ')
        Path = Path + 1; // ȥ������Ŀո��

    Ret = SetPWD(Path);
    if(Ret)
    {
        printf("���󣺵�ǰ����·�����ʧ��");
        if(-1 == Ret)
            printf("-- Ŀ¼�����ڻ��޷�����");
        else if(-2 == Ret)
            printf("-- Ŀ¼�޷�����");
        printf("\r\n");
        return (TRUE);
    }

    return (TRUE);
}

//-----------------------------------------------------------------------------
//����:
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
bool_t Sh_PrintWorkPath(void)
{
    printf("%s", g_pWokingPath);
    return (TRUE);
}
//-----------------------------------------------------------------------------
//����: ɾ���ļ���Ŀ¼
//����: -r �ݹ�ɾ��(��ɾ��Ŀ¼��)
//����:
//��ע:
//-----------------------------------------------------------------------------
static bool_t SH_Remove(const char *Param)
{

    u16 Offset;
    char *PathParam, *Temp, *Path;
    s32 Ret = 0;
    u8 RemoveAll = 0;

    PathParam = (char*)Param;
    if(NULL == PathParam)
        return (FALSE);

    Temp = strstr(PathParam, "-r");
    if(Temp)
    {
        PathParam = (char*)Temp + 2; // ȥ��"-p"������
        RemoveAll = 1;
    }

    while(*PathParam == ' ')
        PathParam = PathParam + 1; // ȥ������Ŀո��

    Path = malloc(257);
    if(NULL == Path)
    {
        printf("�ڴ治��\r\n");
        return (TRUE);
    }

    memset(Path, 0, 257);
    Offset = strlen(PathParam);
    if(Offset < 257)
    {
        memcpy(Path, PathParam, Offset);
        if((RemoveAll) && ('/' !=Path[Offset-1]))
            Path[Offset++] = '/';
        Path[Offset] = '\0';

        if(RemoveAll)
        {
            struct stat Info;
            DIR *Dir;
            struct dirent *Dirent;

            Ret = stat(Path, &Info);
            if(Ret)
                printf("��ʽ����\r\n");

            if((!Ret) && S_ISDIR(Info.st_mode))
            {
                Dir = opendir(Path);
                if(Dir)
                {
                    while((Dirent = readdir(Dir)) != NULL)
                    {
                        strcpy(Path+Offset, Dirent->d_name);
                        Ret = remove(Path);
                        if(Ret)
                        {
                            printf("�ļ�\"%s\"�޷�ɾ��(�ļ�ʹ����)\r\n", Dirent->d_name);
                            break;
                        }
                    }
                    closedir(Dir);
                }
            }
        }

        if(!Ret)
        {
            Ret = remove(Path);
            if(Ret)
                printf("�ļ��޷�ɾ��(�ļ�ʹ����)\r\n");
        }
    }
    free(Path);
    return (TRUE);
}

//-----------------------------------------------------------------------------
//����: ���ļ�
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
static bool_t SH_Cat(const char *Param)
{
    char *Path = (char *)Param;
    int Fd;
    char Buf[64];
    s32 Ret;
    u8 i;

    if(NULL == Path)
        return (FALSE);

    while(*Path == ' ')
        Path = Path + 1; // ȥ������Ŀո��,�����ļ�ϵͳ·������,�ǲ������Կո�ʼ��


    Fd = open(Param, O_RDONLY, 0);
    if(NULL == Fd)
    {
        printf("�ļ��޷���");
        return (FALSE);
    }

    printf("16������ʾģʽ:\r\n");
    do{
        Ret = read(Fd, Buf, 64);
        for(i = 0; i < Ret; i++)
            printf("%02x", Buf[i]);
    }while(Ret == 64);

    close(Fd);

    return (TRUE);
}

//-----------------------------------------------------------------------------
//����: �ļ�ϵͳ���shell���װ
//����:
//����:
//��ע:
//-----------------------------------------------------------------------------
void FsShellInstall(void)
{
    u32 CmdNums;

    CmdNums = sizeof(FsCmdTable) / sizeof(struct ShellCmdTab);

    s_pCmdRsc = malloc(CmdNums * sizeof(*s_pCmdRsc));
    if(NULL == s_pCmdRsc)
        return ;

    Sh_InstallCmd(FsCmdTable, s_pCmdRsc, CmdNums);

    g_pWokingPath = malloc(257);
    if(NULL == g_pWokingPath)
        return ;

    memset(g_pWokingPath, '\0', 257);
    g_pWokingPath[0] = '/';

    Sh_PrintWorkPathSet(); // ������ǰ����·��
}