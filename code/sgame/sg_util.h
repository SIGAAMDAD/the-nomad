// sg_util.h

#ifndef __SG_UTIL__
#define __SG_UTIL__

#pragma once

typedef struct vector_s vector_t;
typedef struct list_s list_t;
typedef struct string_s string_t;

void vector_init( vector_t *vec, int itemSize );
void vector_shutdown( vector_t *vec );
void vector_at( vector_t *vec, int index, void *dest );
void vector_append( vector_t *vec, int count, const void *data );
void vector_clear( const vector_t *vec );
int vector_size( const vector_t *vec );

int parse_csv( const char *text, char *strings[MAX_STRING_CHARS], int maxStrings );

#endif
