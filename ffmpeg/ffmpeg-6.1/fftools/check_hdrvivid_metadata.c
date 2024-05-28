/*
 * Copyright (c) 2023 UWA(UHD World Association)
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * hdr vivid metadata file integrity verifier
 */


/*
*
*return value: 0 -->metadata file ok!;
*             -1 -->check parameter number wrong;
*             -2 -->file open failed;
*             -3 -->not good file(contains non digit characters for none comment line)
*             -4 -->empty line number found
*             -5 -->file close failed
*             -6 --> linetocheck is out of range!
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "libavcodec/put_bits.h"
#include "check_hdrvivid_metadata.h"
#include "libavutil/hdr_dynamic_vivid_metadata.h"

extern char *vmetafile;
FILE * fp_vmetafile;
char metadataline[512];

int check_cmd(int    c, char **s){
  int linenum = 0;
  int line_to_check = 0;
  int len;

  if((c != 3) &&(c != 4)){
    printf("Check or show command not correct!\ncheck command format is: $ffmpeg_vividenc check metadatafilename [line_to_check]!\nshow  command format is: $ffmpeg_vividenc show  metadatafilename [line_to_show]!\n");
    return -1;
  }
  fp_vmetafile=fopen(s[2], "r");
  if(fp_vmetafile == NULL){
    printf("metadata file open failed!\n");
    return -2;
  }else{
    do{
      if(fgets(metadataline, sizeof(metadataline), fp_vmetafile)){
       linenum++;
	   if(is_empty_line(metadataline)){
         printf("In line %d, empty line found! please remove it!\n",linenum);
         return -4;
       }

	   len=strlen(metadataline);
       if(metadataline[0]!='#'){ //only the first character can be '#' for a comment line!!! rest characters can be only digital
         for(int i=0;i<len;i++){
	       if(!isdigit(metadataline[i])&&metadataline[i]!=' '&&metadataline[i]!='\n'&&metadataline[i]!=0x0d&&metadataline[i]!=-1){
             printf("Metadata file contains non-digital characters or is not standard txt file in line %d! bad ASCII value of the character is 0x%02x\nPlease correct your metadata file first!\n",linenum,metadataline[i]);
             if(fclose(fp_vmetafile)){
               printf("metadata file close failed!\n");
	           return -5;
		     }
             return -3;
	       }
	      }
        }//not a comment line!
      }else{
     //   printf("In func %s line %d fgets() return NULL, reach eof!\n",__func__,__LINE__);
      }
    }while(!feof(fp_vmetafile));
  }//file open OK and all lines are checked!

  if(fclose(fp_vmetafile)){
    printf("metadata file close failed!\n");
	return -5;
  }

  if(c==4){
    line_to_check=atoi(s[3]);
    if(line_to_check==0){
      printf("check command is not correct! line number should be between 1 and %d (inclusive)!\n",linenum);
      return -6;
    }
    if(line_to_check > linenum ||line_to_check < 1){
      printf("check command is not correct! line number should be between 1 and %d (inclusive)!\n",linenum);
      return -6;
    }
  }
  return 0; //ret = 0, check cmd is good
}
/**
* return: 0 --> Not a blank line ; -3 blank line found
*/
bool is_empty_line(char *line)
{
  char * ch;

  // Iterate through each character.
  for (ch = line; *ch != '\0'; ++ch)
  {
    if (!isspace(*ch))
    {
      // Found a non-whitespace character.
      return false;
      break;
    }
  }
  return true;
}

