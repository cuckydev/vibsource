#include "VideoSys.h"

#include "MemorySys.h"

namespace VideoSys
{
	DR_MOVE dr_move;
	DR_AREA dr_area;
	DR_OFFSET dr_offset;

	VSyncCb *vsync_cb = NULL;
	VSyncCb *vsync_cb_tail = NULL;
	VSyncCb *vsync_cb_end = NULL;

	struct Buffer
	{
		struct Work
		{
			long size;
			PACKET *buffer;
			long max_used;
			PACKET *work;
			long flag;
		} *work;
		GsOT *ot;
	} buffer[2] = {};

	int buffer_i = 0;

	long width = 0;
	long height = 0;

	int frame_rate_div = 0;
	int frame_rate = 0;

	long xoff = 0;
	long yoff = 0;

	long disp_y1 = 0;

	int fnt_stream = 0;
	
	int frame_div = 0;

	bool frame_a3 = false;

	u_long logo_fade = 0;

	void FlipBuffer()
	{
		// Flip buffer index
		buffer_i = (buffer_i + 1) % 2;
	}

	void OnDrawSync()
	{
		return;
	}

	void OnVSync()
	{
		// Run VSync callbacks
		for (VSyncCb *cb = vsync_cb; cb < vsync_cb_tail; cb++)
			(*cb)();
	}

	void Init(u_short x_res, bool preserve, int div, bool a3, bool init_3d)
	{
		// Get frame rate
		frame_div = div;
		frame_a3 = a3;

		frame_rate = (GetVideoMode() == MODE_NTSC) ? 60 : 50;
		frame_rate_div = (div > 0) ? (frame_rate / div) : frame_rate;

		// Initialize GS
		SetDispMask(0);
		
		GsInitGraph(x_res, 240, preserve ? (GsOFSGPU | GsRESET3) : GsOFSGPU, 0, 0);
		GsClearDispArea(0, 0, 0);

		disp_y1 = a3 ? 256 : 0;
		GsDefDispBuff(0, 0, 0, disp_y1);

		if (init_3d)
			GsInit3D();

		SetDispMask(1);

		// Calculate screen area
		width = HWD0;
		height = VWD0;
		if (init_3d)
		{
			xoff = -width / 2;
			yoff = -height / 2;
		}
		else
		{
			xoff = 0;
			yoff = 0;
		}

		// Open debug font
		FntLoad(960, 256);
		fnt_stream = FntOpen(xoff + 0x10, yoff + 0x10, width - 0x20, height - 0x20, 0, 512);

		// Initialize ordering tables
		for (int i = 0; i < 2; i++)
		{
			GsOT *ot = new GsOT;
			ot->length = 11;
			ot->org = new GsOT_TAG[1 << 11];
			ot->offset = 0;
			buffer[i].ot = ot;

			Buffer::Work *work = new Buffer::Work;
			work->size = 0x6000;
			work->max_used = 0;
			work->flag = 0;
			work->buffer = new PACKET[0x6000 / sizeof(PACKET)];
			buffer[i].work = work;
		}
		GsClearOt(0, 0, buffer[buffer_i].ot);

		// Setup work buffer
		Buffer::Work *work = buffer[buffer_i].work;
		assert(work->flag == 0);

		work->flag = 1;
		work->work = GsGetWorkBase();
		GsSetWorkBase(work->buffer);

		// Setup draw sync callback
		DrawSyncCallback(OnDrawSync);

		// Setup vsync callback
		logo_fade = 0;

		if ((vsync_cb_end - vsync_cb) < 8)
		{
			// Allocate new array
			int current_size = vsync_cb_tail - vsync_cb;
			VSyncCb *new_cb = (VSyncCb*)MemorySys::malloc(sizeof(VSyncCb) * 8);
			memmove((u_char*)new_cb, (u_char*)vsync_cb, current_size * sizeof(VSyncCb));

			// Free old array
			if ((vsync_cb_end - vsync_cb) != 0)
				MemorySys::free(vsync_cb);

			// Update pointers
			vsync_cb_tail = new_cb + current_size;
			vsync_cb_end = new_cb + 8;
			vsync_cb = new_cb;
		}

		VSyncCallback(OnVSync);
	}

	void Quit()
	{
		printf("Packet report: max %d of %d\n", buffer[0].work->max_used, buffer[0].work->size);

		for (int i = 0; i < 2; i++)
		{
			if (buffer[i].ot != NULL)
			{
				if (buffer[i].ot->org != NULL)
					delete[] buffer[i].ot->org;
				delete buffer[i].ot;
			}
			if (buffer[i].work != NULL)
			{
				delete[] buffer[i].work->buffer;
				delete buffer[i].work;
			}
		}
	}

	void Reset()
	{
		ResetGraph(1);
		VSync(0);
		SetDispMask(0);
	}

