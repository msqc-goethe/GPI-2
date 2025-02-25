################################################
# Check and select device
# ----------------------------------
AC_DEFUN([ACX_USABLE_DEVICE],[
        if test x${with_infiniband} != xno -a x${with_ethernet} != xno -a x${with_bxi} != xno; then
           TITLE([Checking for device(s):])
           AC_MSG_ERROR([Concurrently Infiniband, PORTALS and Ethernet is not supported])
        elif test x${with_infiniband} != xno -a x${with_ethernet} = xno -a x${with_bxi} = xno; then
           TITLE([Checking for Infiniband])
           ACX_INFINIBAND
           if test x${HAVE_INFINIBAND} = x0; then
              AC_MSG_ERROR([Infiniband requested, but can not use it])
           fi
        elif test x${with_infiniband} = xno -a x${with_ethernet} != xno -a x${with_bxi} = xno; then
           TITLE([Checking for Ethernet])
           ACX_ETHERNET
           if test x${HAVE_TCP} = x0; then
              AC_MSG_ERROR([Ethernet requested, but can not use it])
           fi
	elif test x${with_infiniband} = xno -a x${with_ethernet} = xno -a x${with_bxi} != xno; then
	   TITLE([Checking for PORTALS])
	   ACX_PORTALS
	   if test x${HAVE_PORTALS} = x0; then
	      AC_MSG_ERROR([PORTALS requested, but can not use it])
	   fi
        else
	   TITLE([Infiniband, PORTALS or Ethernet is required, checking for Infiniband...])
           with_infiniband=yes
           ACX_INFINIBAND
           if test x${HAVE_INFINIBAND} = x0; then
	      AC_MSG_NOTICE([Infiniband can not be used])
	      TITLE([Checking for PORTALS])
	      ACX_PORTALS
	      if test x${HAVE_PORTALS} = x0; then
		AC_MSG_NOTICE([PORTALS can not be used])
              	TITLE([Checking for Ethernet])
              	ACX_ETHERNET
              	if test x${HAVE_TCP} = x0; then
              		 AC_MSG_ERROR([Neither Infiniband nor Ethernet are usable])
		fi
              fi
           fi
        fi

	# COPY DEFAULT FILES FOR TESTING
	AM_CONDITIONAL([WITH_PORTALS],[test x${HAVE_PORTALS} = x1])
	if [test x${HAVE_PORTALS} = x1]; then
	   options="$options Portals"
	fi
        AM_CONDITIONAL([WITH_ETHERNET], test x${HAVE_TCP} = x1)
        if [test x${HAVE_TCP} = x1]; then
           options="$options Ethernet"
        fi
        AM_CONDITIONAL([WITH_INFINIBAND],[test x${HAVE_INFINIBAND} = x1])
 	AM_CONDITIONAL([WITH_INFINIBAND_EXT],[test x${HAVE_INFINIBAND_EXT} = x1 -a x$infiniband_ext != xno])
        if [test x${HAVE_INFINIBAND} = x1]; then
           if [test x${HAVE_INFINIBAND_EXT} = x1 -a x$infiniband_ext != xno]; then
              options="$options Infiniband Extensions"
	   else
	      options="$options Infiniband"
	   fi
	   if [test x${HAVE_INFINIBAND_DEVICES} = x1]; then
	      options="$options with actual devices"
	   else
	      options="$options without actual devices"
	   fi
        fi
	])

