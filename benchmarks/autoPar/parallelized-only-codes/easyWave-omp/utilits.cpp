/*
 * EasyWave - A realtime tsunami simulation program with GPU support.
 * Copyright (C) 2014  Andrey Babeyko, Johannes Spazier
 * GFZ German Research Centre for Geosciences (http://www.gfz-potsdam.de)
 *
 * Parts of this program (especially the GPU extension) were developed
 * within the context of the following publicly funded project:
 * - TRIDEC, EU 7th Framework Programme, Grant Agreement 258723
 *   (http://www.tridec-online.eu)
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved by
 * the European Commission - subsequent versions of the EUPL (the "Licence"),
 * complemented with the following provision: For the scientific transparency
 * and verification of results obtained and communicated to the public after
 * using a modified version of the work, You (as the recipient of the source
 * code and author of this modified version, used to produce the published
 * results in scientific communications) commit to make this modified source
 * code available in a repository that is easily and freely accessible for a
 * duration of five years after the communication of the obtained results.
 * 
 * You may not use this work except in compliance with the Licence.
 * 
 * You may obtain a copy of the Licence at:
 * https://joinup.ec.europa.eu/software/page/eupl
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing permissions and
 * limitations under the Licence.
 */
// last modified 11.07.2012
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "utilits.h"
#define ERRORFILE "error.msg"
#include <omp.h> 
int iscomment(int );
class cMsg Msg;
class cErrMsg Err;
class cLog Log;
// ==========================  Class Message  =======================

cMsg::cMsg()
{
  (this) -> enabled = 1;
  (this) ->  setchannel (1);
  (this) ->  setfilename ("default.msg");
}

cMsg::~cMsg()
{
}

void cMsg::enable()
{
  (this) -> enabled = 1;
}

void cMsg::disable()
{
  (this) -> enabled = 0;
}

void cMsg::setfilename(const char *newfilename)
{
  sprintf((this) -> fname,"%s",newfilename);
}

void cMsg::setchannel(int newchannel)
{
  (this) -> channel = newchannel;
}

int cMsg::print(const char *fmt,... )
{
  if (!((this) -> enabled)) 
    return 0;
  if (((this) -> channel & 1)) {
    va_list arglst;
    __builtin_va_start(arglst,fmt);
    vprintf(fmt,arglst);
    __builtin_va_end(arglst);
  }
  if (((this) -> channel & 2)) {
    va_list arglst;
    __builtin_va_start(arglst,fmt);
    FILE *fp = fopen(((this) -> fname),"at");
    vfprintf(fp,fmt,arglst);
    fclose(fp);
    __builtin_va_end(arglst);
  }
  return 0;
}
// =================== END Class Message =================
// ==========================  Class Error Message  =======================

cErrMsg::cErrMsg()
{
  (this) -> enabled = 1;
  (this) ->  setfilename ("error.msg");
  (this) ->  setchannel ((1 | 2));
}

cErrMsg::~cErrMsg()
{
}

int cErrMsg::post(const char *fmt,... )
{
  if (!((this) -> enabled)) 
    return 0;
  if (((this) -> channel & 1)) {
    va_list arglst;
    __builtin_va_start(arglst,fmt);
    vprintf(fmt,arglst);
    __builtin_va_end(arglst);
    printf("\n");
  }
  if (((this) -> channel & 2)) {
    va_list arglst;
    __builtin_va_start(arglst,fmt);
    FILE *fp = fopen(((this) -> fname),"at");
    vfprintf(fp,fmt,arglst);
    fprintf(fp,"\n");
    fclose(fp);
    __builtin_va_end(arglst);
  }
  return - 1;
}

char *cErrMsg::msgAllocateMem()
{
  char msg[256];
  sprintf(msg,"Error allocating memory");
  return strdup(msg);
}

char *cErrMsg::msgOpenFile(const char *fname)
{
  char msg[256];
  sprintf(msg,"Cannot open file %s",fname);
  return strdup(msg);
}

char *cErrMsg::msgReadFile(const char *fname,int line,const char *expected)
{
  char msg[256];
  sprintf(msg,"Error reading file %s, line number %-d. Expected: %s",fname,line,expected);
  return strdup(msg);
}
// =================== END Class Message =================
// =======================  Class Log  =======================

