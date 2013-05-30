/*
 * lice.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "lice.h"

/* opaque data type                                                    {{{ */
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
};                                                                  /* }}} */

lice_t *lice_new(void)                                              /* {{{ */
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
  handler->linewidth = LICE_LINEWIDTH_MINIMUM;

  return handler;

error:
  free(handler);
  return NULL;
#undef EMPTYSTR
}                                                                   /* }}} */

void lice_free(lice_t *handler)                                     /* {{{ */
{
  if (handler == NULL)
    return;

  free(handler->buf);
  free(handler->author);
  free(handler->email);
  free(handler->version);
  free(handler->year);

  free(handler);
}                                                                   /* }}} */

char *lice_licetype_str(enum lice_licetype licetype)                /* {{{ */
{
  char *str;

  switch (licetype) {
    case LICE_LT_GPL:   str = "GPL";    break;
    case LICE_LT_LGPL:  str = "LGPL";   break;
    default:            str = "OTHER";  break;
  }

  return str;
}                                                                   /* }}} */

char *lice_progtype_str(enum lice_progtype progtype)                /* {{{ */
{
  char *str;

  switch (progtype) {
    case LICE_PT_PROGRAM:   str = "program";  break;
    case LICE_PT_LICENSE:   str = "license";  break;
    default:                str = "other";    break;
  }

  return str;
}                                                                   /* }}} */

enum lice_licetype lice_licetype_type(const char *str)              /* {{{ */
{
  enum lice_licetype type;
  size_t len = strlen(str);

  if (strncmp(str, "gpl", len) == 0 || strncmp(str, "GPL", len) == 0)
    type = LICE_LT_GPL;
  else if (strncmp(str, "lgpl", len) == 0 || strncmp(str, "LGPL", len) == 0)
    type = LICE_LT_LGPL;
  else
    type = LICE_LT_OTHER;

  return type;
}                                                                   /* }}} */

enum lice_progtype lice_progtype_type(const char *str)              /* {{{ */
{
  enum lice_progtype type;
  size_t len = strlen(str);

  if (strncmp(str, "program", len) == 0 || strncmp(str, "PROGRAM", len) == 0)
    type = LICE_PT_PROGRAM;
  else if (strncmp(str, "license", len) == 0 || strncmp(str, "LICENSE", len) == 0)
    type = LICE_PT_LICENSE;
  else
    type = LICE_PT_OTHER;

  return type;
}                                                                   /* }}} */

int lice_setopt(lice_t *handler, enum lice_option opt, ...)         /* {{{ */
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
#undef REPLACESTR
}                                                                   /* }}} */

