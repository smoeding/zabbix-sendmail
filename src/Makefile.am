# flags
AM_CFLAGS  = $(ZABBIX_CPPFLAGS)
AM_LDFLAGS = -module -avoid-version -shared -export-dynamic

# Config file for Zabbix to load the module
zbx_sendmail_ladir = $(sysconfdir)/zabbix
dist_zbx_sendmail_la_DATA = sendmail.conf

# Redefine installation target directories
pkglibdir = $(libdir)/zabbix/modules

pkglib_LTLIBRARIES = zbx_sendmail.la

zbx_sendmail_la_SOURCES = zbx_mailstats.c sendmail.h mailstats.h

# Remove unneeded file
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/zbx_sendmail.la