bool is_valid_line(char * line,int linenum){ //either it is a comment line or  if it is not a comment line, then the rest contains only digital or space or end of line characters
  int len;
  if(line==NULL){
    printf("line %d is NULL\n",linenum);
    return false;
  }
  len=strlen(line);
  if(line[0]!='#'){ //only the first character can be '#' for a comment line!!! rest characters can be only digital
    for(int i=0;i<len;i++){
      if(!isdigit(line[i])&&line[i]!=' '&&line[i]!='\n'&&line[i]!=0x0d&&line[i]!=-1){
        printf("Metadata file contains non-digital characters or is not standard txt file in line %d! bad ASCII value of the character is 0x%02x\nPlease correct your metadata file first!\n",linenum,line[i]);
        return false;
      }
    }
    return true; //it is not a comment line but contains no non digital characters!
  }else
    return true;//it is a comment line!
}
/**
*input: linetocheck: if 0, then check all the lines ,otherwise check that line!
*       show_metadata_details: if 1, then print out metadata details
*return: 0 --> all metadata are OK; -1 --> value out of range error in metadata; -2 --> file open error; -3 --> empty line found
*
*/
int check_vivid_metadata_file(char * file,int linetocheck,bool show_metadata_details){
  int ret = 0;
  int linenum = 0,total_valid_linenum=0;
  FILE *fp;
  fp=fopen(file,"r");
  if(!fp){
    perror("Error opening file");
    return -2;
  }
  do{
    if(fgets(metadataline, sizeof(metadataline), fp)){
      linenum++;
      if(metadataline[0]!='#')
        total_valid_linenum++;
      if(!is_empty_line(metadataline)&&metadataline[0]!='#'){
        if(linetocheck==0 ||linenum==linetocheck)
        ret=check_vivid_metadata(metadataline,linenum,show_metadata_details); // return 0 --> OK ; -1 error in metadata!
      }else if(metadataline[0]=='#'&&(linetocheck==0||linetocheck!=linenum)){
        ret = 0;//it is comment line AND either we are checking whole file or it is not THE line we are checking ,so we ignore it AND  will continue to get the next line to check!
      }else if(metadataline[0]=='#'&&linetocheck==linenum){
      //we are asked to check this line and it IS a comment line, so we will jump out the while loop by set ret = 2 and tell it is comment line!
        printf("metadata in line %d of file %s is comment line!\n",linetocheck,file);
        ret = 2 ;
      }else{
        printf("Line %d is empty, please delete this line!\n",linenum);
        fclose(fp);
        return -3;
      }
    }else{//no more line get, so fgets return NULL
      if(linetocheck==0)
        printf("All the metadata in file %s are OK!, total valid metadata line number is %d\n",file,total_valid_linenum);
      else
        printf("metadata in line %d of file %s is OK!\n",linetocheck,file);
      ret = 1; //all lines are checked, and no error found, so jump out of do...while loop.
    }
  }while(ret==0);

  if(ret == 1 || ret == 2) //All the metadata are OK if ret == 1, and certain line is OK for if ret == 2.
    ret = 0;

  fclose(fp);
  return ret;
}

