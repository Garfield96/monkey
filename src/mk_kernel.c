/*-*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2014, Eduardo Silva P. <edsiper@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public  License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <ctype.h>
#include <pthread.h>
#include <sys/utsname.h>

#include "mk_kernel.h"
#include "mk_string.h"
#include "mk_utils.h"
#include "mk_server.h"

int mk_kernel_init()
{
    mk_kernel_runver = mk_kernel_version();
    return 0;
}

int mk_kernel_version()
{
    int a, b, c;
    int len;
    int pos;
    char *p, *t;
    char *tmp;
    struct utsname uts;

    if (uname(&uts) == -1) {
        mk_libc_error("uname");
    }

    len = strlen(uts.release);

    /* Fixme: this don't support Linux Kernel 10.x.x :P */
    a = (*uts.release - '0');

    /* Second number */
    p = (uts.release) + 2;
    pos = mk_string_char_search(p, '.', len - 2);
    if (pos <= 0) {
        return -1;
    }

    tmp = mk_string_copy_substr(p, 0, pos);
    if (!tmp) {
        return -1;
    }
    b = atoi(tmp);
    mk_mem_free(tmp);

    /* Last number (it needs filtering) */
    t = p = p + pos + 1;
    do {
        t++;
    } while (isdigit(*t));

    tmp = mk_string_copy_substr(p, 0, t - p);
    if (!tmp) {
        return -1;
    }
    c = atoi(tmp);
    mk_mem_free(tmp);

    return MK_KERNEL_VERSION(a, b, c);
}

/* Detect specific Linux Kernel features that we may use */
int mk_kernel_features()
{
    int flags = 0;

    /* TCP Auto Corking */
    if (mk_kernel_runver >= MK_KERNEL_VERSION(3, 14, 0) &&
        mk_socket_tcp_autocorking() == MK_TRUE) {
        flags |= MK_KERNEL_TCP_AUTOCORKING;
    }

    /* SO_REUSEPORT */
    if (mk_kernel_runver >= MK_KERNEL_VERSION(3, 9, 0)) {
        flags |= MK_KERNEL_SO_REUSEPORT;
    }

    /* TCP_FASTOPEN */
    if (mk_kernel_runver >= MK_KERNEL_VERSION(3, 7, 0)) {
        flags |= MK_KERNEL_TCP_FASTOPEN;
    }

    config->kernel_features = flags;
    return flags;
}

int mk_kernel_features_print(char *buffer, size_t size)
{
    int offset = 0;
    int features = 0;
    char *tmp;

    if (config->kernel_features & MK_KERNEL_TCP_FASTOPEN) {
        offset += snprintf(buffer, size - offset, "%s", "TCP_FASTOPEN ");
        features++;
    }

    if (config->kernel_features & MK_KERNEL_SO_REUSEPORT) {
        offset += snprintf(buffer + offset, size - offset, "%s", "SO_REUSEPORT ");
        features++;
    }

    if (config->kernel_features & MK_KERNEL_TCP_AUTOCORKING) {
        snprintf(buffer + offset, size - offset, "%s", "TCP_AUTOCORKING ");
        features++;
    }

    return features;
}
