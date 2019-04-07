#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "applibs_versions.h"
#include "epoll_timerfd_utilities.h"

#include <applibs/gpio.h>
#include <applibs/log.h>

#include "mt3620_rdb.h"
#include "mx_draw.h"

static uint8_t colorid;
static mx_color_t c = { 0, 0, 0 };

static int power = 1;
static int row = 0;

const struct timespec pollPanelCheckPeriod = { 0, 300000 };
const struct timespec dly = { 0, 0 };

static int epollFd = -1;
static int pollPanelTimerFd = -1;
static int r1Fd = -1;
static int g1Fd = -1;
static int b1Fd = -1;
static int r2Fd = -1;
static int g2Fd = -1;
static int b2Fd = -1;
static int aFd = -1;
static int bFd = -1;
static int cFd = -1;
static int dFd = -1;
static int clkFd = -1;
static int stbFd = -1;
static int oeFd = -1;

static volatile sig_atomic_t terminationRequired = false;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
void TerminationHandler(int signalNumber)
{
	terminationRequired = true;
}

void DrawScanLine(void)
{
	const struct timespec delay = { 1, 0 };
	
	mx_color_t colors[] = {
		MX_COLOR_BLACK,
		MX_COLOR_RED,
		MX_COLOR_GREEN,
		MX_COLOR_BLUE,
		MX_COLOR_YELLOW,
		MX_COLOR_CYAN,
		MX_COLOR_PINK,
		MX_COLOR_WHITE
	};

	for (int i = 0; i < 8; i++)
	{
		for (int row = 0; row < MX_PANEL_ROWS; row++)
		{
			for (int col = 0; col < MX_PANEL_COLS; col++)
				MX_DrawPixel(row, col, colors[i]);
		}
		nanosleep(&delay, NULL);
	}
}