char * vmeta_read_one_line(FILE *fp){
  char * line;
  line=fgets(metadataline, sizeof(metadataline), fp);
  if(line!=NULL){
     return line;
  }else{
     rewind(fp);
	 line=fgets(metadataline, sizeof(metadataline), fp);
     return line;
  }
}
/*
*Input:          char *s one string from a whole line of metadata file
*Output:         uint8_t *data destination binary buf
*Output:return : buf size valid for metadata sidedata
*/
#define MAX_VIVID_METADATA_SIZE 80 /*in bytes */
int get_vivid_metadata_from_one_line_string(char *s, uint8_t *data, int *size)
{
  PutBitContext pbc, *pb = &pbc;
  AVDynamicHDRVivid m;
  int ret;
  const char delim[2] = " ";
  uint16_t count = 0;

  if (!s)
    return AVERROR(ENOMEM);

  strtok(s, delim); /* get the first token and discard it because it is just frame number in metadata file*/
  init_put_bits(pb, data, MAX_VIVID_METADATA_SIZE);
  if (ret < 0)
    return ret;

  put_bits(pb, 8, 0x26);  //country code
  count += 8;
  put_bits(pb, 16, 0x04); //terminal_provide_code
  count += 16;
  put_bits(pb, 16, 0x05); //terminal_provide_oriented_code
  count += 16;

  m.system_start_code = atoi(strtok(NULL, delim));
  put_bits(pb, 8,m.system_start_code);
  count += 8;

  // T/UWA 005.1-2022, table 11
  if (m.system_start_code >= 0x01 && m.system_start_code <= 0x07) {
    m.num_windows = 1;
    for (int w = 0; w < m.num_windows; w++) {
      AVHDRVividColorTransformParams *params = &m.params[w];
      params->minimum_maxrgb.num  = atoi(strtok(NULL, delim));
      put_bits(pb,12,params->minimum_maxrgb.num);
      count += 12;
      params->average_maxrgb.num  = atoi(strtok(NULL, delim));
      put_bits(pb,12,params->average_maxrgb.num);
      count += 12;
      params->variance_maxrgb.num  = atoi(strtok(NULL, delim));
      put_bits(pb,12,params->variance_maxrgb.num);
      count += 12;
      params->maximum_maxrgb.num  = atoi(strtok(NULL, delim));
      put_bits(pb,12,params->maximum_maxrgb.num);
      count += 12;
    }

    for (int w = 0; w < m.num_windows; w++) {
      AVHDRVividColorTransformParams *params = &m.params[w];
      params->tone_mapping_mode_flag = atoi(strtok(NULL, delim));
      put_bits(pb, 1,params->tone_mapping_mode_flag);
      count += 1;
      if (params->tone_mapping_mode_flag) {
        params->tone_mapping_param_num = atoi(strtok(NULL, delim));
        put_bits(pb, 1,params->tone_mapping_param_num)/* + 1*/;
        count += 1;
        for (int i = 0; i < params->tone_mapping_param_num + 1; i++) {
          AVHDRVividColorToneMappingParams *tm_params = &params->tm_params[i];
          tm_params->targeted_system_display_maximum_luminance.num = atoi(strtok(NULL, delim));
          put_bits(pb, 12,tm_params->targeted_system_display_maximum_luminance.num);
          count += 12;
          tm_params->base_enable_flag = atoi(strtok(NULL, delim));
          put_bits(pb, 1,tm_params->base_enable_flag);
          count += 1;
          if (tm_params->base_enable_flag) {
            tm_params->base_param_m_p.num = atoi(strtok(NULL, delim));
            put_bits(pb, 14,tm_params->base_param_m_p.num);
            count += 14;
            tm_params->base_param_m_m.num= atoi(strtok(NULL, delim));
            put_bits(pb,  6,tm_params->base_param_m_m.num);
            count += 6;
            tm_params->base_param_m_a.num = atoi(strtok(NULL, delim));
            put_bits(pb, 10,tm_params->base_param_m_a.num);
            count += 10;
            tm_params->base_param_m_b.num = atoi(strtok(NULL, delim));
            put_bits(pb, 10,tm_params->base_param_m_b.num);
            count += 10;
            tm_params->base_param_m_n.num = atoi(strtok(NULL, delim));
            put_bits(pb,  6,tm_params->base_param_m_n.num);
            count += 6;
            tm_params->base_param_k1 = atoi(strtok(NULL, delim));
            put_bits(pb, 2,tm_params->base_param_k1);
            count += 2;
            tm_params->base_param_k2 = atoi(strtok(NULL, delim));
            put_bits(pb, 2,tm_params->base_param_k2);
            count += 2;
            tm_params->base_param_k3 = atoi(strtok(NULL, delim));
            put_bits(pb, 4,tm_params->base_param_k3);
            count += 4;
            tm_params->base_param_Delta_enable_mode = atoi(strtok(NULL, delim));
            put_bits(pb, 3,tm_params->base_param_Delta_enable_mode);
            count += 3;
            tm_params->base_param_Delta.num = atoi(strtok(NULL, delim));
            put_bits(pb, 7,tm_params->base_param_Delta.num);
            count += 7;
          }
          tm_params->three_Spline_enable_flag = atoi(strtok(NULL, delim));
          put_bits(pb, 1,tm_params->three_Spline_enable_flag);
		  count += 1;
          if (tm_params->three_Spline_enable_flag) {
            AVHDRVivid3SplineParams *three_spline;
            tm_params->three_Spline_num = atoi(strtok(NULL, delim));
            put_bits(pb, 1,tm_params->three_Spline_num)/* + 1*/;
            count += 1;
            for (int j = 0; j < tm_params->three_Spline_num + 1; j++) {
              three_spline = &tm_params->three_spline[j];
              three_spline->th_mode = atoi(strtok(NULL, delim));
              put_bits(pb, 2,three_spline->th_mode);
              count += 2;
              if (three_spline->th_mode == 0 || three_spline->th_mode == 2) {
                three_spline->th_enable_mb.num = atoi(strtok(NULL, delim));
                put_bits(pb, 8,three_spline->th_enable_mb.num);
                count += 8;
              }
              three_spline->th_enable.num = atoi(strtok(NULL, delim));
              put_bits(pb, 12,three_spline->th_enable.num);
              count += 12;
              three_spline->th_delta1.num = atoi(strtok(NULL, delim));
              put_bits(pb, 10,three_spline->th_delta1.num);
              count += 10;
              three_spline->th_delta2.num = atoi(strtok(NULL, delim));
              put_bits(pb, 10,three_spline->th_delta2.num);
              count += 10;
              three_spline->enable_strength.num = atoi(strtok(NULL, delim));
              put_bits(pb,  8,three_spline->enable_strength.num);
              count += 8;
            }
          }//end of tm_params->three_Spline_enable_flag
        } //end of params->tone_mapping_param_num
      } //end of params->tone_mapping_mode_flag
      params->color_saturation_mapping_flag= atoi(strtok(NULL, delim));
      put_bits(pb, 1,params->color_saturation_mapping_flag);
      count += 1;
      if (params->color_saturation_mapping_flag) {
        params->color_saturation_num = atoi(strtok(NULL, delim));
        put_bits(pb, 3,params->color_saturation_num);
        count += 3;
        for (int i = 0; i < params->color_saturation_num; i++) {
          params->color_saturation_gain[i].num = atoi(strtok(NULL, delim));
          put_bits(pb, 8,params->color_saturation_gain[i].num);
          count += 8;
        }
      }//end of params->color_saturation_mapping_flag
    } //end of 2nd m->num_windows
  }//end of m->system_start_code

  *size = count/8 + 1; //change from number of bits to number of bytes
  flush_put_bits(pb);
  return 0;
}

