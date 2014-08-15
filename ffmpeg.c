#include "quakedef.h"
#include "thread.h"
#include "sys_lib.h"
#include "qsound.h"

#ifdef _WIN32
#define inline _inline
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"


// not cool
typedef struct AVCodecTag
{
	enum CodecID id;
	unsigned int tag;
} AVCodecTag;


#ifndef _WIN32
#include <pthread.h>
#endif

#define avformat_alloc_context "avformat_alloc_context"
#if LIBAVFORMAT_VERSION_MAJOR == 52
	#if LIBAVFORMAT_VERSION_MINOR == 25
		#if LIBAVFORMAT_VERSION_MICRO == 0
			#define avformat_alloc_context "av_alloc_format_context"
		#endif
	#endif
#endif



#ifdef GLQUAKE
extern int				glx, gly, glwidth, glheight;
#include "gl_model.h"
#include "gl_local.h"
#endif


#define Q_ROUND_POWER2(in, out) {						\
	_mathlib_temp_int1 = in;							\
	for (out = 1; out < _mathlib_temp_int1; out <<= 1)	\
	;												\
}


/*
 * Dynamic loading
 */

/* libavcodec */
void		(*ffm_avcodec_init)			(void);
void		(*ffm_avcodec_register_all)		(void);
int 		(*ffm_avcodec_encode_video)		(AVCodecContext *avctx, uint8_t *buf, int buf_size, const AVFrame *pict);
int		(*ffm_avcodec_encode_audio)			(AVCodecContext *avctx, uint8_t *buf, int buf_size, const short *samples);
AVCodec *	(*ffm_avcodec_find_encoder_by_name)	(const char *name);
AVCodec *	(*ffm_av_codec_next)			(AVCodec *c);
AVFrame *	(*ffm_avcodec_alloc_frame)		(void);
int		(*ffm_avpicture_alloc)			(AVPicture *picture, int pix_fmt, int width, int height);
int		(*ffm_avcodec_open)			(AVCodecContext *avctx, AVCodec *codec);
int 		(*ffm_avcodec_close)			(AVCodecContext *avctx);
AVCodec *	(*ffm_avcodec_find_encoder)		(enum CodecID id);
int		(*ffm_avpicture_get_size)		(int pix_fmt, int width, int height);
int		(*ffm_avpicture_fill)			(AVPicture *picture, uint8_t *ptr, int pix_fmt, int width, int height);

/* libswsscale */
int 		(*ffm_sws_scale)				(struct SwsContext *context, uint8_t* srcSlice[], int srcStride[], int srcSliceY, int srcSliceH, uint8_t* dst[], int dstStride[]);
struct SwsContext *(*ffm_sws_getContext)				(int srcW, int srcH, enum PixelFormat srcFormat, int dstW, int dstH, enum PixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, double *param);


/* libavformat */
AVOutputFormat *	(*ffm_av_oformat_next)			(AVOutputFormat *f);
int 			(*ffm_url_fopen)			(ByteIOContext **s, const char *filename, int flags);
int 			(*ffm_url_fclose)			(ByteIOContext *s);
void			(*ffm_av_register_all)			(void);
int			(*ffm_av_write_frame)			(AVFormatContext *s, AVPacket *pkt);
int			(*ffm_av_write_header)			(AVFormatContext *s);
void			(*ffm_av_init_packet)			(AVPacket *pkt);
int			(*ffm_av_set_parameters)		(AVFormatContext *s, AVFormatParameters *ap);
AVFormatContext *	(*ffm_avformat_alloc_context)		(void);
void 			(*ffm_dump_format)			(AVFormatContext *ic, int index, const char *url, int is_output);
int			(*ffm_av_write_trailer)			(AVFormatContext *s);
AVStream *		(*ffm_av_new_stream)			(AVFormatContext *s, int id);
int 			(*ffm_av_find_default_stream_index)	(AVFormatContext *s);
int			(*ffm_av_interleaved_write_frame)		(AVFormatContext *s, AVPacket *pkt);
enum CodecID           (*ffm_av_codec_get_id)                  (const struct AVCodecTag **tags, unsigned int tag);
unsigned int           (*ffm_av_codec_get_tag)                 (const struct AVCodecTag **tags, enum CodecID id);

/* libavutil */
void 			(*ffm_av_log_set_callback)	(void (*)(void*, int, const char*, va_list));
int64_t 	(*ffm_av_rescale_q)			(int64_t a, AVRational bq, AVRational cq);


