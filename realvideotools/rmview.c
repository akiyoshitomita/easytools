// Real media file view (SJIS)
// CONT name 
//   古い RealVideoのファイルのタイトルなどがSJISとなっており
//   文字化けが発生していたため、SJISの名称を表示するプログラムを作成
//
// 利用方法
//   $ make
//   $ rmview [ファイル名]
//
// 出力結果(CSV形式)
//   ファイル名,変換方式,タイトル,作成者,著作権,コメント,プログラムメッセージ
//
// 既知の問題
//   ファイル名やコメントの,をエスケープしない
//
//   参考資料
//	https://wiki.multimedia.cx/index.php/RealMedia


#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <string.h>
#include <iconv.h>

#define BUFSIZE 1024
#define ERR001 "%s,error,,,,,%s\n"

void view_help();
int existFile();

#pragma pack (1)
typedef struct {
  char   type[4];
  int   size;
  short   version;
  int   fileversion;
  int   headernum;
} RmfHeader;

typedef struct {
  char  type[4];
  int  size;
} pHeader;
#pragma pack (0)



int main(int argc, char *argv[]){

  RmfHeader   rmfh;
  pHeader     phead;
  int         i, tmp, len, c_tl, c_al, c_crl, c_cml, c_len;
  int         flgSjis, flgCopy, flgUnkw, flgCont;
  short       ver;
  char        *buf, *buf8;
  char        *p1, *p2, *plen;
  size_t      p1len, p2len;
  char        *c_t, *c_a, *c_cr , *c_cm, *ptmp;
  iconv_t     ic;

  buf  = (char *)malloc(BUFSIZE);
  buf8 = (char *)malloc(BUFSIZE);
  ic   = iconv_open("UTF-8", "Shift-JIS");

  // オプションを入力していない場合はエラーとする
  if(argc != 2){ 
    view_help();
    return 0;
  }
  // 入力ファイルが存在しない場合のエラー
  if(!existFile(argv[1])){
    printf(ERR001, argv[1] , "File Not Found");
    return 4;
  }

  // 入力ファイル読み取り
  FILE *fr = fopen(argv[1], "r");
  
  // RMF ヘッダを受信
  tmp = fread(&rmfh, sizeof(rmfh), 1, fr);  
  if(
    tmp != 1 ||        // 読み込み失敗時 
    memcmp(rmfh.type, ".RMF",4) != 0 ||   // ヘッダチェック
    bswap_32(rmfh.size) != 0x12 ||     // サイズ
    bswap_16(rmfh.version) != 0x01     // バージョン
  ) {
    printf(ERR001, argv[1] , "FILE FORMAT ERROR");
    return 12;  
  }

  flgCont = 0;
  while(fread(&phead, sizeof(phead), 1, fr) == 1){
    if(memcmp(phead.type, "CONT", 4) == 0){
      if(flgCont){
        fprintf(stderr, "Duplicate Cont area\n");
        return 12; 
      }
      flgCont = 1;
      if(fseek(fr, 0- sizeof(phead), SEEK_CUR)) { return 16; } 
      len = bswap_32(phead.size);  
      if(fread(buf, len, 1, fr) != 1){   
        fprintf(stderr, "READ FILE ERROR\n");
        return 12; 
      }
      p1 = buf;
      // ヘッダのコピー
      ptmp = buf + sizeof(phead);
      //c_tl = bswap_16( ptmp* );
      ver = bswap_16(*(short *)ptmp);
      ptmp += sizeof(short);
      if( ver != 0){
        fprintf(stderr, "ERROR: CONT VERSION\n");
	return 20;
      } 
      c_tl = bswap_16(*(short *)ptmp); 
      ptmp += sizeof(short);
      c_t  = ptmp; 
      ptmp += c_tl;
      c_al = bswap_16(*(short *)ptmp); 
      ptmp += sizeof(short);
      c_a  = ptmp; 
      ptmp += c_al;
      c_crl = bswap_16(*(short *)ptmp); 
      ptmp += sizeof(short);
      c_cr  = ptmp; 
      ptmp += c_crl;
      c_cml = bswap_16(*(short *)ptmp); 
      ptmp += sizeof(short);
      c_cm  = ptmp; 
      ptmp += c_cml;

      if( len != ptmp - buf){
        fprintf(stderr, "ERROR: CONT FORMAT\n");
	return 20;
      } 
     
      // 文字コードのパターン確認 
      flgSjis = 0;
      flgCopy = 0;
      flgUnkw = 0;

      ptmp = c_t; 
      for(i = 0; i < c_tl; i++){
	if(*ptmp < 0x20)      { flgUnkw = 1; }
	else if(*ptmp < 0x7f) {}
	else if(*ptmp < 0x9f) { flgSjis = 1; break; }
	else if(*ptmp == 0xa9){ flgCopy = 1; }
	else if(*ptmp < 0xdf) { flgUnkw = 1; }
	else if(*ptmp < 0xef) { flgSjis = 1; break; }
	else                  { flgUnkw = 1; }
	ptmp++;
      }

      ptmp = c_a; 
      for(i = 0; i < c_al; i++){
	if(*ptmp < 0x20)      { flgUnkw = 1; }
	else if(*ptmp < 0x7f) {}
	else if(*ptmp < 0x9f) { flgSjis = 1; break; }
	else if(*ptmp == 0xa9){ flgCopy = 1; }
	else if(*ptmp < 0xdf) { flgUnkw = 1; }
	else if(*ptmp < 0xef) { flgSjis = 1; break; }
	else                  { flgUnkw = 1; }
	ptmp++;
      }

      ptmp = c_cr; 
      for(i = 0; i < c_crl; i++){
	if(*ptmp < 0x20)      { flgUnkw = 1; }
	else if(*ptmp < 0x7f) {}
	else if(*ptmp < 0x9f) { flgSjis = 1; break; }
	else if(*ptmp == 0xa9){ flgCopy = 1; }
	else if(*ptmp < 0xdf) { flgUnkw = 1; }
	else if(*ptmp < 0xef) { flgSjis = 1; break; }
	else                  { flgUnkw = 1; }
	ptmp++;
      }

      ptmp = c_cm; 
      for(i = 0; i < c_cml; i++){
	if(*ptmp == 0x00)     {}      
	else if(*ptmp < 0x20) { flgUnkw = 1; }
	else if(*ptmp < 0x7f) {}
	else if(*ptmp < 0x9f) { flgSjis = 1; break; }
	else if(*ptmp == 0xa9){ flgCopy = 1; }
	else if(*ptmp < 0xdf) { flgUnkw = 1; }
	else if(*ptmp < 0xef) { flgSjis = 1; break; }
	else                  { flgUnkw = 1; }
	ptmp++;
      }

      if(flgUnkw) {
        fprintf(stderr, "UNKNOWN ENCODE\n");
	return 24;
      }

      // 出力を作成
      p2 = buf8;
      p2len = BUFSIZE;
     
      tmp = snprintf(p2, p2len, "%s,success,", argv[1]);
      p2 += tmp; p2len -= tmp;
      
      // SJISの変換
      if(flgSjis){
	// title部分
        p1 = c_t;
        tmp = c_tl; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          printf(ERR001, argv[1] , "SJIS ENCODE ERROR");
	  return 28;
        }
        tmp = snprintf(p2, p2len, ",");
        p2 += tmp; p2len -= tmp;

	// auther部分
        p1 = c_a;
        tmp = c_al; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          printf(ERR001, argv[1] , "SJIS ENCODE ERROR");
	  return 28;
        }
        tmp = snprintf(p2, p2len, ",");
        p2 += tmp; p2len -= tmp;

	// copyright部分
        p1 = c_cr;
        tmp = c_crl; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          printf(ERR001, argv[1] , "SJIS ENCODE ERROR");
	  return 28;
        }
        tmp = snprintf(p2, p2len, ",");
        p2 += tmp; p2len -= tmp;

	// comment部分
        p1 = c_cm;
        tmp = c_cml; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          printf(ERR001, argv[1] , "SJIS ENCODE ERROR");
	  return 28;
        }
        tmp = snprintf(p2, p2len, ",encode SJIS to UTF8");
        p2 += tmp; p2len -= tmp;


      } else if( flgCopy){
	// title 
        p1 = c_t;	
	for(i=0; i<c_tl;i++){
          if(*p1 == 0xa9){ *p2 = 0xc2; p2++; p2len--; }
	  *p2 = *p1; p2++; p2len--; p1++;}
        tmp = snprintf(p2, p2len, ",");
        p2 += tmp; p2len -= tmp;
        p1 = c_a;	
	for(i=0; i<c_al;i++){
          if(*p1 == 0xa9){ *p2 = 0xc2; p2++; p2len--; }
	  *p2 = *p1; p2++; p2len--; p1++;}
        tmp = snprintf(p2, p2len, ",");
        p2 += tmp; p2len -= tmp;
        p1 = c_cr;	
	for(i=0; i<c_crl;i++){
          if(*p1 == 0xa9){ *p2 = 0xc2; p2++; p2len--; }
	  *p2 = *p1; p2++; p2len--; p1++;}
        tmp = snprintf(p2, p2len, ",");
        p2 += tmp; p2len -= tmp;
        p1 = c_cm;	
	for(i=0; i<c_cml;i++){
          if(*p1 == 0xa9){ *p2 = 0xc2; p2++; p2len--; }
	  *p2 = *p1; p2++; p2len--; p1++;}
        tmp = snprintf(p2, p2len, ",change copyright symble");
        p2 += tmp; p2len -= tmp;

      } else if( flgUnkw){
        fprintf(stderr, "UNKNOWN CHARSET\n");
	return 32;

      } else {
	memcpy(p2,c_t,c_tl); p2 += c_tl; *p2 = ','; p2++;
	memcpy(p2,c_a,c_al); p2 += c_al; *p2 = ','; p2++;
	memcpy(p2,c_cr,c_crl); p2 += c_crl; *p2 = ','; p2++;
	if(*p2 ==0){
	  *p2 = ','; p2++;
	} else {
	  memcpy(p2,c_cm,c_cml); p2 += c_cml; *p2 = ','; p2++;
	}
        tmp = snprintf(p2, p2len, ",not change");
        p2 += tmp; p2len -= tmp;
	
      } 
      //tmp = bswap_32(*(int *)(buf8+4));
    } else {  // 次のエリアまでコピー
      // ファイルポインタを戻す
      if(fseek(fr, 0- sizeof(phead), SEEK_CUR)) { return 12; }
      tmp = bswap_32(phead.size);  
      while(tmp > 0)
      {
        if(tmp > BUFSIZE){
          if(fread(buf, BUFSIZE,1, fr) != 1){
            printf(ERR001, argv[1] , "FILE READ ERROR");
            return 12;
          }
          tmp -= BUFSIZE;
        } else {
          if(fread(buf, tmp,1, fr) != 1){
            printf(ERR001, argv[1] , "FILE READ ERROR");
            return 12;
          }
          tmp = 0;
        }
      }

    }

  }  
  fclose(fr);
  if(flgCont){
    printf("%s\n", buf8);
  }
  else {
    printf(ERR001, argv[1] , "NO CONT AREA");
    return 4;
  }

  return 0;
}

// ファイルの存在確認
int existFile(const char* path){
  FILE* fp = fopen(path, "r");
  if (fp == NULL){
    return 0;
  }
  fclose(fp);
  return 1;
}

// ヘルプ表示
void view_help(){
  fprintf(stderr, "rmview の使い方\n");
  fprintf(stderr, "rmview [入力ファイル名] \n");
  fprintf(stderr, "\n");
  fprintf(stderr, " 出力は ファイル名,変換方式,タイトル,作成者,著作権,コメント,プログラム内メッセージ \n");
}
