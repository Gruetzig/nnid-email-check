#include <3ds.h>
#include <stdio.h>

int main () {
    gfxInitDefault();

    consoleInit(GFX_TOP, NULL);
    printf("NNID Email check\n");
    actInit(false);
    Handle sharedmemHandle;
    char __attribute__((aligned(0x1000))) sharedmemBuffer[0x1000] = { 0 };
    svcCreateMemoryBlock(&sharedmemHandle, (u32)sharedmemBuffer, sizeof(sharedmemBuffer), 0, MEMPERM_READWRITE);
    Result rc = ACT_Initialize(0xFFFFFFFF, sizeof(sharedmemBuffer), sharedmemHandle);

    char accountID[0x11];
    char mailBuffer[0x101];

    Handle completionEvent;
    svcCreateEvent(&completionEvent, RESET_ONESHOT);

    rc = ACT_GetAccountInfo(accountID, 0x11, 0xFE, 0x8); //https://www.3dbrew.org/wiki/ACT_Services#DataBlocks
    if (R_FAILED(rc)) {
        printf("GetAccountInfo failed: %08lx", rc);
        goto loop;
    } 
    rc = ACT_AcquireAccountInfo(0xFE, INFO_TYPE_MAIL_ADDRESS, completionEvent);
    if (R_FAILED(rc)) {
        printf("AcquireAccountInfo failed: %08lx", rc);
        goto loop;
    } 

    rc = svcWaitSynchronization(completionEvent, 60000000000LL);
    if (R_FAILED(rc) || (R_SUCCEEDED(rc) && R_DESCRIPTION(rc) == RD_TIMEOUT)) {
        printf("timeout %08lX", rc);
        goto loop;
    }

    u32 rs;
    rc = ACT_GetAsyncResult(&rs, mailBuffer, sizeof(mailBuffer), 5); //https://www.3dbrew.org/wiki/ACTU:GetAsyncResult
    if (R_FAILED(rc)) {
        printf("GetAsyncResult failed: %08lx", rc);
        goto loop;
    }

    printf("ID: %s\nE-Mail Address: %s\n", accountID, mailBuffer);

loop:
    svcCloseHandle(sharedmemHandle);
    svcCloseHandle(completionEvent);
    actExit();

    printf("Press start to exit");
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) break;
    }


    gfxExit();
    return 0;
}