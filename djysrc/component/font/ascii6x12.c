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

//所属模块: ASCII 6x12点阵字体驱动
//作者：mjh
//版本：V1.0.0
//文件描述: ASCII 6x12点阵字体解析
//其他说明:
//修订历史:
//    2. ...
//    1. 日期:2011-05-17
//       作者:mjh
//       新版本号：
//       修改说明:初始版本
//------------------------------------------------------
#include "stdint.h"
#include "stddef.h"
#include "stdio.h"
#include "gkernel.h"
#include "font.h"

// 6x12点阵ASCII字符库
const u8 cs_ascii_6x12[12*256] =
{
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x00
    0x00,0x7E,0x81,0xA5,0x81, 0x81,0xBD,0x99,0x81, 0x81,0x7E,0x00,        // 0x01
    0x00,0x7E,0xFF,0xDB,0xFF, 0xFF,0xC3,0xE7,0xFF, 0xFF,0x7E,0x00,        // 0x02
    0x00,0x00,0x00,0x6C,0xFE, 0xFE,0xFE,0xFE,0x7C, 0x38,0x10,0x00,        // 0x03
    0x00,0x00,0x00,0x10,0x38, 0x7C,0xFE,0x7C,0x38, 0x10,0x00,0x00,        // 0x04
    0x00,0x00,0x18,0x3C,0x3C, 0xE7,0xE7,0xE7,0x18, 0x18,0x3C,0x00,        // 0x05
    0x00,0x00,0x18,0x3C,0x7E, 0xFF,0xFF,0x7E,0x18, 0x18,0x3C,0x00,        // 0x06
    0x00,0x00,0x00,0x00,0x00, 0x18,0x3C,0x3C,0x18, 0x00,0x00,0x00,        // 0x07
    0xFF,0xFF,0xFF,0xFF, 0xE7,0xC3,0xC3,0xE7, 0xFF,0xFF,0xFF,0xFF,        // 0x08
    0x00,0x00,0x00,0x00,0x3C, 0x66,0x42,0x42,0x66, 0x3C,0x00,0x00,        // 0x09
    0xFF,0xFF,0xFF,0xC3, 0x99,0xBD,0xBD,0x99, 0xC3,0xFF,0xFF,0xFF,        // 0x0A
    0x00,0x1E,0x0E,0x1A,0x32, 0x78,0xCC,0xCC,0xCC, 0xCC,0x78,0x00,        // 0x0B
    0x00,0x3C,0x66,0x66,0x66, 0x66,0x3C,0x18,0x7E, 0x18,0x18,0x00,        // 0x0C
    0x00,0x3F,0x33,0x3F,0x30, 0x30,0x30,0x30,0x70, 0xF0,0xE0,0x00,        // 0x0D
    0x00,0x7F,0x63,0x7F,0x63, 0x63,0x63,0x63,0x67, 0xE7,0xE6,0xC0,        // 0x0E
    0x00,0x00,0x18,0x18,0xDB, 0x3C,0xE7,0x3C,0xDB, 0x18,0x18,0x00,        // 0x0F
    0x00,0xC0,0xE0,0xF0,0xF8, 0xFE,0xF8,0xF0,0xE0, 0xC0,0x80,0x00,        // 0x10
    0x00,0x06,0x0E,0x1E,0x3E, 0xFE,0x3E,0x1E,0x0E, 0x06,0x02,0x00,        // 0x11
    0x00,0x18,0x3C,0x7E,0x18, 0x18,0x18,0x7E,0x3C, 0x18,0x00,0x00,        // 0x12
    0x00,0x66,0x66,0x66,0x66, 0x66,0x66,0x66,0x00, 0x66,0x66,0x00,        // 0x13
    0x00,0x7F,0xDB,0xDB,0xDB, 0x7B,0x1B,0x1B,0x1B, 0x1B,0x1B,0x00,        // 0x14
    0x00,0xC6,0x60,0x38,0x6C, 0xC6,0xC6,0x6C,0x38, 0x0C,0xC6,0x7C,        // 0x15
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0xFE,0xFE, 0xFE,0xFE,0x00,        // 0x16
    0x00,0x18,0x3C,0x7E,0x18, 0x18,0x18,0x7E,0x3C, 0x18,0x7E,0x00,        // 0x17
    0x00,0x18,0x3C,0x7E,0x18, 0x18,0x18,0x18,0x18, 0x18,0x18,0x00,        // 0x18
    0x00,0x18,0x18,0x18,0x18, 0x18,0x18,0x18,0x7E, 0x3C,0x18,0x00,        // 0x19
    0x00,0x00,0x00,0x00,0x18, 0x0C,0xFE,0x0C,0x18, 0x00,0x00,0x00,        // 0x1A
    0x00,0x00,0x00,0x00,0x30, 0x60,0xFE,0x60,0x30, 0x00,0x00,0x00,        // 0x1B
    0x00,0x00,0x00,0x00,0x00, 0xC0,0xC0,0xC0,0xFE, 0x00,0x00,0x00,        // 0x1C
    0x00,0x00,0x00,0x00,0x28, 0x6C,0xFE,0x6C,0x28, 0x00,0x00,0x00,        // 0x1D
    0x00,0x00,0x00,0x10,0x38, 0x38,0x7C,0x7C,0xFE, 0xFE,0x00,0x00,        // 0x1E
    0x00,0x00,0x00,0xFE,0xFE, 0x7C,0x7C,0x38,0x38, 0x10,0x00,0x00,        // 0x1F
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x20
    0x00,0x00,0x10,0x10,0x10, 0x10,0x10,0x00,0x00, 0x10,0x00,0x00,        // 0x21
    0x00,0x00,0x6C,0x48,0x48, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x22
    0x00,0x00,0x14,0x14,0x28, 0x7C,0x28,0x7C,0x28, 0x50,0x50,0x00,        // 0x23
    0x00,0x00,0x10,0x38,0x40, 0x40,0x38,0x48,0x70, 0x10,0x10,0x00,        // 0x24
    0x00,0x00,0x20,0x50,0x20, 0x0C,0x70,0x08,0x14, 0x08,0x00,0x00,        // 0x25
    0x00,0x00,0x00,0x00,0x18, 0x20,0x20,0x54,0x48, 0x34,0x00,0x00,        // 0x26
    0x00,0x00,0x10,0x10,0x10, 0x10,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x27
    0x00,0x00,0x08,0x08,0x10, 0x10,0x10,0x10,0x10, 0x10,0x08,0x08,        // 0x28
    0x00,0x00,0x20,0x20,0x10, 0x10,0x10,0x10,0x10, 0x10,0x20,0x20,        // 0x29
    0x00,0x00,0x10,0x7C,0x10, 0x28,0x28,0x00,0x00, 0x00,0x00,0x00,        // 0x2A
    0x00,0x00,0x00,0x10,0x10, 0x10,0xFC,0x10,0x10, 0x10,0x00,0x00,        // 0x2B
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x18, 0x10,0x30,0x20,        // 0x2C
    0x00,0x00,0x00,0x00,0x00, 0x00,0x7C,0x00,0x00, 0x00,0x00,0x00,        // 0x2D
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x30, 0x30,0x00,0x00,        // 0x2E
    0x00,0x00,0x04,0x04,0x08, 0x08,0x10,0x10,0x20, 0x20,0x40,0x00,        // 0x2F
    0x00,0x00,0x38,0x44,0x44, 0x44,0x44,0x44,0x44, 0x38,0x00,0x00,        // 0x30
    0x00,0x00,0x30,0x10,0x10, 0x10,0x10,0x10,0x10, 0x7C,0x00,0x00,        // 0x31
    0x00,0x00,0x38,0x44,0x04, 0x08,0x10,0x20,0x44, 0x7C,0x00,0x00,        // 0x32
    0x00,0x00,0x38,0x44,0x04, 0x18,0x04,0x04,0x44, 0x38,0x00,0x00,        // 0x33
    0x00,0x00,0x0C,0x14,0x14, 0x24,0x44,0x7C,0x04, 0x0C,0x00,0x00,        // 0x34
    0x00,0x00,0x3C,0x20,0x20, 0x38,0x04,0x04,0x44, 0x38,0x00,0x00,        // 0x35
    0x00,0x00,0x1C,0x20,0x40, 0x78,0x44,0x44,0x44, 0x38,0x00,0x00,        // 0x36
    0x00,0x00,0x7C,0x44,0x04, 0x08,0x08,0x08,0x10, 0x10,0x00,0x00,        // 0x37
    0x00,0x00,0x38,0x44,0x44, 0x38,0x44,0x44,0x44, 0x38,0x00,0x00,        // 0x38
    0x00,0x00,0x38,0x44,0x44, 0x44,0x3C,0x04,0x08, 0x70,0x00,0x00,        // 0x39
    0x00,0x00,0x00,0x00,0x30, 0x30,0x00,0x00,0x30, 0x30,0x00,0x00,        // 0x3A
    0x00,0x00,0x00,0x00,0x18, 0x18,0x00,0x00,0x18, 0x30,0x20,0x00,        // 0x3B
    0x00,0x00,0x00,0x0C,0x10, 0x60,0x80,0x60,0x10, 0x0C,0x00,0x00,        // 0x3C
    0x00,0x00,0x00,0x00,0x00, 0x7C,0x00,0x7C,0x00, 0x00,0x00,0x00,        // 0x3D
    0x00,0x00,0x00,0xC0,0x20, 0x18,0x04,0x18,0x20, 0xC0,0x00,0x00,        // 0x3E
    0x00,0x00,0x00,0x18,0x24, 0x04,0x08,0x10,0x00, 0x30,0x00,0x00,        // 0x3F
    0x00,0x38,0x44,0x44,0x4C, 0x54,0x54,0x4C,0x40, 0x44,0x38,0x00,        // 0x40
    0x00,0x00,0x30,0x10,0x28, 0x28,0x28,0x7C,0x44, 0xEC,0x00,0x00,        // 0x41
    0x00,0x00,0xF8,0x44,0x44, 0x78,0x44,0x44,0x44, 0xF8,0x00,0x00,        // 0x42
    0x00,0x00,0x3C,0x44,0x40, 0x40,0x40,0x40,0x44, 0x38,0x00,0x00,        // 0x43
    0x00,0x00,0xF0,0x48,0x44, 0x44,0x44,0x44,0x48, 0xF0,0x00,0x00,        // 0x44
    0x00,0x00,0xFC,0x44,0x50, 0x70,0x50,0x40,0x44, 0xFC,0x00,0x00,        // 0x45
    0x00,0x00,0x7C,0x20,0x28, 0x38,0x28,0x20,0x20, 0x70,0x00,0x00,        // 0x46
    0x00,0x00,0x3C,0x44,0x40, 0x40,0x4C,0x44,0x44, 0x38,0x00,0x00,        // 0x47
    0x00,0x00,0xEC,0x44,0x44, 0x7C,0x44,0x44,0x44, 0xEC,0x00,0x00,        // 0x48
    0x00,0x00,0x7C,0x10,0x10, 0x10,0x10,0x10,0x10, 0x7C,0x00,0x00,        // 0x49
    0x00,0x00,0x3C,0x08,0x08, 0x08,0x48,0x48,0x48, 0x30,0x00,0x00,        // 0x4A
    0x00,0x00,0xEC,0x44,0x48, 0x50,0x70,0x48,0x44, 0xE4,0x00,0x00,        // 0x4B
    0x00,0x00,0x70,0x20,0x20, 0x20,0x20,0x24,0x24, 0x7C,0x00,0x00,        // 0x4C
    0x00,0x00,0xEC,0x6C,0x6C, 0x54,0x54,0x44,0x44, 0xEC,0x00,0x00,        // 0x4D
    0x00,0x00,0xEC,0x64,0x64, 0x54,0x54,0x54,0x4C, 0xEC,0x00,0x00,        // 0x4E
    0x00,0x00,0x38,0x44,0x44, 0x44,0x44,0x44,0x44, 0x38,0x00,0x00,        // 0x4F
    0x00,0x00,0x78,0x24,0x24, 0x24,0x38,0x20,0x20, 0x70,0x00,0x00,        // 0x50
    0x00,0x00,0x38,0x44,0x44, 0x44,0x44,0x44,0x44, 0x38,0x1C,0x00,        // 0x51
    0x00,0x00,0xF8,0x44,0x44, 0x44,0x78,0x48,0x44, 0xE0,0x00,0x00,        // 0x52
    0x00,0x00,0x34,0x4C,0x40, 0x38,0x04,0x04,0x64, 0x58,0x00,0x00,        // 0x53
    0x00,0x00,0xFC,0x90,0x10, 0x10,0x10,0x10,0x10, 0x38,0x00,0x00,        // 0x54
    0x00,0x00,0xEC,0x44,0x44, 0x44,0x44,0x44,0x44, 0x38,0x00,0x00,        // 0x55
    0x00,0x00,0xEC,0x44,0x44, 0x28,0x28,0x28,0x10, 0x10,0x00,0x00,        // 0x56
    0x00,0x00,0xEC,0x44,0x44, 0x54,0x54,0x54,0x54, 0x28,0x00,0x00,        // 0x57
    0x00,0x00,0xC4,0x44,0x28, 0x10,0x10,0x28,0x44, 0xC4,0x00,0x00,        // 0x58
    0x00,0x00,0xEC,0x44,0x28, 0x28,0x10,0x10,0x10, 0x38,0x00,0x00,        // 0x59
    0x00,0x00,0x7C,0x44,0x08, 0x10,0x10,0x20,0x44, 0x7C,0x00,0x00,        // 0x5A
    0x00,0x00,0x38,0x20,0x20, 0x20,0x20,0x20,0x20, 0x20,0x20,0x38,        // 0x5B
    0x00,0x00,0x40,0x20,0x20, 0x20,0x10,0x10,0x08, 0x08,0x08,0x00,        // 0x5C
    0x00,0x00,0x38,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x38,        // 0x5D
    0x00,0x00,0x10,0x10,0x28, 0x44,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x5E
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xFC,        // 0x5F
    0x00,0x00,0x10,0x08,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x60
    0x00,0x00,0x00,0x00,0x38, 0x44,0x3C,0x44,0x44, 0x3C,0x00,0x00,        // 0x61
    0x00,0x00,0xC0,0x40,0x58, 0x64,0x44,0x44,0x44, 0xF8,0x00,0x00,        // 0x62
    0x00,0x00,0x00,0x00,0x3C, 0x44,0x40,0x40,0x44, 0x38,0x00,0x00,        // 0x63
    0x00,0x00,0x0C,0x04,0x34, 0x4C,0x44,0x44,0x44, 0x3C,0x00,0x00,        // 0x64
    0x00,0x00,0x00,0x00,0x38, 0x44,0x7C,0x40,0x40, 0x3C,0x00,0x00,        // 0x65
    0x00,0x00,0x1C,0x20,0x7C, 0x20,0x20,0x20,0x20, 0x7C,0x00,0x00,        // 0x66
    0x00,0x00,0x00,0x00,0x34, 0x4C,0x44,0x44,0x44, 0x3C,0x04,0x38,        // 0x67
    0x00,0x00,0xC0,0x40,0x58, 0x64,0x44,0x44,0x44, 0xEC,0x00,0x00,        // 0x68
    0x00,0x00,0x10,0x00,0x70, 0x10,0x10,0x10,0x10, 0x7C,0x00,0x00,        // 0x69
    0x00,0x00,0x10,0x00,0x78, 0x08,0x08,0x08,0x08, 0x08,0x08,0x70,        // 0x6A
    0x00,0x00,0xC0,0x40,0x5C, 0x48,0x70,0x50,0x48, 0xDC,0x00,0x00,        // 0x6B
    0x00,0x00,0x30,0x10,0x10, 0x10,0x10,0x10,0x10, 0x7C,0x00,0x00,        // 0x6C
    0x00,0x00,0x00,0x00,0xE8, 0x54,0x54,0x54,0x54, 0xFC,0x00,0x00,        // 0x6D
    0x00,0x00,0x00,0x00,0xD8, 0x64,0x44,0x44,0x44, 0xEC,0x00,0x00,        // 0x6E
    0x00,0x00,0x00,0x00,0x38, 0x44,0x44,0x44,0x44, 0x38,0x00,0x00,        // 0x6F
    0x00,0x00,0x00,0x00,0xD8, 0x64,0x44,0x44,0x44, 0x78,0x40,0xE0,        // 0x70
    0x00,0x00,0x00,0x00,0x34, 0x4C,0x44,0x44,0x44, 0x3C,0x04,0x0C,        // 0x71
    0x00,0x00,0x00,0x00,0x6C, 0x30,0x20,0x20,0x20, 0x7C,0x00,0x00,        // 0x72
    0x00,0x00,0x00,0x00,0x3C, 0x44,0x38,0x04,0x44, 0x78,0x00,0x00,        // 0x73
    0x00,0x00,0x00,0x20,0x7C, 0x20,0x20,0x20,0x20, 0x1C,0x00,0x00,        // 0x74
    0x00,0x00,0x00,0x00,0xCC, 0x44,0x44,0x44,0x4C, 0x34,0x00,0x00,        // 0x75
    0x00,0x00,0x00,0x00,0xEC, 0x44,0x44,0x28,0x28, 0x10,0x00,0x00,        // 0x76
    0x00,0x00,0x00,0x00,0xEC, 0x44,0x54,0x54,0x54, 0x28,0x00,0x00,        // 0x77
    0x00,0x00,0x00,0x00,0xCC, 0x48,0x30,0x30,0x48, 0xCC,0x00,0x00,        // 0x78
    0x00,0x00,0x00,0x00,0xEC, 0x44,0x24,0x28,0x18, 0x10,0x10,0x78,        // 0x79
    0x00,0x00,0x00,0x00,0x7C, 0x48,0x10,0x20,0x44, 0x7C,0x00,0x00,        // 0x7A
    0x00,0x00,0x08,0x10,0x10, 0x10,0x10,0x20,0x10, 0x10,0x10,0x08,        // 0x7B
    0x00,0x00,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x10,0x10,0x00,        // 0x7C
    0x00,0x00,0x20,0x10,0x10, 0x10,0x10,0x08,0x10, 0x10,0x10,0x20,        // 0x7D
    0x00,0x00,0x00,0x00,0x00, 0x00,0x24,0x58,0x00, 0x00,0x00,0x00,        // 0x7E
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x7F
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x80
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x81
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x82
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x83
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x84
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x85
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x86
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x87
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x88
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x89
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x8A
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x8B
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x8C
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x8D
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x8E
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x8F
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x90
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x91
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x92
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x93
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x94
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x95
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x96
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x97
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x98
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x99
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x9A
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x9B
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x9C
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x9D
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x9E
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0x9F
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA0
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA1
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA2
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA3
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA4
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA5
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA6
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA7
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA8
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xA9
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xAA
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xAB
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xAC
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xAD
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xAE
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xAF
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB0
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB1
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB2
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB3
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB4
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB5
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB6
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB7
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB8
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xB9
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xBA
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xBB
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xBC
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xBD
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xBE
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xBF
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC0
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC1
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC2
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC3
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC4
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC5
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC6
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC7
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC8
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xC9
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xCA
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xCB
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xCC
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xCD
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xCE
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xCF
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD0
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD1
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD2
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD3
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD4
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD5
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD6
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD7
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD8
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xD9
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xDA
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xDB
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xDC
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xDD
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xDE
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xDF
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE0
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE1
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE2
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE3
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE4
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE5
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE6
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE7
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE8
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xE9
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xEA
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xEB
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xEC
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xED
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xEE
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xEF
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF0
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF1
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF2
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF3
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF4
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF5
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF6
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF7
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF8
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xF9
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xFA
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xFB
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xFC
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xFD
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xFE
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,        // 0xFF
};

