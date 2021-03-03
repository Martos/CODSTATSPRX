#include "stdafx.h"

#include <cell/codec/gifdec.h>
#include <cell/codec/jpgdec.h>
#include <cell/codec/jpgenc.h>
#include <cell/codec/pngcom.h>
#include <cell/codec/pngdec.h>
#include <cell/codec/pngenc.h>
#include <cell/gcm.h>

#include <sysutil\sysutil_msgdialog.h>
#include <sysutil\sysutil_oskdialog.h>
#include <sysutil\sysutil_oskdialog_ext.h>

#include <cellstatus.h>
#include <sys/prx.h>
#include <sys/random_number.h>

#include <stdlib.h>

#include "printf.h"

#define THREAD_NAME         "ogl_main_thread"
#define STOP_THREAD_NAME    "ogl_main_stop_thread"

#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

static sys_ppu_thread_t ogl_main_tid = -1;

SYS_MODULE_INFO( OGLTestPRX, 0, 1, 1);
SYS_MODULE_START( _OGLTestPRX_prx_entry );

SYS_LIB_DECLARE_WITH_STUB( LIBNAME, SYS_LIB_AUTO_EXPORT, STUBNAME );
SYS_LIB_EXPORT( _OGLTestPRX_export_function, LIBNAME );

using namespace cell::Gcm;

#define CB_SIZE	(0x10000)
#define HOST_SIZE (1*1024*1024)

size_t strlenA(const char *s) {
	const char *p = s;
	while (*s) ++s;
	return s - p;
}

int readFile(const char * path, unsigned char * buffer, int size)
{
	int fd, ret = 0;

	if (cellFsOpen(path, CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		uint64_t read_e = 0, pos; //, write_e
		cellFsLseek(fd, 0, CELL_FS_SEEK_SET, &pos);

		if (cellFsRead(fd, (void *)buffer, size, &read_e) == CELL_FS_SUCCEEDED)
		{
			ret = 1;
		}

		cellFsClose(fd);
	}

	buffer[size] = '\0';
	return ret;
}

void my_dialog2(int button, void *userdata)
{
	switch (button) {
	case CELL_MSGDIALOG_BUTTON_OK:

	case CELL_MSGDIALOG_BUTTON_NONE:

	case CELL_MSGDIALOG_BUTTON_ESCAPE:
		//dialog_action = 1;
		break;
	default:
		break;
	}
}

static void ogl_main_thread(uint64_t arg) {
	int LOG;
	int ret;
	printf("OGL started \n");
	sys_timer_usleep(5 * 1000 * 1000); //5 second delay

	unsigned char fileBuff[128] = { 0x00 };
	readFile("/dev_hdd0/tmp/COD_STATS.bin", fileBuff, 32);
	printf("FILE: %02X:%02X:%02X:%02X\n", fileBuff[0], fileBuff[1], fileBuff[2], fileBuff[3]);

	cellMsgDialogOpen(CELL_MSGDIALOG_TYPE_PROGRESSBAR_SINGLE, "Welcome to ELITE!", my_dialog2, (void*)0x0000aaab, NULL);
	cellMsgDialogProgressBarSetMsg(CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, "XP: 89570");
	cellMsgDialogProgressBarInc(CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, 30);

	while (1) {
		//printf("OGL-SPRX EXEC .. \n");
		unsigned char *p = (unsigned char *)0xFB3EBC;
		unsigned char *scoreBuf = (unsigned char *)0xFB3EB8;
		unsigned char sep[1] = { 0xFF };
		//printf("KILL: %02X:%02X:%02X:%02X\n", p[0], p[1], p[2], p[3]);
		ret = cellFsOpen("/dev_hdd0/tmp/COD_STATS.bin", CELL_FS_O_RDWR | CELL_FS_O_CREAT /*| CELL_FS_O_APPEND*/, &LOG, NULL, 0);
		if (ret == 1)
		{
			cellFsClose(LOG);
		}
		else {
			uint64_t sw = 4096;
			unsigned char tmpKillSum[4] = { 0x00 };
			
			if (fileBuff[3] != 0xFF) {
				tmpKillSum[0] = p[0] + fileBuff[0];
				tmpKillSum[1] = p[1] + fileBuff[1];
				tmpKillSum[2] = p[2] + fileBuff[2];
				tmpKillSum[3] = p[3] + fileBuff[3];
			}

			int var = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
			printf("SUM = %d\n", var);
			unsigned char res[4];

			res[0] = (var >> 24) & 0xFF;
			res[1] = (var >> 16) & 0xFF;
			res[2] = (var >> 8) & 0xFF;
			res[3] = var & 0xFF;
			printf("%x %x %x %x\n", res[0], res[1], res[2], res[3]);

			cellFsWrite(LOG, (const void *)tmpKillSum, (uint64_t)4, &sw);

			int cont = 0;
			for (cont = 0; cont < 12; cont++) {
				cellFsWrite(LOG, (const void *)sep, (uint64_t)1, &sw);
			}

			cellFsWrite(LOG, (const void *)scoreBuf, (uint64_t)4, &sw);

			for (cont = 0; cont < 12; cont++) {
				cellFsWrite(LOG, (const void *)sep, (uint64_t)1, &sw);
			}

			cellFsClose(LOG);
		}
		sys_timer_usleep(1 * 1000 * 1000); //1 second delay
	}
}

// An exported function is needed to generate the project's PRX stub export library
extern "C" int _OGLTestPRX_export_function(void)
{
    return CELL_OK;
}

extern "C" int _OGLTestPRX_prx_entry(void)
{
	sys_ppu_thread_create(&ogl_main_tid, ogl_main_thread, NULL, 0, 0x4000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME);
    return SYS_PRX_RESIDENT;
}
