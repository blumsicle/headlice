/*
 * lice.c
 */
#include <stdlib.h>
#include <stdio.h>          /* *** remove this **** */
#include <stdarg.h>
#include <string.h>

#include "lice.h"

#ifndef LINEWIDTH_DEFAULT
# define LINEWIDTH_DEFAULT 70
#endif

struct lice_t {
  char *buf;
  off_t size;

  enum lice_licetype licetype;
  enum lice_progtype progtype;

  char  *author;
  char  *email;
  char  *version;
  char  *year;
  size_t linewidth;
};

lice_t *lice_new(void)
{
#define EMPTYSTR(p) do { p = strdup(""); if (p == NULL) goto error; } while (0)
  lice_t *handler;

  handler = calloc(1, sizeof *handler);
  if (handler == NULL)
    return NULL;

  /* set all char *'s to malloc'd empty strings to be free'd later */
  EMPTYSTR(handler->buf);
  EMPTYSTR(handler->author);
  EMPTYSTR(handler->email);
  EMPTYSTR(handler->version);
  EMPTYSTR(handler->year);

  /* set default linewidth as it must be > 0 */
  handler->linewidth = LINEWIDTH_DEFAULT;

  return handler;

error:
  free(handler);
  return NULL;
#undef EMPTYSTR
}

void lice_free(lice_t *handler)
{
  if (handler == NULL)
    return;

  free(handler->buf);
  free(handler->author);
  free(handler->email);
  free(handler->version);
  free(handler->year);

  free(handler);
}

int lice_setopt(lice_t *handler, enum lice_option opt, ...)
{
  va_list args;

  va_start(args, opt);
  switch (opt) {
    case LICE_OPT_AUTHOR:
      printf("oldauthor:    %s\n", handler->author);
      printf("author:       %s\n\n", va_arg(args, char *));
      break;

    case LICE_OPT_EMAIL:
      printf("oldemail:     %s\n", handler->email);
      printf("email:        %s\n\n", va_arg(args, char *));
      break;

    case LICE_OPT_LICETYPE:
      printf("oldlicetype:  %ld\n", (long)handler->licetype);
      printf("licetype:     %ld\n\n", va_arg(args, long));
      break;

    case LICE_OPT_LINEWIDTH:
      printf("oldlinewidth  %ld\n", handler->linewidth);
      printf("linewidth:    %ld\n\n", va_arg(args, long));
      break;

    case LICE_OPT_PROGTYPE:
      printf("oldprogtype:  %ld\n", (long)handler->progtype);
      printf("progtype:     %ld\n\n", va_arg(args, long));
      break;

    case LICE_OPT_VERSION:
      printf("oldversion    %s\n", handler->version);
      printf("version:      %s\n\n", va_arg(args, char *));
      break;

    case LICE_OPT_YEAR:
      printf("oldyear:      %s\n", handler->year);
      printf("year:         %s\n\n", va_arg(args, char *));
      break;

    default:
      return -1;
  }
  va_end(args);

  return 0;
}

int lice_format(lice_t *handler)
{
  return 0;
}

char *lice_toarray(lice_t *handler)
{
  return handler->buf;
}