/* libavcodec */
struct lib_functions libavcodec_functions[] =
	{
	{	"avcodec_init",	(void *)&ffm_avcodec_init},
	{	"avcodec_register_all", (void *)&ffm_avcodec_register_all},
	{	"avcodec_encode_video", (void *)&ffm_avcodec_encode_video},
	{	"avcodec_encode_audio", (void *)&ffm_avcodec_encode_audio},
	{	"avcodec_find_encoder_by_name", (void *)&ffm_avcodec_find_encoder_by_name},
	{	"av_codec_next", (void *)&ffm_av_codec_next},
	{	"avcodec_alloc_frame", (void *)&ffm_avcodec_alloc_frame},
	{	"avcodec_open", (void *)&ffm_avcodec_open},
	{	"avcodec_close", (void *)&ffm_avcodec_close},
	{	"avcodec_find_encoder", (void *)&ffm_avcodec_find_encoder},
	{	"avpicture_get_size", (void *)&ffm_avpicture_get_size},
	{	"avpicture_fill", (void *)&ffm_avpicture_fill},
	{	"avpicture_alloc", (void *)&ffm_avpicture_alloc},
	{NULL}
	};


/* libswsscale */
struct lib_functions libswscale_functions[] =
	{
	{	"sws_scale", (void *)&ffm_sws_scale},
	{	"sws_getContext", (void *)&ffm_sws_getContext},
	{NULL}
	};

/* libavformat */
struct lib_functions libavformat_functions[] =
	{
	{	"av_oformat_next", (void *)&ffm_av_oformat_next},
	{	"url_fopen", (void *)&ffm_url_fopen},
	{	"url_fclose", (void *)&ffm_url_fclose},
	{	"av_register_all", (void *)&ffm_av_register_all},
	{	"av_write_frame", (void *)&ffm_av_write_frame},
	{	"av_write_header", (void *)&ffm_av_write_header},
	{	"av_init_packet", (void *)&ffm_av_init_packet},
	{	"av_set_parameters", (void *)&ffm_av_set_parameters},
	{	"av_alloc_format_context", (void *)&ffm_avformat_alloc_context},
	{	"dump_format", (void *)&ffm_dump_format},
	{	"av_write_trailer", (void *)&ffm_av_write_trailer},
	{	"av_new_stream", (void *)&ffm_av_new_stream},
	{	"av_find_default_stream_index", (void *)&ffm_av_find_default_stream_index},
	{	"av_interleaved_write_frame", (void *)&ffm_av_interleaved_write_frame},
	{	"av_codec_get_id", (void *)&ffm_av_codec_get_id},
	{	"av_codec_get_tag", (void *)&ffm_av_codec_get_tag},
	{NULL}
	};


/* libavutil */
struct lib_functions libavutil_functions[] =
	{
	{	"av_log_set_callback", (void *)&ffm_av_log_set_callback},
	{	"av_rescale_q", (void *)&ffm_av_rescale_q},
	{NULL}
	};

struct libs
{
	char *name;
	struct lib_functions *functions;
};

#ifndef _WIN32
struct libs libstoload[] =
	{
	{"libavcodec.so", libavcodec_functions},
	{"libswscale.so", libswscale_functions},
	{"libavformat.so", libavformat_functions},
	{"libavutil.so", libavutil_functions},
	{NULL}
	};
#else
char libstoload[][] =
	{
	{"avcodec.dll", libavcodec_functions},
	{"swscale.dll", libswscale_functions},
	{"avformat.dll", libavformat_functions},
	{"avutil.dll", libavutil_functions},
	{NULL}
	};
#endif


int FFM_Loadlib(void)
{
	struct libs *ptr;

	ptr = libstoload;

	while (ptr->name)
	{
		if (LIB_Load(ptr->name, ptr->functions, true))
			return 1;
		ptr++;
	}

	
		
	return 0;
}


/*
 * Windows hack
 */
#ifdef _WIN32
#define HACK_FOR_STACK_ALIGNMENT_ON_16_BYTE_BOUNDARY
#endif 

#ifdef HACK_FOR_STACK_ALIGNMENT_ON_16_BYTE_BOUNDARY
__declspec(naked)
static int hacked_avcodec_encode_video(
         AVCodecContext* c,
         uint8_t* video_outbuf,
         int video_outbuf_size,
         const AVFrame* picture)
{
    _asm {
        push ebp
        mov ebp,esp
        sub esp,__LOCAL_SIZE /*
                              * not really needed, 
                              * but safer against errors
                              * when changing this function
                              */
        /* adjust stack to 16 byte boundary */
        and esp,~0x0f
    }
    ffm_avcodec_encode_video(c, video_outbuf,
                      video_outbuf_size, picture);
    _asm {
        mov esp,ebp
        pop ebp
        ret
    }
}
#endif


/*
 * normal stuff
 */

void OnChangeffmvideo(cvar_t *var, char *value, qbool *cancel);
void OnChangeffmoutputformat(cvar_t *var, char *value, qbool *cancel);
void OnChangeffmfps(cvar_t *var, char *value, qbool *cancel);
void OnChangeffmres(cvar_t *var, char *value, qbool *cancel);
void OnChangeffmaudio (cvar_t *var, char *value, qbool *cancel);
void OnChangeffmrd(cvar_t *var, char *value, qbool *cancel);

