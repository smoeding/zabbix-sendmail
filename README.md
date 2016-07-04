zabbix-sendmail
===============

A native Zabbix Agent module to monitor Sendmail mail statistics. It
implements additional item keys and allows the Zabbix Agent to read message
counters and sizes for each mailer from the Sendmail statistics file.

Build
-----

The package uses `libtool` and `autotools` to build. The following commands
should get you going:

```
$ libtoolize
$ aclocal -I m4
$ autoheader
$ autoconf
$ automake --add-missing
$ ./configure
$ make
```

The package needs access to the Zabbix source code during compilation. It
expects to find the sources in `/usr/src/zabbix`. This can be changed by
adding the `--with-zabbix` option to the `configure` call.

There is also a Makefile target `deb` to build a Debian package.

Setup
-----

Install the file `sendmail.so` in the location defined by the Zabbix Agent
`LoadModulePath` configuration parameter. The Debian package installs the
file in `/usr/lib/zabbix/modules`.

Add the setting `LoadModule=sendmail.so` to the Zabbix Agent configuration
file. This will be created automatically by the Debian package.

The user that runs the Zabbix Agent (`zabbix` in a default setup) needs to
have permission to read the Sendmail statistics file. There are two
possibilities:

- Add the user `zabbix` to the necessary group (`smmsp` on Debian).
- Make sure the file is world-readable (if your security policy allows this).

You can test the setup by running the following command:

```
$ zabbix_agentd --print
```

The output should include a number of `sendmail.*` keys. Then the Zabbix
Agent can be restarted to pick up the new keys and make them available for
monitoring.

Also make sure that Sendmail is actually providing data in the statistics
file. Check your sendmail configuration file for the path of the staistics
file:

```
$ grep StatusFile /etc/mail/sendmail.cf
```

You can use the `mailstats` utility to look at the data from the running
Sendmail daemon.

```
$ mailstats
MTA statistics...
Statistics from Wed Jul 22 20:02:04 2015
 M   msgsfr  bytes_from   msgsto    bytes_to  msgsrej msgsdis msgsqur  Mailer
 3       15        145K        0          0K        0       0       0  smtp
 4     1071      61483K      137      11664K     1681       0       0  esmtp
 7        0          0K       21        292K        0       0       0  relay
 8      907      14463K     1867      66122K       97       0       0  local
=====================================================================
 T     1993      76091K     2025      78078K     1778       0       0
 C    35374                 2044                15610
```

This output also tells you the number of the mailer (column M).

Usage
-----

You need to configure monitoring items in Zabbix using the provided keys.
There are three keys that monitor global statistics of the Sendmail daemon:

- `sendmail.conn.from`: number of connections that have been initiated by a
  remote host.
- `sendmail.conn.to`: number of times the daemon has tried to connect to a
  remote host.
- `sendmail.conn.rejected`: the number of times the daemon has rejected a
  connection request from a remote.

Configuration of these three keys in Zabbix items requires the filename of
the Sendmail statistics file as argument.

The following keys are available for monitoring individual mailer statistics:

- `sendmail.mailer.msgs.from`: number of messages received from the mailer.
- `sendmail.mailer.kbytes.from`: number of bytes received from the mailer.
- `sendmail.mailer.msgs.to`: number of messages sent via the mailer.
- `sendmail.mailer.kbytes.to`: number of bytes sent via the mailer.
- `sendmail.mailer.msgs.rejected`: number of messages rejected by the mailer.
- `sendmail.mailer.msgs.discarded`: number of messages discarded by the
  mailer.
- `sendmail.mailer.msgs.quarantined`: number of messages quarantined by the
  mailer.

These keys require the filename of the Sendmail statistics file and the
number of the mailer as arguments. Use the `mailstats` utility provided by
Sendmail to check the configured mailers and their respective number.

Copyrights
----------

Copyright (c) 2016 Stefan Möding

except for the following files

`src/mailstats.h`:

```
Copyright (c) 1998, 1999 Proofpoint, Inc. and its suppliers.
All rights reserved.
Copyright (c) 1983 Eric P. Allman.  All rights reserved.
Copyright (c) 1988, 1993
The Regents of the University of California.  All rights reserved.
```

`m4/ax_lib_zabbix.m4`:

```
Copyright (c) 2016 Ryan Armstrong <ryan@cavaliercoder.com>
```
