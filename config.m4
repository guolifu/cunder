dnl $Id$
dnl config.m4 for extension thunder

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

 PHP_ARG_WITH(thunder, for thunder support,
 Make sure that the comment is aligned:
 [  --with-thunder             Include thunder support])

dnl Otherwise use enable:

 PHP_ARG_ENABLE(thunder, whether to enable thunder support,
 Make sure that the comment is aligned:
 [  --enable-thunder           Enable thunder support])

if test "$PHP_THUNDER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-thunder -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/thunder.h"  # you most likely want to change this
  dnl if test -r $PHP_THUNDER/$SEARCH_FOR; then # path given as parameter
  dnl   THUNDER_DIR=$PHP_THUNDER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for thunder files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       THUNDER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$THUNDER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the thunder distribution])
  dnl fi

  dnl # --with-thunder -> add include path
  dnl PHP_ADD_INCLUDE($THUNDER_DIR/include)

  dnl # --with-thunder -> check for lib and symbol presence
  dnl LIBNAME=thunder # you may want to change this
  dnl LIBSYMBOL=thunder # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $THUNDER_DIR/$PHP_LIBDIR, THUNDER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_THUNDERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong thunder lib version or lib not found])
  dnl ],[
  dnl   -L$THUNDER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(THUNDER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(thunder, thunder.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
