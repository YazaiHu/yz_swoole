/*================================================================
*   Copyright (C) 2015 All rights reserved.
*
*   文件名称：NetUtil.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年09月21日
*   描    述：
*
#pragma once
================================================================*/

#ifndef __NETUTIL_H__
#define __NETUTIL_H__

int getServiceIp(char** ppIp);
int getHostName(char** ppHostName);
unsigned int ip2long(const char* pIp);

#endif
