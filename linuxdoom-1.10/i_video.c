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
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscalls/proc.h>
#include <syscalls/filesystem.h>
#include <syscalls/syscall_generic.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>
// #include <unistd.h>
//
// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
// #include <X11/keysym.h>
//
// #include <X11/extensions/XShm.h>
// // Had to dig up XShm.c for this one.
// // It is in the libXext, but not in the XFree86 headers.
// #ifdef LINUX
// int XShmGetEventBase(Display *dpy); // problems with g++?
// #endif

// #include <sys/socket.h>
// #include <sys/time.h>
// #include <sys/types.h>

// #include <errnos.h>
// #include <netinet/in.h>
// #include <signal.h>

#include "d_main.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"

#include "doomdef.h"

#define POINTER_WARP_COUNTDOWN 1

// Display *X_display = 0;
// Window X_mainWindow;
// Colormap X_cmap;
// Visual *X_visual;
// GC X_gc;
struct InputEvent input_event;
int input_event_fd = -1;
// int X_screen;
FILE *fb_file = NULL;
int fb_width;
int fb_height;
uint8_t *image_data_256;
uint8_t *color_palette;
uint8_t *image_data_row;

// MIT SHared Memory extension.
boolean doShm;

int cursorx = 0;
int cursory = 0;

// XShmSegmentInfo X_shminfo;
// int X_shmeventtype;

//
//  Translates the key currently in X_event
//

// based on raw event codes from linux
int translateKeyboardCode(int keycode) {
    int rc;

    switch (keycode) {
    case 105:
        rc = KEY_LEFTARROW;
        break;
    case 106:
        rc = KEY_RIGHTARROW;
        break;
    case 108:
        rc = KEY_DOWNARROW;
        break;
    case 103:
        rc = KEY_UPARROW;
        break;
    case 1:
        rc = KEY_ESCAPE;
        break;
    case 28:
        rc = KEY_ENTER;
        break;
    case 15:
        rc = KEY_TAB;
        break;
    case 59:
        rc = KEY_F1;
        break;
    case 60:
        rc = KEY_F2;
        break;
    case 61:
        rc = KEY_F3;
        break;
    case 62:
        rc = KEY_F4;
        break;
    case 63:
        rc = KEY_F5;
        break;
    case 64:
        rc = KEY_F6;
        break;
    case 65:
        rc = KEY_F7;
        break;
    case 66:
        rc = KEY_F8;
        break;
    case 67:
        rc = KEY_F9;
        break;
    case 68:
        rc = KEY_F10;
        break;
    case 87:
        rc = KEY_F11;
        break;
    case 88:
        rc = KEY_F12;
        break;
    case 14:
    case 111:
        rc = KEY_BACKSPACE;
        break;
    case 119:
        rc = KEY_PAUSE;
        break;
    case 13:
    case 117:
        rc = KEY_EQUALS;
        break;
    case 74:
    case 118:
        rc = KEY_MINUS;
        break;
    case 42:
    case 54:
        rc = KEY_RSHIFT;
        break;
    case 29:
    case 97:
        rc = KEY_RCTRL;
        break;
    case 56:
    case 125:
    case 100:
    case 126:
        rc = KEY_RALT;
        break;

    // numbers
    case 2:
        rc = '1';
        break;
    case 3:
        rc = '2';
        break;
    case 4:
        rc = '3';
        break;
    case 5:
        rc = '4';
        break;
    case 6:
        rc = '5';
        break;
    case 7:
        rc = '6';
        break;
    case 8:
        rc = '7';
        break;
    case 9:
        rc = '8';
        break;
    case 10:
        rc = '9';
        break;
    case 11:
        rc = '0';
        break;

    // letters
    case 16:
        rc = 'q';
        break;
    case 17:
        rc = 'w';
        break;
    case 18:
        rc = 'e';
        break;
    case 19:
        rc = 'r';
        break;
    case 20:
        rc = 't';
        break;
    case 21:
        rc = 'y';
        break;
    case 22:
        rc = 'u';
        break;
    case 23:
        rc = 'i';
        break;
    case 24:
        rc = 'o';
        break;
    case 25:
        rc = 'p';
        break;
    case 30:
        rc = 'a';
        break;
    case 31:
        rc = 's';
        break;
    case 32:
        rc = 'd';
        break;
    case 33:
        rc = 'f';
        break;
    case 34:
        rc = 'g';
        break;
    case 35:
        rc = 'h';
        break;
    case 36:
        rc = 'j';
        break;
    case 37:
        rc = 'k';
        break;
    case 38:
        rc = 'l';
        break;
    case 44:
        rc = 'z';
        break;
    case 45:
        rc = 'x';
        break;
    case 46:
        rc = 'c';
        break;
    case 47:
        rc = 'v';
        break;
    case 48:
        rc = 'b';
        break;
    case 49:
        rc = 'n';
        break;
    case 50:
        rc = 'm';
        break;
    default:
        rc = 0;
        break;
    }

    return rc;
}

