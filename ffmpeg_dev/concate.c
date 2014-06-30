
#ifdef _MSC_VER
#define inline __inline
//#define strdup _strdup
#define snprintf _snprintf
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <libavutil/common.h>
#include <libavutil/timestamp.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavformat/avformat.h>
#include "concate.h"
#include "log.h"

typedef struct ConcatStream {
    int64_t last_mux_dts;
    int out_stream_index;
    int64_t duration;
} ConcatStream;

typedef struct {
    char *url;
    int64_t start_time;
    int64_t duration;
    ConcatStream *streams;
    int nb_streams;
} ConcatFile;

struct ConcatContext{
    char* out_url;
    ConcatFile *files;
    ConcatFile *cur_file;
    unsigned nb_files;
    AVFormatContext *ifmt_ctx;
    AVFormatContext *ofmt_ctx;
    int interrupt_request;
};

static int add_input_file(ConcatContext* ctx, char* filename, ConcatFile **rfile, unsigned *nb_files_alloc);
//static int open_inputs(ConcatContext *ctx);
//static int open_output(ConcatContext *ctx);
static int open_file(ConcatContext *ctx, unsigned fileno);
static int open_next_file(ConcatContext *ctx);
static int match_streams(ConcatContext* ctx);
//static int match_streams_by_codec(ConcatContext *ctx);
static int copy_stream_props(AVStream *dst, AVStream *src);

static int interupt_callback(void* opaque)
{
    ConcatContext *ctx = (ConcatContext*)opaque;
    if (ctx)
        return ctx->interrupt_request;
    return 0;
}

void concat_interrupt_request(ConcatContext *ctx, int interrupt)
{
    if (ctx)
        ctx->interrupt_request = interrupt;
}

static void concate_log(void *ptr, int level, const char *fmt, va_list vl)
{
    static int print_prefix = 1;
    static int count;
    static char prev[2048];
    char line[2048];
    static int is_atty;
    AVClass* avc = ptr ? *(AVClass **)ptr : NULL;
    line[0] = 0;

    if (print_prefix && avc) {
        if (avc->parent_log_context_offset) {
            AVClass** parent = *(AVClass ***)(((uint8_t *)ptr) +
                avc->parent_log_context_offset);
            if (parent && *parent) {
                snprintf(line, sizeof(line), "[%s @ %p] ",
                    (*parent)->item_name(parent), parent);
            }
        }

        snprintf(line + strlen(line), sizeof(line)-strlen(line), "[%s @ %p] ",
            avc->item_name(ptr), ptr);
    }

    vsnprintf(line + strlen(line), sizeof(line)-strlen(line), fmt, vl);
    print_prefix = strlen(line) && line[strlen(line) - 1] == '\n';
    if (print_prefix && !strncmp(line, prev, sizeof line)) {
        count++;
        if (is_atty == 1)
            fprintf(stderr, "    Last message repeated %d times\r", count);
        return;
    }

    if (count > 0) {
        fprintf(stderr, "    Last message repeated %d times\n", count);
        count = 0;
    }

    LOG("%s", line);
    av_strlcpy(prev, line, sizeof(line));
}

ConcatContext* concat_new()
{
    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(concate_log);

    ConcatContext* context = av_mallocz(sizeof(*context));

    av_register_all();

    return context;
}

void concat_free(ConcatContext *ctx)
{
    if (!ctx)
        return;

    if (ctx->ofmt_ctx && !(ctx->ofmt_ctx->flags & AVFMT_NOFILE))
        avio_close(ctx->ofmt_ctx->pb);

    av_free(ctx);
}

int  concat_set_inputs(ConcatContext *ctx, const char *inputs)
{
    const char* p0 = inputs;
    const char* p = p0;
    char* str = NULL;
    int cnt = 0;
    int len = 0;
    ConcatFile *rfile = NULL;
    unsigned nb_files_alloc = 0;

    for (;;)
    {
        if (*p != '\0' && *p != '|')
        {
            p++;
            continue;
        }

        if (p > p0)
        {
            len = p - p0;
        }
        else
            break;

        if (len > 0)
        {
            str = calloc(len + 1, sizeof(char));
            memcpy(str, p0, len);

            printf("[%d] %s\n", cnt++, str);

            add_input_file(ctx, str, &rfile, &nb_files_alloc);
        }

        if (*p != '\0')
            ++p;
        p0 = p;
    }

    return 0;
}