#include "gkernel.h"
#include "font.h"
#include "ascii6x12.h"

bool_t __Font_Ascii6x12LoadFont(void *);
void __Font_Ascii6x12UnloadFont(void);
bool_t __Font_Ascii6x12GetCharBitMap(u32 charcode, u32 size,u32 resv,
                                    struct tagRectBitmap *bitmap);


bool_t __Font_Ascii6x12LoadFont(void * ziku)
{
    // Nothing ...

    return true;
}

void __Font_Ascii6x12UnloadFont(void)
{
    // Nothing ...
}

#define ASCII6x12_DEF_CHAR_CODE 0x00

//----点阵提取-----------------------------------------------------------------
//功能: 提取ascii 8*16点阵字体，如果charcode超出0~0xff的范围，显示编码为0x00的字
//      符，ascii包含扩展的ascii码
//参数: charcode，待显示的ascii码得ucs4编码
//      size，无效
//      resv，无效
//      bitmap，保存所提取的点阵的位图，缓冲区由调用者提供
//返回: true=正常，false=charcode不是该字体所支持的字符集范围，但此时仍然返回
//      默认字符的点阵
//-----------------------------------------------------------------------------
bool_t __Font_Ascii6x12GetCharBitMap(u32 charcode, u32 size,u32 resv,
                                    struct tagRectBitmap *bitmap)
{
    u8 i;
    u32 offset;
    bool_t result = true;


