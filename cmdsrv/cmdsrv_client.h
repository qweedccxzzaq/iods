
#if !defined(_CS_CLIENT_H_)
#define _CS_CLIENT_H_

extern int cmdsrv_client_connect(unsigned int cs_addr, 
                                 unsigned short int cs_port);

extern int cmdsrv_client_datapath_expire_set(unsigned int cs_addr, 
                                             unsigned short int cs_port,
                                             int enabled);

#endif