cvar_t	ffm_video_codec 	=	{"ffm_video_codec", "ffv1", 0, OnChangeffmvideo};
cvar_t	ffm_output_format	=	{"ffm_outputformat", "avi", 0, OnChangeffmoutputformat};
cvar_t	ffm_fps				=	{"ffm_fps", "30", 0, OnChangeffmfps};
cvar_t	ffm_video_width 	=	{"ffm_video_width", "800", 0, OnChangeffmres};
cvar_t	ffm_video_height 	=	{"ffm_video_height", "600", 0, OnChangeffmres};
cvar_t	ffm_audio_codec 	=	{"ffm_audio_codec", "pcm_u16le", 0, OnChangeffmaudio};
cvar_t	ffm_audio_bit_rate	=	{"ffm_audio_bit_rate", "64000"};
cvar_t	ffm_audio_sample_rate	=	{"ffm_audio_sample_rate", "44100"};
cvar_t	ffm_audio_channels	=	{"ffm_audio_channels", "2"};
cvar_t	ffm_video_bitrate	=	{"ffm_video_bitrate", "100000"};
cvar_t	ffm_thread			=	{"ffm_thread", "1"};
cvar_t	ffm_debug			=	{"ffm_debug", "1"};
cvar_t	ffm_record_duration	=	{"ffm_record_duration", "0", 0, OnChangeffmrd};

struct ffm
{
	qbool		setup;
	qbool		recording;
	qbool		settings_broken;

	qbool		record_audio;

	char		*filename;
	FILE		*file;
	double		time;
	double		endtime;

	int		output_buffer_size;

	AVCodecContext	*video_codec_context;
	AVCodec		*video_codec;
	AVStream	*video_stream;

	AVCodecContext	*audio_codec_context;
	AVCodec		*audio_codec;
	AVStream	*audio_stream;

	AVFrame		*picture;
	AVFrame		*gl_picture;
	AVOutputFormat	*output_format;
	AVFormatContext	*format_context;
	AVPacket	packet;
	AVPacket	audio_packet;


	uint8_t		*picture_buffer;
	uint8_t		*output_buffer;
	uint8_t		*gl_buffer;
	uint8_t		*gl_swap_buffer;

	uint8_t		audio_output_buffer[44100];
	short 		audio_samples[44100];

	unsigned int	framecount;
	unsigned int	captured_audio_samples;
};

struct ffm	*ffm;

int		dwidth, dheight;

#ifdef GLQUAKE

extern unsigned short ramps[3][256];
//applies hwgamma to RGB data
static void applyHWGamma(byte *buffer, int size) {
	int i;

	if (vid_hwgamma_enabled) {
		for (i = 0; i < size; i += 3) {
			buffer[i + 0] = ramps[0][buffer[i + 0]] >> 8;
			buffer[i + 1] = ramps[1][buffer[i + 1]] >> 8;
			buffer[i + 2] = ramps[2][buffer[i + 2]] >> 8;
		}
	}
}
#endif

static void flip(byte *in, byte *out){
	int i;
	for (i = 0; i < dheight; i++)
	{
		memcpy ( out + (i * dwidth * 3),
		in + (((dheight - i) - 1) *
		dwidth * 3),
		dwidth * 3);
	}
}


struct frames_list_s {
	struct frames_list_s *next;
	uint8_t	*buffer;
	uint8_t	*audio_buffer;
	qbool	has_audio;
	int done;
};

struct frames_list_s	*frames;


qbool forked;
void FFM_End(void);

cvar_t	ffm_video_width, ffm_video_height;

int FFM_Encode_Frame(struct frames_list_s *f, struct SwsContext *img_convert_context)
{
	int out_size;


	flip(f->buffer, ffm->gl_swap_buffer);
#ifdef GLQUAKE
	applyHWGamma(ffm->gl_swap_buffer, dwidth * dheight * 3);
#endif
	ffm_avpicture_fill((AVPicture *) ffm->gl_picture, ffm->gl_swap_buffer, PIX_FMT_RGB24,  dwidth, dheight);
	ffm_sws_scale(img_convert_context, ffm->gl_picture->data, ffm->gl_picture->linesize, 0, dheight, ffm->picture->data, ffm->picture->linesize);

#ifdef _WIN32
out_size = hacked_avcodec_encode_video(ffm->video_codec_context, ffm->output_buffer, ffm->output_buffer_size, ffm->picture);
#else
out_size = ffm_avcodec_encode_video(ffm->video_codec_context, ffm->output_buffer, ffm->output_buffer_size, ffm->picture);
#endif
	ffm->packet.dts = ffm->packet.pts = ffm->framecount++;
	
	if (ffm->video_codec_context->coded_frame->key_frame)
	{
		ffm->packet.flags |= PKT_FLAG_KEY;
	}

	ffm->packet.stream_index = ffm->video_stream->index;
	ffm->packet.data = ffm->output_buffer;
	ffm->packet.size = out_size;

	printf("%i %i\n", ffm->video_stream->time_base.num, ffm->video_stream->time_base.den);
	printf("%i %i %s\n", CODEC_TYPE_VIDEO, ffm->video_stream->codec->codec_type, ffm->video_stream->codec->codec->name);
	printf("packet_size: %i\n", ffm->packet.size);
	fflush(stdout);
	ffm_av_interleaved_write_frame(ffm->format_context, &ffm->packet);
	
	if (f->has_audio && ffm->record_audio)
	{
		printf("audio happening!\n");
		ffm_av_init_packet(&ffm->audio_packet);

		ffm->audio_packet.size = ffm_avcodec_encode_audio(ffm->audio_stream->codec, ffm->audio_output_buffer, 44100, (short *)f->audio_buffer);

		if (ffm->audio_stream->codec->coded_frame->pts != AV_NOPTS_VALUE)
		{
			ffm->audio_packet.pts = ffm_av_rescale_q(ffm->audio_stream->codec->coded_frame->pts, ffm->audio_stream->codec->time_base, ffm->audio_stream->time_base);
		}

		ffm->audio_packet.flags |= PKT_FLAG_KEY;
		ffm->audio_packet.stream_index = ffm->audio_stream->index;
		ffm->audio_packet.data = ffm->audio_output_buffer;

		ffm_av_interleaved_write_frame(ffm->format_context, &ffm->audio_packet);
	}

	f->done = 2;

	return 0;
}

