/*================================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   文件名称：netutil.c
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2015年09月21日
 *   描    述：
 *
 #include "netutil.h"
 ================================================================*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include "NetUtil.h"
#include "swoole.h"

unsigned int ip2long(const char* pIp)
{
    size_t nAddrLen = strlen(pIp);
#ifdef HAVE_INET_PTON
    struct in_addr ip;
#else
    unsigned long int ip;
#endif

#ifdef HAVE_INET_PTON
    if (nAddrLen == 0 || inet_pton(AF_INET, addr, &ip) != 1) {
        return 0;
    }
    return ntohl(ip.s_addr));
#else
    if (nAddrLen == 0 || (ip = inet_addr(pIp)) == INADDR_NONE) {
        /* The only special case when we should return -1 ourselves,
         * because inet_addr() considers it wrong. We return 0xFFFFFFFF and
         * not -1 or ~0 because of 32/64bit issues. */
        if (nAddrLen == sizeof("255.255.255.255") - 1 &&
            !memcmp(pIp, "255.255.255.255", sizeof("255.255.255.255") - 1)
            ) {
            return (0xFFFFFFFF);
        }
        return  0;
    }
    return(ntohl(ip));
#endif
}

int getServiceIp(char** ppIp)
{
    struct sockaddr_in *sin = NULL;
    struct ifaddrs *ifa = NULL, *lsif;
    if (getifaddrs(&lsif)) {
        return SW_ERR;
    }
    char* pTmp = sw_malloc(16);
    if (pTmp == NULL) {
        return SW_ERR;
    }
    for(ifa=lsif; ifa != NULL; ifa=ifa->ifa_next) {
        if(ifa->ifa_addr->sa_family == AF_INET)
        {
            sin = (struct sockaddr_in *)ifa->ifa_addr;
            char* ip = inet_ntoa(sin->sin_addr);
            if (strcmp(ip, "127.0.0.1") == 0) {
                continue;
            } else {
                snprintf(pTmp, 16, "%s", ip);
                break;
            }
        }
    }
    freeifaddrs(lsif);
    if(*ppIp != NULL) {
        sw_free(*ppIp);
    }
    *ppIp = pTmp;
    return SW_OK;
}

int getHostName(char** ppHostName) {
    char* pTmp = sw_malloc(64);
    if (pTmp == NULL) {
        return SW_ERR;
    }
    gethostname(pTmp, 64);
    *ppHostName = pTmp;
    return SW_OK;
}

