#ifndef CONFIG_H_
#define CONFIG_H_

#define PROG_NAME "Server"
#define FILE_NAME_LOG "server_log"
#define FILE_NAME_ERR "server_err"
#define FILE_NAME_CFG "server_cfg"
#define SRV_NAME "server"
#define SLAVE_NAME "slave"

#define CLIENT_MAX 10
#define WRK_SEM_TIMEOUT 30000000
#define BUFFER_SIZE 80

#define MSG_VER_START "Started"
#define MSG_VER_CFG "Configured"
#define MSG_VER_WORK "Working..."
#define MSG_VER_STOP "Stopped"

#define MSG_ERR_CFGFILE "Configuration file reading error"
#define MSG_ERR_MSGREC "MsgReceive"
#define MSG_ERR_MSGSEND "MsgSend"
#define MSG_ERR_MSGREPLY "MsgReply"
#define MSG_ERR_MSGEOK "MsgError EOK"
#define MSG_ERR_MSGERR "MsgError"

#define CFG_PAR_WRKAM "wrk_amount"
#define CFG_PAR_CLIENTMAX "client_max"

#endif /* CONFIG_H_ */
