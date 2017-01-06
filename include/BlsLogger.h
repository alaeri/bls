/*
 * BlsLogger.h
 *
 *  Created on: 2014-10-13
 *      Author: chenyuliang01
 *
 * Copyright (c) 2014, chenyuliang01@baidu.com baiducam All Rights Reserved.
 */

#ifndef BLSLOGGER_H_
#define BLSLOGGER_H_

#include <com_log.h>

/**
 * 初始化日志模块，程序开始时执行一次
 * @param conf_path 配置文件相对路径 conf/log_conf.conf
 */
void init_bls_logger(const char*conf_path);

/**
 * 关闭日志模块
 */
void close_bls_logger();

#define BMS_LOG(fmt, args...) com_writelog("BMS", fmt, ##args)

#define DEBUG_LOG(fmt, args...) com_writelog(COMLOG_DEBUG, fmt, ##args)
#define TRACE(fmt, args...) com_writelog(COMLOG_TRACE, fmt, ##args)
#define NOTICE(fmt, args...) com_writelog(COMLOG_NOTICE, fmt, ##args)
#define WARNING(fmt, args...) com_writelog(COMLOG_WARNING, fmt, ##args)
#define FATAL(fmt, args...) com_writelog(COMLOG_FATAL, fmt, ##args)

#define SYS_DEBUG(fmt, args...) com_writelog(COMLOG_DEBUG, "[SYSTEM] " fmt, ##args)
#define SYS_TRACE(fmt, args...) com_writelog(COMLOG_TRACE, "[SYSTEM] " fmt, ##args)
#define SYS_NOTICE(fmt, args...) com_writelog(COMLOG_NOTICE, "[SYSTEM] " fmt, ##args)
#define SYS_WARNING(fmt, args...) com_writelog(COMLOG_WARNING, "[SYSTEM] " fmt, ##args)
#define SYS_FATAL(fmt, args...) com_writelog(COMLOG_FATAL, "[SYSTEM] " fmt, ##args)

#define CLIENT_DEBUG(client, fmt, args...) \
    com_writelog(COMLOG_DEBUG, "[%s] " fmt, client->id, ##args)
#define CLIENT_TRACE(client, fmt, args...) \
    com_writelog(COMLOG_TRACE, "[%s] " fmt, client->id, ##args)
#define CLIENT_NOTICE(client, fmt, args...) \
    com_writelog(COMLOG_NOTICE, "[%s] " fmt, client->id, ##args)
#define CLIENT_WARNING(client, fmt, args...) \
    com_writelog(COMLOG_WARNING, "[%s] " fmt, client->id, ##args)
#define CLIENT_FATAL(client, fmt, args...) \
    com_writelog(COMLOG_FATAL, "[%s] " fmt, client->id, ##args)

#endif /* BLSLOGGER_H_ */