void I_ShutdownGraphics(void) {}

static struct InputEvent tmp_input_event;
void NextEventNonBlocking(struct InputEvent *event) {
    if (input_event_fd < 0) {
        I_Error("Input event file descriptor not initialized");
    }

    if (event->generated_at.seconds != 0 || event->generated_at.nanoseconds != 0) {
        return; //tmp is filled
    }
    _fread(input_event_fd, sizeof(struct InputEvent), event, 1);
}

void NextEventBlocking(struct InputEvent *event) { 
    if (input_event_fd < 0) {
        I_Error("Input event file descriptor not initialized");
    }

    if (tmp_input_event.generated_at.seconds != 0 || tmp_input_event.generated_at.nanoseconds != 0) {
        *event = tmp_input_event;
        tmp_input_event.generated_at.seconds = 0;
        tmp_input_event.generated_at.nanoseconds = 0;
        return; //tmp is filled
    }
    _fread(input_event_fd, sizeof(struct InputEvent), event, 0); //without nonblocking flag
}

boolean EventPending() {
    NextEventNonBlocking(&tmp_input_event);
    return tmp_input_event.generated_at.seconds != 0 || tmp_input_event.generated_at.nanoseconds != 0;
}

//
// I_StartFrame
//
void I_StartFrame(void) {
    // er?
}

static int mouse_state = 0;
boolean mousemoved = false;
boolean shmFinished;

void I_GetEvent(void) {

    event_t event;

    // put event-grabbing stuff in here
    NextEventBlocking(&input_event);
    switch (input_event.device_type) {
    case InputeventDeviceType_Keyboard:
        event.data1 = translateKeyboardCode(input_event.code);
        event.type = input_event.value ? ev_keydown : ev_keyup;
        D_PostEvent(&event);
        break;
    // case KeyPress:
    //     event.type = ev_keydown;
    //     event.data1 = xlatekey();
    //     D_PostEvent(&event);
    //     // fprintf(stderr, "k");
    //     break;
    // case KeyRelease:
    //     event.type = ev_keyup;
    //     event.data1 = xlatekey();
    //     D_PostEvent(&event);
    //     // fprintf(stderr, "ku");
    //     break;
    case InputeventDeviceType_Mouse:
        event.type = ev_mouse;
        int code = input_event.code;
        if (code == 0x110) { // left button
            if (input_event.value) {
                mouse_state |= 1;
                event.data1 = mouse_state;
            } else {
                // set before clearing bit as in original impl
                event.data1 = mouse_state;
                mouse_state &= ~1;
            }
            event.data2 = 0;
            event.data3 = 0;
            D_PostEvent(&event);
            break;
        }
        if (code == 0x111) { // right button
            if (input_event.value) {
                mouse_state |= 4;
                event.data1 = mouse_state;
            } else {
                // set before clearing bit as in original impl
                event.data1 = mouse_state;
                mouse_state &= ~4;
            }
            event.data2 = 0;
            event.data3 = 0;
            D_PostEvent(&event);
            break;
        }
        if (code == 0x112) { // middle button
            if (input_event.value) {
                mouse_state |= 2;
                event.data1 = mouse_state;
            } else {
                // set before clearing bit as in original impl
                event.data1 = mouse_state;
                mouse_state &= ~2;
            }
            event.data2 = 0;
            event.data3 = 0;
            D_PostEvent(&event);
            break;
        }
        // case ButtonPress:
        //     event.type = ev_mouse;
        //     event.data1 = (X_event.xbutton.state & Button1Mask) |
        //                   (X_event.xbutton.state & Button2Mask ? 2 : 0) |
        //                   (X_event.xbutton.state & Button3Mask ? 4 : 0) |
        //                   (X_event.xbutton.button == Button1) |
        //                   (X_event.xbutton.button == Button2 ? 2 : 0) |
        //                   (X_event.xbutton.button == Button3 ? 4 : 0);
        //     event.data2 = event.data3 = 0;
        //     D_PostEvent(&event);
        //     // fprintf(stderr, "b");
        //     break;
        // case ButtonRelease:
        //     event.type = ev_mouse;
        //     event.data1 = (X_event.xbutton.state & Button1Mask) |
        //                   (X_event.xbutton.state & Button2Mask ? 2 : 0) |
        //                   (X_event.xbutton.state & Button3Mask ? 4 : 0);
        //     // suggest parentheses around arithmetic in operand of |
        //     event.data1 = event.data1 ^
        //                   (X_event.xbutton.button == Button1 ? 1 : 0) ^
        //                   (X_event.xbutton.button == Button2 ? 2 : 0) ^
        //                   (X_event.xbutton.button == Button3 ? 4 : 0);
        //     event.data2 = event.data3 = 0;
        //     D_PostEvent(&event);
        //     // fprintf(stderr, "bu");
        //     break;
        //
        if (code == 0x0) { // x movement
            event.type = ev_mouse;
            event.data1 = mouse_state;
            event.data2 = input_event.value << 2;
            event.data3 = 0;
            D_PostEvent(&event);
            break;
        }
        if (code == 0x1) { // y movement
            event.type = ev_mouse;
            event.data1 = mouse_state;
            event.data2 = 0;
            event.data3 = input_event.value << 2;
            D_PostEvent(&event);
            break;
        }
        // case MotionNotify:
        //     event.type = ev_mouse;
        //     event.data1 = (X_event.xmotion.state & Button1Mask) |
        //                   (X_event.xmotion.state & Button2Mask ? 2 : 0) |
        //                   (X_event.xmotion.state & Button3Mask ? 4 : 0);
        //     event.data2 = (X_event.xmotion.x - lastmousex) << 2;
        //     event.data3 = (lastmousey - X_event.xmotion.y) << 2;
        //
        //     if (event.data2 || event.data3) {
        //         lastmousex = X_event.xmotion.x;
        //         lastmousey = X_event.xmotion.y;
        //         if (X_event.xmotion.x != X_width / 2 &&
        //             X_event.xmotion.y != X_height / 2) {
        //             D_PostEvent(&event);
        //             // fprintf(stderr, "m");
        //             mousemoved = false;
        //         } else {
        //             mousemoved = true;
        //         }
        //     }
        //     break;

    default:
        break;
    }
}

