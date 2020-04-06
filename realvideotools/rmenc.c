// Real media file encoder (SJIS to UTF-8)
// CONT name 
//   古い RealVideoのファイルのタイトルなどがSJISとなっており
//   文字化けが発生していたため、UTF-8に変換するプログラムを途中まで作成
//
//   作成したが1フレームあたりがエラーとなることがわかったため
//   作成を終了します。
//   
//   以下の方法でMP4に変換後に修正することで対応しました。
//     $ mplayer -dumpstream [入力ファイル.rm]
//     $ ffmpeg -i stream.dump [出力ファイル.mp4]
//     $ rm stream.dump
//     $ AtomicParsley [出力ファイル.mp4] --title [タイトル] \
//         --commnet [コメント] --copyright [著作権者]
//     $ rm [出力ファイル.mp4]
//   * 最終出力ファイルは 出力ファイル名-temp-xxxxx.mp4 の名称になります。
//   今後簡単なヘルを記載する予定です。
//
//   参考資料
//	https://wiki.multimedia.cx/index.php/RealMedia


#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <string.h>
#include <iconv.h>

#define BUFSIZE 1024

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
  int         flgSjis, flgCopy, flgUnkw;
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
  if(argc != 3){ 
    view_help();
    return 0;
  }
  // 入力ファイルが存在しない場合のエラー
  if(!existFile(argv[1])){
    printf("入力ファイル %s が存在しません\n", argv[1] );
    printf("ファイル名を確認してください\n");
    return 4;
  }

  // 入力ファイルが存在しない場合のエラー
  if(existFile(argv[2])){
    printf("出力ファイル %s が存在します\n", argv[2] );
    printf("上書きをしないので別の名前に変更して再実行してください\n");
    return 4;
  }

  // 入力ファイル読み取り
  FILE *fr = fopen(argv[1], "r");
  
  // 入力ファイル読み取り
  FILE *fw = fopen(argv[2], "w");

  // RMF ヘッダを受信
  tmp = fread(&rmfh, sizeof(rmfh), 1, fr);  
  if(
    tmp != 1 ||        // 読み込み失敗時 
    memcmp(rmfh.type, ".RMF",4) != 0 ||   // ヘッダチェック
    bswap_32(rmfh.size) != 0x12 ||     // サイズ
    bswap_16(rmfh.version) != 0x01     // バージョン
  ) {
    fprintf(stderr, "READ FILE ERROR\n");
    return 12;  
  }
  // ココに書き込みを追加
  fwrite(&rmfh, sizeof(rmfh), 1, fw);

  while(fread(&phead, sizeof(phead), 1, fr) == 1){
    if(memcmp(phead.type, "CONT", 4) == 0){
      if(fseek(fr, 0- sizeof(phead), SEEK_CUR)) { return 16; } 
      len = bswap_32(phead.size);  
      if(fread(buf, len, 1, fr) != 1){   
        fprintf(stderr, "READ FILE ERROR\n");
        return 12; 
      }
      p1 = buf;
      p2 = buf8;
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
	if(*ptmp < 0x20)      { flgUnkw = 1; }
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
     
      // ヘッダ部分のコピー(サイズ未挿入);
      memcpy(buf8, buf, sizeof(phead));
      p2    += sizeof(phead);
      p2len -= sizeof(phead);
      
      // バージョンの挿入 
      *p2    = 0;  p2++;  p2len--;
      *p2    = 0;  p2++;  p2len--;

      // SJISの変換
      if(flgSjis){
	// auther部分
        plen = p2;  // lenghtの場所
        p2 += 2; p2len -= 2;

        p1 = c_a;
        tmp = c_al; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          fprintf(stderr, "SJIS ENCODE ERROR\n");
	  return 28;
        }
        *(short *)plen = bswap_16( (short) (p2 - plen - 2));
	// title部分
        plen = p2;  // lenghtの場所
        p2 += 2; p2len -= 2;

        p1 = c_t;
        tmp = c_tl; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          fprintf(stderr, "SJIS ENCODE ERROR\n");
	  return 28;
        }
        *(short *)plen = bswap_16( (short) (p2 - plen - 2));
	// copyright部分
        plen = p2;  // lenghtの場所
        p2 += 2; p2len -= 2;

        p1 = c_cr;
        tmp = c_crl; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          fprintf(stderr, "SJIS ENCODE ERROR\n");
	  return 28;
        }
        *(short *)plen = bswap_16( (short) (p2 - plen - 2));
	// comment部分
        plen = p2;  // lenghtの場所
        p2 += 2; p2len -= 2;

        p1 = c_cm;
        tmp = c_cml; 
        if( iconv(ic, &p1, &tmp, &p2, &p2len) < 0){
          fprintf(stderr, "SJIS ENCODE ERROR\n");
	  return 28;
        }
        *(short *)plen = bswap_16( (short) (p2 - plen - 2));

        *(int *)(buf8 + 4) = bswap_32(p2-buf8);
      } else if( flgCopy){
	      printf("not build now \n");
	      return 0;
      } else {
	      printf("not build now \n");
	      return 0;
      } 
      tmp = bswap_32(*(int *)(buf8+4));
      fwrite(buf8, tmp , 1, fw);
    } else {  // 次のエリアまでコピー
      // ファイルポインタを戻す
      if(fseek(fr, 0- sizeof(phead), SEEK_CUR)) { return 12; }
      tmp = bswap_32(phead.size);  
      while(tmp > 0)
      {
        if(tmp > BUFSIZE){
          if(fread(buf, BUFSIZE,1, fr) != 1){
            fprintf(stderr, "READ FILE ERROR\n");
            return 12;
          }
          fwrite(buf, BUFSIZE, 1, fw);
          tmp -= BUFSIZE;
        } else {
          if(fread(buf, tmp,1, fr) != 1){
            fprintf(stderr, "READ FILE ERROR\n");
            return 12;
          }
          fwrite(buf, tmp, 1, fw);
          tmp = 0;
        }
      }

    }

  }  

  fclose(fr);

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
  printf("rmenc の使い方\n");
  printf("rmenc [入力ファイル名] [出力ファイル名]\n");
}