int  concat_set_output(ConcatContext *ctx, char *filename)
{
    int ret = 0;
    AVFormatContext *ofmt_ctx = NULL;

    ctx->out_url = strdup(filename);

    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, ctx->out_url);
    if (!ofmt_ctx) {
        ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", ctx->out_url);
        if (!ofmt_ctx)
            LOG("Could not create output context, ret=%d", ret);
        return ret;
    }

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, ctx->out_url, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOG("Could not open output file '%s', error=%d", ctx->out_url, ret);
            return ret;
        }
    }

    ctx->ofmt_ctx = ofmt_ctx;

    return 0;
}

void ustime_to_string(int64_t us, char* str)
{
    int ret = sprintf(str, "%"PRId64":%02d:%02d.%03d",
        us / 3600000000, (int)((us / 60000000) % 60),
        (int)((us / 1000000) % 60), (int)((us / 1000) % 1000));
}


int  concat_process(ConcatContext *ctx)
{
    int ret, fileno = 0, i = 0;
    int64_t delta;
    AVPacket pkt;
    AVStream *instream = NULL, *outstream = NULL;
    ConcatStream *cs = NULL;
    int64_t dtsus = 0, ptsus = 0, durationus = 0, deltaus = 0;
    char dtstime[32], ptstime[32], deltatime[32], durationtime[32];

    LOG("start process concat ......");

    ret = open_file(ctx, 0);
    if (ret < 0) {
        LOG("open fileno 0 failed, ret = %d", ret);
    }

    ret = avformat_write_header(ctx->ofmt_ctx, NULL);
    if (ret < 0) {
        LOG("avformat_write_header failed error=%d(%s)", ret, av_err2str(ret));
        return ret;
    }

    av_init_packet(&pkt);
    
    for (;;) {

        ret = av_read_frame(ctx->ifmt_ctx, &pkt);
        
        if (ret == AVERROR_EOF) {
            ret = open_next_file(ctx);
            if (ret < 0)
                break;
            ++fileno;
            continue;
        }

        cs = &ctx->cur_file->streams[pkt.stream_index];
        if (cs->out_stream_index < 0) {
            LOG("unknown matched out_stream_index for this packet");
            av_packet_unref(&pkt);
            continue;
        }

        if (pkt.stream_index != cs->out_stream_index)
            LOG("pkt stream index changed, pkt.stream_index=%d, cs->out_stream_index=%d", cs->out_stream_index);

        // convert timestamp from in_stream time_base to out_stream time_base
        instream = ctx->ifmt_ctx->streams[pkt.stream_index];
        outstream = ctx->ofmt_ctx->streams[cs->out_stream_index];

        pkt.dts = av_rescale_q(pkt.dts, instream->time_base, outstream->time_base);
        pkt.pts = av_rescale_q(pkt.pts, instream->time_base, outstream->time_base);
        pkt.duration = (int)av_rescale_q(pkt.duration, instream->time_base, outstream->time_base);

        // for log readable timestamp
        dtsus = av_rescale_q(pkt.dts, outstream->time_base, AV_TIME_BASE_Q);
        ptsus = av_rescale_q(pkt.pts, outstream->time_base, AV_TIME_BASE_Q);
        durationus = av_rescale_q(pkt.duration, outstream->time_base, AV_TIME_BASE_Q);
        ustime_to_string(dtsus, dtstime);
        ustime_to_string(ptsus, ptstime);
        ustime_to_string(durationus, durationtime);
        LOG("fileno[%d], pkt.stream_index=%d, pkt.dts=%"PRId64"(%s), pkt.pts=%"PRId64"(%s), pkt.duration=%d(%s)",
            fileno, pkt.stream_index, pkt.dts, dtstime, pkt.pts, ptstime, pkt.duration, durationtime);

        // add delta
        delta = av_rescale_q(ctx->cur_file->start_time - ctx->ifmt_ctx->start_time, AV_TIME_BASE_Q, outstream->time_base);
        if (pkt.pts != AV_NOPTS_VALUE)
            pkt.pts += delta;
        if (pkt.dts != AV_NOPTS_VALUE)
            pkt.dts += delta;

        // log after delta
        dtsus = av_rescale_q(pkt.dts, outstream->time_base, AV_TIME_BASE_Q);
        ptsus = av_rescale_q(pkt.pts, outstream->time_base, AV_TIME_BASE_Q);
        deltaus = av_rescale_q(delta, outstream->time_base, AV_TIME_BASE_Q);
        ustime_to_string(dtsus, dtstime);
        ustime_to_string(ptsus, ptstime);
        ustime_to_string(deltaus, deltatime);
        LOG("fileno[%d], pkt.stream_index=%d, pkt.dts=%"PRId64"(%s), pkt.pts=%"PRId64"(%s), delta=%"PRId64"(%s)",
            fileno, pkt.stream_index, pkt.dts, dtstime, pkt.pts, ptstime, delta, deltatime);
        
        // make sure increased dts
//         if (cs->cur_dts != AV_NOPTS_VALUE &&
//             pkt.dts == cs->cur_dts) {
//             LOG("fileno[%d], pkt.stream_index=%d, pkt.dts==cs->cur_dts, dts=%"PRId64, fileno, pkt.stream_index, pkt.dts);
// 
//             pkt.dts += 40;
// 
//             //av_packet_unref(&pkt);
//             //continue;
//         }
//         cs->cur_dts = pkt.dts;
        if (!(ctx->ofmt_ctx->oformat->flags & AVFMT_NOTIMESTAMPS)) {
            if ((outstream->codec->codec_type == AVMEDIA_TYPE_AUDIO || outstream->codec->codec_type == AVMEDIA_TYPE_VIDEO) &&
                pkt.dts != AV_NOPTS_VALUE &&  cs->last_mux_dts != AV_NOPTS_VALUE) {
                int64_t max = cs->last_mux_dts + !(ctx->ofmt_ctx->oformat->flags & AVFMT_TS_NONSTRICT);
                if (pkt.dts < max) {
                    int loglevel = max - pkt.dts > 2 || outstream->codec->codec_type == AVMEDIA_TYPE_VIDEO ? AV_LOG_WARNING : AV_LOG_DEBUG;
                    LOG("Non-monotonous DTS in output stream "
                        "%d:%d; previous: %"PRId64", current: %"PRId64"; ",
                        fileno, outstream->index, cs->last_mux_dts, pkt.dts);
                    LOG("changing to %"PRId64". This may result "
                        "in incorrect timestamps in the output file.\n",
                        max);
                    if (pkt.pts >= pkt.dts)
                        pkt.pts = FFMAX(pkt.pts, max);
                    pkt.dts = max;
                }
            }
            if (pkt.dts != AV_NOPTS_VALUE &&
                pkt.pts != AV_NOPTS_VALUE &&
                pkt.dts > pkt.pts) {
                LOG("Invalid DTS: %"PRId64" PTS: %"PRId64" in output stream %d:%d\n",
                    pkt.dts, pkt.pts, fileno, outstream->index);
                pkt.pts = AV_NOPTS_VALUE;
                pkt.dts = AV_NOPTS_VALUE;
            }
        }
        cs->last_mux_dts = pkt.dts;

        // set new stream_index
        pkt.stream_index = cs->out_stream_index;

        ret = av_interleaved_write_frame(ctx->ofmt_ctx, &pkt);
        if (ret < 0) {
            LOG("av_interleaved_write_frame, Error muxing packet, error = %d\n", ret);
            break;
        }
        av_free_packet(&pkt);
    }
    
    
    if (ret == AVERROR_EOF) {
        ret = av_write_trailer(ctx->ofmt_ctx);
        if (ret < 0) {
            LOG("av_write_trailer failed ret = %d", ret);
        }

//         ctx->ofmt_ctx->duration = 0;
// 
//         for (i = 0; i < ctx->nb_files; i++)
//             ctx->ofmt_ctx->duration += ctx->files[i].duration;
// 
//         ustime_to_string(ctx->ofmt_ctx->duration, durationtime);
//         LOG("out file duration = %"PRId64"(%s)", ctx->ofmt_ctx->duration, durationtime);

    }

    LOG("finish process concat ......");

    return ret;
}