void FFM_Encode(void)
{
	struct frames_list_s	*f;
	static struct SwsContext *img_convert_context = NULL;

	if (img_convert_context == NULL)
	{
		img_convert_context = ffm_sws_getContext(dwidth, dheight, PIX_FMT_RGB24, (int)ffm_video_width.value, (int)ffm_video_height.value, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		if (img_convert_context == NULL)
		{
			printf("FFM_Encode: img_convert_context error\n");
			return;
		}
	}

	f = frames;
	while (f)
	{
		if (f->done == 1)
		{
			FFM_Encode_Frame(f, img_convert_context);
			f = f->next;
		}
		else if (f->done == 0)
		{
			break;
		}
		else if (f->done == 2)
		{
			f = f->next;
		}
	}	
}

void *FFM_Child_Thread (void *noargs)
{

	struct frames_list_s	*f;
	static struct SwsContext *img_convert_context = NULL;
	
	if (img_convert_context == NULL)
	{
		img_convert_context = ffm_sws_getContext(dwidth, dheight, PIX_FMT_RGB24, (int)ffm_video_width.value, (int)ffm_video_height.value, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		if (img_convert_context == NULL)
		{
			printf("error\n");
			forked = false;
			return NULL;
		}
	}

	while (forked)
	{
		f = frames;
		while (f)
		{
			if (f->done == 1)
			{
				FFM_Encode_Frame(f, img_convert_context);
				f = f->next;
			}
			else if (f->done == 0)
			{
				break;
			}
			else if (f->done == 2)
			{
				f = f->next;
			}
		}	
		Sys_MSleep(20);

	}

	FFM_End();
	return noargs;
}

qbool FFM_Codec_Is_Usable_With_Outputformat(AVCodec *codec, AVOutputFormat *format)
{
	if (codec == NULL || format == NULL)
		return false;

	if (ffm_av_codec_get_id(format->codec_tag, ffm_av_codec_get_tag(format->codec_tag, codec->id)) != CODEC_ID_NONE)
		return true;
	return false;

}

void FFM_Check_Settings(void)
{
	qbool cancel = false;
	AVRational *r;

	ffm->settings_broken = true;
	
	if (ffm->output_format == NULL)
	{
		Com_Printf("No outputformat selected, please set ffm_output_format. For a list of available output formats ffm_list_output_format\n");
		cancel = true;
	}

	if (ffm->video_codec == NULL)
	{
		Com_Printf("No video codec selected, please set ffm_video_codec. For a list of available codecs ffm_list_codecs\n");
		cancel = true;
	}

	if (ffm->audio_codec == NULL && ffm->record_audio)
	{
		Com_Printf("No audio codec selected, please set ffm_video_codec. For a list of available codecs ffm_list_codecs\n");
		cancel = true;
	}


	if (cancel)
		return;

	if (FFM_Codec_Is_Usable_With_Outputformat(ffm->video_codec, ffm->output_format) == false)
	{
		Com_Printf("The selected video codec \"%s\" is not supported by the outputformat \"%s\".\n", ffm->video_codec->name, ffm->output_format->name);
		return;
	}

	if (ffm->record_audio)
	{
		if (FFM_Codec_Is_Usable_With_Outputformat(ffm->audio_codec, ffm->output_format) == false)
		{
			Com_Printf("The selected video codec \"%s\" is not supported by the outputformat \"%s\".\n", ffm->video_codec->name, ffm->output_format->name);
			return;
		}
	}
	
	r = (AVRational *)ffm->video_codec->supported_framerates;
	if (r)
	{
		cancel = true;
		while(r)
		{
			if (r->num == ffm_fps.value)
			{
				cancel = false;
				break;
			}
			r++;
		}
	}

	if (cancel)
	{
		Com_Printf("Wrong fps settings for codec \"%s\". Suitable fps:\n", ffm->video_codec->name);
		r = (AVRational *)ffm->video_codec->supported_framerates;
		if (r)
		{
			cancel = true;
			while(r)
			{
				Com_Printf("%i ", r->num);
				r++;
			}
			Com_Printf("\n");
		}
		return;
	}

	ffm->settings_broken = false;
}

void OnChangeffmvideo (cvar_t *var, char *value, qbool *cancel)
{
	AVCodec *p;

	*cancel = false;

	p = ffm_avcodec_find_encoder_by_name (value);

	if (p == NULL)
	{
		Com_Printf("%s does not exist\n", value);
		Cvar_Set(var,"none");
		*cancel = true;
		return;
	}

	if (p->type != CODEC_TYPE_VIDEO)
	{
		Com_Printf("%s is not a video codec\n", value);
		Cvar_Set(var,"none");
		*cancel = true;
		return;
	}

	ffm->video_codec = p;

	Cvar_Set(var, value);

	FFM_Check_Settings();
	return;
}

void OnChangeffmaudio (cvar_t *var, char *value, qbool *cancel)
{
	AVCodec *p;

	*cancel = false;

	if (strcmp(value, "none") == 0)
	{
		ffm->record_audio = false;
	}

	p = ffm_avcodec_find_encoder_by_name (value);

	if (p == NULL)
	{
		Com_Printf("%s does not exist\n", value);
		Cvar_Set(var,"none");
		*cancel = true;
		return;
	}

	if (p->type != CODEC_TYPE_AUDIO)
	{
		Cvar_Set(var,"none");
		Com_Printf("%s is not a audio codec\n", value);
		*cancel = true;
		return;
	}

	if (FFM_Codec_Is_Usable_With_Outputformat(p, ffm->output_format) == false)
	{
		Com_Printf("%s can not be with the selected output format \"%s\".\n", p->name, ffm_output_format.string);
		*cancel = true;
		return;
	}

	ffm->record_audio = true;

	ffm->audio_codec = p;

	Cvar_Set(var, value);
	FFM_Check_Settings();
	return;
}

void OnChangeffmres (cvar_t *var, char *value, qbool *cancel)
{
	int i;
	*cancel = false;
	i = atoi(value);
	if (i % 2 == 0)
		Cvar_Set(var, va("%i\n",i));
	else 
		Com_Printf("not a multiple of 2!\n");
	
	FFM_Check_Settings();
	return;
}

void OnChangeffmfps (cvar_t *var, char *value, qbool *cancel)
{
	float f;
	*cancel = false;
	if (ffm->recording)
	{
		Com_Printf("You cant change this while recording\n");
		*cancel = true;
		return;
	}
	f = Q_atof(value);
	ffm->time = 1.0f /f;

	Cvar_Set(var, value);
	FFM_Check_Settings();
	return;
}


void OnChangeffmoutputformat (struct cvar_s *var, char *value, qbool *cancel)
{
	AVCodec *p;
	AVOutputFormat *f;

	*cancel = false;

	if (ffm->recording)
	{
		Com_Printf("Cant be changed while recording\n");
		*cancel = true;
		return;
	}

	p = ffm_avcodec_find_encoder_by_name(ffm_video_codec.string);

	if (p == NULL)
	{
		Com_Printf("No video codec set\n");
		*cancel = true;
		return;
	}

	f = ffm_av_oformat_next(NULL);

	while (f)
	{
		if (!strcmp(value, f->name))
		{
			break;
		}
		f = ffm_av_oformat_next(f);
	}

	if (f == NULL)
	{
		Com_Printf("%s is not a valid output format\n", value);
		*cancel = true;
		return;
	}

	ffm->output_format = f;

	FFM_Check_Settings();
	Cvar_Set(var, (char *)f->name);

	return;
}

void OnChangeffmrd(struct cvar_s *var, char *value, qbool *cancel)
{
	float f;
	*cancel = false;

	if (ffm->recording)
	{
		Com_Printf("Cant be changed while recording\n");
		*cancel = true;
		return;
	}

	f = atof(value);

	if (f < 0)
	{
		*cancel = true;
		return;
	}
}

char *FFM_Codec_Type_Enum(int en)
{
	switch (en)
	{
		case CODEC_TYPE_UNKNOWN:
			return "UNKOWN Codec";
		case CODEC_TYPE_VIDEO:
			return "Video Codec";
		case CODEC_TYPE_AUDIO:
			return "Audio Codec";

		default:
			return "not suitable";
	}
}

void FFM_List_Codecs_f (void) 
{
	AVCodec *p;

	p = ffm_av_codec_next(NULL);

	while (p)
	{
		if (p->encode != NULL)
		{
			Com_Printf("%20s :",p->name);
			Com_Printf("%s\n",FFM_Codec_Type_Enum(p->type));	
		}
		p = ffm_av_codec_next(p);
	}
}

void FFM_Record_Start_f (void)
{

	int i, ret;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("You need to give a filename !\n");
		return;
	}

	if (ffm->settings_broken == true)
	{
		Com_Printf("Your settings are wrong!\n");
		FFM_Check_Settings();
		return;
	}

	if (ffm->output_format == NULL)
	{
		Com_Printf("No outputformat !\n");
		return;
	}

#ifdef GLQUAKE
	dwidth = glwidth;
	dheight = glheight;
#else
	dwidth = vid.width;
	dheight = vid.height;
#endif

	ffm->filename = strdup (Cmd_Argv(1));

	ffm->format_context = ffm_avformat_alloc_context();

	if (ffm_record_duration.value > 0)
		ffm->endtime = ffm_record_duration.value + cls.realrealtime;

	if (ffm->format_context == NULL)
	{
		Com_Printf("Could not alloc Format Context\n");
		return;
	}

	ffm->format_context->oformat = ffm->output_format;

	if (!ffm->format_context->flags & AVFMT_NOFILE)
	{
		snprintf(ffm->format_context->filename, sizeof(ffm->format_context->filename), "%s", ffm->filename);
	}

/*
 * Audio
 */

	if (ffm->record_audio)
	{
		Com_Printf("recording audio!\n");

		ffm->audio_stream = ffm_av_new_stream(ffm->format_context, 0);
			
		if (ffm->audio_stream == NULL)
		{
			Com_Printf("Could not allocate Audio Stream\n");
			return;
		}

		ffm->audio_stream->codec->codec_id	= ffm->audio_codec->id;
		ffm->audio_stream->codec->codec_type	= CODEC_TYPE_AUDIO;
		ffm->audio_stream->codec->bit_rate	= 64000;//ffm_audio_bit_rate.value;
		ffm->audio_stream->codec->sample_rate	= 44100;//ffm_audio_sample_rate.value;
		ffm->audio_stream->codec->channels	= 2;//ffm_audio_channels.value;

		if (ffm_avcodec_open(ffm->audio_stream->codec, ffm->audio_codec) < 0)
		{
			Com_Printf("Could not open audio codec \"%s\".\n", ffm->audio_codec->name);
			return;
		}

		ffm->audio_stream->codec->time_base.num = 1;
		printf("%i %i %i %i %i %i %i\n", 	ffm->audio_stream->codec->channels,
						ffm->audio_stream->codec->codec_type,
						ffm->audio_stream->codec->bit_rate,
						ffm->audio_stream->codec->sample_rate,
						ffm->audio_stream->codec->frame_size,
						ffm->audio_stream->codec->time_base.num,
						ffm->audio_stream->codec->time_base.den);

	}



/*
 * Video
 */

	ffm->video_stream = ffm_av_new_stream(ffm->format_context, 1);

	if (ffm->video_stream == NULL)
	{
		Com_Printf("Could not allocate Video Stream\n");
		return;
	}

	//ffm->video_codec = ffm_avcodec_find_encoder_by_name(ffm_video_codec.string);

	if (ffm->video_codec == NULL)
	{
		Com_Printf ("Could not load video codec \"%s\"\n", ffm_video_codec.string);
		return;
	}

	ffm->video_codec_context = ffm->video_stream->codec;

	if (ffm->time == 0)
	{
		ffm->time = 1.0f/ffm_fps.value;
	}

	ffm->picture	= ffm_avcodec_alloc_frame();
	ffm->gl_picture = ffm_avcodec_alloc_frame();

	ffm->video_codec_context->bit_rate	= ffm_video_bitrate.value;
	ffm->video_codec_context->width 	= ffm_video_width.value;
	ffm->video_codec_context->height	= ffm_video_height.value;
	ffm->video_codec_context->time_base.den = ffm_fps.value;
	ffm->video_codec_context->time_base.num	= 1;
	ffm->video_codec_context->gop_size	= 10;
	ffm->video_codec_context->max_b_frames	= 0;
	ffm->video_codec_context->pix_fmt	= PIX_FMT_YUV420P;
	ffm->video_codec_context->codec_type	= CODEC_TYPE_VIDEO;
	ffm->video_codec_context->codec_id	= ffm->video_codec->id;

	ffm->output_buffer_size = ffm_video_width.value * ffm_video_height.value * 4;

	if ((ret = ffm_avcodec_open(ffm->video_codec_context, ffm->video_codec)) < 0)
	{
		Com_Printf("Could not open codec \"%s\".\n", ffm->video_codec->name);
		return;
	}

	ffm->picture_buffer = (uint8_t *) malloc((ffm_avpicture_get_size(PIX_FMT_YUV420P, ffm_video_width.value, ffm_video_height.value))*2); // no clue why the 2 fixes stuff... it shouldnt

	ffm->picture->data[0] = ffm->picture_buffer;
	ffm->picture->data[1] = ffm->picture->data[0] + (int)(ffm_video_width.value * ffm_video_height.value);
	ffm->picture->data[2] = ffm->picture->data[1] + (int)(ffm_video_width.value * ffm_video_height.value) / 4 ;
	ffm->picture->linesize[0] = ffm_video_width.value;
	ffm->picture->linesize[1] = ffm_video_width.value /2;
	ffm->picture->linesize[2] = ffm_video_width.value /2;

	ffm->output_buffer = malloc(ffm->output_buffer_size);

	ffm_avpicture_fill((AVPicture *)ffm->picture, ffm->picture_buffer, PIX_FMT_YUV420P, ffm_video_width.value, ffm_video_height.value);

	ffm->gl_buffer = (uint8_t *) malloc((ffm_avpicture_get_size(PIX_FMT_RGB24, dwidth, dheight)));

	ffm->gl_picture->data[0] = ffm->gl_buffer;
	ffm->gl_picture->data[1] = ffm->gl_picture->data[0] + dwidth * dheight ;
	ffm->gl_picture->data[2] = ffm->gl_picture->data[1] + (dwidth * dheight) / 4 ;
	ffm->gl_picture->linesize[0] = dwidth ;
	ffm->gl_picture->linesize[1] = dwidth /2;
	ffm->gl_picture->linesize[2] = dwidth /2;


	ffm->gl_swap_buffer = (uint8_t *) malloc((ffm_avpicture_get_size(PIX_FMT_RGB24, dwidth, dheight)));


	ffm_dump_format(ffm->format_context, 0, ffm->filename, 1);

	if (ffm_av_set_parameters(ffm->format_context, NULL) < 0 )
	{
		Com_Printf("FFM: parameter error\n");
		if (ffm->record_audio)
			free(ffm->audio_output_buffer);
		free(ffm->picture_buffer);
		free(ffm->output_buffer);
		free(ffm->gl_buffer);
		return;
	}
	
	if (!(ffm->output_format->flags & AVFMT_NOFILE))
	{
		if (ffm_url_fopen(&ffm->format_context->pb, ffm->filename, URL_WRONLY) <0)
		{
			Com_Printf("Could not open file %s\n", ffm->filename);
			if (ffm->record_audio)
				free(ffm->audio_output_buffer);
			free(ffm->picture_buffer);
			free(ffm->output_buffer);
			free(ffm->gl_buffer);
			return;
		}
		Com_Printf("opening file %s\n", ffm->filename);
	}


	ffm_av_init_packet(&ffm->packet);

	for (i = 0; i < ffm->format_context->nb_streams ; i++);

	Com_Printf("streams %d\n", ffm->format_context->nb_streams);

	ffm_av_write_header(ffm->format_context);

	Com_Printf("Recording to %s with video codec %s\n", ffm->filename, ffm_video_codec.string);

	ffm->recording = true;
}

void FFM_Record_Stop_f (void)
{

	if (!ffm->recording)
		return;

	if (ffm->filename)
		Com_Printf ("Stopping recodring of %s\n", ffm->filename);
	else
		Com_Printf ("Stopping recording withoug FFM_Filename set. Strange !\n");

	if (ffm_thread.value == 1)
	{
		forked = false;
		Com_Printf("File will now be written please be patient\n");
	}
	else
	{
		FFM_End();
	}

	Com_Printf("%f\n", ffm->time);

	ffm->recording = false;
}

void FFM_End(void)
{

	ffm_av_write_trailer(ffm->format_context);

	ffm_avcodec_close(ffm->video_stream->codec);

	if (!(ffm->output_format->flags & AVFMT_NOFILE))
	{
		ffm_url_fclose(ffm->format_context->pb);

	}

	Com_Printf("%s, is now ready\n", ffm->filename);

	free(ffm->filename);
	free(ffm->picture_buffer);
	free(ffm->output_buffer);
	free(ffm->gl_buffer);
	free(ffm->gl_swap_buffer);

	memset(ffm, sizeof(struct ffm), 0);

	forked = false;
}

void FFM_Check_Frame_List (void)
{
	struct frames_list_s *t;
	int frame[3];
	struct frames_list_s *f;

	if (frames == NULL)
		return;

	f = frames;

	frame[0] = frame[1] = frame[2] = 0;

	while (f)
	{
		if (f->done == 2)
		{
			t = f->next;
			if (ffm->record_audio)
				free(f->audio_buffer);
			free(f->buffer);
			free(f);
			if (f == frames)
				frames = t;
			f = t;
			frame[2]++;
		}
		else
		{
			if (ffm_debug.value == 1)
			{
				if (f->done == 0)
					frame[0]++;
				else if (f->done == 1)
					frame[1]++;
				f = f->next;
			}
			else
			{
				return;
			}
		}
	}
	if (ffm_debug.value == 1)
		Com_Printf("0: %i -- 1: %i -- 2: %i\n", frame[0], frame[1], frame[2]);

}

struct frames_list_s *FFM_Create_New_Frame (void)
{
	struct frames_list_s *f;

	if (frames == NULL)
	{
		frames = (struct frames_list_s *) malloc(sizeof(struct frames_list_s));
		f = frames;
		memset(f, 0, sizeof(struct frames_list_s));
	}
	else 
	{
		f = frames;
		while (f->next)
			f = f->next;
		f->next = (struct frames_list_s *) malloc(sizeof(struct frames_list_s));
		f = f->next;
		memset(f, 0, sizeof(struct frames_list_s));
	}

	f->buffer =  (uint8_t *) malloc((ffm_avpicture_get_size(PIX_FMT_RGB24, dwidth, dheight)));
	
	if (ffm->record_audio)
		f->audio_buffer = (uint8_t *) malloc(44100*sizeof(uint8_t));

	return f;

}

void FFM_Frame(void)
{
	extern int snd_linear_count;
	static double lasttime;
	struct frames_list_s *frame;
	extern short *snd_out;

	if (ffm == NULL)
		return;

	if (ffm->recording == false)
		return;

	if (ffm_thread.value == 1)
	{
		if (forked == false)
		{
			forked = true;
			if (!Thread_Run(FFM_Child_Thread))
			{
				Com_Printf("Could not create thread\n");
				forked = false;
			}
		}
	}

	FFM_Check_Frame_List();


	if(ffm->record_audio)
	{
		memcpy (ffm->audio_samples + (ffm->captured_audio_samples << 1), snd_out, snd_linear_count * shm->format.channels);
		ffm->captured_audio_samples += (snd_linear_count >> 1);
	}


	if (lasttime + ffm->time <= cls.realrealtime)
	{
		frame = FFM_Create_New_Frame();
		lasttime = cls.realrealtime;

#ifdef GLQUAKE
		glReadPixels(0, 0, dwidth, dheight, GL_RGB, GL_UNSIGNED_BYTE, frame->buffer);
#endif
		if (ffm->record_audio)
		{
	//		if (ffm->captured_audio_samples >= (int)(0.5 + cls.frametime * shm->format.speed)) {
				// We have enough audio samples to match one frame of video
				//Capture_WriteAudio (captured_audio_samples, (byte *)capture_audio_samples);
				Com_Printf("Audio Frame\n");
				memcpy(frame->audio_buffer, ffm->audio_samples, ffm->captured_audio_samples);
				ffm->captured_audio_samples = 0;
				frame->has_audio = true;
	//		}
		}
			
		frame->done = 1;
	}

	if (ffm_thread.value == 0)
	{
		FFM_Encode();
	}

	if (ffm_record_duration.value > 0)
		if (ffm->endtime <= cls.realrealtime)
			FFM_Record_Stop_f();
}

void FFM_List_Output_Formats_f (void)
{
	AVOutputFormat	*p;
	AVCodec		*c;

	p = ffm_av_oformat_next(NULL);

	while (p)
	{
		Com_Printf("%s:\n", p->name);
		c = ffm_avcodec_find_encoder(p->audio_codec);
		if (c)
			Com_Printf("     audio: %s - ", c->name);
		c = ffm_avcodec_find_encoder(p->video_codec);
		if (c)
			Com_Printf("     video: %s", c->name);
		p = ffm_av_oformat_next(p);
		Com_Printf("\n");
	}
}

void FFM_Callback(void* ptr, int level, const char* fmt, va_list vl)
{
	AVClass* avc= ptr ? *(AVClass**)ptr : NULL;

	if (ffm_debug.value == 0)
		return;

	if (avc)
	{
		Com_Printf("[%s @ %p]\n", avc->item_name(ptr), avc);
	}

	Com_VPrintf(fmt, vl);
}

void FFM_Init(void)
{

	if(FFM_Loadlib())
	{
		Com_Printf("ERROR loading ffmpeg stuff.\n");
		return;
	}
	else
	{
		Com_Printf("FFMPeg loaded.\n");
	}

	ffm = calloc(1, sizeof(struct ffm));
	if (ffm == NULL)
	{
		Com_Printf("FFMpeg calloc failure!\n");
		return;
	}
	
	ffm_avcodec_init();

	ffm_avcodec_register_all();

	ffm_av_register_all();

	ffm_av_log_set_callback(&FFM_Callback);

	Cmd_AddCommand("ffm_list_codecs", FFM_List_Codecs_f);
	Cmd_AddCommand("ffm_list_output_format", FFM_List_Output_Formats_f);
	Cmd_AddCommand("ffm_record_start", FFM_Record_Start_f);
	Cmd_AddCommand("ffm_record_stop", FFM_Record_Stop_f);

	Cvar_Register(&ffm_debug);
	Cvar_Register(&ffm_fps);
	Cvar_Register(&ffm_video_codec);
	Cvar_Register(&ffm_audio_codec);
	Cvar_Register(&ffm_video_width);
	Cvar_Register(&ffm_video_height);
	Cvar_Register(&ffm_video_bitrate);
	Cvar_Register(&ffm_output_format);
	Cvar_Register(&ffm_thread);
	Cvar_Register(&ffm_record_duration);
}

void FFM_Quit (void)
{
	forked = 0;
}

qbool FFM_IsCapturing(void)
{
	if (ffm == NULL)
		return false;
	if (ffm->recording == true && cls.demoplayback && ffm_thread.value == 0)
		return true;

	return false;
}

double FFM_FrameTime(void)
{
	return bound(1.0/1000.0f, ffm->time, 1);
}
