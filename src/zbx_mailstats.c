/**************************************************************************
 *
 * Copyright (c) 2016 Stefan Moeding <stm@kill-9.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 **************************************************************************/


#include "config.h"


#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

/* Sendmail includes */
#include "sendmail.h"
#include "mailstats.h"

/* Zabbix includes */
#include "module.h"


static struct statistics sendmail_stat;


/* timeout setting for item processing */
static int item_timeout = 0;

/* function prototype to be used for keys[] */
int zbx_module_mailstats(AGENT_REQUEST *request, AGENT_RESULT *result);

static ZBX_METRIC keys[] = {
  { "sendmail.conn.from",               CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st"   },
  { "sendmail.conn.to",                 CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st"   },
  { "sendmail.conn.rejected",           CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st"   },
  { "sendmail.mailer.msgs.from",        CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { "sendmail.mailer.kbytes.from",      CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { "sendmail.mailer.msgs.to",          CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { "sendmail.mailer.kbytes.to",        CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { "sendmail.mailer.msgs.rejected",    CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { "sendmail.mailer.msgs.discarded",   CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { "sendmail.mailer.msgs.quarantined", CF_HAVEPARAMS, zbx_module_mailstats, "/var/lib/sendmail/sendmail.st,1" },
  { NULL }
};

/*****************************************************************************
 *
 * Function: zbx_module_api_version
 *
 * Purpose: returns version number of the module interface
 *
 * Return value: ZBX_MODULE_API_VERSION_ONE - the only version supported by
 *               Zabbix currently
 *
 */

int zbx_module_api_version() {
  return ZBX_MODULE_API_VERSION_ONE;
}


/*****************************************************************************
 *
 * Function: zbx_module_item_timeout
 *
 * Purpose: set timeout value for processing of items
 *
 * Parameters: timeout - timeout in seconds, 0 - no timeout set
 *
 */

void zbx_module_item_timeout(int timeout) {
  item_timeout = timeout;
}


/*****************************************************************************
 *
 * Function: zbx_module_item_list
 *
 * Purpose: returns list of item keys supported by the module
 *
 * Return value: list of item keys
 *
 */

ZBX_METRIC *zbx_module_item_list() {
  return keys;
}


/*****************************************************************************
 *
 * Function: zbx_module_mailstats
 *
 * Purpose: a main entry point for processing of an item
 *
 * Parameters: request - structure that contains item key and parameters
 *              request->key - item key without parameters
 *              request->nparam - number of parameters
 *              request->timeout - processing should not take longer than
 *                                 this number of seconds
 *              request->params[N-1] - pointers to item key parameters
 *
 *             result - structure that will contain result
 *
 * Return value: SYSINFO_RET_FAIL - function failed, item will be marked
 *                                 as not supported by zabbix
 *               SYSINFO_RET_OK - success
 *
 * Comment: get_rparam(request, N-1) can be used to get a pointer to the Nth
 *          parameter starting from 0 (first parameter). Make sure it exists
 *          by checking value of request->nparam.
 *
 */
int zbx_module_mailstats(AGENT_REQUEST *request, AGENT_RESULT *result) {
  struct stat	 sb;
  long int	 mailer = 0L;
  char		*statsfile;
  char		*param;
  int		 fd;

  /*
   * Check expected number of arguments
   */
  if (strncmp(request->key, "sendmail.conn.", 14) == 0) {
    if (request->nparam != 1) {
      SET_MSG_RESULT(result, strdup("Invalid number of parameters (expected 1)."));
      return SYSINFO_RET_FAIL;
    }
  }
  else if (strncmp(request->key, "sendmail.mailer.", 16) == 0) {
    if (request->nparam != 2) {
      SET_MSG_RESULT(result, strdup("Invalid number of parameters (expected 2)."));
      return SYSINFO_RET_FAIL;
    }
  }
  else {
    SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
    return SYSINFO_RET_FAIL;
  }

  /*
   * Collect and parse arguments
   */
  switch(request->nparam) {
  case 2:
    param = get_rparam(request, 1);
    errno = 0;

    mailer = strtol(param, NULL, 10);

    if ((errno != 0) || (mailer < 0) || (mailer >= MAXMAILERS)) {
      SET_MSG_RESULT(result, strdup("Invalid mailer number."));
      return SYSINFO_RET_FAIL;
    }

    /* fall through to get first parameter */

  case 1:
    statsfile = get_rparam(request, 0);

    if (stat(statsfile, &sb) == -1) {
      SET_MSG_RESULT(result, strdup("Invalid statistics file."));
      return SYSINFO_RET_FAIL;
    }
    break;

  default:
    SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
    return SYSINFO_RET_FAIL;
  }

  /*
   * Read the statistics file
   */
  fd = open(statsfile, O_RDONLY|O_RDONLY);

  if (fd < 0) {
    SET_MSG_RESULT(result, strdup("Unable to open statistics file."));
    return SYSINFO_RET_FAIL;
  }

  if (read(fd, &sendmail_stat, sizeof(sendmail_stat)) != sizeof(sendmail_stat)) {
    close(fd);
    SET_MSG_RESULT(result, strdup("Unable to read statistics file."));
    return SYSINFO_RET_FAIL;
  }

  close(fd);

  /*
   * Check magic cookies
   */
  if (sendmail_stat.stat_magic != STAT_MAGIC) {
    SET_MSG_RESULT(result, strdup("Wrong magic number in statistics file."));
    return SYSINFO_RET_FAIL;
  }

  if (sendmail_stat.stat_version != STAT_VERSION) {
    SET_MSG_RESULT(result, strdup("Wrong version number in statistics file."));
    return SYSINFO_RET_FAIL;
  }

  if (sendmail_stat.stat_size != sizeof(sendmail_stat)) {
    SET_MSG_RESULT(result, strdup("Wrong size of statistics file."));
    return SYSINFO_RET_FAIL;
  }

  /*
   * Get value for key from statistics
   */
  if (strcmp(request->key, "sendmail.conn.from") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_cf);
  }
  else if (strcmp(request->key, "sendmail.conn.to") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_ct);
  }
  else if (strcmp(request->key, "sendmail.conn.rejected") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_cr);
  }
  else if (strcmp(request->key, "sendmail.mailer.msgs.from") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_nf[mailer]);
  }
  else if (strcmp(request->key, "sendmail.mailer.kbytes.from") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_bf[mailer]);
  }
  else if (strcmp(request->key, "sendmail.mailer.msgs.to") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_nt[mailer]);
  }
  else if (strcmp(request->key, "sendmail.mailer.kbytes.to") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_bt[mailer]);
  }
  else if (strcmp(request->key, "sendmail.mailer.msgs.rejected") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_nr[mailer]);
  }
  else if (strcmp(request->key, "sendmail.mailer.msgs.discarded") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_nd[mailer]);
  }
  else if (strcmp(request->key, "sendmail.mailer.msgs.quarantined") == 0) {
    SET_UI64_RESULT(result, sendmail_stat.stat_nq[mailer]);
  }
  else {
    SET_MSG_RESULT(result, strdup("Invalid key."));
    return SYSINFO_RET_FAIL;
  }

  return SYSINFO_RET_OK;
}


/*****************************************************************************
 *
 * Function: zbx_module_init
 *
 * Purpose: the function is called on agent startup
 *          It should be used to call any initialization routines
 *
 * Return value: ZBX_MODULE_OK - success
 *               ZBX_MODULE_FAIL - module initialization failed
 *
 * Comment: the module won't be loaded in case of ZBX_MODULE_FAIL
 *
 */
int zbx_module_init() {
  return ZBX_MODULE_OK;
}

/*****************************************************************************
 *
 * Function: zbx_module_uninit
 *
 * Purpose: the function is called on agent shutdown
 *          It should be used to cleanup used resources if there are any
 *
 * Return value: ZBX_MODULE_OK - success
 *               ZBX_MODULE_FAIL - function failed
 *
 */
int zbx_module_uninit() {
  return ZBX_MODULE_OK;
}
