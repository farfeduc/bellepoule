#!/bin/bash
lighty-disable-mod bellepouledebug;
if test ${?} -ne 0 ; then
  exit -1;
fi

/etc/init.d/lighttpd stop;
if test ${?} -ne 0 ; then
  exit -1;
fi

exit 0;