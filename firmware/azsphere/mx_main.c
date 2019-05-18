
#include "mx_board.h"

static uint8_t colorid;
static mx_color_t c = { 0, 0, 0 };

const struct timespec pollPanelCheckPeriod = { 0, 300000 };

static int epollFd = -1;
static int pollPanelTimerFd = -1;

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

void PollPanelTimerEventHandler(EventData *eventData)
{
	//Log_Debug("Hit timer.\n");

	if (ConsumeTimerFdEvent(pollPanelTimerFd) != 0) {
		terminationRequired = true;
		return;
	}
	MX_RenderMatrix();
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

	MX_GpioInit();

	return 0;
}

void SystemDeinitialize(void)
{
	CloseFdAndPrintError(epollFd, "epoll");
	CloseFdAndPrintError(pollPanelTimerFd, "pollpaneltimer");

	MX_GpioDeInit();
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
	MX_DisplayTestPattern();

	// Use epoll to wait for events and trigger handlers, until an error or SIGTERM happens
	while (!terminationRequired) {
		if (WaitForEventAndCallHandler(epollFd) != 0) {
			terminationRequired = true;
		}
		Loop();
	}

	SystemDeinitialize();

	Log_Debug("Matrix display exiting.\n");
	return 0;
}
