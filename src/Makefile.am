# flags
AM_CFLAGS  = $(ZABBIX_CPPFLAGS)
AM_LDFLAGS = -module -avoid-version -shared -export-dynamic

# Config file for Zabbix to load the module
sendmail_ladir = $(sysconfdir)/zabbix/zabbix_agentd.d
dist_sendmail_la_DATA = sendmail.conf

# Redefine installation target directories
pkglibdir = $(libdir)/zabbix/modules

pkglib_LTLIBRARIES = sendmail.la

sendmail_la_SOURCES = zbx_mailstats.c sendmail.h mailstats.h

# Remove unneeded file
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/sendmail.la
