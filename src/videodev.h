/*
 * Copyright (C) 2003 Peter De Schrijver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __VIDEODEV_H__
#define __VIDEODEV_H__

typedef unsigned short u16;
typedef unsigned int u32;

struct video_capability
{
       char name[32];
       int type;
       int channels;   /* Num channels */
       int audios;     /* Num audio devices */
       int maxwidth;   /* Supported width */
       int maxheight;  /* And height */
       int minwidth;   /* Supported width */
       int minheight;  /* And height */
};

struct video_picture
{
       u16     brightness;
       u16     hue;
       u16     colour;
       u16     contrast;
       u16     whiteness;      /* Black and white only */
       u16     depth;          /* Capture depth */
       u16     palette;        /* Palette in use */
#define VIDEO_PALETTE_GREY     1       /* Linear greyscale */
#define VIDEO_PALETTE_HI240    2       /* High 240 cube (BT848) */
#define VIDEO_PALETTE_RGB565   3       /* 565 16 bit RGB */
#define VIDEO_PALETTE_RGB24    4       /* 24bit RGB */
#define VIDEO_PALETTE_RGB32    5       /* 32bit RGB */ 
#define VIDEO_PALETTE_RGB555   6       /* 555 15bit RGB */
#define VIDEO_PALETTE_YUV422   7       /* YUV422 capture */
#define VIDEO_PALETTE_YUYV     8
#define VIDEO_PALETTE_UYVY     9       /* The great thing about standards is ... */
#define VIDEO_PALETTE_YUV420   10
#define VIDEO_PALETTE_YUV411   11      /* YUV411 capture */
#define VIDEO_PALETTE_RAW      12      /* RAW capture (BT848) */
#define VIDEO_PALETTE_YUV422P  13      /* YUV 4:2:2 Planar */
#define VIDEO_PALETTE_YUV411P  14      /* YUV 4:1:1 Planar */
#define VIDEO_PALETTE_YUV420P  15      /* YUV 4:2:0 Planar */
#define VIDEO_PALETTE_YUV410P  16      /* YUV 4:1:0 Planar */
#define VIDEO_PALETTE_PLANAR   13      /* start of planar entries */
#define VIDEO_PALETTE_COMPONENT 7      /* start of component entries */
};

struct video_window
{
       u32   x,y;                    /* Position of window */
       u32   width,height;           /* Its size */
       u32   chromakey;
       u32   flags;
       struct  video_clip *clips;      /* Set only */
       int     clipcount;
#define VIDEO_WINDOW_INTERLACE  1
#define VIDEO_WINDOW_CHROMAKEY  16      /* Overlay by chromakey */
#define VIDEO_CLIP_BITMAP       -1 
/* bitmap is 1024x625, a '1' bit represents a clipped pixel */
#define VIDEO_CLIPMAP_SIZE      (128 * 625)
};

#define VIDIOCGCAP             _IOR('v',1,struct video_capability)     /* Get capabilities */
#define VIDIOCGCHAN            _IOWR('v',2,struct video_channel)       /* Get channel info (sources) */
#define VIDIOCSCHAN            _IOW('v',3,struct video_channel)        /* Set channel  */
#define VIDIOCGTUNER           _IOWR('v',4,struct video_tuner)         /* Get tuner abilities */
#define VIDIOCSTUNER           _IOW('v',5,struct video_tuner)          /* Tune the tuner for the current channel */
#define VIDIOCGPICT            _IOR('v',6,struct video_picture)        /* Get picture properties */
#define VIDIOCSPICT            _IOW('v',7,struct video_picture)        /* Set picture properties */
#define VIDIOCCAPTURE          _IOW('v',8,int)                         /* Start, end capture */
#define VIDIOCGWIN             _IOR('v',9, struct video_window)        /* Get the video overlay window */
#define VIDIOCSWIN             _IOW('v',10, struct video_window)       /* Set the video overlay window - passes clip list for hardware smarts , chromakey etc */
#define VIDIOCGFBUF            _IOR('v',11, struct video_buffer)       /* Get frame buffer */
#define VIDIOCSFBUF            _IOW('v',12, struct video_buffer)       /* Set frame buffer - root only */
#define VIDIOCKEY              _IOR('v',13, struct video_key)          /* Video key event - to dev 255 is to all - cuts capture on all DMA windows with this key (0xFFFFFFFF == all) */
#define VIDIOCGFREQ            _IOR('v',14, unsigned long)             /* Set tuner */
#define VIDIOCSFREQ            _IOW('v',15, unsigned long)             /* Set tuner */
#define VIDIOCGAUDIO           _IOR('v',16, struct video_audio)        /* Get audio info */
#define VIDIOCSAUDIO           _IOW('v',17, struct video_audio)        /* Audio source, mute etc */
#define VIDIOCSYNC             _IOW('v',18, int)                       /* Sync with mmap grabbing */
#define VIDIOCMCAPTURE         _IOW('v',19, struct video_mmap)         /* Grab frames */
#define VIDIOCGMBUF            _IOR('v',20, struct video_mbuf)         /* Memory map buffer info */
#define VIDIOCGUNIT            _IOR('v',21, struct video_unit)         /* Get attached units */
#define VIDIOCGCAPTURE         _IOR('v',22, struct video_capture)      /* Get subcapture */
#define VIDIOCSCAPTURE         _IOW('v',23, struct video_capture)      /* Set subcapture */
#define VIDIOCSPLAYMODE        _IOW('v',24, struct video_play_mode)    /* Set output video mode/feature */
#define VIDIOCSWRITEMODE       _IOW('v',25, int)                       /* Set write mode */
#define VIDIOCGPLAYINFO        _IOR('v',26, struct video_info)         /* Get current playback info from hardware */
#define VIDIOCSMICROCODE       _IOW('v',27, struct video_code)         /* Load microcode into hardware */
#define VIDIOCGVBIFMT          _IOR('v',28, struct vbi_format)         /* Get VBI information */
#define VIDIOCSVBIFMT          _IOW('v',29, struct vbi_format)         /* Set VBI information */


#endif // __VIDEODEV_H__