// return -1 --> error in metadata!; v >= 0 --> is valid data ;
int check_value(char * token,int min,int max,int linenum,int count,const char *name,bool show_metadata_details){
  int v,len;
  if(token==NULL){
    printf("In metadata file line %d ,the metadata is not correct! no enough data\n",linenum);
    return -1;
  }

  len=strlen(token);
  if(len==1&&token[0]==0x0a){ //unix line, and this case should not appear, value will be 0 using atoi, but is NOT valid data.
    printf("In metadata file line %d, data NOT valid!, it appears no enough data in this line!\n", linenum);
    return -1;
  }
  if(len==2&&token[0]==0x0d&&token[1]==0x0a){ //windows file format
    printf("In metadata file line %d, data NOT valid!, it appears no enough data!\n", linenum);
    return -1;
  }

  v=atoi(token);
  if(v>=min&&v<max){
    if(show_metadata_details){
      printf("%s=%d/%d",name,v,max-1);
      if(!strcmp(name,"tone_mapping_param_num")){
        printf(", i.e metadata number is 1, but actual number is %d",v+1);
      }
      if(!strcmp(name,"3Spline_num")){
        printf(", i.e metadata number is 1, so actual number is %d",v+1);
      }
      printf("\n");
    }
    return v;
  }else{
    printf("Error in metadata file line %d, in position %d, the value is %d which is not good, should be >= %d and < %d! or previous data are not correct which lead to wrong intepretation of this data!\n",linenum,count+1,v,min,max);
	return -1;
  }
}

