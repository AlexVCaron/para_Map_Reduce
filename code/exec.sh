



NB_ITER = "10";
DIRECTORY = "exec_data";
NB_FILES_MAX = "4000";
CAP_MAX = "4000";
SUFFIX = ".tarin.r{i_s#_run}.data"
MODE = "exp"

if [ ! -d $DIRECTORY ]; then
	mkdir $DIRECTORY
fi

map_reduce -i $NB_ITER -o $DIRECTORY -n $NB_FILES_MAX -c $CAP_MAX -s $SUFFIX -m $MODE

	