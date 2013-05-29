/*
 * headlice.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "lice.h"

static const char *progname;

static struct {
  char *author;
  char *email;
  char *version;
  char *year;

  size_t linewidth;

  enum lice_licetype licetype;
  enum lice_progtype progtype;
} config;

static enum lice_licetype get_licetype(const char *str)
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
}

static enum lice_progtype get_progtype(const char *str)
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
}

static int parseoptions(int argc, char **argv)
{
  int opt;

  static const char *shortopts = "a:e:ht:v:w:p:y";
  static const struct option longopts[] = {
    { "author",           required_argument, 0, 'a' },
    { "email",            required_argument, 0, 'e' },
    { "help",             no_argument,       0, 'h' },
    { "license-type",     required_argument, 0, 't' },
    { "license-version",  required_argument, 0, 'v' },
    { "line-width",       required_argument, 0, 'w' },
    { "program-type",     required_argument, 0, 'p' },
    { "year",             required_argument, 0, 'y' },
    { 0, 0, 0, 0 }
  };

  for (;;) {
    opt = getopt_long(argc, argv, shortopts, longopts, NULL);
    if (opt == -1)
      break;

    switch (opt) {
      case 'a':
        if (config.author == NULL)
          config.author = optarg;
        break;

      case 'e':
        if (config.email == NULL)
          config.email = optarg;
        break;

      case 'h':
        printf("*** create a usage() damnit!! ***\n");
        exit(EXIT_SUCCESS);
        break;

      case 't':
        if (config.licetype == LICE_LT_NONE)
          config.licetype = get_licetype(optarg);
        break;

      case 'v':
        if (config.version == NULL)
          config.version = optarg;
        break;

      case 'w':
        if (config.linewidth == 0) {
          char *ptr;
          long width = strtol(optarg, &ptr, 10);

          if (*ptr != '\0') {
            fprintf(stderr, "%s: invalid --line-width argument -- '%s'\n",
                progname, optarg);
            return -1;
          }

          if (width <= 0) {
            fprintf(stderr, "%s: --line-width argument must be positive\n",
                progname);
            return -1;
          }

          config.linewidth = width;
        }
        break;

      case 'p':
        if (config.progtype == LICE_PT_NONE)
          config.progtype = get_progtype(optarg);
        break;

      case 'y':
        if (config.year == NULL)
          config.year = optarg;
        break;

      default:
        return -1;
    }
  }

  return 0;
}

int main(int argc, char **argv)
{

  lice_t *handler;
  FILE *fp = NULL;
  char *licehead;

  progname = argv[0];

  if (parseoptions(argc, argv) == -1) {
    fprintf(stderr, "Try --help for more information\n");
    exit(EXIT_FAILURE);
  }

  if (optind < argc) {
    fp = fopen(argv[optind], "a");
    if (fp == NULL) {
      fprintf(stderr, "%s: could not open %s for writing (%s)\n", progname,
          argv[optind], strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  handler = lice_new();
  if (handler == NULL) {
    fprintf(stderr, "%s: could not create license (%s)\n", progname,
        strerror(errno));
    exit(EXIT_FAILURE);
  }

  lice_setopt(handler, LICE_OPT_AUTHOR,    config.author);
  lice_setopt(handler, LICE_OPT_EMAIL,     config.email);
  lice_setopt(handler, LICE_OPT_VERSION,   config.version);
  lice_setopt(handler, LICE_OPT_YEAR,      config.year);
  lice_setopt(handler, LICE_OPT_LINEWIDTH, config.linewidth);
  lice_setopt(handler, LICE_OPT_LICETYPE,  config.licetype);
  lice_setopt(handler, LICE_OPT_PROGTYPE,  config.progtype);

  if (lice_format(handler) == -1) {
    fprintf(stderr, "%s: could not format license (%s)\n", progname,
        strerror(errno));
    exit(EXIT_FAILURE);
  }

  licehead = lice_toarray(handler);
  fputs(licehead, fp ? fp : stdout);

  lice_free(handler);
  exit(EXIT_SUCCESS);
}
