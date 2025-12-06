#ifdef DEBUG
#include <pspdebug.h>
#endif


#include <pspdisplay.h>


#include <pspsdk.h>
#include <psptypes.h>
#include <pspkerror.h>
#include <pspkerneltypes.h>
#include <pspthreadman.h>
#include <pspctrl.h>

#include <stdbool.h>
#include <inttypes.h>

#define str(s) #s // For stringizing defines
#define xstr(s) str(s)

#ifdef DEBUG
#define DEBUG_PRINT(...) pspDebugScreenKprintf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define MODULE_NAME "WaitTest"
#define MAJOR_VER 1
#define MINOR_VER 1

#define MODULE_OK       0
#define MODULE_ERROR    1


static SceUID commandEventId = -1;
#define COMMAND_READY_EVENT  		0x02

//
// PSP SDK
//
// We are building a kernel mode prx plugin
PSP_MODULE_INFO(MODULE_NAME, PSP_MODULE_KERNEL, MAJOR_VER, MINOR_VER);

// We don't allocate any heap memory, so set this to 0.
PSP_HEAP_SIZE_KB(0);

// We don't need a main thread since we only do basic setup during module start and won't stall module loading.
// This will make us be called from the module loader thread directly, instead of a secondary kernel thread.
PSP_NO_CREATE_MAIN_THREAD();

// We don't need any of the newlib features since we're not calling into stdio or stdlib etc
PSP_DISABLE_NEWLIB();

//
// Forward declarations
//
static int main_thread(SceSize args, void *argp);
static int start_main_thread(void);
static int stop_main_thread(void);
int module_start(SceSize args, void *argp);
int module_stop(SceSize args, void *argp);

static SceUID g_mainThreadId = -1;

int CoreWait() {
    unsigned int k1 = pspSdkSetK1(0);

    int resp = sceKernelWaitEventFlag(commandEventId, COMMAND_READY_EVENT, PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR, nullptr, nullptr);

    pspSdkSetK1(k1);

    return resp;

}

// The main thread.
static
int main_thread(SceSize args, void *argp)
{
    SceCtrlData pad;
    SceCtrlData prevPad = {0};

    while (true){
        if (sceDisplayWaitVblankStartCB() < 0)
            break;

        sceCtrlPeekBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_CIRCLE && !(prevPad.Buttons & PSP_CTRL_CIRCLE)) {            
            sceKernelSetEventFlag(commandEventId, COMMAND_READY_EVENT);
        }

        prevPad = pad;
    }

    return 0;
}

static
int start_main_thread(void)
{
    int result;
    SceUID thid;

    // name, entry, initPriority, stackSize, PspThreadAttributes, SceKernelThreadOptParam
    thid = sceKernelCreateThread(MODULE_NAME "MainThread", main_thread, 0x11, 0x800, 0, 0);
    if (thid >= 0) {
        DEBUG_PRINT("Starting main thread\n");
        result = sceKernelStartThread(thid, 0, 0);
        if(result < 0) {
            DEBUG_PRINT("Failed to start main thread: ret 0x%08x\n", result);
        }

        g_mainThreadId = thid;
    }
    else {
        result = thid;
        DEBUG_PRINT("Failed to create main thread: ret 0x%08x\n", result);
    }

    return result;
}

static
int stop_main_thread(void)
{
    int result = 0;
    SceUID thid = g_mainThreadId;

    if(thid >= 0) {
        // Unblock sceKernelSleepThreadCB() and have thread begin cleanup
        result = sceKernelWakeupThread(thid);
        if(result < 0) {
            DEBUG_PRINT("Failed to wakeup main thread: ret 0x%08x\n", result);
        }

        // Wait for the main thread to clean up and exit
        DEBUG_PRINT("Waiting for main thread exit ...\n");
        result = sceKernelWaitThreadEnd(thid, NULL);
        if(result < 0) {
            // Thread did not stop, force terminate and delete it
            DEBUG_PRINT("Failed to wait for main thread exit: ret 0x%08x\n", result);
            DEBUG_PRINT("Terminating and deleting main thread\n");
            result = sceKernelTerminateDeleteThread(thid);
            if(result >= 0) {
                g_mainThreadId = -1;
            }
            else {
                DEBUG_PRINT("Failed to terminate delete callback thread: ret 0x%08x\n", result);
            }
        }
        else {
            DEBUG_PRINT("Deleting main thread\n");
            // Thead stopped cleanly, delete it
            result = sceKernelDeleteThread(thid);
            if(result >= 0) {
                DEBUG_PRINT("Main thread cleanup complete.\n");
                g_mainThreadId = -1;
            }
            else {
                DEBUG_PRINT("Failed to delete main thread: ret 0x%08x\n", result);
            }
        }
    }

    return result;
}

//
// Module Event Handlers
//

// Called during module init
int module_start(SceSize args, void *argp)
{
    DEBUG_PRINT(MODULE_NAME " v" xstr(MAJOR_VER) "." xstr(MINOR_VER) " Module Start\n");

    int result = start_main_thread();

    if(result < 0) {
        return MODULE_ERROR;
    }

    DEBUG_PRINT("Started.\n");

    return MODULE_OK;
}

// Called during module deinit
int module_stop(SceSize args, void *argp)
{
    DEBUG_PRINT("Stopping ...\n");

    int result = stop_main_thread();

    if(result < 0) {
        return MODULE_ERROR;
    }

    DEBUG_PRINT(MODULE_NAME " v" xstr(MAJOR_VER) "." xstr(MINOR_VER) " Module Stop\n");

    return MODULE_OK;
}