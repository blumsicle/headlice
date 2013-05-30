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
static lice_t *handler;

static struct config {                                              /* {{{ */
  char *author;
  char *email;
  char *version;
  char *year;

  size_t linewidth;

  enum lice_licetype licetype;
  enum lice_progtype progtype;

  FILE *fp;
} config;                                                           /* }}} */

static void usage(void);
static void cleanup(void);
static int parseoptions(int argc, char **argv);

static void usage(void)                                             /* {{{ */
{
  int i;

  fprintf(stderr, "Usage: %s [OPTION]... [FILE]\n"
                  "Write desired license in comment block to stdout or "
                  "append to FILE.\n\n", progname);

  fprintf(stderr, "  -a, --author=NAME              replace %%author%% with "
                                          "NAME in license output\n");
  fprintf(stderr, "  -e, --email=EMAIL              replace %%email%% with "
                                          "EMAIL in license output\n");
  fprintf(stderr, "  -h, --help                     display this help and "
                                          "exit\n");
  fprintf(stderr, "  -t, --license-type=LICENSE     select LICENSE template "
                                          "to use\n");
  fprintf(stderr, "  -v, --license-version=VERSION  replace %%licever%% with "
                                          "VERSION in license output\n");
  fprintf(stderr, "  -w, --line-width=WIDTH         maximum output line "
                                          "width\n");
  fprintf(stderr, "  -p, --program-type=PROGRAM     type of program the "
                                          "license pertains to\n");
  fprintf(stderr, "  -y, --year=YEAR                replace %%year%% with "
                                          "YEAR in license output\n\n");

  fprintf(stderr, "WIDTH is in integer that must be greater than %ld.\n",
      (long)LICE_LINEWIDTH_MINIMUM);

  fprintf(stderr, "LICENSE can be one of:");
  for (i = 1; i <= LICE_LT_OTHER; i++)
    fprintf(stderr, " %s", lice_licetype_str(i));
  fprintf(stderr, ".\n");

  fprintf(stderr, "PROGRAM can be one of:");
  for (i = 1; i <= LICE_PT_OTHER; i++)
    fprintf(stderr, " %s", lice_progtype_str(i));
  fprintf(stderr, ".\n\n");

  fprintf(stderr, "The default directory of the licenses is: %s\n\n", LICEPATH);
  fprintf(stderr, "NOTE: As stated previously, if FILE is provided, the "
                  " license will be\n*appended* to FILE.  It will not "
                  "overwrite FILE, nor will it be placed\npreceeding the "
                  "previous content.\n");
}                                                                   /* }}} */

static void cleanup(void)                                           /* {{{ */
{
  if (handler)
    lice_free(handler);
  if (config.fp)
    fclose(config.fp);
}                                                                   /* }}} */

static int parseoptions(int argc, char **argv)                      /* {{{ */
{
  int opt;

  static const char *shortopts = "a:e:hp:t:v:w:y:";
  static const struct option longopts[] = {
    { "author",           required_argument, 0, 'a' },
    { "email",            required_argument, 0, 'e' },
    { "help",             no_argument,       0, 'h' },
    { "program-type",     required_argument, 0, 'p' },
    { "license-type",     required_argument, 0, 't' },
    { "license-version",  required_argument, 0, 'v' },
    { "line-width",       required_argument, 0, 'w' },
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
        usage();
        exit(EXIT_SUCCESS);

      case 'p':
        if (config.progtype == LICE_PT_NONE)
          config.progtype = lice_progtype_type(optarg);
        break;

      case 't':
        if (config.licetype == LICE_LT_NONE)
          config.licetype = lice_licetype_type(optarg);
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

          if (width < LICE_LINEWIDTH_MINIMUM) {
            fprintf(stderr,
                "%s: --line-width argument must be greater than %ld\n",
                progname, (long)LICE_LINEWIDTH_MINIMUM);
            return -1;
          }

          config.linewidth = width;
        }
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
}                                                                   /* }}} */

int main(int argc, char **argv)                                     /* {{{ */
{
  char *licehead;

  progname = argv[0];

  atexit(cleanup);

  if (parseoptions(argc, argv) == -1) {
    fprintf(stderr, "Try --help for more information\n");
    exit(EXIT_FAILURE);
  }

  if (optind < argc && argv[optind][0] != '-') {
    config.fp = fopen(argv[optind], "a");
    if (config.fp == NULL) {
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

  /* TODO: right now, these options are being set regardless of their
   * contents; lice_setopt() can handle NULL and invalid options, but this
   * should be fixed. */
  lice_setopt(handler, LICE_OPT_AUTHOR,    config.author);
  lice_setopt(handler, LICE_OPT_EMAIL,     config.email);
  lice_setopt(handler, LICE_OPT_VERSION,   config.version);
  lice_setopt(handler, LICE_OPT_YEAR,      config.year);
  lice_setopt(handler, LICE_OPT_LINEWIDTH, config.linewidth);
  lice_setopt(handler, LICE_OPT_LICETYPE,  config.licetype);
  lice_setopt(handler, LICE_OPT_PROGTYPE,  config.progtype);

  if (lice_get(handler) == -1) {
    fprintf(stderr, "%s: could not format license (%s)\n", progname,
        strerror(errno));
    exit(EXIT_FAILURE);
  }

  lice_format(handler);

  licehead = lice_toarray(handler);
  fputs(licehead, config.fp ? config.fp : stdout);

  exit(EXIT_SUCCESS);
}                                                                   /* {{{ */