################################################
# Check and set INFINIBAND path
# ----------------------------------
AC_DEFUN([ACX_INFINIBAND],[
	if test "x$with_infiniband" != xno; then
   	   if test "x$with_infiniband" != xyes; then
	      # User specifies path(s)
	      ac_path_infiniband=$with_infiniband
      	      ac_inc_infiniband=$ac_path_infiniband/include/infiniband
	      AC_CHECK_FILE($ac_inc_infiniband/verbs.h,
	      	      [HAVE_INF_HEADER=1],[HAVE_INF_HEADER=0])
	      AC_CHECK_FILE($ac_inc_infiniband/verbs_exp.h,
			    [HAVE_INFINIBAND_EXT=1],[HAVE_INFINIBAND_EXT=0])
	      for iblib in libibverbs.so libibverbs.a; do
	          for iblib_path in lib lib64; do
	      	      ac_lib_infiniband=$ac_path_infiniband/$iblib_path
		      AC_CHECK_FILE($ac_lib_infiniband/$iblib,[HAVE_INF_LIB=1],[HAVE_INF_LIB=0])
		      if test ${HAVE_INF_LIB} = 1; then
	      	          break
		      fi
		  done
	          if test ${HAVE_INF_LIB} = 1; then
	             break
	  	  fi
  	      done
   	   else
	      # Try to determine include path(s)
	      inc_paths=`cpp -v /dev/null >& cppt`
	      inc_paths=`sed -n '/^#include </,/^End/p' cppt | sed '1d;$d'`
	      rm -f cppt
	      for ibinc in $inc_paths; do
	      	  ac_inc_infiniband=$ibinc/infiniband
		  AC_CHECK_FILE($ac_inc_infiniband/verbs.h,
		  	      	  [HAVE_INF_HEADER=1],[HAVE_INF_HEADER=0])
	      	  if [test ${HAVE_INF_HEADER} = 1 -a x$infiniband_ext != xno]; then
		     AC_CHECK_FILE($ac_inc_infiniband/verbs_exp.h,
				   [HAVE_INFINIBAND_EXT=1],[HAVE_INFINIBAND_EXT=0])
	      	     break
	      	  fi
	      done
	      # Try to determine library path(s)
	      for ibinc in $inc_paths; do
	      	  ac_path_infiniband=${ibinc%/include*}
		  for iblib in libibverbs.so libibverbs.a; do
	              for iblib_path in lib lib64; do
	      	      	  ac_lib_infiniband=$ac_path_infiniband/$iblib_path
		  	  AC_CHECK_FILE($ac_lib_infiniband/$iblib,[HAVE_INF_LIB=1],[HAVE_INF_LIB=0])
		          if test ${HAVE_INF_LIB} = 1; then
	      	      	     break
		          fi
		      done
	              if test ${HAVE_INF_LIB} = 1; then
	              	 break
	  	      fi
  		  done
		  if test ${HAVE_INF_LIB} = 1; then
	             break
	  	  fi
	      done
	      # If the above lib search fails, use autotools
	      if test ${HAVE_INF_LIB} != 1; then
 	         ac_lib_infiniband=
	      	 AC_CHECK_LIB([ibverbs],[ibv_open_device],[HAVE_INF_LIB=1],[HAVE_INF_LIB=0])
  	      fi
   	   fi
	fi
	if test ${HAVE_INF_HEADER} = 1 -a ${HAVE_INF_LIB} = 1; then
	   ACX_IB_DEVI($ac_inc_infiniband,$ac_lib_infiniband,[HAVE_INFINIBAND=1],[HAVE_INFINIBAND=0])
	   AC_SUBST(ac_inc_infiniband,[-I$ac_inc_infiniband])
	   if test ! -z $ac_lib_infiniband; then
	      AC_SUBST(ac_lib_infiniband,["-L$ac_lib_infiniband -libverbs"])
	   else
	      AC_SUBST(ac_lib_infiniband,[-libverbs])
	   fi
	else
	   HAVE_INFINIBAND=0
	fi
	])

AC_DEFUN([ACX_IB_DEVI],[
	AC_MSG_CHECKING([whether ibverbs contains IBV_LINK_LAYER_ETHERNET and basic device routines])

	AC_LANG_PUSH(C)

cat >conftest_ib.c <<_ACEOF
#include <verbs.h>
#include <assert.h>
#include <stdio.h>

int main(){
int a = IBV_LINK_LAYER_ETHERNET;
struct ibv_device **device_list;
int num_devices;
device_list = ibv_get_device_list(&num_devices);
if (device_list != NULL){
ibv_free_device_list(device_list);
}
return num_devices;
}
_ACEOF

	OLD_CFLAGS=$CFLAGS
	if test ! -z $2; then
	   CFLAGS="$AM_CFLAGS $CFLAGS -Wno-unused-variable -I$1 -L$2 -libverbs"
	else
	   CFLAGS="$AM_CFLAGS $CFLAGS -Wno-unused-variable -I$1 -libverbs"
        fi
	AS_IF($CC conftest_ib.c $CFLAGS -o conftest_ib.exe,
	  [AC_MSG_RESULT([yes]);
	  $3],
	  [AC_MSG_RESULT([no]);
	  $4]
	  )
	CFLAGS=$OLD_CFLAGS
	AC_LANG_POP([C])

AC_MSG_CHECKING([whether there are actual IB devices])
AS_IF(test `./conftest_ib.exe; echo $?` -gt 0,
	   [AC_MSG_RESULT([yes]);
	   HAVE_INFINIBAND_DEVICES=1],
	   [AC_MSG_RESULT([no]);
	   HAVE_INFINIBAND_DEVICES=0]
	   )
])

################################################
# Check and set ETHERNET path
# ----------------------------------
AC_DEFUN([ACX_ETHERNET],[
	AC_CHECK_HEADER(netinet/tcp.h,[HAVE_TCP=1],[HAVE_TCP=0])
	])

