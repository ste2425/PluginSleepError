#include <pspkernel.h>
#include <pspdisplay.h>
#include <psptypes.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <string.h>
#include <pspiofilemgr.h>

#include "kernel.h"

#include "blit.h"
#include "kubridge.h"
#include "fonts.h"

#include "scepaf.h"

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res) ((res) >= 0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)    ((res) < 0)

#define PRX_PATH "ms0:/SEPLUGINS/core.PRX"

PSP_MODULE_INFO("overlay", 1, 0, 1);
PSP_MAIN_THREAD_ATTR(1);

void drawOverlay(const char *line1, const char *line2);

static int LoadModule(const char *path) {
    SceUID modID = -1;

    modID = kuKernelLoadModule(path, 0, nullptr);
    
    return modID;
}

int thid;
int running;
int lastResp = 99;

// main thread
int main_thread(SceSize args, void *argp){
    SceCtrlData pad;
    SceCtrlData prevPad = {0};

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    LoadModule(PRX_PATH); 


    while (running){
        if (sceDisplayWaitVblankStartCB() < 0)
            break;

        blit_setup();

        sceCtrlPeekBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_TRIANGLE && !(prevPad.Buttons & PSP_CTRL_TRIANGLE)) {  
            
            drawOverlay("Overlay Example", "WAITING FOR EVENT...");
            lastResp = CoreWait();
        } else  if (pad.Buttons & PSP_CTRL_CROSS) {
            char statusMsg[128];
            scePaf_snprintf(statusMsg, sizeof(statusMsg), "Resp From Event: %d", lastResp);
            drawOverlay("Overlay Example", statusMsg);
        }

        prevPad = pad;
    }

    return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp){
    running = 0;
    thid = sceKernelCreateThread("image_display", main_thread, 0x10, 4*1024, PSP_THREAD_ATTR_USER, NULL);
    if (thid >= 0){
        running = 1;
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

int module_stop(SceSize args, void *argp){
    if (running){
        running = 0;
        SceUInt time = 200*1000;
        int ret = sceKernelWaitThreadEnd(thid, &time);
        if (ret < 0)
            sceKernelTerminateDeleteThread(thid);
    }
    return 0;
}

void drawOverlay(const char *line1, const char *line2) {
    blit_Gfx *gfx = blit_gfx_pointer();
    font_Data *font = font_data_pointer();

    int window_width = 300;
    int window_height = gfx->height - 40;
    int menu_start_y = (gfx->height - window_height) / 2;
    int menu_start_x = (gfx->width - window_width) / 2;

    int messageOneY = gfx->height / 2 - font->height;
    int messageTwoY = gfx->height / 2;

    // top, left, right border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y - 3, window_width, 2); // top
    blit_rect_fill(menu_start_x - 3, menu_start_y, 2, window_height); // left
    blit_rect_fill(menu_start_x + window_width + 2, menu_start_y, 2, window_height); // right

    // top backgrund half
    blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
    int topBackgroundHeight = messageOneY - menu_start_y;
    blit_rect_fill(menu_start_x, menu_start_y, window_width, topBackgroundHeight);

    // middle text
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_string_windowed_ctr(messageOneY, menu_start_x, window_width, line1);
    blit_string_windowed_ctr(messageTwoY, menu_start_x, window_width, line2);

    // bottom background half
    blit_set_color(0xffffff, RGBT(80, 58, 147, 128));
    blit_rect_fill(menu_start_x, messageTwoY + font->height, window_width, window_height - topBackgroundHeight - 16);

    // bottom border
    blit_set_color(0xffffff, RGBT(80, 58, 147, 0));
    blit_rect_fill(menu_start_x, menu_start_y + window_height + 2, window_width, 2);
}
