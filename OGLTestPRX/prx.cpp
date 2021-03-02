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

#include "printf.h"

#define THREAD_NAME         "ogl_main_thread"
#define STOP_THREAD_NAME    "ogl_main_stop_thread"

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

	cellMsgDialogOpen(CELL_MSGDIALOG_TYPE_PROGRESSBAR_SINGLE, "Welcome to ELITE!", my_dialog2, (void*)0x0000aaab, NULL);
	cellMsgDialogProgressBarSetMsg(CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, "XP: 89570");
	cellMsgDialogProgressBarInc(CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, 30);

	while (1) {
		//printf("OGL-SPRX EXEC .. \n");
		char *p = (char *)0xFB3EBC;
		char *scoreBuf = (char *)0xFB3EB8;
		char sep[1] = { 0xFF };
		printf("KILL: %02X:%02X:%02X:%02X\n", p[0], p[1], p[2], p[3]);
		ret = cellFsOpen("/dev_hdd0/tmp/COD_STATS.bin", CELL_FS_O_RDWR | CELL_FS_O_CREAT /*| CELL_FS_O_APPEND*/, &LOG, NULL, 0);
		if (ret == 1)
		{
			cellFsClose(LOG);
		}
		else {
			uint64_t sw = 4096;
			cellFsWrite(LOG, (const void *)p, (uint64_t)4, &sw);

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