cLog::cLog()
{
  (this) -> enabled = 0;
  (this) -> timestamp_enabled = 1;
  (this) ->  setfilename ("default.log");
  (this) ->  setchannel (2);
}

cLog::~cLog()
{
}

void cLog::start(const char *filename)
{
  int ifenabled = 0;
  (this) ->  setfilename (filename);
  (this) -> enabled = 1;
  if (((this) -> timestamp_enabled)) {
    ifenabled = 1;
    (this) ->  timestamp_disable ();
  }
  (this) ->  print (" =============  Starting new log at %s",(utlCurrentTime()));
  if (ifenabled) 
    (this) ->  timestamp_enable ();
}

void cLog::timestamp_enable()
{
  (this) -> timestamp_enabled = 1;
}

void cLog::timestamp_disable()
{
  (this) -> timestamp_enabled = 0;
}

int cLog::print(const char *fmt,... )
{
  if (!((this) -> enabled)) 
    return 0;
  va_list arglst;
  __builtin_va_start(arglst,fmt);
  FILE *fp = fopen(((this) -> fname),"at");
  if (((this) -> timestamp_enabled)) 
    fprintf(fp,"%s --> ",(utlCurrentTime()));
  vfprintf(fp,fmt,arglst);
  fprintf(fp,"\n");
  fclose(fp);
  __builtin_va_end(arglst);
  return 0;
}
// =================== END Class Log =================

char *utlCurrentTime()
{
  time_t timer;
  char *cp;
  timer = time(0L);
  cp = asctime((localtime((&timer))));
  cp[strlen(cp) - 1] = '\0';
  return cp;
}

int utlPostError(const char *message)
{
  FILE *fp = fopen("error.msg","at");
  fprintf(fp,"%s",(utlCurrentTime()));
  fprintf(fp," -> %s\n",message);
  fclose(fp);
  return - 1;
}

char *utlErrMsgMemory()
{
  return strdup("Cannot allocate memory");
}

char *utlErrMsgOpenFile(const char *fname)
{
  char msg[256];
  sprintf(msg,"Cannot open file %s",fname);
  return strdup(msg);
}

char *utlErrMsgEndOfFile(const char *fname)
{
  char msg[256];
  sprintf(msg,"Unexpected end of file: %s",fname);
  return strdup(msg);
}

char *utlErrMsgReadFile(const char *fname,int line,const char *expected)
{
  char msg[256];
  sprintf(msg,"Error reading file %s, line number %-d. Expected: %s",fname,line,expected);
  return strdup(msg);
}
/*** Command-line processing ***/

int utlCheckCommandLineOption(int argc,char **argv,const char *option,int letters_to_compare)
{
  int k;
  for (k = 1; k <= argc - 1; k += 1) {
    if (argv[k][0] != '-' && argv[k][0] != '/') 
      continue; 
    if (!(strncmp((argv[k] + 1),option,letters_to_compare))) 
      break; 
  }
  if (k == argc) 
    return 0;
   else 
    return k;
}
/***************************************************************************/
/***                                                                     ***/
/***                    S T R I N G   H A N D L I N G                    ***/
/***                                                                     ***/
/***************************************************************************/

int utlStringReadOption(char *record,char *option_name,char *contents)
//  Reads specified option
//  Option-   [option-name]=contents
//  Returns 0 by success, otherwise -1
{
  int found;
  int length;
  char *cp;
  char *cpe;
  char buf[64];
  cp = record;
  found = 0;
  contents[0] = '\0';
  while(( *cp) != '\0'){
    cp = strchr(cp,'[');
    if (cp == 0L) 
      break; 
    cpe = strchr(cp + 1,']');
    if (cpe == 0L) 
      break; 
    length = (cpe - cp - 1);
    strncpy(buf,(cp + 1),length);
    buf[length] = '\0';
    if (!(strcmp(buf,option_name))) {
// found option name
// skip to option value
      cp = strchr(cpe + 1,'=');
      if (cp == 0L) 
        break; 
      while((isspace(( *(++cp)))))
        ;
// no value assigned
      if (( *cp) == '\0') 
        ;
       else 
// no value assigned
if (( *cp) == '[') 
        ;
       else 
// quoted string
if (( *cp) == '"') {
        cpe = strchr(cp + 1,'"');
        if (cpe == 0L) 
          break; 
        length = (cpe - cp - 1);
        strncpy(contents,(cp + 1),length);
        contents[length] = '\0';
      }
       else 
// string without quotation
        sscanf(cp,"%s",contents);
      found = 1;
      break; 
// found option name
    }
    cp++;
  }
  if (found) 
    return 0;
   else 
    return - 1;
}