	void Flip(bool vsync)
	{
		// Wait for drawing to finish
		if (!CheckCallback())
			DrawSync(0);

		if (logo_fade == 50)
		{
			// Fill screen with black
			GsBOXF box;
			box.attribute = GsALON;
			box.r = 0;
			box.g = 0;
			box.b = 0;
			box.x = -HWD0 / 2;
			box.y = -VWD0 / 2;
			box.w = HWD0;
			box.h = VWD0;
			GsSortBoxFill(&box, buffer[buffer_i].ot, (1 << buffer[buffer_i].ot->length) - 1);

			// Setup draw move
			RECT rc;
			rc.x = 0;
			rc.y = disp_y1 * PSDIDX;
			rc.w = HWD0;
			rc.h = VWD0;
			SetDrawMove(&dr_move, &rc, 0, disp_y1 * (PSDIDX ^ 1));

			// Link draw move
			addPrim(buffer[buffer_i].ot->org + (1 << buffer[buffer_i].ot->length) - 1, &dr_move);
		}
		else if (logo_fade > 0)
		{
			// Fill screen with sprite
			GsSPRITE sprite;
			sprite.attribute = (2 << 24); // 15bpp
			sprite.u = 0;
			sprite.v = 0;
			sprite.y = -VWD0 / 2;
			sprite.r = (logo_fade << 7) / 100;
			sprite.g = sprite.r;
			sprite.b = sprite.r;

			// Write sprite in 256 pixel columns
			for (int x = 0; x < HWD0; x += 256)
			{
				sprite.x = -HWD0 / 2 + x;
				int w = HWD0 - x;
				if (w > 0x100)
					w = 0x100;
				sprite.w = w;
				sprite.tpage = (x >> 6) + (((disp_y1 * PSDIDX) >> 8) << 4);
				GsSortFastSprite(&sprite, buffer[buffer_i].ot, (1 << buffer[buffer_i].ot->length) - 1);
			}
		}

		// Wait for vsync
		if (vsync && frame_div > 0 && !CheckCallback())
			VSync((frame_div != 1) ? frame_div : 0);

		if (frame_a3)
		{
			// Setup display environment
			GsDISPENV.disp.y = 0;
			if (PSDIDX != 0)
				GsDISPENV.disp.y = disp_y1;
			PutDispEnv(&GsDISPENV);

			// Update buffer indices
			if (++PSDCNT == 0)
				PSDCNT = 1;
			PSDIDX = (PSDIDX == 0);
			GsSetDrawBuffClip();

			// Update draw environment
			GsDRAWENV.clip.y = (PSDIDX == 0) ? disp_y1 : 0;
			GsDRAWENV.ofs[1] = VWD0 / 2;
			GsDRAWENV.ofs[1] = (PSDIDX == 0) ? (disp_y1 + GsDRAWENV.ofs[1]) : GsDRAWENV.ofs[1];

			SetDrawArea(&dr_area, &GsDRAWENV.clip);

			GsOT_TAG *tag = buffer[buffer_i].ot->org + (1 << buffer[buffer_i].ot->length) - 1;
			addPrim(tag, &dr_area);

			// Update draw offset
			SetDrawOffset(&dr_offset, (u_short*)GsDRAWENV.ofs);
			addPrim(tag, &dr_offset);
		}

		// Clear screen
		GsSortClear(0, 0, 0, buffer[buffer_i].ot);

		// Reset work
		Buffer::Work *work = buffer[buffer_i].work;
		assert(work->flag != 0);
		work->flag = 0;

		// Swap work base
		long used_bytes = ((long)GsGetWorkBase() - (long)work->buffer);
		assert(used_bytes <= work->size);
		if (work->max_used < used_bytes)
			work->max_used = used_bytes;

		GsSetWorkBase(work->work);

		// Draw to screen
		GsDrawOt(buffer[buffer_i].ot);
		DisplayFnt();

		// Swap buffers
		FlipBuffer();
		GsClearOt(0, 0, buffer[buffer_i].ot);

		// Start work
		Buffer::Work *reset_work = buffer[buffer_i].work;
		assert(reset_work->flag == 0);
		reset_work->flag = 1;

		reset_work->work = GsGetWorkBase();
		GsSetWorkBase(reset_work->buffer);

		// Move debug text down?
		WriteFnt("\n\n");
	}

	void WriteFnt(const char *a)
	{
		FntPrint(fnt_stream, "%s", a);
	}

	void DisplayFnt()
	{
		FntFlush(fnt_stream);
	}

	GsOT *GetOT()
	{
		return buffer[buffer_i].ot;
	}

	void AddVSyncCB(VSyncCb cb)
	{
		// Ensure there's enough space
		assert((vsync_cb_end - vsync_cb) > (vsync_cb_tail - vsync_cb));

		SwEnterCriticalSection();
		if (vsync_cb_tail == vsync_cb_end) // Shouldn't be possible with the assert?
		{
			// TODO
		}
		else
		{
			// Add callback
			*vsync_cb_tail++ = cb;
		}
		SwExitCriticalSection();
	}

	void RemoveVSyncCb(VSyncCb cb)
	{
		SwEnterCriticalSection();

		SwExitCriticalSection();
	}
}
