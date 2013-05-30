/*
 * lice.h
 *
 * Copyright (C) 2013  Brian Blumberg <blumbri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LICE_H
#define LICE_H

#ifndef LICE_LINEWIDTH_MINIMUM
# define LICE_LINEWIDTH_MINIMUM 50
#endif


typedef struct lice_t lice_t;

enum lice_option {
  LICE_OPT_AUTHOR,
  LICE_OPT_EMAIL,
  LICE_OPT_LICETYPE,
  LICE_OPT_LINEWIDTH,
  LICE_OPT_PROGTYPE,
  LICE_OPT_VERSION,
  LICE_OPT_YEAR
};

enum lice_licetype {
  LICE_LT_NONE = 0,
  LICE_LT_GPL,
  LICE_LT_LGPL,
  LICE_LT_OTHER
};

enum lice_progtype {
  LICE_PT_NONE = 0,
  LICE_PT_PROGRAM,
  LICE_PT_LICENSE,
  LICE_PT_OTHER
};

lice_t *lice_new(void);
void lice_free(lice_t *handler);
char *lice_licetype_str(enum lice_licetype licetype);
char *lice_progtype_str(enum lice_progtype progtype);
enum lice_licetype lice_licetype_type(const char *str);
enum lice_progtype lice_progtype_type(const char *str);
int lice_setopt(lice_t *handler, enum lice_option opt, ...);
int lice_get(lice_t *handler);
int lice_format(lice_t *handler);
char *lice_toarray(lice_t *handler);

#endif