int utlReadSequenceOfInt(char *line,int *value)
//   Reads sequence of integers from a string. Returns number of read
{
  #define MAX_Ints 100
  int N = 0;
  int itmp;
  char *cp = line;
  while(1){
    while(!(isdigit(( *cp))) && ( *cp) != '\0' && ( *cp) != ';')
      cp++;
    if (( *cp) == '\0' || ( *cp) == ';') 
      return N;
    if ((sscanf(cp,"%d",&itmp))) 
      value[N++] = itmp;
    if (N == 100) 
      return -N;
    while((isdigit(( *cp))))
      cp++;
  }
}

int utlReadSequenceOfDouble(char *line,double *value)
//   Reads sequence of doubles from a string. Returns number of read
{
  #define MAX_Doubles 100
  int N = 0;
  double dtmp;
  char *cp = line;
  while(1){
    while((isspace(( *cp))))
      cp++;
    if (( *cp) == '\0' || ( *cp) == ';') 
      return N;
    if ((sscanf(cp,"%lf",&dtmp))) 
      value[N++] = dtmp;
    if (N == 100) 
      return -N;
    while(!(isspace(( *cp))) && ( *cp) != '\0')
      cp++;
  }
}

int utlPickUpWordFromString(char *string,int n,char *word)
//   Picks up n-th word from string. By error (number words < n) returns -1
{
  char *cp;
  int i;
  for ((cp = string , i = 1); 1; i++) {
    while((isspace(( *cp))))
      cp++;
    if (( *cp) == '\0') {
      word[0] = '\0';
      return - 1;
    }
    if (i == n) 
      break; 
    while(!(isspace(( *cp))) && ( *cp) != '\0')
      cp++;
  }
  sscanf(cp,"%s",word);
  return 0;
}

char *utlPickUpWordFromString(char *string,int n)
//   Picks up n-th word from string. By error (number words < n) returns NULL
{
  char *cp;
  char buf[64];
  int i;
  for ((cp = string , i = 1); 1; i++) {
    while((isspace(( *cp))))
      cp++;
    if (( *cp) == '\0') 
      return 0L;
    if (i == n) 
      break; 
    while(!(isspace(( *cp))) && ( *cp) != '\0')
      cp++;
  }
  sscanf(cp,"%s",buf);
  return strdup(buf);
}

char *utlPickUpWordFromString(char *string,char *pos,char *word)
// Picks up a word from string starting from position pos.
// Return value: pointer to next char after the word or NULL if failed
{
  char *cp;
  if (pos < string) {
    word[0] = '\0';
    return 0L;
  }
  if (pos > string + strlen(string)) {
    word[0] = '\0';
    return 0L;
  }
  for (cp = pos; (isspace(( *cp))); cp++) 
    ;
  if (( *cp) == '\0') {
    word[0] = '\0';
    return 0L;
  }
  sscanf(cp,"%s",word);
  cp += strlen(word);
  return cp;
}

int utlCountWordsInString(char *line)
//   Count words in a string
{
  int nwords = 0;
  char *cp = line;
  while(1){
    while((isspace(( *cp))))
      cp++;
    if (( *cp) == '\0' || ( *cp) == ';') 
      return nwords;
    nwords++;
    while(!(isspace(( *cp))) && ( *cp) != '\0')
      cp++;
  }
}
/***************************************************************************/
/*   Write sequance of words into a string                                 */
/***************************************************************************/

char *utlWords2String(int nwords,char **word)
{
  char *buf;
  int k;
  int lenstr;
  for ((lenstr = 0 , k = 0); k <= nwords - 1; k += 1) {
    lenstr += strlen(word[k]);
  }
  lenstr += nwords + 1;
// add separators plus final null character
  buf = (new char [lenstr]);
  memset(buf,0,lenstr);
  for (k = 0; k <= nwords - 1; k += 1) {
    if (k > 0) 
      strcat(buf," ");
    strcat(buf,word[k]);
  }
  return buf;
}

