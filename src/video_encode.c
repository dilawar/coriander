/*
 * Video encoding functions for Coriander 
 * Copyright (c) 2004, Sergio Rui Silva
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * This code is based on the example code supplied with FFMPEG
 */

#include "coriander.h"

#ifdef HAVE_FFMPEG

#define STREAM_FRAME_RATE 15 /* 15 images/s */

//AVFrame *picture;
uint8_t *video_outbuf;
int video_outbuf_size;

void uyvy411_yuv411p(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height) {
  const register unsigned char *src = frame;
  register unsigned char *y = picture->data[0], 
    *u = picture->data[1], 
    *v = picture->data[2];
  register unsigned int size = width*height/4;
  while (size--) {
    *u++ = *src++; // U2
    *y++ = *src++; // Y0
    *y++ = *src++; // Y1
    *v++ = *src++; // V2
    *y++ = *src++; // Y2
    *y++ = *src++; // Y3
  };
  return;
}

void uyvy422_yuv422p(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height)
{
  const register unsigned char *src = frame;
  register unsigned char *y = picture->data[0], 
    *u = picture->data[1], 
    *v = picture->data[2];
  register unsigned int size = width*height/2;
  while (size--) {
    *u++ = *src++; // U0
    *y++ = *src++; // Y0
    *v++ = *src++; // V0
    *y++ = *src++; // Y1
  };
  return;
}

/* add a video output stream */
AVStream *add_video_stream(AVFormatContext *oc, int codec_id, int width, int height)
{
  AVCodecContext *c;
  AVStream *st;
  
  st = av_new_stream(oc, 0);
  if (!st) {
    fprintf(stderr, "Could not alloc stream\n");
    exit(1);
  }
  
  c = &st->codec;
  c->codec_id = codec_id;
  c->codec_type = CODEC_TYPE_VIDEO;
  
  /* put sample parameters */
  c->bit_rate = 4000000;
  /* resolution must be a multiple of two */
  c->width = width;  
  c->height = height;
  //c->pix_fmt = PIX_FMT_YUV422P;
  //c->dct_algo = FF_DCT_AUTO;
  //c->idct_algo = FF_IDCT_AUTO;
  //c->me_method = ME_ZERO;
  /* frames per second */
  c->frame_rate = STREAM_FRAME_RATE;  
  c->frame_rate_base = 1;
  c->gop_size = 0; /* emit one intra frame every n frames at most */
  /* just for testing, we also add B frames */
  /*
    if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
    c->max_b_frames = 2;
    }
  */
  // some formats want stream headers to be seperate
  if(!strcmp(oc->oformat->name, "mp4") || !strcmp(oc->oformat->name, "mov") || !strcmp(oc->oformat->name, "3gp"))
    c->flags |= CODEC_FLAG_GLOBAL_HEADER;
  
  return st;
}

AVFrame *alloc_picture(int pix_fmt, int width, int height) {
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;
    
    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf, 
                   pix_fmt, width, height);
    return picture;
}
    
int open_video(AVFormatContext *oc, AVStream *st) {
    AVCodec *codec;
    AVCodecContext *c;

    c = &st->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        return -1;
    }

    /* open the codec */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        return -1;
    }

    video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        video_outbuf_size = 200000;
        video_outbuf = malloc(video_outbuf_size);
    }

    /* allocate the encoded raw picture */
    /*
    picture = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!picture) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }
    */

    /* if the output format is not YUV420P, then a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    //tmp_picture = NULL;
    //if (c->pix_fmt != PIX_FMT_YUV420P) {
    /*
    tmp_picture = alloc_picture(PIX_FMT_YUV422P, c->width, c->height);
    if (!tmp_picture) {
      fprintf(stderr, "Could not allocate temporary picture\n");
      return;
    }
    */
    //}
    return 1;
}

int write_video_frame(AVFormatContext *oc, AVStream *st, AVFrame *picture) {
    int out_size, ret;
    AVCodecContext *c;
    AVPacket pkt;

    av_init_packet(&pkt);
    c = &st->codec;

    /*
    uyvy422_yuv422p(frame, tmp_picture, c->width, c->height);
    */

    /*
    avpicture_fill(tmp_picture, frame,
                   PIX_FMT_YUV422, c->width, c->height);
    */

    //picture = NULL;

    /* encode the image */
    /* if zero size, it means the image was buffered */
    /* write the compressed frame in the media file */
    /* XXX: in case of B frames, the pts is not yet valid */
    out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);
    if (out_size != 0) {
      pkt.pts= c->coded_frame->pts;
      if(c->coded_frame->key_frame)
	pkt.flags |= PKT_FLAG_KEY;
      pkt.stream_index= st->index;
      pkt.data= video_outbuf;
      pkt.size= out_size;
      //ret = av_write_frame(oc, st->index, video_outbuf, out_size);
      ret = av_write_frame(oc, &pkt);
    } else {
      ret = 0;
    }
    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        return -1;
    }
    
    return ret;
}

