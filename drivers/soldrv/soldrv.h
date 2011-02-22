/* this is the type of message sent to soldrv to (un)register a proxy:
    SOL_PROXY (RE)SET_PROXY proxy_id proxy_pid.

SOL_PROXY : msg_hdr_type;
(RE)SET_PROXY : msg_hdr_type;
proxy_id : unsigned char;
proxy_pid : pid_t;
*/

#define SOL_RESET_PROXY 0
#define SOL_SET_PROXY   1

typedef struct {
    msg_hdr_type set_or_reset;
    unsigned char proxy_id;
    pid_t proxy_pid;
} proxy_reg_type;