static int match_streams(ConcatContext *ctx)
{
    int i = 0, j = 0,  ret = 0;
    ConcatStream *map = NULL;
    AVStream *istream = NULL;
    AVStream *ostream = NULL;

    if (ctx->cur_file->nb_streams >= ctx->ifmt_ctx->nb_streams)
        return 0;

    map = av_realloc(ctx->cur_file->streams, ctx->ifmt_ctx->nb_streams * sizeof(*map));
    if (!map)
        return AVERROR(ENOMEM);

    ctx->cur_file->streams = map;

    memset(map + ctx->cur_file->nb_streams, 0,
        (ctx->ifmt_ctx->nb_streams - ctx->cur_file->nb_streams) * sizeof(*map));

    for (i = ctx->cur_file->nb_streams; i < ctx->ifmt_ctx->nb_streams; i++) {
        map[i].out_stream_index = -1;
        map[i].last_mux_dts = AV_NOPTS_VALUE;

        istream = ctx->ifmt_ctx->streams[i];

        if (i < ctx->ofmt_ctx->nb_streams) {
            for (j = 0; j < ctx->ofmt_ctx->nb_streams; j++) {
                ostream = ctx->ofmt_ctx->streams[j];
                if (ostream->id == istream->id ||
                    ostream->codec->codec_id == istream->codec->codec_id) {

                    ret = copy_stream_props(ostream, istream);

                    ctx->cur_file->streams[i].out_stream_index = j;
                                        
                    break;
                }
            }
        } else {
            ostream = avformat_new_stream(ctx->ofmt_ctx, NULL);
            if (!ostream)
                return AVERROR(ENOMEM);

            ret = copy_stream_props(ostream, istream);

            // !need this
            ostream->codec->codec_tag = 0;
            
            if (ctx->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                ostream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

            LOG("add new stream index=%d, codec_id=%d", ostream->index, ostream->codec->codec_id);

            ctx->cur_file->streams[i].out_stream_index = i;
        }
    }

    ctx->cur_file->nb_streams = ctx->ifmt_ctx->nb_streams;

    return ret;
}

static int open_output(ConcatContext *ctx)
{
    int i = 0, ret = 0;
    AVFormatContext *ifmt_ctx = ctx->ifmt_ctx;
    AVFormatContext *ofmt_ctx = NULL;
    AVOutputFormat *ofmt = ofmt_ctx->oformat;
    AVStream *st;

    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, ctx->out_url);
    if (!ofmt_ctx) {
        ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", ctx->out_url);
        if (!ofmt_ctx)
            LOG("Could not create output context, ret=%d", ret);
        return ret;
    }

    ctx->ofmt_ctx = ofmt_ctx;

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, ctx->out_url, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOG("Could not open output file '%s', error=%d", ctx->out_url, ret);
            return ret;
        }
    }

    // init out streams
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        st = avformat_new_stream(ofmt_ctx, NULL);
        if (st) {
            copy_stream_props(st, ifmt_ctx->streams[i]);
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    
    av_dump_format(ctx->ofmt_ctx, 0, ctx->out_url, 1);

    return 0;
}

