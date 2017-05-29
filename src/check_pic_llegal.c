#include "sift.h"
#include "imgfeatures.h"
#include "utils.h"

#include <highgui.h>

#include <unistd.h>
#include <sys/time.h>

char *readbuf_p = NULL;

char* img_file_name;

int main( int argc, char** argv ){
    struct timeval tstart,tend;
    gettimeofday(&tstart,NULL);

    //arg_parse( argc, argv );

    char read_buf[1024];

    FILE *fp_error_pic = fopen("error_pic.txt","w+");
    if(fp_error_pic == NULL){
        printf("open error_pic.txt failed\n");
        exit(-1);
    }

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
    IplImage* img;
    char temp_buf[1024];
    for(i = 0;i<line_num;i++){
        memset(temp_buf,0,sizeof(temp_buf));
        sscanf(filename_list[i],"%s",temp_buf);

        fprintf( stderr, "Finding SIFT features...\n" );
        printf("i:%d,before loadimage,filename:%s\n",i,temp_buf);
        img = cvLoadImage(temp_buf, 1 );
        if( ! img ){
            //fatal_error( "unable to load image from %s", img_file_name );
            int flag = remove(temp_buf);
            fprintf(fp_error_pic,"%s",filename_list[i]);
            if(flag != 0){
                perror("remove");
            }
            printf("remove %s success\n",temp_buf);
            cvReleaseImage(&img);
            img = NULL;
            continue;
        }
        cvReleaseImage(&img);
        img = NULL;
        printf("load image success\n");
    }

    gettimeofday(&tend,NULL);
    long use_time = 1000*(tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec)/1000;
    printf("use time:%ld\n",use_time);

    free(filename_list);

    fclose(fp);
    fclose(fp_error_pic);
    return 0;
}
