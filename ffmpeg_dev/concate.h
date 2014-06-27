#ifndef CONCATE_H
#define CONCATE_H

typedef struct ConcatContext    ConcatContext;

ConcatContext* concat_new();

void concat_free(ConcatContext *ctx);

int  concat_set_inputs(ConcatContext *ctx, const char *inputs);

int  concat_set_output(ConcatContext *ctx, char *filename);

int  concat_process(ConcatContext *ctx);

void concat_interrupt_request(ConcatContext *ctx, int interrupt);

#endif