int lice_get(lice_t *handler)                                       /* {{{ */
{
  int fd, saved_errno;
  struct stat sb;
  char filename[PATH_MAX];
  size_t len;

  if (handler == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* TODO: create a new option license-path which provides the directory
   * containing the license.txt files. */
  len = snprintf(filename, PATH_MAX, "%s/", LICEPATH);
  if (len > PATH_MAX - 1) {
    errno = ENAMETOOLONG;
    return -1;
  }

  switch (handler->licetype) {
    case LICE_LT_NONE:
      errno = EINVAL;
      return -1;

    case LICE_LT_GPL:
      strncat(filename, "gpl.lice", PATH_MAX - 1);
      if (strlen(filename) > PATH_MAX - 1) {
        errno = ENAMETOOLONG;
        return -1;
      }
      break;

    case LICE_LT_LGPL:
      strncat(filename, "lgpl.lice", PATH_MAX - 1);
      if (strlen(filename) > PATH_MAX - 1) {
        errno = ENAMETOOLONG;
        return -1;
      }
      break;

    case LICE_LT_OTHER:
      /* TODO: allow other so long as a new option license-name provides the
       * name of the license file. */
      errno = EINVAL;
      return -1;
  }

  fd = open(filename, O_RDONLY);
  if (fd == -1)
    return -1;

  if (fstat(fd, &sb) == -1)
    goto error;

  free(handler->buf);
  handler->buf = calloc(sb.st_size + 1, 1);
  if (handler->buf == NULL)
    goto error;
  handler->size = sb.st_size + 1;

  /* TODO: write() could also fail by writing less bytes to buf then what is
   * available in fd; however, as errno isn't updated, it will have to be
   * ignored for now, until lice gets its own errno. */
  if (read(fd, handler->buf, handler->size) == -1)
    goto error;

  return 0;

error:
  saved_errno = errno;
  close(fd);
  errno = saved_errno;
  return -1;
}                                                                   /* }}} */

int lice_format(lice_t *handler)                                    /* {{{ */
{
  off_t pos, npos;
  off_t newsize, linelen;
  char *newbuf;

  if (handler == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* the following accounts for up to three of each substitution and doesn't
   * subtract the original substituted value; it also adds twice the original
   * buf size to allow for comments; this isn't accurate but does error on
   * the side of having too much space. */
  newsize = strlen(handler->author) + strlen(handler->email)
    + strlen(handler->version) + strlen(handler->year);
  newsize += handler->progtype == LICE_PT_PROGRAM ? strlen("program") : 0;
  newsize += handler->progtype == LICE_PT_LICENSE ? strlen("license") : 0;
  newsize += handler->size*2;

  /* first, we do the substitutions */
  newbuf = calloc(newsize, 1);
  if (newbuf == NULL)
    return -1;

  pos  = 0;
  npos = 0;

  while (pos < handler->size && npos < newsize) {
    if (strncmp(&handler->buf[pos], "%author%", 8) == 0) {
      strcat(newbuf, handler->author);
      npos += strlen(handler->author);
      pos  += 8;
    } else if (strncmp(&handler->buf[pos], "%email%", 7) == 0) {
      strcat(newbuf, handler->email);
      npos += strlen(handler->email);
      pos  += 7;
    } else if (strncmp(&handler->buf[pos], "%progtype%", 10) == 0) {
      char *str = lice_progtype_str(handler->progtype);
      strcat(newbuf, str);
      npos += strlen(str);
      pos  += 10;
    } else if (strncmp(&handler->buf[pos], "%licever%", 9) == 0) {
      strcat(newbuf, handler->version);
      npos += strlen(handler->version);
      pos  += 9;
    } else if (strncmp(&handler->buf[pos], "%year%", 6) == 0) {
      strcat(newbuf, handler->year);
      npos += strlen(handler->year);
      pos  += 6;
    } else
      newbuf[npos++] = handler->buf[pos++];
  }

  free(handler->buf);
  handler->buf  = newbuf;
  handler->size = strlen(newbuf) + 1;

  /* then, add comments and newlines */
  newbuf = calloc(newsize, 1);
  if (newbuf == NULL)
    return -1;

  pos     = 0;
  npos    = 6;
  linelen = 3;

  strcat(newbuf, "/*\n * ");

  while (pos < handler->size && npos < newsize) {
    newbuf[npos] = handler->buf[pos];
    linelen++;

    if (newbuf[npos] == '\n') {
      strcpy(&newbuf[++npos], " * ");
      npos += 2;
      linelen = 3;
    }

    if (linelen == handler->linewidth) {
      while (!isspace(newbuf[npos])) {
        npos--;
        pos--;
      }
      strcpy(&newbuf[npos], "\n * ");
      npos += 3;
      linelen = 3;
    }

    npos++;
    pos++;
  }

  newbuf[npos - 2] = '\0';
  strcat(newbuf, "/\n");

  free(handler->buf);
  handler->buf  = newbuf;
  handler->size = strlen(newbuf) + 1;

  return 0;
}                                                                   /* }}} */

char *lice_toarray(lice_t *handler)                                 /* {{{ */
{
  if (handler == NULL) {
    errno = EINVAL;
    return NULL;
  }

  return handler->buf;
}                                                                   /* }}} */
