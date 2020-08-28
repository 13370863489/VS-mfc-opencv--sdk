#ifndef KEYDEF_H
#define KEYDEF_H

#define NotFondKey -92
#define ErrSendData -93
#define ErrGetData  -94
#define EEPROMOVERCONST -22 //代表储存器的地址超过指定的范围，整数不能超过EEPROM_LEN-4，浮点不能超过EEPROM_LEN-8
#define NOTOPENFILE -31//不能打开文件
#define NOTREADFILE -32 //不能读取文件
#define NOTWRITEFILE -42 //不能写文件，请查看文件是否有只读属性，或被其它文件打开
#define TRANFSERFAIL  -45 //传送数据失败
#define ERRWRITEPASSWORD -47 //写密码不正确
#define ERRREADPASSWORD -48 //读密码不正确
#define EPPROMOVERADD  -49 //读写EEPROM时，地址溢出
#define USBStatusFail -50  //USB操作失败，可能是没有找到相关的指令
#define OPENUSBFAIL  -51  //打开USB文件句柄失败
#define ENCERROR   -52 //使用加密锁加密自定义表达式，生成加密代码时生产错误
#define NOUSBORDRIVER   -53 //无法打开usb设备，可能驱动程序没有安装或没有插入加密锁。
#define LESSCOUNT      -55  //要加密的数据少于8个字节
#define GETVARERROR   -56   //返回结果变量错误
#define NotVaildFile      -65  //不是有效的BIN文件
#define ERRORKEY      -66  //不能写入加密锁，原因是密钥不正确
#define OVERLEN    -67
#define SETCMPFAIL -68  //USB操作失败
#define FAILSetupDiGetClassDevs  -90005
#define FAILSetupDiGetDeviceInterfaceDetail -90006
#define NOUSBKEY   -92 //找不到加密狗。
#define ErrAddrOver    -81
#define ErrWritePWD   -82
#define ErrReadPWD    -83
#define ErrResult -20

#define FAILEDGENKEYPAIR -21;
#define FAILENC          -22
#define FAILDEC          -23
#define FAILPINPWD  	 -24

#define SM2_ADDBYTE 97//加密后的数据会增加的长度
#define MAX_ENCLEN 128 //最大的加密长度分组
#define MAX_DECLEN  (MAX_ENCLEN+SM2_ADDBYTE) //最大的解密长度分组
#define USERID_LEN	  80//最大的用户身份长度


#define ECC_MAXLEN 	  32
#define PIN_LEN 	  16
#define MAX_LEN 2031

//定义命令
#define GETVERSION      0x01
#define GETID      		0x02
#define S_MYENC         0X03
#define S_MYDEC         0X04
#define GETVEREX        0x05
#define SETCAL			0x06
#define	SETID			0X07
#define CAL_TEA         0x08
#define SET_TEAKEY      0x09
#define CAL_TEA_2       0x0c
#define SET_TEAKEY_2    0x0d
#define READBYTE   		0x10
#define WRITEBYTE     	0x11
#define YTREADBUF_2K		0x10
#define YTWRITEBUF_2K      0x11
#define YTREADBUF		0x12
#define YTWRITEBUF      0x13
#define MYRESET         0x20
#define YTREBOOT   		0x24
#define SET_ECC_PARA    0x30
#define GET_ECC_PARA	0x31
#define SET_ECC_KEY     0x32
#define GET_ECC_KEY     0x33
#define MYENC			0x34
#define MYDEC			0X35
#define SET_PIN			0X36
#define GEN_KEYPAIR     0x37
#define YTSIGN          0x51
#define YTVERIFY        0x52
#define GET_CHIPID      0x53
#define YTSIGN_2        0x54
#define WRITESN			0XCE
#define GETSN			0x0F
#define WRITESN			0XCE
#define	SETHIDONLY	    0x55
#define SETREADONLY		0x56





#endif // KEYDEF_H