void PollPanel(void)
{
	// reset row value
	if (row >= MX_PANEL_SCANLINE)
		row = 0;

	// Set output disabled
	GPIO_SetValue(oeFd, GPIO_Value_High);

	// Set the row address
	GPIO_SetValue(aFd, (row & 0x01) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
	GPIO_SetValue(bFd, (row & 0x02) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
	GPIO_SetValue(cFd, (row & 0x04) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
	GPIO_SetValue(dFd, (row & 0x08) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);

	// Set latch low signal
	GPIO_SetValue(stbFd, GPIO_Value_Low);

	// Set the subrow
	int subrow = row + MX_PANEL_SCANLINE;

	for (int col = 0; col < MX_PANEL_COLS; col++) { // clock in row of data at bit level

		//Set color data
		GPIO_SetValue(r1Fd, (frame[row][col].red) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
		GPIO_SetValue(g1Fd, (frame[row][col].green) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
		GPIO_SetValue(b1Fd, (frame[row][col].blue) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
		GPIO_SetValue(r2Fd, (frame[subrow][col].red) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
		GPIO_SetValue(g2Fd, (frame[subrow][col].green) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);
		GPIO_SetValue(b2Fd, (frame[subrow][col].blue) == 0x00 ? GPIO_Value_Low : GPIO_Value_High);

		// Set clock signal
		GPIO_SetValue(clkFd, GPIO_Value_Low);
		GPIO_SetValue(clkFd, GPIO_Value_High);
	}

	// Set latch high signal
	GPIO_SetValue(stbFd, GPIO_Value_High);

	// Set output enabled
	if (power)
		GPIO_SetValue(oeFd, GPIO_Value_Low);

	// Set delay
	nanosleep(&dly, NULL);

	// Increment row value
	row++;
}

void PollPanelTimerEventHandler(EventData *eventData)
{
	//Log_Debug("Hit timer.\n");

	if (ConsumeTimerFdEvent(pollPanelTimerFd) != 0) {
		terminationRequired = true;
		return;
	}

	PollPanel();
}

int SystemInitialized(void)
{
	// Register a SIGTERM handler for termination requests
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = TerminationHandler;
	sigaction(SIGTERM, &action, NULL);

	// Set up a timer to poll matrix panel
	static EventData pollPanelTimerEventData = { .eventHandler = &PollPanelTimerEventHandler };

	epollFd = CreateEpollFd();
	if (epollFd < 0) {
		return -1;
	}

	pollPanelTimerFd = CreateTimerFdAndAddToEpoll(epollFd, &pollPanelCheckPeriod, &pollPanelTimerEventData, EPOLLIN);
	if (pollPanelTimerFd < 0) {
		return -1;
	}

	// Initialize GPIO
	r1Fd = GPIO_OpenAsOutput(MX_GPIO_R1_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	g1Fd = GPIO_OpenAsOutput(MX_GPIO_G1_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	b1Fd = GPIO_OpenAsOutput(MX_GPIO_B1_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	r2Fd = GPIO_OpenAsOutput(MX_GPIO_R2_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	g2Fd = GPIO_OpenAsOutput(MX_GPIO_G2_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	b2Fd = GPIO_OpenAsOutput(MX_GPIO_B2_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	aFd = GPIO_OpenAsOutput(MX_GPIO_A_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	bFd = GPIO_OpenAsOutput(MX_GPIO_B_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	cFd = GPIO_OpenAsOutput(MX_GPIO_C_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	dFd = GPIO_OpenAsOutput(MX_GPIO_D_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	clkFd = GPIO_OpenAsOutput(MX_GPIO_CLK_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	stbFd = GPIO_OpenAsOutput(MX_GPIO_STB_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);
	oeFd = GPIO_OpenAsOutput(MX_GPIO_OE_NUM, GPIO_OutputMode_PushPull, GPIO_Value_Low);

	GPIO_SetValue(oeFd, GPIO_Value_High);

	return 0;
}

void SystemDeinitialize(void)
{
	CloseFdAndPrintError(epollFd, "epoll");
	CloseFdAndPrintError(pollPanelTimerFd, "pollpaneltimer");
	CloseFdAndPrintError(r1Fd, "r1");
	CloseFdAndPrintError(g1Fd, "g1");
	CloseFdAndPrintError(b1Fd, "b1");
	CloseFdAndPrintError(r2Fd, "r2");
	CloseFdAndPrintError(g2Fd, "g2");
	CloseFdAndPrintError(b2Fd, "b2");
	CloseFdAndPrintError(aFd, "a");
	CloseFdAndPrintError(bFd, "b");
	CloseFdAndPrintError(cFd, "c");
	CloseFdAndPrintError(dFd, "d");
	CloseFdAndPrintError(clkFd, "clk");
	CloseFdAndPrintError(clkFd, "stb");
	CloseFdAndPrintError(clkFd, "oe");

}

void Loop(void) {
	static uint8_t x = 0;
	static uint8_t y = 0;

	MX_DrawPixel(x, y, c);

	if (++x == MX_PANEL_ROWS) {
		x = 0;
		if (++y == MX_PANEL_COLS) {
			y = 0;

			colorid++;

			if (colorid == 1)
				c = MX_COLOR_BLACK;
			if (colorid == 2)
				c = MX_COLOR_RED;
			if (colorid == 3)
				c = MX_COLOR_GREEN;
			if (colorid == 4)
				c = MX_COLOR_BLUE;
			if (colorid == 5)
				c = MX_COLOR_YELLOW;
			if (colorid == 6)
				c = MX_COLOR_CYAN;
			if (colorid == 7)
				c = MX_COLOR_PINK;
			if (colorid == 8)
				c = MX_COLOR_WHITE;
			if (colorid == 9)
				colorid = 0;
		}
	}
}

/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
	Log_Debug("Matrix display starting...\n");

	if (SystemInitialized() != 0) {
		terminationRequired = true;
	}

	MX_DisplayTest();
	//MX_DisplayTestPattern();

	// Use epoll to wait for events and trigger handlers, until an error or SIGTERM happens
	while (!terminationRequired) {
		if (WaitForEventAndCallHandler(epollFd) != 0) {
			terminationRequired = true;
		}
		//Loop();
	}

	SystemDeinitialize();

	Log_Debug("Matrix display exiting.\n");
	return 0;
}