static int open_file(ConcatContext *ctx, unsigned fileno)
{
    LOG("start open fileno=%d", fileno);

    ConcatFile *file = &ctx->files[fileno];
    int ret;

    if (ctx->ifmt_ctx)
        avformat_close_input(&ctx->ifmt_ctx);

    ctx->ifmt_ctx = avformat_alloc_context();
    if (!ctx->ifmt_ctx)
        return AVERROR(ENOMEM);

    ctx->ifmt_ctx->interrupt_callback.callback = interupt_callback;
    ctx->ifmt_ctx->interrupt_callback.opaque = ctx;

    ret = avformat_open_input(&ctx->ifmt_ctx, file->url, NULL, NULL);
    if (ret < 0) {
        return ret;
    }

    ret = avformat_find_stream_info(ctx->ifmt_ctx, NULL);
    if (ret < 0) {
        avformat_close_input(&ctx->ifmt_ctx);
        return ret;
    }

    av_dump_format(ctx->ifmt_ctx, 0, file->url, 0);

    ctx->cur_file = file;
    if (file->start_time == AV_NOPTS_VALUE) {
        if (!fileno)
            file->start_time = 0;
        else
            file->start_time = ctx->files[fileno - 1].start_time + ctx->files[fileno - 1].duration;
    }

    LOG("open_file, fileno=%d, start_time=%"PRId64"", fileno, file->start_time);

    ret = match_streams(ctx);

    if (ret < 0)
        LOG("match_streams failed, ret = %d", ret);

    return ret;
}