int utlSubString(char *str,int p1,int p2,char *substr)
//  get substring from p1 to p2 into a buffer substr
{
  char *cp;
  int k;
  if (p1 < 0 || p2 < 0 || p1 > p2 || ((unsigned int )p2) > strlen(str) - 1) 
    return - 1;
  for ((k = 0 , cp = &str[p1]); cp <= &str[p2]; cp += 1) {
    substr[k++] =  *cp;
  }
  substr[k] = '\0';
  return 0;
}

void utlPrintToString(char *string,int position,char *insert)
//   Prints string to another starting from given position
{
  char *cp;
  int i;
  
#pragma omp parallel for private (i)
  for (i = 0; i <= position - 1; i += 1) {
    if (string[i] == '\0') 
      string[i] = ' ';
  }
  for ((cp = insert , i = 0); ( *cp) != '\0'; (i++ , cp++)) 
    string[position + i] =  *cp;
}
/***************************************************************************/
/***                                                                     ***/
/***                      F I L E   H A N D L I N G                      ***/
/***                                                                     ***/
/***************************************************************************/

int iscomment(int ch)
{
  if (ch == ';' || ch == '!') 
    return 1;
   else 
    return 0;
}

int utlFindNextInputField(FILE *fp)
//  Search for next input field in a file skipping blanks, tabs, empty lines, comments (from ';' to EOL)
//  Returns number of lines scanned until input field found
{
  int ch;
  int lines = 0;
  L1:
  while((isspace(ch = fgetc(fp))))
    if (ch == '\n') 
      lines++;
  if ((iscomment(ch))) {
    while((ch = fgetc(fp)) != '\n' && ch != - 1)
      ;
    if (ch == '\n') {
      lines++;
      goto L1;
    }
     else if (ch == - 1) 
      return - 1;
  }
   else if (ch == - 1) 
    return - 1;
  ungetc(ch,fp);
  return lines;
}

int utlReadNextRecord(FILE *fp,char *record,int *line)
{
  int found = 0;
  char *cp;
  char firstchar;
  while(!found && (fgets(record,16384,fp))){
    for (cp = record; ( *cp) == ' ' || ( *cp) == '\t'; cp++) 
      ;
    if (( *cp) != '\n' && ( *cp) != '\r' && ( *cp) != ';') 
      found = 1;
    ( *line)++;
  }
  if (!found) 
    return - 1;
   else {
    firstchar =  *cp;
    while(( *cp) != '\n' && ( *cp) != '\r' && ( *cp) != '\0')
      cp++;
     *cp = '\0';
    return firstchar;
  }
}

int utlReadNextDataRecord(FILE *fp,char *record,int *line)
{
  int found = 0;
  char *cp;
  char firstchar;
  while(!found && (fgets(record,16384,fp))){
    for (cp = record; ( *cp) == ' ' || ( *cp) == '\t'; cp++) 
      ;
    if ((isdigit(( *cp))) || ( *cp) == '-' && (isdigit(( *(cp + 1))))) 
      found = 1;
    ( *line)++;
  }
  if (!found) 
    return - 1;
   else {
    firstchar =  *cp;
    while(( *cp) != '\n' && ( *cp) != '\r' && ( *cp) != '\0')
      cp++;
     *cp = '\0';
    return firstchar;
  }
}

char *utlFileFindRecord(char *fname,char *pattern,char *record)
{
  char *cp;
  int line;
  FILE *fp;
  if ((fp = fopen(fname,"rt")) == 0L) {
    utlPostError((utlErrMsgOpenFile(fname)));
    return 0L;
  }
  line = 0;
  cp = 0L;
  while(utlReadNextRecord(fp,record,&line) != - 1){
    if ((cp = strstr(record,pattern)) == 0L) 
      continue; 
    break; 
  }
  fclose(fp);
  return cp;
}

int utlFileParceToString(FILE *fp,char *pattern)
{
  int ch;
  int pos = 0;
  int ierr = 1;
  while((ch = fgetc(fp)) != - 1){
    if (ch != pattern[pos]) {
      pos = 0;
      continue; 
    }
    pos++;
    if (pattern[pos] == '\0') {
      ierr = 0;
      break; 
    }
  }
  return ierr;
}

int utlGetNumberOfRecords(const char *fname)
{
  FILE *fp;
  int nrec;
  int line = 0;
  char record[1024];
  fp = fopen(fname,"rt");
  if (fp == 0L) 
    return 0;
  nrec = 0;
  while(utlReadNextRecord(fp,record,&line) != - 1)
    nrec++;
  fclose(fp);
  return nrec;
}

