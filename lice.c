/*
 * lice.c
 */
#include <stdlib.h>
#include <stdio.h>          /* *** remove this **** */
#include <stdarg.h>
#include <string.h>
#include <errno.h>

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
#define EMPTYSTR(p) do { p = strdup(""); if (p == NULL) goto error; } while (0)
{
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
#define REPLACESTR(p) \
  do { char *tmp = va_arg(args, char *); \
       if (tmp) { free(p); p = strdup(tmp); if (p == NULL) goto error; } \
  } while (0)
{
  va_list args;
  size_t width;

  if (handler == NULL) {
    errno = EINVAL;
    return -1;
  }

  va_start(args, opt);
  switch (opt) {
    case LICE_OPT_AUTHOR:
      REPLACESTR(handler->author);
      break;

    case LICE_OPT_EMAIL:
      REPLACESTR(handler->email);
      break;

    case LICE_OPT_LICETYPE:
      handler->licetype = va_arg(args, enum lice_licetype);
      break;

    case LICE_OPT_LINEWIDTH:
      width = va_arg(args, size_t);
      if (width > 0)
        handler->linewidth = width;
      break;

    case LICE_OPT_PROGTYPE:
      handler->progtype = va_arg(args, enum lice_progtype);
      break;

    case LICE_OPT_VERSION:
      REPLACESTR(handler->version);
      break;

    case LICE_OPT_YEAR:
      REPLACESTR(handler->year);
      break;

    default:
      return -1;
  }
  va_end(args);

  return 0;

error:
  va_end(args);
  return -1;
}
#undef REPLACESTR

int lice_format(lice_t *handler)
{
  if (handler == NULL) {
    errno = EINVAL;
    return -1;
  }

  return 0;
}

char *lice_toarray(lice_t *handler)
{
  if (handler == NULL) {
    errno = EINVAL;
    return NULL;
  }

  return handler->buf;
}
