#!/bin/sh

## usage :
##        tftp_update.sh DIR SERVER_IP REMOTE_FILE_NAME TARGET QUIET

cd /tmp

# the image update allow one process one time.
if [ "$4" == image ] ; then
	sign_mark="-s $(cat /etc/image_sign)"
	locker=/tmp/image_update.locker

	trap "rm -rf $locker; exit 0" 2
	[ -e "$locker" ] && echo "% doing!" && exit 0
	touch $locker
fi

case "$1" in
get|GET)
	err=`tftp -r $3 $sign_mark -g $2 2>&1`
	if [ -z "$err" ] ; then
	case "$4" in
	startup)
		result=`vtysh -C -f $3`
		[ ! -z "$result" ] && {
			echo "%% Unknown format or invalid commands." 
			rm $3
			exit 0
		}

		cp $3 /config/startup.conf
		;;
	image)
		burn_image.sh $3 $5
		;;
	esac

	rm $3
	else
		echo "$err"
	fi
	;;

put|PUT)
	case "$4" in
	startup) 
		local_file="/config/startup.conf"
		remote_file=$3
		;;
	*)
		local_file=$4
		remote_file=$3
		;;
	esac

	if [ -e $local_file ]; then
	tftp -l $local_file -r $remote_file -p $2
	else
	echo "%% Not such file!"
	fi

	;;
esac

[ "$4" == image ] &&  rm $locker