################################################
# Check and set PORTALS path
# ----------------------------------
AC_DEFUN([ACX_PORTALS],[
	if test "x$with_portals" != xno; then
   	   if test "x$with_portals" != xyes; then
	      # User specifies path(s)
	      ac_path_portals=$with_portals
      	      ac_inc_portals=$ac_path_portals/include/
	      AC_CHECK_FILE($ac_inc_portals/portals4.h,
	      	      [HAVE_PORTALS_HEADER=1],[HAVE_PORTALS_HEADER=0])
	      AC_CHECK_FILE($ac_inc_portals/portals4_bxiext.h,
			    [HAVE_PORTALS_BXI_EXT=1],[HAVE_PORTALS_BXI_EXT=0])
	      AC_CHECK_FILE($ac_inc_portals/portals4_services.h,
			    [HAVE_PORTALS_SERVICES=1],[HAVE_PORTALS_SERVICES=0])
	      for portalslib in libportals.so libportals.a; do
	          for portalslib_path in lib lib64; do
	      	      ac_lib_portals=$ac_path_portals/$portalslib_path
		      AC_CHECK_FILE($ac_lib_portals/$portalslib,[HAVE_PORTALS_LIB=1],[HAVE_PORTALS_LIB=0])
		      if test ${HAVE_PORTALS_LIB} = 1; then
	      	          break
		      fi
		  done
	          if test ${HAVE_PORTALS_LIB} = 1; then
	             break
	  	  fi
  	      done
   	   else
	      # Try to determine include path(s)
	      inc_paths=`cpp -v /dev/null >& cppt`
	      inc_paths=`sed -n '/^#include </,/^End/p' cppt | sed '1d;$d'`
	      rm -f cppt
	      for portalsinc in $inc_paths; do
	      	  ac_inc_portals=$portalsinc
		  AC_CHECK_FILE($ac_inc_portals/portals4.h,
		  	      	  [HAVE_PORTALS_HEADER=1],[HAVE_PORTALS_HEADER=0])
	      	  if [test ${HAVE_PORTALS_HEADER} = 1 -a x$portals_bxi_ext != xno]; then
		     AC_CHECK_FILE($ac_inc_portals/portals4_bxiext.h,
				   [HAVE_PORTALS_BXI_EXT=1],[HAVE_PORTALS_BXI_EXT=0])
	      	     break
	      	  fi
		  if [test ${HAVE_PORTALS_HEADER} = 1 -a x$portals_services != xno]; then
		     AC_CHECK_FILE($ac_inc_portals/portals4_services.h,[HAVE_PORTALS_SERVICES=1],
		   		   [HAVE_PORTALS_SERVICES=0])
		  fi
	      done
	      # Try to determine library path(s)
	      for portalsinc in $inc_paths; do
	      	  ac_path_portals=${portalsinc%/include*}
	          for portalslib in libportals.so libportals.a; do
	              for portalslib_path in lib lib64; do
	      	      	  ac_lib_portals=$ac_path_portals/$portalslib_path
	          	  AC_CHECK_FILE($ac_lib_portals/$portalslib,[HAVE_PORTALS_LIB=1],[HAVE_PORTALS_LIB=0])
	                  if test ${HAVE_PORTALS_LIB} = 1; then
	      	      	     break
	                  fi
	              done
	              if test ${HAVE_PORTALS_LIB} = 1; then
	              	 break
	              fi
  	          done
	          if test ${HAVE_PORTALS_LIB} = 1; then
	             break
	          fi
	      done
	      # If the above lib search fails, use autotools
	      #if test ${HAVE_PORTALS_LIB} != 1; then
 	      #   ac_lib_portals=
	      #	 AC_CHECK_LIB([portals],[PtlInit],[HAVE_PORTALS_LIB=1],[HAVE_PORTALS_LIB=0])
  	      #fi
   	   fi
	fi
	if test ${HAVE_PORTALS_HEADER} = 1 -a ${HAVE_PORTALS_LIB} = 1; then
		HAVE_PORTALS=1
	else
		HAVE_PORTALS=0
	fi
#	if test ${HAVE_INF_HEADER} = 1 -a ${HAVE_INF_LIB} = 1; then
#	   ACX_IB_DEVI($ac_inc_infiniband,$ac_lib_infiniband,[HAVE_INFINIBAND=1],[HAVE_INFINIBAND=0])
#	   AC_SUBST(ac_inc_infiniband,[-I$ac_inc_infiniband])
#	   if test ! -z $ac_lib_infiniband; then
#	      AC_SUBST(ac_lib_infiniband,["-L$ac_lib_infiniband -libverbs"])
#	   else
#	      AC_SUBST(ac_lib_infiniband,[-libverbs])
#	   fi
#	else
#	   HAVE_INFINIBAND=0
#	fi
	])
