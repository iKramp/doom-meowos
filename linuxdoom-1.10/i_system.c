// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <syscalls/proc.h> //provides _time()

#include "doomdef.h"
#include "i_sound.h"
#include "i_video.h"
#include "m_misc.h"

#include "d_net.h"
#include "g_game.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

int mb_used = 6;

void I_Tactile(int on, int off, int total) {
    // UNUSED.
    on = off = total = 0;
}

ticcmd_t emptycmd;
ticcmd_t *I_BaseTiccmd(void) { return &emptycmd; }

int I_GetHeapSize(void) { return mb_used * 1024 * 1024; }

byte *I_ZoneBase(int *size) {
    *size = mb_used * 1024 * 1024;
    return (byte *)malloc(*size);
}

//
// I_GetTime
// returns time in 1/70th second tics
//
int I_GetTime(void) {
    struct TimeInfo time_info;
    static int basetime = 0;

    _time(&time_info);
    struct Duration uepoch = time_info.time_since_epoch;

    if (!basetime) {
        basetime = uepoch.seconds;
    }

    int newtics = (uepoch.seconds - basetime) * TICRATE + uepoch.nanoseconds * TICRATE / 1000000000;
    return newtics;
}

//
// I_Init
//
void I_Init(void) {
#ifndef NOSOUND
    I_InitSound();
#endif
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit(void) {
#ifndef NONET
    D_QuitNetGame();
#endif
#ifndef NOSOUND
    I_ShutdownSound();
    I_ShutdownMusic();
#endif
    M_SaveDefaults();
    I_ShutdownGraphics();
    exit(0);
}

void I_WaitVBL(int count) {
#ifdef SGI
    sginap(1);
#else
#ifdef SUN
    sleep(0);
#else
    uint64_t sleep_nanoseconds = count * (1000000000 / 70);
    uint64_t sleep_seconds = sleep_nanoseconds / 1000000000;
    uint64_t sleep_remaining_nanoseconds = sleep_nanoseconds % 1000000000;
    _sleep(sleep_seconds, sleep_remaining_nanoseconds);
#endif
#endif
}

void I_BeginRead(void) {}

void I_EndRead(void) {}

byte *I_AllocLow(int length) {
    byte *mem;

    mem = (byte *)malloc(length);
    memset(mem, 0, length);
    return mem;
}

//
// I_Error
//
extern boolean demorecording;

void I_Error(char *error, ...) {
    va_list argptr;

    // Message first.
    va_start(argptr, error);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, error, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);

    fflush(stderr);

    // Shutdown. Here might be other errors.
    if (demorecording)
        G_CheckDemoStatus();

#ifndef NONET
    D_QuitNetGame();
#endif
    I_ShutdownGraphics();

    exit(-1);
}
