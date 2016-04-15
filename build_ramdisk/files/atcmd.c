/*	$ssdlinux: runled_bx1.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
/*
 * Copyright (c) 2008-2016 Plat'Home CO., LTD.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Plat'Home CO., LTD. nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_MODEM "/dev/ttyACM0"
#define POWERSW "/sys/class/gpio/gpio165/value"
#define RESETSW "/sys/class/gpio/gpio15/value"
#define POWERSW_U200 "/sys/class/gpio/gpio200/value"
#define RESETSW_U200 "/sys/class/gpio/gpio202/value"
#define LED_R "/sys/class/gpio/gpio47/value"
#define LED_G "/sys/class/gpio/gpio48/value"
#define LED_B "/sys/class/gpio/gpio49/value"
#define TIMEOUT 10
#define BUFSZ 2048

#define CMD_PON "PON"
#define CMD_POFF "POFF"
#define CMD_PRST "PRST"
#define CMD_HRST "HRST"
#define CMD_SMONI "SMONI"
#define CMD_SIND "SIND"
#define CMD_CCLK "CCLK"
#define CMD_CCID "CCID"
#define CMD_CSQ "CSQ"
#define CMD_CTZU "CTZU"
#define CMD_ATI "ATI"
#define CMD_CGSN "CGSN"

#define AT_AT "at\r\n"
#define AT_POFF "at^smso\r\n"
#define AT_POFF_U200 "at+cpwroff\r\n"
#define AT_POFF_UM04 "at*dpwroff\r\n"
#define AT_PRST_U200 "at+cfun=15\r\n"
#define AT_PRST_K "at$40=0\r\n"
#define AT_PRST_UM04 "at*dhwrst\r\n"
#define AT_SMONI "at^smoni\r\n"
#define AT_SIND "at^sind?\r\n"
#define AT_CCLK "at+cclk?\r\n"
#define AT_CCLK_K "at$31?\r\n"
#define AT_CCID "at+ccid\r\n"
#define AT_CCID_K "at$19?\r\n"
#define AT_CCID_U "at*kiccid\r\n"
#define AT_CSQ "at+csq\r\n"
#define AT_CSQ_K "at$30=0\r\n"
#define AT_CSQ_U "at*dlante\r\n"
#define AT_CTZU "at+ctzu=%s\r\n"
#define AT_CTZU2 "at+ctzu?\r\n"
#define AT_ATI "ati\r\n"
#define AT_CGSN "at+cgsn\r\n"
#define AT_CGSN_K "at$10?\r\n"
#define AT_GMI "at+gmi\r\n"
#define AT_GMM "at+gmm\r\n"
#define AT_GMR "at+gmr\r\n"

#define EHS6 "EHS6\n"
#define U200E "U200E\n"
#define U200 "U200\n"
#define KYM11 "KYM11\n"
#define UM04 "UM04\n"

#define EXIST 0
#define NOEXIST -1

static struct termios old;
static char MODEM[32];
static char MNAME[16];

int set_reset_u200(char *);

void usage(char *fname)
{
	printf("3G modem support program for gemalto cinterion EHS6\n");
	printf("# %s [option] CMD CMD CMD ...\n", fname);
	printf("CMD :\n");
	printf("PON\t= power on modem\n"); 
	printf("POFF\t= power off modem\n");
	printf("PRST\t= reboot modem\n");
	printf("HRST\t= modem hardware reset\n");
	printf("SMONI\t= get signal strength (for EHS6)\n");
	printf("SIND\t= get time (for EHS6)\n");
	printf("CCLK\t= get time (for U200/KYM11)\n");
	printf("CSQ\t= get signal quality\n");
	printf("CCID\t= get USIM card identification number\n");
	printf("CTZU\t= set Automatic Time Zone Update(for U200)\n");
	printf("ATI\t= get product identification information\n");
	printf("CGSN\t= get Serial number\n");
	printf("\n");
	printf("option:\n");
	printf("\t-d modemdevice (default is /dev/ttyACM0)\n");
	printf("\n");
}

int chk_device()
{
	return 0;
}

int wait_device(int mode)
{
#define RETRY 30
	int i;

	for(i=0; i<RETRY; i++){
		if(access(MODEM, F_OK) == mode){
			return 0;
		}
		sleep(1);
	}
	return -1;
}

int set_power(char *val)
{
	int fd;

	if((fd = open(POWERSW, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, val, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int set_power_u200(int val, char *gpio_pin)
{
	int fd;

	if(val == 1 && access(MODEM, F_OK) == 0){
		/* already Power On */
		return 0;
	}
	else if(val == 0 && access(MODEM, F_OK) == -1){
		/* already Power Off */
		return 0;
	}

	if((fd = open(gpio_pin, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(write(fd, "0", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	sleep(2);
	if(write(fd, "1", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int set_power_kym11(int val)
{
	int fd;

	if(val == 1 && access(MODEM, F_OK) == 0){
		/* already Power On */
		return 0;
	}
	else if(val == 0 && access(MODEM, F_OK) == -1){
		/* already Power Off */
		return 0;
	}

	if((fd = open(POWERSW_U200, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(write(fd, "1", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	sleep(2);

	if(write(fd, "0", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int set_reset(char *gpio_reset)
{
	int fd;

	if((fd = open(gpio_reset, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, "1", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	sleep(2);
	if(write(fd, "0", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int set_reset_u200(char *gpio_pin)
{
	int fd;

	if((fd = open(gpio_pin, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, "0", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	sleep(3);
	if(write(fd, "1", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int set_reset_kym11(char *gpio_reset)
{
	int fd;

	if((fd = open(gpio_reset, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, "0", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	sleep(1);
	if(write(fd, "1", 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int init_modem(int *fd)
{
	int ret;
	struct termios new;

	/* permission */
	if(chmod(MODEM, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	/* open modem */
	if((*fd = open(MODEM, O_RDWR|O_NOCTTY|O_NONBLOCK)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	/* check MODEM */
	if(ioctl(*fd, TIOCMGET, &ret) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(tcgetattr(*fd, &old) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	memset(&new, 0x0, sizeof(struct termios));
	new.c_cflag = B115200|CRTSCTS|CREAD|CS8|CLOCAL;
	new.c_iflag = IGNPAR|ICRNL;
	new.c_cc[VINTR] = 0;
	new.c_cc[VQUIT] = 0;
	new.c_cc[VERASE] = 0;
	new.c_cc[VKILL] = 0;
	new.c_cc[VEOF] = 4;
	new.c_cc[VTIME] = 0;
	new.c_cc[VMIN] = 1;
	new.c_cc[VSWTC] = 0;
	new.c_cc[VSTART] = 0;
	new.c_cc[VSTOP] = 0;
	new.c_cc[VSUSP] = 0;
	new.c_cc[VEOL] = 0;
	new.c_cc[VREPRINT] = 0;
	new.c_cc[VDISCARD] = 0;
	new.c_cc[VWERASE] = 0;
	new.c_cc[VLNEXT] = 0;
	new.c_cc[VEOL2] = 0;

	tcflush(*fd, TCIFLUSH);
	if(tcsetattr(*fd, TCSANOW, &new) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

int send_atcmd(int fd, char *cmd, char *buf, int wait)
{
	int ret;
	fd_set fds;
	struct timeval tv;

	if(write(fd, cmd, strlen(cmd)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	if((ret = select(fd+1, &fds, NULL, NULL, &tv)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	else if(ret){
		sleep(wait);
		if((ret = read(fd, buf, BUFSZ)) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
		buf[ret] = 0x0;
	}
	else{
		printf("%d: receive timeout\n", __LINE__);
		return -1;
	}
	return 0;
}

void end_modem(int *fd)
{
	if(*fd){
		tcsetattr(*fd, TCSANOW, &old);
		close(*fd);
		*fd = 0;
	}
}

int get_dbm(char *buf)
{
	int i;
	char *p1, *p2;
	char val[BUFSZ];

#ifdef _DEBUG_
//printf("buf=%s\n", buf);
#endif
	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	p1 = buf;
	for(i=1; i<5; i++){
		p1 = strchr(p1, ',');
		p1++;
	}
	if(p1 == NULL){
		return -1;
	}
	/* get end point */
	p2 = strchr(p1+1, ',');
	if(p2 == NULL){
		return -1;
	}
	memcpy(val, p1, p2-p1);
	val[p2-p1] = 0;
	printf("%s\n", val);
#if 0
	put_led(val);
#endif

	return 0;
}

int get_time(char *buf)
{
#define NITZ "nitz"
	int i;
	char *p1;
	char year[3], mon[3], day[3], hour[3], min[3], sec[3];

//printf("buf=%s\n", buf);
	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	if((p1 = strstr(buf, NITZ)) == NULL){
		return -1;
	}
	for(i=0; i<2; i++){
		if((p1 = strchr(p1, ',')) == NULL){
			return -1;
		}
		p1++;
	}
	p1++;	// skip '"'

//printf("p1=%s\n", p1);
	// get year
	memcpy(year, p1, 2);
	year[2] =0;
	p1 += 3;
	// get month
	memcpy(mon, p1, 2);
	mon[2] =0;
	p1 += 3;
	// get day
	memcpy(day, p1, 2);
	day[2] =0;
	p1 += 3;
	// get hour
	memcpy(hour, p1, 2);
	hour[2] =0;
	p1 += 3;
	// get min
	memcpy(min, p1, 2);
	min[2] =0;
	p1 += 3;
	// get sec
	memcpy(sec, p1, 2);
	sec[2] =0;
	p1 += 2;
	if( *p1 != '"'){	// check '"'
		return -1;
	}
	printf("%s%s%s%s20%s.%s\n", mon, day, hour, min, year, sec);

	return 0;
}

int get_cclk(char *buf)
{
#define START "CCLK: "
	char *p1;
	char year[3], mon[3], day[3], hour[3], min[3], sec[3];

//printf("buf=%s\n", buf);
	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	if((p1 = strstr(buf, START)) == NULL){
		return -1;
	}
	p1 += strlen(START);
	p1++;	// skip '"'

//printf("p1=%s\n", p1);
	// get year
	memcpy(year, p1, 2);
	year[2] =0;
	p1 += 3;
	// get month
	memcpy(mon, p1, 2);
	mon[2] =0;
	p1 += 3;
	// get day
	memcpy(day, p1, 2);
	day[2] =0;
	p1 += 3;
	// get hour
	memcpy(hour, p1, 2);
	hour[2] =0;
	p1 += 3;
	// get min
	memcpy(min, p1, 2);
	min[2] =0;
	p1 += 3;
	// get sec
	memcpy(sec, p1, 2);
	sec[2] =0;
	p1 += 2;
	if( *p1 != '+'){	// check '+'
		return -1;
	}
	printf("%s%s%s%s20%s.%s\n", mon, day, hour, min, year,sec);

	return 0;
#undef START
}

int get_cclk_k(char *buf)
{
#define START "at$31?\n"
	char *p1;
	char year[5], mon[3], day[3], hour[3], min[3], sec[3];

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	if((p1 = strstr(buf, START)) == NULL){
		return -1;
	}
	p1 += strlen(START);

	// get year
	memcpy(year, p1, 4);
	year[4] =0;
	p1 += 5;
	// get month
	memcpy(mon, p1, 2);
	mon[2] =0;
	p1 += 3;
	// get day
	memcpy(day, p1, 2);
	day[2] =0;
	p1 += 3;
	// get hour
	memcpy(hour, p1, 2);
	hour[2] =0;
	p1 += 3;
	// get min
	memcpy(min, p1, 2);
	min[2] =0;
	p1 += 3;
	// get sec
	memcpy(sec, p1, 2);
	sec[2] =0;
	printf("%s%s%s%s%s.%s\n", mon, day, hour, min, year,sec);

	return 0;
#undef START
}

int get_quality(char *buf)
{
#define HEAD "+CSQ: "
	char *p1, *p2;
	char val[BUFSZ];
	long rssi;

#ifdef _DEBUG_
//printf("buf=%s\n", buf);
#endif
	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	if((p1 = strstr(buf, HEAD)) == NULL){
		return -1;
	}
	p1 += strlen(HEAD);

	/* get end point */
	p2 = strchr(p1+1, ',');
	if(p2 == NULL){
		return -1;
	}
	memcpy(val, p1, p2-p1);
	val[p2-p1] = 0;
	rssi = strtol(val, NULL, 10);
	printf("%ld\n", (-113 + rssi * 2));

	return 0;
#undef HEAD
}

int get_quality_k(char *buf)
{
#define HEAD "at$30=0\n"
	char *p1;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	if((p1 = strstr(buf, HEAD)) == NULL){
		return -1;
	}
	p1 += strlen(HEAD);
	p1[1] = 0x0;
	printf("%s\n", p1);

	return 0;
#undef HEAD
}

int get_quality_u(char *buf)
{
	char *p1;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(strstr(buf, "SEARCH")){
		return -1;
	}
	/* get start point */
	if((p1 = strchr(buf, ':')) == NULL){
		return -1;
	}
	p1 += 2;
	p1[1] = 0x0;
	printf("%s\n", p1);

	return 0;
}

int get_ccid(char *buf, int offset)
{
#define CCIDLEN 19
	char *p1;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	/* search top */
	if((p1 = strchr(buf, ':')) == NULL){
		return -1;
	}
	p1 += offset;
	p1[CCIDLEN] = 0x0;
	printf("%s\n", p1);
	return 0;
}

int get_ccid_k(char *buf)
{
#define CCIDSTR "at$19?"
	char *p1;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}

	/* search top */
	if((p1 = strstr(buf, CCIDSTR)) == NULL){
		return -1;
	}
	p1 += sizeof(CCIDSTR);
	p1[19] = 0x0;
	printf("%s\n", p1);

	return 0;
#undef CCIDSTR
}

int get_ati(char *buf)
{
	char *p1, *p2;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if((p1 = strstr(buf, "LISA")) != NULL || (p1 = strstr(buf, "TOBY")) != NULL){
		if((p2 = strchr(p1, '\n')) == NULL){
			return -1;
		}
		*p2 = 0x0;
		printf("%s\n", p1);
		return 0;
	}
	else if((p1 = strstr(buf, "Cinterion")) != NULL){
		if((p1 = strstr(buf, "EHS6")) != NULL){
			if((p1 = strstr(buf, "REVISION")) != NULL){
				if((p2 = strchr(p1, '\n')) == NULL){
					return -1;
				}
				*p2 = 0x0;
				printf("Cinterion EHS6 %s\n", p1);
				return 0;
			}
		}
	}
	printf("%s\n", buf);

	return -1;
}

int get_ati_u(char *buf, char* match)
{
	char *p1, *p2;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}

	if((p1 = strstr(buf, match)) == NULL){
		return -1;
	}
	p1 += strlen(match);
	if((p2 = strchr(p1, '\n')) == NULL){
		return -1;
	}
	*p2 = 0x0;
	printf("%s ", p1);

	return 0;
}

int get_ctzu(char *buf)
{
	char *p1,*p2;
	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}
	if(!strstr(buf, "OK")){
		return -1;
	}
	if((p1 = strstr(buf, "+CTZU")) == NULL){
		return 0;
	}
	if((p2 = strchr(p1, '\n')) == NULL){
		return 0;
	}
	*p2 = 0x0;
	printf("%s\n", p1);
	return 0;
}

int get_cgsn(char *buf)
{
#define CGSNSTR "at+cgsn"
	char *p1;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}

	/* search top */
	if((p1 = strstr(buf, CGSNSTR)) == NULL){
		return -1;
	}
	p1 += sizeof(CGSNSTR);
	/* skipped CR */
	for(; *p1 == '\n'; p1++){
		if(*p1 == 0x0)
			return -1;
	}
	p1[15] = 0x0;
	printf("%s\n", p1);
	return 0;
#undef CGSNSTR
}

int get_cgsn_k(char *buf)
{
#define CGSNSTR "at$10?"
	char *p1;

	/* check error */
	if(strstr(buf, "ERROR")){
		return -1;
	}

	/* search top */
	if((p1 = strstr(buf, CGSNSTR)) == NULL){
		return -1;
	}
	p1 += sizeof(CGSNSTR);
	/* skipped CR */
	p1[15] = 0x0;
	printf("%s\n", p1);
	return 0;
#undef CGSNSTR
}

int main(int ac, char *av[])
{
	FILE *fp;
	int fd=0;
	int i=1, j=0;
	char buf[BUFSZ];
	char cmd[16];
	int ret=0;

	if(ac == 1){
		usage(av[0]);
		return 1;
	}

	if(strncmp(av[1], "-d", 2) == 0){
		strncpy(MODEM, av[2], sizeof(MODEM) - 1);
		i = 3;
	}
	else{
		memcpy(MODEM, DEFAULT_MODEM, sizeof(DEFAULT_MODEM));
		i = 1;
	}

	if((fp = popen("/usr/sbin/obsiot-modem.sh", "r")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	fgets(MNAME, sizeof(MNAME)-1, fp);
	pclose(fp);

	while(i < ac){
		if(strncmp(CMD_PON, av[i], sizeof(CMD_PON)) == 0){
			if(access(MODEM, F_OK) != 0){
				if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0){
					set_power("1");
				}
				else if(strncmp(U200, MNAME, sizeof(U200)) == 0){
					set_power_u200(1, POWERSW);
				}
				else if(strncmp(U200E, MNAME, sizeof(U200E)) == 0){
					set_power_u200(1, POWERSW_U200);
				}
				else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
					set_power_kym11(1);
				}
				else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
					set_power_u200(1, POWERSW_U200);
				}

				if(wait_device(EXIST)){
					printf("%d: Can not Power ON!\n", __LINE__);
					return -1;
				}
				if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
					sleep(11);
				}
				else if(strncmp(U200E, MNAME, sizeof(U200E)) == 0){
					sleep(8);
				}
				else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
					sleep(6);
				}
			}
		}
		else if(strncmp(CMD_POFF, av[i], sizeof(CMD_POFF)) == 0){
			if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				set_power_kym11(0);
			}
			else{
				if(access(MODEM, F_OK) != 0){
					printf("%d: MODEM is not exist\n", __LINE__);
					return -1;
				}
				if(fd == 0 && init_modem(&fd)){
					end_modem(&fd);
					return -1;
				}
				if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0){
					set_power("0");
					send_atcmd(fd, AT_AT, buf, 0);
					send_atcmd(fd, AT_POFF, buf, 1);
				}
#if 0
				else if(strncmp(U200, MNAME, sizeof(U200)) == 0
						 || strncmp(U200E, MNAME, sizeof(U200E)) == 0){
					send_atcmd(fd, AT_AT, buf, 0);
					send_atcmd(fd, AT_POFF_U200, buf, 1);
				}
#else
				else if(strncmp(U200, MNAME, sizeof(U200)) == 0){
					set_power_u200(0, POWERSW);
				}
				else if(strncmp(U200E, MNAME, sizeof(U200E)) == 0){
					set_power_u200(0, POWERSW_U200);
				}
#endif
				else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
					send_atcmd(fd, AT_AT, buf, 0);
					send_atcmd(fd, AT_POFF_UM04, buf, 1);
				}
			}
			if(wait_device(NOEXIST)){
//				printf("%d: Can not Power OFF!\n", __LINE__);
				return -1;
			}
			sleep(1);
		}
		else if(strncmp(CMD_HRST, av[i], sizeof(CMD_HRST)) == 0){
			end_modem(&fd);
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0){
				if(access(MODEM, F_OK) == 0){
					set_reset(RESETSW);
				}
				else{
					printf("%d: Can not Reset at the power off.\n", __LINE__);
					return -1;
				}
			}
			else if(strncmp(U200, MNAME, sizeof(U200)) == 0){
				set_reset(RESETSW);
			}
			else if(strncmp(U200E, MNAME, sizeof(U200E)) == 0){
				set_reset_u200(RESETSW_U200);
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				if(access(MODEM, F_OK) == 0){
					end_modem(&fd);
					set_reset_kym11(RESETSW_U200);
				}
				else{
					printf("%d: Can not Reset at the power off.\n", __LINE__);
					return -1;
				}
			}
			else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				if(access(MODEM, F_OK) == 0){
					set_reset_u200(RESETSW_U200);
				}
				else{
					printf("%d: Can not Reset at the power off.\n", __LINE__);
					return -1;
				}
			}
			if(wait_device(NOEXIST)){
				printf("%d: Can not Hard Reset!\n", __LINE__);
				return -1;
			}
			if(wait_device(EXIST)){
				printf("%d: Can not Hard Reset!\n", __LINE__);
				return -1;
			}
			if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				sleep(11);
			}
			else if(strncmp(U200E, MNAME, sizeof(U200E)) == 0){
				sleep(8);
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				sleep(6);
			}
		}
		else if(strncmp(CMD_PRST, av[i], sizeof(CMD_PRST)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0){
				set_power("1");
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_POFF, buf, 1);
				end_modem(&fd);
			}
			else if(strncmp(U200, MNAME, sizeof(U200)) == 0
					|| strncmp(U200E, MNAME, sizeof(U200E)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_PRST_U200, buf, 1);
				end_modem(&fd);
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_PRST_K, buf, 1);
				end_modem(&fd);
			}
			else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_PRST_UM04, buf, 1);
				end_modem(&fd);
			}
			if(access(MODEM, F_OK) == 0){
				if(wait_device(NOEXIST)){
					printf("%d: Can not Soft Reset!\n", __LINE__);
					return -1;
				}
			}
			if(wait_device(EXIST)){
				printf("%d: Can not Soft Reset!\n", __LINE__);
				return -1;
			}
			if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				sleep(11);
			}
			else if(strncmp(U200E, MNAME, sizeof(U200E)) == 0){
				sleep(8);
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				sleep(6);
			}
		}
		else if(strncmp(CMD_SMONI, av[i], sizeof(CMD_SMONI)) == 0){
			if(access(MODEM, F_OK) == 0){
				if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0){
					if(fd == 0 && init_modem(&fd)){
						end_modem(&fd);
						return -1;
					}
					send_atcmd(fd, AT_SMONI, buf, 1);
					if(get_dbm(buf)){
						return -1;
					}
				}
				else{
					printf("%d: This MODEM is not support.\n", __LINE__);
					ret = -1;
					break;
				}
			}
			else{
				ret = -1;
				printf("%d: MODEM is not found.\n", __LINE__);
				break;
			}
		}
		else if(strncmp(CMD_CSQ, av[i], sizeof(CMD_CSQ)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0
						|| strncmp(U200E, MNAME, sizeof(U200E)) == 0
						|| strncmp(U200, MNAME, sizeof(U200)) == 0){
				send_atcmd(fd, AT_CSQ, buf, 1);
				if(get_quality(buf)){
					return -1;
				}
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_CSQ_K, buf, 1);
				if(get_quality_k(buf)){
					return -1;
				}
			}
			else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_CSQ_U, buf, 1);
				if(get_quality_u(buf)){
					return -1;
				}
			}
		}
		else if(strncmp(CMD_CCID, av[i], sizeof(CMD_CCID)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0
						|| strncmp(U200E, MNAME, sizeof(U200E)) == 0
						|| strncmp(U200, MNAME, sizeof(U200)) == 0){
				send_atcmd(fd, AT_CCID, buf, 1);
				if(get_ccid(buf, 2)){
					return -1;
				}
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_CCID_K, buf, 1);
				if(get_ccid_k(buf)){
					return -1;
				}
			}
			else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_CCID_U, buf, 1);
				if(get_ccid(buf, 3)){
					return -1;
				}
			}
		}
		else if(strncmp(CMD_ATI, av[i], sizeof(CMD_ATI)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0
						|| strncmp(U200E, MNAME, sizeof(U200E)) == 0
						|| strncmp(U200, MNAME, sizeof(U200)) == 0){
				send_atcmd(fd, AT_ATI, buf, 1);
				if(get_ati(buf)){
					ret = -1;
					break;
				}
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_GMI, buf, 0);
				if(get_ati_u(buf, ": ")){
					ret = -1;
					break;
				}
				send_atcmd(fd, AT_GMM, buf, 0);
				if(get_ati_u(buf, ": ")){
					ret = -1;
					break;
				}
				send_atcmd(fd, AT_GMR, buf, 0);
				if(get_ati_u(buf, ": ")){
					ret = -1;
					break;
				}
				puts("");
			}
			else if(strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_GMI, buf, 1);
				if(get_ati_u(buf, "\n\n\n")){
					ret = -1;
					break;
				}
				send_atcmd(fd, AT_GMM, buf, 1);
				if(get_ati_u(buf, "\n\n\n")){
					ret = -1;
					break;
				}
				send_atcmd(fd, AT_GMR, buf, 1);
				if(get_ati_u(buf, "\n\n\n")){
					ret = -1;
					break;
				}
				puts("");
			}
		}
		else if(strncmp(CMD_CTZU, av[i], sizeof(CMD_CTZU)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(U200E, MNAME, sizeof(U200E)) == 0
						|| strncmp(U200, MNAME, sizeof(U200)) == 0){
				if(av[i+1] == NULL || (av[i+1] != NULL && av[i+1][0] != '0' && av[i+1][0] != '1')){
					send_atcmd(fd, AT_CTZU2, buf, 1);
					if(get_ctzu(buf)){
						ret = -1;
						break;
					}
				}
				else{
					// get CTZU parameter
					i++;
					sprintf(cmd, AT_CTZU, av[i]);
					send_atcmd(fd, cmd, buf, 1);
					if(get_ctzu(buf)){
						ret = -1;
						break;
					}
				}
			}
			else{
				printf("%d: This MODEM is not support.\n", __LINE__);
				ret = -1;
				break;
			}
		}
		else if(strncmp(CMD_SIND, av[i], sizeof(CMD_SIND)) == 0){
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0){
				if(fd == 0 && init_modem(&fd)){
					end_modem(&fd);
					return -1;
				}
				for(j=0; j<60; j++){
					send_atcmd(fd, AT_SIND, buf, 1);
					if(!get_time(buf)){
						break;
					}
				}
				if(j==5){
					ret = -1;
					break;
				}
			}
			else{
				printf("%d: This MODEM is not support.\n", __LINE__);
				ret = -1;
				break;
			}
		}
		else if(strncmp(CMD_CCLK, av[i], sizeof(CMD_CCLK)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(U200E, MNAME, sizeof(U200E)) == 0
						|| strncmp(U200, MNAME, sizeof(U200)) == 0
						|| strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				for(j=0; j<6; j++){
					send_atcmd(fd, AT_CCLK, buf, 1);
					if(!get_cclk(buf)){
						break;
					}
				}
				if(j==5){
					ret = -1;
					break;
				}
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_CCLK_K, buf, 1);
				if(!get_cclk_k(buf)){
					ret = -1;
					break;
				}
			}
			else{
				printf("%d: This MODEM is not support.\n", __LINE__);
				ret = -1;
				break;
			}
		}
		else if(strncmp(CMD_CGSN, av[i], sizeof(CMD_CGSN)) == 0){
			if(fd == 0 && init_modem(&fd)){
				end_modem(&fd);
				return -1;
			}
			if(strncmp(EHS6, MNAME, sizeof(EHS6)) == 0
						|| strncmp(U200E, MNAME, sizeof(U200E)) == 0
						|| strncmp(U200, MNAME, sizeof(U200)) == 0
						|| strncmp(UM04, MNAME, sizeof(UM04)) == 0){
				send_atcmd(fd, AT_CGSN, buf, 1);
				if(get_cgsn(buf)){
					ret = -1;
					break;
				}
			}
			else if(strncmp(KYM11, MNAME, sizeof(KYM11)) == 0){
				send_atcmd(fd, AT_AT, buf, 0);
				send_atcmd(fd, AT_CGSN_K, buf, 1);
				if(get_cgsn_k(buf)){
					ret = -1;
					break;
				}
			}
		}
		else{
			printf("unknown CMD(%s)\n", av[i]);
			break;
		}
		i++;
	}
	end_modem(&fd);
	return ret;
}