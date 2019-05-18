#pragma once

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mx_board.h"

#include "applibs_versions.h"
#include "epoll_timerfd_utilities.h"
//
#include <applibs/gpio.h>
#include <applibs/log.h>
//
#include "mt3620_rdb.h"
#include "mx_board.h"
#include "mx_draw.h"

int MX_GpioInit(void);
void MX_GpioDeInit(void);
void MX_RenderMatrix(void);
