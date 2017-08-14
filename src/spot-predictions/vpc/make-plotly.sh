#!/bin/bash

HERE=./results

REGION=$1
INST=$2
FILE=$HERE/$REGION-$INST.pgraph

echo "<head>"
echo "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.6/d3.min.js\"></script>"
echo "<script src=\"https://code.jquery.com/jquery-2.1.4.min.js\"></script>"
echo "<script src=\"https://d14fo0winaifog.cloudfront.net/plotly-basic.js\"></script>"
echo "</head>"
echo "<body>"
DATE=`/bin/date`
echo "<h3>Graph Generated on " $DATE "</h3>"
echo "<p>"
echo "<div id=\"myDiv\"></div>"
echo "<script>"
echo "var trace1 = {"
DATAX=`awk '{print $1}' $FILE | sed 's/$/,/g'`
XVALS="x: [$DATAX]"
XVALS=`echo $XVALS | sed 's/,]/]/'`
echo $XVALS","
DATAY=`awk '{print $2}' $FILE | sed 's/$/,/g'`
YVALS="y: [$DATAY]"
YVALS=`echo $YVALS | sed 's/,]/]/'`
echo $YVALS","
echo "mode: 'lines+markers',"
echo "type: 'scatter'"
echo "};"
echo "var data = [trace1];"
echo "var layout = {"
echo "title: '$REGION $INST Cost of Instance Durability',"
echo "xaxis: {"
echo "title: 'Minimum Duration, in Hours, Until Possible Termination with Probability Guarantee >= 0.95',"
echo "titlefont: {"
echo "family: 'Courier New, monospace',"
echo "size: 14,"
echo "color: '#7f7f7f'"
echo "}"
echo "},"
echo "yaxis: {"
echo "title: 'Bid, in Dollars, to Submit to Ensure Durability Duration',"
echo "titlefont: {"
echo "family: 'Courier New, monospace',"
echo "size: 12,"
echo "color: '#7f7f7f'"
echo "}"
echo "}"
echo "};"
echo "Plotly.newPlot('myDiv', data,layout);"
echo "</script>"
echo "<h3>How to use this Graph</h3>"
echo "AWS is entitled to terminate your instance as soon as the price in the AWS Spot Tier is greater than"
echo "or equal to the maximum bid price you specify when you launch your spot instance.  The greater your"
echo "specified bid, in general, the longer before the instance will be terminated."
echo "<p>"
echo "To assure a minimum duration before a termination is possible (with probability >= 0.95), find"
echo "the duration on the x-axis.  The corresponding value on the y-axis from the curve is the bid price that, if specified"
echo "as the AWS maximum bid at launch time, will assure at least that duration."
echo "<p>"
echo "Use this price to request an instance in the Availability Zone that currently has the lowest market price."
echo "</body>"
echo "</html>"