int utlGetNumberOfDataRecords(const char *fname)
{
  FILE *fp;
  int nrec;
  int line = 0;
  char record[1024];
  fp = fopen(fname,"rt");
  if (fp == 0L) 
    return 0;
  nrec = 0;
  while(utlReadNextDataRecord(fp,record,&line) != - 1)
    nrec++;
  fclose(fp);
  return nrec;
}

int utlFileReadOption(char *fname,char *option_name,char *contents)
{
  FILE *fp;
  int found;
  int line;
  char record[1024];
  fp = fopen(fname,"rt");
  if (!fp) 
    return 0;
  for ((line = 0 , found = 0); utlReadNextRecord(fp,record,&line) != - 1; ) {
    if (utlStringReadOption(record,option_name,contents) == 0) {
      found = 1;
      break; 
    }
  }
  fclose(fp);
  if (found) 
    return 0;
   else 
    return - 1;
}

char *utlFileChangeExtension(const char *fname,const char *newext)
//   Change file extension. Return new file name
{
  char buf[256];
  char *cp;
  int len;
  int extlen;
  len = (strlen(fname));
  extlen = (strlen(newext));
  memset(buf,0,(len + extlen + 1));
  strcpy(buf,fname);
  for (cp = &buf[len - 1]; ( *cp) != '.' && cp >= &buf[0]; cp--) 
    ;
  if (( *cp) == '.') 
    sprintf(cp + 1,"%s",newext);
   else 
    sprintf(&buf[len],".%s",newext);
  return strdup(buf);
}

char *utlFileAddExtension(const char *fname,const char *addext)
//   Add new extension. Return new file name
{
  char buf[256];
  char *cp;
  int len;
  int extlen;
  len = (strlen(fname));
  extlen = (strlen(addext));
  memset(buf,0,(len + extlen + 1));
  sprintf(buf,"%s.%s",fname,addext);
  return strdup(buf);
}

char *utlFileRemoveExtension(const char *fname)
//   Remove file extension. Return file name without extension
{
  char buf[256];
  char *cp;
  strcpy(buf,fname);
  for (cp = &buf[strlen(fname) - 1]; ( *cp) != '.' && cp >= &buf[0]; cp--) 
    ;
  if (( *cp) == '.') 
     *cp = '\0';
  return strdup(buf);
}

int utlFileRemoveExtension(char *newname,const char *fname)
//   Remove file extension
{
  char *cp;
  strcpy(newname,fname);
  for (cp = &newname[strlen(newname) - 1]; ( *cp) != '.' && cp >= &newname[0]; cp--) 
    ;
  if (( *cp) == '.') 
     *cp = '\0';
  return 0;
}

int utlReadXYdata(char *fname,double **x,double **y)
// Reads XY ASCII data file. Returns number of data points read
{
  FILE *fp;
  char record[256];
  int n;
  int line = 0;
  double xv;
  double yv;
  fp = fopen(fname,"rt");
  if (fp == 0L) 
    return utlPostError((utlErrMsgOpenFile(fname)));
// Calculate number of data lines
  for (n = 0; utlReadNextDataRecord(fp,record,&line) != - 1; ) {
    if (sscanf(record,"%lf %lf",&xv,&yv) != 2) 
      return utlPostError((utlErrMsgReadFile(fname,line,"X Y")));
    n++;
  }
// Allocate memory
   *x = (new double [n]);
   *y = (new double [n]);
// Read data
  rewind(fp);
  line = 0;
  for (int k = 0; k <= n - 1; k += 1) {
    utlReadNextDataRecord(fp,record,&line);
    sscanf(record,"%lf %lf",&( *x)[k],&( *y)[k]);
  }
  return n;
}
/****************************************************************/
/***                                                          ***/
/***                        MATH & GEOMETRY                   ***/
/***                                                          ***/
/****************************************************************/

double utlRandom(int &seed)
//========================================================
// UNIFORM RANDOM NUMBER GENERATOR                         |
// (MIXED CONGRUENTIAL METHOD)                             |
//                                                         |
// <INPUT>                                                 |
//  x: initial seed for random number generator (integer)  |
//                                                         |
// <OUTPUT>                                                |
//  ru: normal random number                               |
//  x: seed for next call (integer)                        |
//                                                         |
// --------------------------------------------------------+
{
  seed = seed * 65539;
  if (seed < 0) 
    seed = seed + 2147483647 + 1;
  return ((double )seed) * 0.4656613e-9;
}