//
// I_StartTic
//
void I_StartTic(void) {
    while (EventPending())
        I_GetEvent();

    mousemoved = false;
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void) {
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate(void) {

    static int lasttic;
    int tics;
    int i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm) {

        i = I_GetTime();
        tics = i - lasttic;
        lasttic = i;
        if (tics > 20)
            tics = 20;

        for (i = 0; i < tics * 2; i += 2)
            screens[0][(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0xff;
        for (; i < 20 * 2; i += 2)
            screens[0][(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0x0;
    }

    //screens[0] == image_data_256
    for (i = 0; i < SCREENHEIGHT; i++) {
        for (int j = 0; j < SCREENWIDTH; j++) {
            uint8_t color_index = screens[0][i * SCREENWIDTH + j];
            uint8_t r = color_palette[color_index * 3];
            uint8_t g = color_palette[color_index * 3 + 1];
            uint8_t b = color_palette[color_index * 3 + 2];
            image_data_row[j * 3 + 0] = r;
            image_data_row[j * 3 + 1] = g;
            image_data_row[j * 3 + 2] = b;
        }

        fseek(fb_file, i * fb_width, SEEK_SET);
        fwrite(image_data_row, SCREENWIDTH * 3, 1, fb_file);
        
    }
    fflush(fb_file);
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr) {
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

void I_InitGraphics(void) {
    static int firsttime = 1;

    if (!firsttime)
        return;
    firsttime = 0;

    char *events_file = "/input/all";
    //flags: read
    input_event_fd = _fopen(strlen(events_file), events_file, -1, 1, -1);
    if (input_event_fd < 0) {
        I_Error("Could not open input event file %s", events_file);
    }

    fb_file = fopen("/fb", "+");
    if (!fb_file) {
        I_Error("Could not open framebuffer file /fb");
    }

    if (fread(&fb_width, sizeof(fb_width), 1, fb_file) != 1) {
        I_Error("Could not read framebuffer width");
    }

    if (fread(&fb_height, sizeof(fb_height), 1, fb_file) != 1) {
        I_Error("Could not read framebuffer height");
    }

    image_data_256 = malloc(SCREENWIDTH * SCREENHEIGHT);
    if (!image_data_256) {
        I_Error("Could not allocate memory for image_data_256");
    }
    image_data_row = malloc(SCREENWIDTH * 3);
    if (!image_data_row) {
        I_Error("Could not allocate memory for image_data_row");
    }

    screens[0] = (unsigned char *)(image_data_256);
}

unsigned exptable[256];

void InitExpand(void) {
    int i;

    for (i = 0; i < 256; i++)
        exptable[i] = i | (i << 8) | (i << 16) | (i << 24);
}

double exptable2[256 * 256];

void InitExpand2(void) {
    int i;
    int j;
    // UNUSED unsigned	iexp, jexp;
    double *exp;
    union {
        double d;
        unsigned u[2];
    } pixel;

    printf("building exptable2...\n");
    exp = exptable2;
    for (i = 0; i < 256; i++) {
        pixel.u[0] = i | (i << 8) | (i << 16) | (i << 24);
        for (j = 0; j < 256; j++) {
            pixel.u[1] = j | (j << 8) | (j << 16) | (j << 24);
            *exp++ = pixel.d;
        }
    }
    printf("done.\n");
}