    // 得到字符字模在数组中的偏移量
    if(charcode > 0xff)
    {
        offset = ASCII6x12_DEF_CHAR_CODE;
        result = false;
    }else
        offset = charcode*16;
    bitmap->PixelFormat = CN_SYS_PF_GRAY1;
    bitmap->width = 6;
    bitmap->height = 12;
    bitmap->linebytes = 1;
    if(bitmap->bm_bits != NULL)
    {
        for(i=0; i<12; i++)
        {
            bitmap->bm_bits[i] = cs_ascii_6x12[offset + i];
        }
    }
    return result;
}

//----获取字符宽度-------------------------------------------------------------
//功能: 取某字符的点阵宽度
//参数: CharCode,被查询的字符
//返回: 字符宽度
//-----------------------------------------------------------------------------
s32 __Font_Ascii6x12GetCharWidth(u32 CharCode)
{
    return 6;
}

//----获取字符高度-------------------------------------------------------------
//功能: 取某字符的点阵高度
//参数: CharCode,被查询的字符
//返回: 字符高度
//-----------------------------------------------------------------------------
s32 __Font_Ascii6x12GetCharHeight(u32 CharCode)
{
    return 12;
}

//----安装ascii6x12字体--------------------------------------------------------
//功能: 安装ascii字体，当系统使用西方字符界面时，使用这个字符集。特别注意，
//      gb2312已经包含了英文字体，在使用中文的界面中可以不安装ascii字体。
//参数: 无意义
//返回: 1=成功，0=失败
//-----------------------------------------------------------------------------
ptu32_t ModuleInstall_FontAscii6x12Font(ptu32_t para)
{
    static struct tagFontRsc font_a6x12;

    font_a6x12.MaxWidth = 6;
    font_a6x12.MaxHeight = 12;
    font_a6x12.Attr = CN_FONT_TYPE_DOT;
    font_a6x12.LoadFont = __Font_Ascii6x12LoadFont;
    font_a6x12.UnloadFont = __Font_Ascii6x12UnloadFont;
    font_a6x12.GetBitmap = __Font_Ascii6x12GetCharBitMap;
    font_a6x12.GetCharWidth = __Font_Ascii6x12GetCharWidth;
    font_a6x12.GetCharHeight = __Font_Ascii6x12GetCharHeight;

    if(Font_InstallFont(&font_a6x12, CN_FONT_ASCII_6x12))
    {
        printf("ascii 6x12 font install sucess\n\r");
        return 1;
    }else
    {
        Djy_SaveLastError(EN_GK_FONT_INSTALL_ERROR);
        printf("ascii 6x12 font install fail\n\r");
        return 0;
    }
}