double utlRandom(double avg,double err)
// return value which randomly deviates from avg: value = avg +-err
// be sure to call srand() before calling this function
{
  return avg + (((double )(rand())) / 2147483647 - 0.5) * 2 * err;
}

double utlNormal(int &seed,double avg,double stdev)
//========================================================
// NORMAL RANDOM NUMBER GENERATOR (BOX-MULLER METHOD)      |
//                                                         |
// <INPUT>                                                 |
//  x: initial seed for random number generator            |
//  av: average valuec  st: standard deviation             |
//                                                         |
// <OUTPUT>                                                |
//  rn: normal random numberc                              |
//                                                         |
// --------------------------------------------------------+
{
  double r1;
  double r2;
  double w1;
  double w2;
  r1 = utlRandom(seed);
  r2 = utlRandom(seed);
  w1 = log10(r1);
  w1 = sqrt((- 2) * w1);
  w2 = sin(2 * 3.14159265358979 * r2);
  return avg + stdev * w1 * w2;
}

int utlPointInPolygon(int nvrtx,double *xvrtx,double *yvrtx,double x,double y)
// Check if point lies inside polygon
{
  int l;
  int ln;
  int nsec;
  double yside;
  for ((nsec = 0 , l = 0); l <= nvrtx - 1; l += 1) {
    ln = l + 1;
    if (ln == nvrtx) 
      ln = 0;
    if ((xvrtx[l] - x) * (xvrtx[ln] - x) >= 0) 
      continue; 
    yside = yvrtx[l] + (yvrtx[ln] - yvrtx[l]) / (xvrtx[ln] - xvrtx[l]) * (x - xvrtx[l]);
    if (yside > y) 
      nsec++;
  }
  if (nsec / 2 * 2 != nsec) 
    return 1;
   else 
    return 0;
}
/****************************************************************/
/***                                                          ***/
/***                      O T H E R S                         ***/
/***                                                          ***/
/****************************************************************/

void utlTimeSplit(double ctime,int &nHour,int &nMin,int &nSec,int &nMsec)
// ctime is time in seconds with milliseconds as fraction
{
  int fullSec;
  fullSec = ((int )ctime);
  nMsec = ((int )((ctime - fullSec) * 1000 + 0.1));
  nHour = fullSec / 3600;
  nMin = (fullSec - nHour * 3600) / 60;
  nSec = fullSec - nHour * 3600 - nMin * 60;
  return ;
}

char *utlTimeSplitString(double ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  static char buf[32];
  utlTimeSplit(ctime,nHour,nMin,nSec,nMsec);
  if (nMsec > 0) 
    sprintf(buf,"%2.2d:%2.2d:%2.2d.%3.3d",nHour,nMin,nSec,nMsec);
   else 
    sprintf(buf,"%2.2d:%2.2d:%2.2d",nHour,nMin,nSec);
  return buf;
}

char *utlTimeSplitString(int ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  static char buf[32];
  utlTimeSplit((double )ctime,nHour,nMin,nSec,nMsec);
  sprintf(buf,"%2.2d:%2.2d:%2.2d",nHour,nMin,nSec);
  return buf;
}

int utlTimeHour(double ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  utlTimeSplit(ctime,nHour,nMin,nSec,nMsec);
  return nHour;
}

int utlTimeHour(int ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  utlTimeSplit((double )ctime,nHour,nMin,nSec,nMsec);
  return nHour;
}

int utlTimeMin(double ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  utlTimeSplit(ctime,nHour,nMin,nSec,nMsec);
  return nMin;
}

int utlTimeMin(int ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  utlTimeSplit((double )ctime,nHour,nMin,nSec,nMsec);
  return nMin;
}

int utlTimeSec(double ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  utlTimeSplit(ctime,nHour,nMin,nSec,nMsec);
  return nSec;
}

int utlTimeSec(int ctime)
{
  int nHour;
  int nMin;
  int nSec;
  int nMsec;
  utlTimeSplit((double )ctime,nHour,nMin,nSec,nMsec);
  return nSec;
}