// return 0 --> OK ; -1 error in metadata!
int check_vivid_metadata(char *s,int linenum,bool show_metadata_details)
{
  AVDynamicHDRVivid m;
  int ret = 0;
  const char delim[2] = " ";
  uint16_t count = 0; //count indicate the number of token in the metadata string
  char * token;
  int system_start_code;
  int tone_mapping_mode_flag;
  int base_enable_flag;
  int three_Spline_enable_flag;
  int color_saturation_mapping_flag;
#if 0 //
  int len;
  len=strlen(s);
  if(s[len-2]==' '||s[len-3]==' '){ // if len-2 is  space character for unix format txt file, len-3 is for dos format txt file. because dos has 0d 0a at end of line, but unix format only contains 0a
    printf("Error! at the end of metadata line %d , it contains space character(s), please remove unneeded blank space characters at the end of this line！len =%d,  s is %s\n",linenum,len,s);
    return -1;
  }
#endif
  if(!is_valid_line(s,linenum))
    return -1;

  token=strtok(s, delim);count++;  /* get the first token and discard it because it is just frame number in metadata file*/
  if (ret < 0)
    return ret;

  system_start_code=check_value(strtok(NULL, delim),1,8,linenum,count++,"system_start_code",show_metadata_details);
  if( system_start_code < 0)  return -1;

  // T/UWA 005.1-2022, table 11
  if (system_start_code >= 0x01 && system_start_code <= 0x07) {
    m.num_windows = 1;
    for (int w = 0; w < m.num_windows; w++) {
      if(check_value(strtok(NULL, delim),0,1<<minimum_maxrgb_BIT,linenum,count++,"minimum_maxrgb",show_metadata_details) < 0)	 return -1;
      if(check_value(strtok(NULL, delim),0,1<<average_maxrgb_BIT,linenum,count++,"average_maxrgb",show_metadata_details) < 0)	 return -1;
      if(check_value(strtok(NULL, delim),0,1<<variance_maxrgb_BIT,linenum,count++,"variance_maxrgb",show_metadata_details) < 0) return -1;
      if(check_value(strtok(NULL, delim),0,1<<maximum_maxrgb_BIT,linenum,count++,"maximum_maxrgb",show_metadata_details) < 0) return -1;
    }
    for (int w = 0; w < m.num_windows; w++) {
      tone_mapping_mode_flag = check_value(strtok(NULL, delim),0,2,linenum,count++,"tone_mapping_mode_flag",show_metadata_details);
      if (tone_mapping_mode_flag != 0 && tone_mapping_mode_flag != 1)
        return -1;
      else if(tone_mapping_mode_flag == 1) {
		int tone_mapping_param_num = check_value(strtok(NULL, delim),0,2,linenum,count++,"tone_mapping_param_num",show_metadata_details);
		if(tone_mapping_param_num !=0 && tone_mapping_param_num != 1) return -1;
        for (int i = 0; i < tone_mapping_param_num + 1; i++) {
          if(check_value(strtok(NULL, delim),0,1<<targeted_system_display_BIT,linenum,count++,"targeted_system_display_maximum_luminance",show_metadata_details) < 0) return -1;
          base_enable_flag = check_value(strtok(NULL, delim),0,2,linenum,count++,"base_enable_flag",show_metadata_details);
          if(base_enable_flag !=0 && base_enable_flag !=1)
            return -1;
          if (base_enable_flag) {
            if(check_value(strtok(NULL, delim),0,1<<Base_param_m_p_BIT,linenum,count++,"base_param_m_p",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_m_m_BIT,linenum,count++,"base_param_m_m",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_m_a_BIT,linenum,count++,"base_param_m_a",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_m_b_BIT,linenum,count++,"base_param_m_b",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_m_n_BIT,linenum,count++,"base_param_m_n",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_K1_BIT,linenum,count++,"base_param_K1",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_K2_BIT,linenum,count++,"base_param_K2",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_K3_BIT,linenum,count++,"base_param_K3",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_Delta_mode_BIT,linenum,count++,"base_param_Delta_enable_mode",show_metadata_details) < 0) return -1;
            if(check_value(strtok(NULL, delim),0,1<<Base_param_Delta_BIT,linenum,count++,"base_param_Delta",show_metadata_details) < 0) return -1;
          }
          three_Spline_enable_flag = check_value(strtok(NULL, delim),0,2,linenum,count++,"3Spline_enable_flag",show_metadata_details);
          if(three_Spline_enable_flag != 0 && three_Spline_enable_flag != 1)
            return -1;
          if (three_Spline_enable_flag == 1) {
            int three_Spline_num= check_value(strtok(NULL, delim),0,2,linenum,count++,"3Spline_num",show_metadata_details);
            if(three_Spline_num != 0 && three_Spline_num != 1)
              return -1;
            for (int j = 0; j < three_Spline_num + 1; j++) {
              int th_mode = check_value(strtok(NULL, delim),0,1<<P3Spline_TH_mode_BIT,linenum,count++,"3Spline_TH_enable_mode",show_metadata_details);
              if(th_mode < 0)
			   return -1;
              if (th_mode == 0 || th_mode == 2) {
                if(check_value(strtok(NULL, delim),0,1<<P3Spline_TH_MB_BIT,linenum,count++,"3Spline_TH_enable_MB",show_metadata_details) < 0) return -1;
              }
              if(check_value(strtok(NULL, delim),0,1<<P3Spline_TH1_BIT,linenum,count++,"3Spline_TH",show_metadata_details) < 0) return -1;
              if(check_value(strtok(NULL, delim),0,1<<P3Spline_TH2_BIT,linenum,count++,"3Spline_TH_Delta1",show_metadata_details) < 0) return -1;
              if(check_value(strtok(NULL, delim),0,1<<P3Spline_TH3_BIT,linenum,count++,"3Spline_TH_Delta2",show_metadata_details) < 0) return -1;
              if(check_value(strtok(NULL, delim),0,1<<P3Spline_Strength_BIT,linenum,count++,"3Spline_Strength",show_metadata_details) < 0) return -1;
            }
          }//end of tm_params->three_Spline_enable_flag
        } //end of params->tone_mapping_param_num
      } //end of params->tone_mapping_mode_flag
	  color_saturation_mapping_flag = check_value(strtok(NULL, delim),0,2,linenum,count++,"color_saturation_mapping_flag",show_metadata_details);
	  if(color_saturation_mapping_flag !=0 && color_saturation_mapping_flag != 1)
        return -1;

      if (color_saturation_mapping_flag == 1) {
        int color_saturation_num=check_value(strtok(NULL, delim),0,1<<color_saturation_num_BIT,linenum,count++,"color_saturation_num",show_metadata_details);
        if(color_saturation_num < 0) return -1;
        for (int i = 0; i < color_saturation_num; i++) {
          if(check_value(strtok(NULL, delim),0,1<<color_saturation_gain_BIT,linenum,count++,"color_saturation_gain",show_metadata_details) < 0) return -1;
        }
      }//end of params->color_saturation_mapping_flag
    } //end of 2nd m->num_windows
  }//end of m->system_start_code

  token=strtok(NULL, delim);count++;
  if(token!=NULL && !(token[0]==0xd || token[0]==0xa)){ //deal with extra characters! this line contains extra displayable characters(here only digital characters are possible), so ask to remove it! if last valid token is followed by space character(s), then it return either token[0]=0x0d and token[1]=0x0a(dos format) or token[0]=0x0a(unix format)
    printf("In line %d, position %d, %d is extra number, please remove it!\n",linenum,count,atoi(token));
    return -1;
  }
  return 0; //matadata are OK!
}