static int open_next_file(ConcatContext *ctx)
{
    int ret = 0;
    unsigned fileno = ctx->cur_file - ctx->files;

    if (ctx->cur_file->duration == AV_NOPTS_VALUE)
        ctx->cur_file->duration = ctx->ifmt_ctx->duration;

    if (++fileno >= ctx->nb_files)
        return AVERROR_EOF;
    
    ret = open_file(ctx, fileno);

    return ret;
}

static int add_input_file(ConcatContext* ctx, char* filename, ConcatFile **rfile, unsigned *nb_files_alloc)
{
    printf("add_input_file, filename=%s\n", filename);

    ConcatFile *file = NULL;
    char* url = filename;

    if (ctx->nb_files >= *nb_files_alloc) {
        size_t n = FFMAX(*nb_files_alloc * 2, 16);
        ConcatFile *new_files;
        if (n <= ctx->nb_files || n > SIZE_MAX / sizeof(*ctx->files) ||
            !(new_files = av_realloc(ctx->files, n * sizeof(*ctx->files))))
            return AVERROR(ENOMEM);
        ctx->files = new_files;
        *nb_files_alloc = n;
    }

    file = &ctx->files[ctx->nb_files++];
    memset(file, 0, sizeof(*file));
    *rfile = file;

    file->url = url;
    file->start_time = AV_NOPTS_VALUE;
    file->duration = AV_NOPTS_VALUE;

    return 0;
}

int ff_alloc_extradata(AVCodecContext *avctx, int size)
{
    int ret;

    if (size < 0 || size >= INT32_MAX - FF_INPUT_BUFFER_PADDING_SIZE) {
        avctx->extradata_size = 0;
        return AVERROR(EINVAL);
    }
    avctx->extradata = av_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (avctx->extradata) {
        memset(avctx->extradata + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
        avctx->extradata_size = size;
        ret = 0;
    }
    else {
        avctx->extradata_size = 0;
        ret = AVERROR(ENOMEM);
    }
    return ret;
}

static int copy_stream_props(AVStream *dst, AVStream *src)
{
    int ret = 0;

    if (dst->codec->codec_id || !src->codec->codec_id) {
        if (dst->codec->extradata_size < src->codec->extradata_size) {
            ret = ff_alloc_extradata(dst->codec,
                src->codec->extradata_size);
            if (ret < 0) {
                LOG("ff_alloc_extradata failed, ret=%d", ret);
                //return ret;
            }
        }
        memcpy(dst->codec->extradata, src->codec->extradata,
            src->codec->extradata_size);
        LOG("copy_stream_props, copy codec extradata");
        return 0;
    }

    ret = avcodec_copy_context(dst->codec, src->codec);
    if (ret < 0) {
        LOG("avcodec_copy_context failed, ret=%d", ret);
        return ret;
    }
    
    // ?? need these
    //dst->codec->codec_tag = 0;
    //dst->id = src->id;
    dst->r_frame_rate = src->r_frame_rate;
    dst->avg_frame_rate = src->avg_frame_rate;
    //st->time_base = source_st->time_base; // time_base AVStream和AVCodec可能不匹配
    dst->sample_aspect_ratio = src->sample_aspect_ratio;

    return 0;
}