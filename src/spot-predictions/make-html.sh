#!/bin/bash

HERE=/mnt/data
BIN=/root/bin

REGION=$1
INSTTYPE=$2
EVEN=""

if ( test -z "$INSTTYPE" ) ; then
echo "must input the specifc spot instance image type (UNIX, Windows, SUSE)"
exit 1
fi

if ( test -z "$REGION" ) ; then
	echo "must specify region"
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

echo "<h2>Spot Price Predictions by Availability Zone, AWS $REGION</h2>"
echo "<h3>$NOW</h3>"

for AZ in `cat $HERE/$REGION.az.txt` ; do
	echo "<h2>$AZ $INSTTYPE</h2>"
	echo "<table cellspacing='0'>"
	echo "<thead>"
	echo "<tr>"
		echo "<th>Instance Type</th>"
		echo "<th>Time</th>"
		echo "<th>Bid $</th>"
		echo "<th>Duration</th>"
	echo "</tr>"
	echo "</thead>"
	echo "<tbody>"
	for TYPE in `cat $HERE/instance-types.txt` ; do
		VDATA=`grep pred $HERE/$AZ/$TYPE.$INSTTYPE.pred | tail -n 1 | awk '{printf "%10.0f %7.4f %0.2f", $2,$6,$14}'`
#		DDATA=`grep pred $HERE/$AZ/$TYPE.$INSTTYPE.duration | tail -n 1 | awk '{printf "%10.0f %7.2f %0.2f", $2,$6/3600,$14}'`
		NDX=`cat $HERE/$AZ/$TYPE.$INSTTYPE.ndx`
		DDATA=`head -n $NDX $HERE/$AZ/$TYPE.$INSTTYPE.temp5 | tail -n 1 | awk '{printf "%0.2f", $2/3600}'`	
		TTIME=`echo $VDATA | awk '{print $1}'`
		PTIME=`$BIN/ptime $TTIME`
		BID=`echo $VDATA | awk '{print $2}'`
		BIDP=`echo $VDATA | awk '{print $3}'`
		DUR=`echo $DDATA | awk '{print $1}'`
		DURP=`cat $HERE/$AZ/$TYPE.$INSTTYPE.prob`
		if ( ! test -z "$EVEN" ) ; then
			echo '<tr class="even">'
			EVEN=""
		else
			echo "<tr>"
			EVEN="1"
		fi
			echo "<td>$TYPE</td>"
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
			echo "<td>$DUR hrs ($DURP)</td>"
		echo "</tr>"
	done
	echo "</tbody>"
	echo "</table>"
done

echo "</body>"
echo "</html>"

