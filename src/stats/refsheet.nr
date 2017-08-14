.nf
.if n .ta .25i +.5i +1i
.if t .ta .15i +.25i +.65i
.if t .po .75i
.if n .po 1i
.if t .ps 12p
.if t .vs 13p
.if t .ta .25i +.35i +1i
.de SP
.\" .if t .sp 3p
.if t \l'\\n(.lu'\u
..
.de PG\"program name purpose ver date lines
.if t .ft B
.if n .ft I
.if !'\\$5'' .ne \\$5v+1v
.SP
\\$1:  \\$2   \\$3   \\$4 
.ft R
..
.de AG\"flag value purpose [default]
.ds _D
.if !'\\$4'' .ds _D (default: \\$4)
.\"	\s10\f(CB\\$1\s0\fR	\\$2	\\$3 \\*(_D
	\fB\\$1	\\$2	\fR\\$3 \\*(_D
..
.de HE
.if t .sp |.15i
.ft B
.ps 18p
|STAT Reference Sheet  \(co 1986-\n(yr Gary Perlman   \n(mo/\n(dy/\n(yr
.ps
.ft R
.if t .sp |.35i
..
.de SO\"standard options
Each program name, purpose, version and date is followed by its options.  \fIStandard |STAT options:
.AG -L  "" "print program limits (i.e., data set size limites)
.AG -O  "" "print program options and operands with current values (as on this sheet)
.AG -V  "" "print program version number and date (as on this sheet)
..
.\"----------------------------------------
.HE
.SO
.PG "abut"      "join corresponding lines of files" 5.2 5/31/88
.AG -c ""       "cycle through input files"
.AG -f width    "width of format string" "tabs between files
.AG -n ""       "number output lines"
.AG "" [files]  "read files
.AG "" [-]      "read from standard input
.PG "anova" "multi-factor analysis of variance" 5.3 1/29/87
.AG "" [names]  "factor names" "RANDOM A B ... DATA"
.\"==============================================================
.PG calc "algebraic modeling calculator" 5.3 10/1/88
.AG "" [files]  "input files
.AG "" [-]      "read from standard input
.\"==============================================================
.PG colex "column extraction/formatting" 5.3 9/23/90
.AG -c ""        "use character columns instead of fields"
.AG -f ""        "force-fill columns with NA string
.AG -F format    "default column format (aine)"         "a"
.AG -i ""        "ignore missing input columns
.AG -q ""        "quote output columns
.AG -t ""        "inhibit tabs between output columns"
.AG -v ""        "validate data types in columns
.AG "" "columns" "columns to extract (preceded by optional formats)
.\"==============================================================
.PG "contab" "contingency tables and chi-square" 5.4 2/03/87
.AG -b   ""      "insert blank lines in tables
.AG -c contents  "request cell contents (edprct)
.AG -i nfactors  "maximum number of factors in interactions" "all
.AG -s  ""       "don't print significance tests" print
.AG -y  ""       "don't use Yates correction for continuity" "use it
.AG ""  [names]  "factor names" "A B ... DATA
.\"==============================================================
.PG desc "descriptive statistics, tables, and histograms" 5.2 1/20/87
.AG -c ""        "cumulative frequencies or proportions
.AG -f ""        "request table of frequencies
.AG -F Ho        "F-test against mean Ho" "0
.AG -h ""        "request a histogram
.AG -i width     "interval for tables and histograms" "from data
.AG -m min       "minimum allowable value"
.AG -M max       "maximum allowable value
.AG -o ""        "request order statistics
.AG -p ""        "request table of proportions
.AG -s ""        "request summary statistics
.AG -t "Ho"      "t-test against mean Ho" "0
.AG -v ""        "print statistics in name=value format
.\"==============================================================
.PG dm "data manipulation" 5.9 10/1/88 3
.AG "" [Efile] "file of expressions
.AG "" [exprs] "expressions
.\"==============================================================
.PG dprime "d' and beta computations for signal detection data" 5.3 08/22/90 4
.AG -p ""         "request computation of precision"
.AG "" "[hr far]" "hit rate/false alarm rate
.AG "" "[H F M C]" "Hits False-alarms Misses Correct-rejections
.\"==============================================================
.PG dsort "sort data lines" 5.0 6/12/86 9
.AG -a ""         "order fields alphabetically"              
.AG -c ""         "ignore case in alphabetic comparisons"   
.AG -e ""         "order fields as xxx.yyyEzz numbers"    
.AG -i ""         "order fields as integers"              
.AG -l lines      "maximum number of input lines"            1000
.AG -n ""         "order fields as xxx.yyy numbers"      
.AG -r ""         "reverse sorting order of lines"      
.AG "" "columns"  "columns to sort on (preceded by optional formats)
.\"==============================================================
.PG features "tabulate features of items" 3.1 02/12/90 11
.AG -c col-delim  "column separator string"        |
.AG -f infile     "input file name"                - (standard input)
.AG -i width      "item label width"               15
.AG -h ""         "horizontal format for header"
.AG -l leadchar   "leader character after label"   _
.AG -n no-string  "string indicating 'NO'"          
.AG -p pagesize   "page size"                      60
.AG -w colwidth   "width of feature columns"       1
.AG -y yes-string "string indicating 'YES'"        #
.AG "" features   "feature names with optional individual =widths
.\"==============================================================
.PG ff "fast text formatter" 87.6 02/08/87 24
.AG -b ""      "break all lines of text--do no filling"
.AG -B chars   "line break characters"
.AG -c ""      "center all text lines"
.AG -d ""      "delete blank space around input lines"
.AG -D ""      "delete blank input lines"
.AG -f title   "page footer three-part title" none
.AG -F lines   "page footer size"                    "5 lines
.AG -h title   "page header three-part title"        "|File: *||Page: #|
.AG -H lines   "page header size"                    "5 lines
.AG -i spaces  "text indentation"                    "0 spaces
.AG -I spaces  "indent after line breaks"            "0 spaces
.AG -j ""      "justify the right margin of the text
.AG -n ""      "number output text lines
.AG -N spaces  "width of line numbers"               "4 spaces
.AG -p ""      "paginate output
.AG -P lines   "page size"                           "66 lines
.AG -s lines   "line spacing"                        "1 line
.AG -t pos     "absolute or relative tab stop
.AG -T spaces  "uniform tab stops
.AG -u ""      "show text with upper-case initial letters
.AG -w spaces  "page line width"                     "72 spaces
.AG "" [files]  "files to be formatted
.AG "" [-]      "format standard input
.\"==============================================================
.PG fpack "pack files info plain text archive" 1.3 11/02/87 4
.AG -f ""       "act forcefully about operations
.AG -v ""       "verbose information about operations"
.AG "" [files]  "files to be packed
.\".if t .po 4.5i
.\"==============================================================
.PG linex   "line extraction" 5.0 12/01/86 4
.AG -n  ""    "number lines
.AG -s  ""    "be silent about missing lines
.AG "" lines  "lines to extract
.\"==============================================================
.PG maketrix "make a matrix" 5.0 2/27/85 3
.AG -s  ""     "be silent about uneven last line
.AG "" [ncols] "number of columns" 2
.\"==============================================================
.PG manstat "get manual entry for |STAT programs" 2
.AG ""  [programs] "get info on these programs" "intro
.\"==============================================================
.PG "oneway" "one-way between groups t-test/anova" 5.5 01/20/87 7
.AG -p ""            "print error bar plots
.AG -s splitter      "special number to split groups"       "-1"
.AG -t ""            "request t-test format for two group data
.AG -u ""            "use unweighted means solution"        "use weighted
.AG -w "width"       "plot width"                           60
.AG "" "[names]" "group names" "Group-1, Group-2, ..."
.\"==============================================================
.PG pair "paired data analysis and scatterplots" 5.2 1/20/87 13
.AG -b bottom     "minimum y axis plot value"        "from input
.AG -c pchar      "plotting character"               "1 2 3 ... *"
.AG -f ""         "don't print plot box frame"       "frame plot
.AG -h height     "plot height (in lines)"           19
.AG -l left       "minimum x axis plot value"        "from input
.AG -p ""         "request plot
.AG -r right      "maximum x axis plot value"        "from input
.AG -s ""         "request summary statistics
.AG -t top        "maximum y axis plot value"        "from input
.AG -w width      "plot width (in columns)"          50
.AG -x xname      "name of first variable"           "Column 1
.AG -y yname      "name of second variable"          "Column 2
.\"==============================================================
.\" .HE
.PG perm "permute line order" "5.5" 06/12/86 9
.AG -a ""         "order lines alphabetically"               
.AG -c ""         "ignore case in alphabetic comparisons"   
.AG -e ""         "order lines as xxx.yyyEzz numbers"     
.AG -i ""         "order lines as integers"               
.AG -l lines      "maximum number of input lines"            1000
.AG -n ""         "order lines as xxx.yyy numbers"       
.AG -r ""         "reverse sorting order of lines"      
.AG -s seed       "integer random number generator seed"     0
.\"==============================================================
.PG probdist "probability distribution functions" 5.4 08/21/89 8
.AG -q ""             "use quick computational forms"
.AG -s seed           "integer random number generator seed"   0
.AG -v  ""            "print verbose output
.AG "" [function      "prob, crit-val/quantile, or rand
.AG "" distribution   "f, t, uniform, chi-square/x, normal/z, binomial
.AG "" [params]       "distribution parameters
.AG "" value]         "stat, prob, or count to match function
.\"==============================================================
.PG rankind "rank order analysis for independent conditions" 5.3 02/04/87 7
.AG -p ""         "print fivenum plots"
.AG -r ""         "print average ranks for conditions"
.AG -s splitter   "condition separator value"               -1
.AG -w width      "plot width"                              60
.AG -y ""         "don't use Yates' correction for continuity"    "use it
.AG "" [names]    "condition names"                         "Cond-1 Cond-2 ...
.\"==============================================================
.PG rankrel "rank order analysis for related conditions" 5.2 2/4/87 6
.AG -c maxcases  "maximum number of input cases"           100
.AG -r ""        "print average ranks for conditions"
.AG -s ""        "run comparative significance tests"      TRUE
.AG -y ""        "don't use Yates' correction for continuity"    "use it
.AG "" [names]   "condition names"                         "Cond-1 Cond-2 ...
.\"==============================================================
.PG ranksort "rank order columns of data" 5.1 11/28/86 3
.AG -r  ""       "reverse rank ordering
.AG -l  lines    "maximum number of input lines"           100
.\"==============================================================
.PG regress "multiple linear regression and correlation" 5.5 1/27/87 7
.AG -c "" "print covariance matrix
.AG -e "" "save regression equation in regress.eqn
.AG -p "" "partial correlation analysis
.AG -r "" "don't print regression analysis" "print
.AG -s "" "print sums of squares
.AG "" [names] "variable names" "[REG] A B ..."
.\"==============================================================
.PG repeat "repeat a file or string" 5.1 10/13/86 7
.AG "-f" ""         "insert formfeeds before each copy
.AG "-i" ""         "repeat lines within files
.AG -l lines        "maximum number of input lines"            1000
.AG -n count        "number of times to repeat input files"    1
.AG "" [names]      "files to be repeated
.AG "" [-]          "repeat standard input
.\"==============================================================
.PG reverse "reverse lines, fields, characters" 5.1 10/13/86 4
.AG -c ""           "reverse characters within lines
.AG -f ""           "reverse fields within lines
.AG -l ""           "reverse lines"                         "with no options
.\"==============================================================
.PG series "print a series of numbers" 5.4 10/24/86 4
.AG "" start        "first number in series
.AG "" end          "last number in series
.AG "" [step]       "series stepsize"                         unit
.\"==============================================================
.PG stats "print summary statistics" 5.2 1/20/87 3
.AG -v ""         "print requested statistics in name=value format
.AG "" [requests] "n min max sum ss mean var sd skew kurt se" "all"
.\"==============================================================
.PG transpose "transpose matrix type input" 5.1 05/15/89 2
.AG -f width      "format width of fields"           "tabs between fields
.\"==============================================================
.PG ts "time series analysis and plots" 5.2 1/20/87 12
.AG -a ""         "no axes around plots
.AG -b base       "base number of times series
.AG -c lag        "autocorrelations of lags 1:lag
.AG -e ""         "echo time series (see l option)
.AG -i width      "set interval width"                      1
.AG -l length     "set series length"
.AG -n number     "number output"
.AG -p  ""        "request plot"
.AG -P type       "specify plot style"
.AG -s  ""        "print summary statistics (default)
.AG -w width      "plot width"                              70
.\"==============================================================
.PG validata "data validation and consistency checking" 5.1 02/03/87 1
