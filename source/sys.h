#ifndef _SYS_H
#define _SYS_H

void Sys_init(void);
void N720_PwrOn(void);
void N720_PwrDown(void);



extern unsigned char bN720RstFlag;
extern unsigned char bHeartBeatFlag;
extern unsigned char bPwrOnFlag;
#endif
