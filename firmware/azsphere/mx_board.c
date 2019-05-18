#include "mx_board.h"
#include "mx_draw.h"
#include "delay.h"

static int power = 1;
static int row = 0;

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

int MX_GpioInit(void)
{
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

void MX_GpioDeInit(void)
{
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

void MX_RenderMatrix(void)
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
	usleep(0);

	// Increment row value
	row++;
}