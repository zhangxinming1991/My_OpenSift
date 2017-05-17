/*
  This program detects image features using SIFT keypoints. For more info,
  refer to:
  
  Lowe, D. Distinctive image features from scale-invariant keypoints.
  International Journal of Computer Vision, 60, 2 (2004), pp.91--110.
  
  Copyright (C) 2006-2012  Rob Hess <rob@iqengines.com>

  Note: The SIFT algorithm is patented in the United States and cannot be
  used in commercial products without a license from the University of
  British Columbia.  For more information, refer to the file LICENSE.ubc
  that accompanied this distribution.

  Version: 1.1.2-20100521
*/

#include "sift.h"
#include "imgfeatures.h"
#include "utils.h"

#include <highgui.h>

#include <unistd.h>
#include <sys/time.h>

#define OPTIONS ":o:m:i:s:c:r:n:b:dxh"

/*************************** Function Prototypes *****************************/

static void usage( char* );
static void arg_parse( int, char** );

/******************************** Globals ************************************/

char* pname;
char* img_file_name;
char* out_file_name = NULL;
char* out_img_name = NULL;
int intvls = SIFT_INTVLS;
double sigma = SIFT_SIGMA;
double contr_thr = SIFT_CONTR_THR;
int curv_thr = SIFT_CURV_THR;
int img_dbl = SIFT_IMG_DBL;
int descr_width = SIFT_DESCR_WIDTH;
int descr_hist_bins = SIFT_DESCR_HIST_BINS;
int display = 0;

char *readbuf_p = NULL;

void replace_char(char *src,char match,char replace){
    char *pos = src;
    while((*pos) != '\0'){
        if((*pos) == match)
            *pos = replace;

        pos++;
    }
}


/********************************** Main *************************************/

int main( int argc, char** argv )
{
  struct timeval tstart,tend;
  gettimeofday(&tstart,NULL);

  IplImage* img;
  struct feature* features;
  int n = 0;

  //arg_parse( argc, argv );

  char read_buf[1024];

  FILE *fp = fopen("hello_world.txt","r+");//hello_world.txt store all the filename of the pictures
  if(fp == NULL){
      printf("open filename_list failed\n");
      exit(-1);
  }

  memset(read_buf,0,1024);
  readbuf_p = fgets(read_buf,sizeof(read_buf)-1,fp);
  if(readbuf_p != NULL)
    printf("read:%s",read_buf);
  else{
      printf("faild to read line mes\n");
      exit(-1);
  }

  int line_num = 0;
  sscanf(read_buf,"%d",&line_num);
  printf("line num:%d\n",line_num);

  char** filename_list = (char **)malloc(sizeof(char *) * line_num);//store the filename of picture

  int i = 0;
  for(i = 0;i<line_num;i++)
      filename_list[i] = (char *) malloc(sizeof(char) * 1024);

  for(i = 0;i<line_num;i++){
      memset(filename_list[i],0,sizeof(1024));
      readbuf_p = fgets(filename_list[i],1024,fp);//store the filename to filename_list
      if(readbuf_p == NULL){
          printf("i:%d,faild to read filename_mes\n",i);
          exit(-1);
      }
   //   printf("%s\n",filename_list[i]);
  }

  /***export all pictures***/
  char temp_buf[1024];
  for(i = 0;i<line_num;i++){
      memset(temp_buf,0,sizeof(temp_buf));
      sscanf(filename_list[i],"%s",temp_buf);
 //    printf("i:%d,%s\n",i,temp_buf);

      fprintf( stderr, "Finding SIFT features...\n" );
      img = cvLoadImage(temp_buf, 1 );
      if( ! img )
        fatal_error( "unable to load image from %s", img_file_name );
      n = _sift_features( img, &features, intvls, sigma, contr_thr, curv_thr,
                  img_dbl, descr_width, descr_hist_bins );
      fprintf( stderr, "Found %d features.\n", n );

      if( display )//show the feature in the picture
        {
          draw_features( img, features, n );
          display_big_img( img, temp_buf );
          cvWaitKey( 0 );
        }

      out_file_name = temp_buf;
      if( out_file_name != NULL ){//export the feature to the file
          char export_dirname[10];
          char export_filename[1024+10];

          memset(export_filename,0,1024+10);
          memset(export_dirname,0,10);

          strcpy(export_dirname,"export/");
          strcat(export_filename,export_dirname);

          replace_char(out_file_name,'/','_');
          strcat(export_filename,out_file_name);
          strcat(export_filename,".txt");

          export_features( export_filename, features, n );
      }


      if( out_img_name != NULL )//save the picture with the feature
        cvSaveImage( out_img_name, img, NULL );
  }
  /***export all pictures***/

  gettimeofday(&tend,NULL);
  long use_time = 1000*(tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec)/1000;

  /***record the extract message to file***/
  char result_mes[200];//result message
  memset(result_mes,0,200);
  sprintf(result_mes,"[%s]extract %d pic,use time:%ld ms\n",argv[1],line_num,use_time);
  printf("%s",result_mes);

  char task_mes[200];//result filename
  memset(task_mes,0,200);
  strcat(task_mes,"result_mes/");
  strcat(task_mes,argv[1]);

  //printf("task_mes:%s\n",task_mes);

  FILE *fp_result = fopen(task_mes,"w+");
  fputs(result_mes,fp_result);
  /***record the extract message to file***/

  fclose(fp_result);
  fclose(fp);
  free(filename_list);
  return 0;
}


