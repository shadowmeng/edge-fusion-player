//#include "stdafx.h"
#include "multid3d_api.h"


#include <windows.h>
#ifdef USE_D3D9_MATH
#include "video/out/multid3d/YZDisplayOutput.h"
#include "video/out/multid3d/YZDirector3DAdapter.h"

extern "C"
{
#include "misc/dispatch.h"
#include "misc/rendezvous.h"
}

#include "osdep/threads.h"
#else
#include "YZDisplayOutput.h"
#include "YZDirector3DAdapter.h"
#endif
#include <stdio.h>


YZDirector3DAdapter *m_Director3DAdapter = NULL;
//vector<YZDisplayOutput *> m_DisplayObjects;
#ifdef USE_D3D9_MATH
static void* run_message_loop(void *w32)
{
    MSG msg;
	printf("enter run_message_loop\n");

	m_Director3DAdapter = new YZDirector3DAdapter();
	m_Director3DAdapter->Initialize();

	
	mp_rendezvous(w32, 1);
	
    while (GetMessageW(&msg, 0, 0, 0) > 0)
        DispatchMessageW(&msg);

    // Even if the message loop somehow exits, we still have to respond to
    // external requests until termination is requested.
    //while (!w32->terminate)
    //    mp_dispatch_queue_process(w32->dispatch, 1000);
    return NULL;
}
#endif


int multid3d_initialize()
{
	int test;
#ifdef USE_D3D9_MATH
	if (pthread_create(NULL, NULL, run_message_loop, &test))
        return -1;
	mp_rendezvous(&test, 0);
#else
	m_Director3DAdapter = new YZDirector3DAdapter();
	m_Director3DAdapter->Initialize();
#endif
	//sleep(20);
	return 0;
}

void multi3d_copyimg(int w, int h, int bpp, void *buf, int plane)
{
	m_Director3DAdapter->UpdateTexture(w, h, bpp, buf, plane);
}

void multi3d_draw()
{
	m_Director3DAdapter->DO_Render();
}

void multi3d_setcolormatrix(void *color)
{
	m_Director3DAdapter->DO_SetColor(color);
}

void multi3d_flip_page()
{
	m_Director3DAdapter->D3A_Present();
}

void multi3d_saveFile(const char *filename)
{
	m_Director3DAdapter->DO_SaveConfig(filename);
}

void multi3d_loadFile(const char *filename)
{
	m_Director3DAdapter->DO_LoadConfig(filename);
}