# para_Map_Reduce

## Local exectution

- the applications needs 2 things in order to run properly : 
  - a path to a config file formatted as the next section describes it
  - the per said config file
  
## Config File

A config file containing some basic informations about files needed to be processed is required for the application to run. Please refer to 
https://github.com/boiteaclou/para_Map_Reduce/blob/master/f_data_in.template for an example of the config file to supply to the executable.

## Executable Arguments

The following arguments are available when calling the runner :

| Parameter |  Type  | Parameter argument                                                                        | Default value                  | Bounds                              |
|:---------:|:------:|-------------------------------------------------------------------------------------------|:------------------------------:|:-----------------------------------:|
| \-i       |  uint  | number of runner iterations                                                               | 1                              | [1, ...]                            |
| \-t       |  uint  | maximum number of thread working                                                          | thread::hardware_concurrency() | [2, thread::hardware_concurrency()] |
| \-n       |  uint  | wanted total number of files treated ( see below for the actual number of files treated ) | number of files available      | [number of files available, ...]    |
| \-m       | string | execution mode for the test ( see below for their descriptions )                          | "static"                       | ["static", "lin", "exp"]            |
| \-c       |  uint  | maximum cap to pass to sequential (only in parallel executions)                           | 1024                           | [0, ...]                            |
| \-o       | string | path of the data output                                                                   | same as .exe                   | path needs to exist                 |
| \-s       | string | suffix for the data files ( see below how to integrate the run number )                   | ""                             | not applicable                      | 
| \-l       | string | name of the standard output logging file                                                  | "cout.log"                     | not applicable                      |

## Data file suffix format :

It is possible to enclose the runner iteration number in the suffix parameter (\-s) by placing the tag \{i_s\#\} in the suffix (e.g.  ".r.execlol.r\{i_s\#\}").

## Execution modes :

- Static : run the runner once, for the wanted number of word as calculated below
- Lin    : linearly increase the number of available files up to the wanted number of word as calculated below
- Exp    : exp increase the number of available files up to the wanted number of word as calculated below ( the value n calculated below is used to increment the number of files )

## Actual number of files treated :

The following calculation is done when a wanted number of files is given by the parameter \-i :

       n = log( wanted number of files \ number of files available ) / log(2) + 0.5f
       
 in order to obtain a number of files corresponding to 2^n * ( number of files available ), which is greater or equal to the wanted number.

## Linked repositories

All test files are included as a subtree to each of the branches of the project, so they can decide when to update their test material and their config file as well.

## Submission norms

- Local config files must have this signature, so they are ignored to the commit : f_data_in_*