/************************** Function Definitions *****************************/

// print usage for this program
static void usage( char* name )
{
  fprintf(stderr, "%s: detect SIFT keypoints in an image\n\n", name);
  fprintf(stderr, "Usage: %s [options] <img_file>\n", name);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h               Display this message and exit\n");
  fprintf(stderr, "  -o <out_file>    Output keypoints to text file\n");
  fprintf(stderr, "  -m <out_img>     Output keypoint image file (format" \
	  " determined by extension)\n");
  fprintf(stderr, "  -i <intervals>   Set number of sampled intervals per" \
	  " octave in scale space\n");
  fprintf(stderr, "                   pyramid (default %d)\n",
	  SIFT_INTVLS);
  fprintf(stderr, "  -s <sigma>       Set sigma for initial gaussian"	\
	  " smoothing at each octave\n");
  fprintf(stderr, "                   (default %06.4f)\n", SIFT_SIGMA);
  fprintf(stderr, "  -c <thresh>      Set threshold on keypoint contrast" \
	  " |D(x)| based on [0,1]\n");
  fprintf(stderr, "                   pixel values (default %06.4f)\n",
	  SIFT_CONTR_THR);
  fprintf(stderr, "  -r <thresh>      Set threshold on keypoint ratio of" \
	  " principle curvatures\n");
  fprintf(stderr, "                   (default %d)\n", SIFT_CURV_THR);
  fprintf(stderr, "  -n <width>       Set width of descriptor histogram" \
	  " array (default %d)\n", SIFT_DESCR_WIDTH);
  fprintf(stderr, "  -b <bins>        Set number of bins per histogram" \
	  " in descriptor array\n");
  fprintf(stderr, "                   (default %d)\n", SIFT_DESCR_HIST_BINS);
  fprintf(stderr, "  -d               Toggle image doubling (default %s)\n",
	  SIFT_IMG_DBL == 0 ? "off" : "on");
  fprintf(stderr, "  -x               Turn off keypoint display\n");
}



/*
  arg_parse() parses the command line arguments, setting appropriate globals.
  
  argc and argv should be passed directly from the command line
*/
static void arg_parse( int argc, char** argv )
{
  //extract program name from command line (remove path, if present)
  pname = basename( argv[0] );

  //parse commandline options
  while( 1 )
    {
      char* arg_check;
      int arg = getopt( argc, argv, OPTIONS );
      if( arg == -1 )
	break;

      switch( arg )
	{
	  // catch unsupplied required arguments and exit
	case ':':
	  fatal_error( "-%c option requires an argument\n"		\
		       "Try '%s -h' for help.", optopt, pname );
	  break;

	  // read out_file_name
	case 'o':
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  out_file_name = optarg;
	  break;

	  // read out_img_name
	case 'm':
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  out_img_name = optarg;
	  break;
	  
	  // read intervals
	case 'i':
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is an integer
	  intvls = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read sigma
	case 's' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  sigma = strtod( optarg, &arg_check );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires a floating point argument\n" \
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read contrast_thresh
	case 'c' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  contr_thr = strtod( optarg, &arg_check );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires a floating point argument\n" \
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read curvature_thresh
	case 'r' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  curv_thr = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read descr_width
	case 'n' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  descr_width = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read descr_histo_bins
	case 'b' :
	  // ensure argument provided
	  if( ! optarg )
	    fatal_error( "error parsing arguments at -%c\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  
	  // parse argument and ensure it is a floating point number
	  descr_hist_bins = strtol( optarg, &arg_check, 10 );
	  if( arg_check == optarg  ||  *arg_check != '\0' )
	    fatal_error( "-%c option requires an integer argument\n"	\
			 "Try '%s -h' for help.", arg, pname );
	  break;
	  
	  // read double_image
	case 'd' :
	  img_dbl = ( img_dbl == 1 )? 0 : 1;
	  break;

	  // read display
	case 'x' :
	  display = 0;
	  break;

	  // user asked for help
	case 'h':
	  usage( pname );
	  exit(0);
	  break;

	  // catch invalid arguments
	default:
	  fatal_error( "-%c: invalid option.\nTry '%s -h' for help.",
		       optopt, pname );
	}
    }

  // make sure an input file is specified
  if( argc - optind < 1 )
    fatal_error( "no input file specified.\nTry '%s -h' for help.", pname );

  // make sure there aren't too many arguments
  if( argc - optind > 1 )
    fatal_error( "too many arguments.\nTry '%s -h' for help.", pname );

  // copy image file name from command line argument
  img_file_name = argv[optind];
}
