// SIM908 GPS related functions
#ifndef SIM908_H_
#define SIM908_H_

void initSIM908(void);
void SIM908_pwr_on(void);
void SIM908_autobaud_init(void);
void sendWithHWFlowCtrl(char *buf, unsigned int bytes);
unsigned char SIM908sendCmd(unsigned char cmd);

#endif
