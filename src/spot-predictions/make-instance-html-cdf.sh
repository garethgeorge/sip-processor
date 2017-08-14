#!/bin/bash

HERE=/mnt/data
BIN=/root/bin

INSTTYPE=$1
EVEN=""

if ( test -z "$INSTTYPE" ) ; then
echo "must input the specifc spot instance image type (UNIX, Windows, SUSE)"
exit 1
fi

echo '<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">'
echo "<head>"
	echo '<meta http-equiv="content-type" content="text/html; charset=utf-8" />'
	echo '<meta name="description" content="AWS Spot Bids by AZ" />'
	echo '<link href='http://fonts.googleapis.com/css?family=Dosis' rel='stylesheet' type='text/css' />'
	echo '<link rel="stylesheet" type="text/css" href="./table.css" title="Table" media="all" />'
	echo '<title>AWS Spot Bids by AZ</title>'
echo "</head>"
echo '<body class="light blue smaller freestyle01">'

NOW=`date`

echo "<h2>Spot Price Predictions by Instance Type</h2>"
echo "<h3>$NOW</h3>"



for TYPE in `cat $HERE/instance-types.txt` ; do
	echo "<h3>$TYPE $INSTTYPE</h3>"
	echo "<table cellspacing='0'>"
	echo "<thead>"
	echo "<tr>"
		echo "<th>Region/Availability Zone</th>"
		echo "<th>Time</th>"
		echo "<th>Bid $</th>"
		echo "<th>Duration</th>"
	echo "</tr>"
	echo "</thead>"
	echo "<tbody>"
	for REGION in `cat $HERE/regions.txt` ; do
	for AZ in `cat $HERE/$REGION.az.txt` ; do
		VDATA=`grep pred $HERE/$AZ/$TYPE.$INSTTYPE.pred | tail -n 1 | awk '{printf "%10.0f %7.4f %0.2f", $2,$6,$14}'`
		DDATA=`grep pred $HERE/$AZ/$TYPE.$INSTTYPE.duration | tail -n 1 | awk '{printf "%10.0f %7.2f %0.2f", $2,$6/3600,$14}'`
		SZ=`wc -l $HERE/$AZ/$TYPE.$INSTTYPE.duration | awk '{print $1}'`
                NDX=`echo $SZ | awk '{printf "%8.0f",0.04 * $1}'`
                DURCDF=`awk '{print $4}' $HERE/$AZ/$TYPE.$INSTTYPE.duration | sort -n | head -n $NDX | tail -n 1 | awk '{printf "%7.2f", $1/3600}'`
		TTIME=`echo $VDATA | awk '{print $1}'`
		PTIME=`$BIN/ptime $TTIME`
		BID=`echo $VDATA | awk '{print $2}'`
		BIDP=`echo $VDATA | awk '{print $3}'`
		DUR=`echo $DDATA | awk '{print $2}'`
		DURP=`echo $DDATA | awk '{print $3}'`
		if ( ! test -z "$EVEN" ) ; then
			echo '<tr class="even">'
			EVEN=""
		else
			echo "<tr>"
			EVEN="1"
		fi
			echo "<td>$AZ</td>"
			echo "<td>$PTIME</td>"
			if ( test -z "$BID" ) ; then
				BID="none"
			fi
			if ( test -z "$BIDP" ) ; then
				BIDP="none"
			fi
			echo "<td>\$$BID ($BIDP)</td>"
			if ( test -z "$DUR" ) ; then
				DUR="none"
			fi
			if ( test -z "$DURP" ) ; then
				DURP="none"
			fi
			if ( test "$DURP" != "none" ) ; then
                                if ( test "$DUR" != "none" ) ; then
                                TEMPP=`echo $DURP $DUR | awk -v "cdf=$DURCDF" '{if($1 < 0.95) print cdf; else print $2;}'`
                                DUR="$TEMPP"
                                DURP="0.95"
                                fi
                        fi
			echo "<td>$DUR hrs ($DURP)</td>"
		echo "</tr>"
	done
	done
	echo "</tbody>"
	echo "</table>"
done

echo "</body>"
echo "</html>"