int close_video(AVFormatContext *oc, AVStream *st) {
    avcodec_close(&st->codec);
    /*
    av_free(picture->data[0]);
    av_free(picture);
    if (tmp_picture) {
        av_free(tmp_picture->data[0]);
        av_free(tmp_picture);
    }
    */
    av_free(video_outbuf);
    return 1;
}

/* 
 * Encode an image to JPEG using FFMPEG
 */

int jpeg_write(AVFrame *jpeg_picture, unsigned int width, unsigned int height, int fmt, char *filename, float quality, char *comment) {
  AVCodecContext *jpeg_c = NULL;
  uint8_t *jpeg_outbuf   = NULL;
  unsigned int jpeg_outbuf_size, jpeg_size;
  int jpeg_fd;
  
  jpeg_c = avcodec_alloc_context();

  //jpeg_c->bit_rate = 10000000; // heuristic...
  jpeg_c->width = width;
  jpeg_c->height = height;
  jpeg_c->pix_fmt = PIX_FMT_YUV420P;
  //jpeg_c->dct_algo = FF_DCT_AUTO;
  //jpeg_c->idct_algo = FF_IDCT_AUTO;
  /* set the quality */
  /* XXX: a parameter should be used */
  //jpeg_picture->quality = (int) 1 + FF_LAMBDA_MAX * ( (100-quality)/100.0 ); 
  jpeg_picture->quality = (int) 1+4096*((100-quality)/100.0);
  jpeg_c->flags |= CODEC_FLAG_QSCALE;
 
  //sprintf(jpeg_c->comment, comment);

  if (avcodec_open(jpeg_c, &mjpeg_encoder) < 0) {
    av_free(jpeg_c);
    return -1;
  }
  
  jpeg_outbuf_size = 3*width*height; // heuristic...
  jpeg_outbuf = av_malloc(jpeg_outbuf_size);

  jpeg_size = avcodec_encode_video(jpeg_c, jpeg_outbuf, jpeg_outbuf_size, jpeg_picture);
  if (jpeg_size > 0) {
    jpeg_fd = open(filename, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG);
    write(jpeg_fd, jpeg_outbuf, jpeg_size);
    close(jpeg_fd);
  }
  
  avcodec_close(jpeg_c);
  av_free(jpeg_outbuf);
  av_free(jpeg_c);
  
  return 1;
}


/* don't use this: FFMPEG is faster...
 
// Encode an image to JPEG using LibJPEG-MMX



int ImageToFile_JPEG(unsigned char *image_buffer, unsigned int image_width, unsigned int image_height, char *filename, char *comment) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];	// pointer to JSAMPLE row[s]
  FILE * outfile;		// target file
  unsigned int row_stride;	// physical row width in image buffer

#define JPEG_QUALITY 75

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "Can't open %s\n", filename);
    return -1;
  }

  cinfo.image_width = image_width; 	// image width and height, in pixels 
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		// # of color components per pixel 
  cinfo.in_color_space = JCS_RGB; 	// colorspace of input image 
  //cinfo.jpeg_color_space = JCS_YCbCr; // colorspace of ouput image 
  cinfo.dct_method = JDCT_FASTEST;
  //cinfo.JFIF_minor_version = 2;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE);

  jpeg_stdio_dest(&cinfo, outfile);

  jpeg_start_compress(&cinfo, TRUE);

  jpeg_write_marker(&cinfo, JPEG_COM, comment, strlen(comment));

  row_stride = image_width * 3; // JSAMPLEs per row in image_buffer
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & image_buffer[cinfo.next_scanline*row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  //gettimeofday(&tstop, NULL);
  //fprintf(stderr, "Ellapsed time = %d (us).\n", (tstop.tv_usec - tstart.tv_usec));

  fclose(outfile);
  jpeg_destroy_compress(&cinfo);

  return 1;
}
*/

#endif