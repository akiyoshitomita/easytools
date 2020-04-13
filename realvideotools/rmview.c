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
#define ERR001  "{\"file\":\"%s\",\"state\":\"error\",\"msg\":\"%s\"}\n"
#define SUCCESS "{\"file\":\"%s\",\"state\":\"success\",\"data\":{%s},\"msg\":\"%s\"}\n"

// SJIS変換した場合にON
#define FLG_SJIS 0x00000001 
// (c)マークを変換した場合にON
#define FLG_COPY 0x00000002
// 無変換で出力した場合
#define FLG_NONE 0x00000004
// 変換元データが0x00でパディングされていた場合
// ただし 0x00以降にパディング文字以外が存在した場合はエラーとする
#define FLG_0PAD 0x02000000
// 変換データが正しくなかった場合
#define FLG_UNKW 0x01000000

void view_help();
int existFile();
int AutoEnc();

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

  RmfHeader   rmfh;	// RMF file メインヘッダ
  pHeader     phead;	// プロパティヘッダ
  iconv_t     ic;	// iconv用変換コード (SJIS => UTF-8固定)
  int         flg;      // フラグ保存場所
  int         flgCont;  // COSTプロパティの重複チェック用
  // CONT フィールド処理用
  int         c_tl, c_al, c_crl, c_cml;	// タイトル,作成者,著作権,コメントの長さ
  char        *c_t, *c_a, *c_cr, *c_cm; // 開始位置のポインタ(ibuf内部)
  short       ver;	// バージョンの値の一時保管
  
  char        *ibuf;    // 入力側バッファ先頭ポインタ
  char        *obuf;    // 出力側バッファ先頭ポインタ
  char        *p1;	// 入力側可変ポインタ
  char        *p2;      // 出力側可変ポインタ
  char        *p3;      // 一時利用可変ポインタ
  size_t      p1len;    // 入力バッファサイズ
  size_t      p2len;    // 出力バッファの残りサイズ

  int         i, tmp, len;	// 一時利用


  // 廃止予定
  //int         flgSjis, flgCopy, flgUnkw, flgCont; // 各フラグ
  char        *ptmp;

  ibuf  = (char *)malloc(BUFSIZE);
  obuf = (char *)malloc(BUFSIZE);
  ic   = iconv_open("UTF-8", "Shift-JIS");

  p2    = obuf;
  p2len = BUFSIZE;
  flg   = 0;
  flgCont = 0;

  // 文字コードのパターン確認 （削除予定) 
//  flgSjis = 0;
//  flgCopy = 0;
//  flgUnkw = 0;

  // オプションを入力していない場合はエラーとする
  if(argc != 2){ 
    view_help();
    return 0;
  }

  // 入力ファイル読み取り
  FILE *fr = fopen(argv[1], "r");
  // 入力ファイルが存在しない場合のエラー
  if(fr == 0){
    printf(ERR001, argv[1] , "File Not Found");
    return 4;
  }
  
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

  while(fread(&phead, sizeof(phead), 1, fr) == 1){
    if(memcmp(phead.type, "CONT", 4) == 0){
      // CONTプロパティが2つあった場合エラーとする
      if(flgCont){
        fprintf(stderr, "Duplicate Cont area\n");
        return 12; 
      }
      flgCont = 1;

      // CONT プロパティをメモリに読み込む
      if(fseek(fr, 0- sizeof(phead), SEEK_CUR)) { return 16; } 
      len = bswap_32(phead.size);  
      if(fread(ibuf, len, 1, fr) != 1){   
        fprintf(stderr, "READ FILE ERROR\n");
        return 12; 
      }

      p1  = ibuf; // 可変ポインタ 
      // プロパティヘッダ分の移動
      p1 += sizeof(phead);
      // バージョンの判断
      ver = bswap_16(*(short *)p1);
      p1 += sizeof(short);
      if( ver != 0){
        fprintf(stderr, "ERROR: CONT VERSION\n");
	return 20;
      } 
      // タイトル部分の処理 
      for(i = 0; i < 4; i++){ 
        p1len = bswap_16(*(short *)p1); 	// タイトル部分長さ取得
        p1 += sizeof(short);	
        if(p1len){			// タイトル部分がある場合
          p3 = p1;			// タイトルテキスト部ポインタ取得	
        	p1 += p1len;			// ポインタを次の位置へ移動
  	if(i == 0) tmp = sprintf(p2, "\"title\":\"");
  	if(i == 1) tmp = sprintf(p2, "\"author\":\"");
  	if(i == 2) tmp = sprintf(p2, "\"copyright\":\"");
  	if(i == 3) tmp = sprintf(p2, "\"comment\":\"");
  	p2 += tmp; p2len -= tmp;
          flg |= AutoEnc(ic,&p3, &p1len, &p2, &p2len);
  	*p2 = '"'; p2++; p2len--; 	// 	
  	*p2 = ','; p2++; p2len--; 	// カンマの挿入	
        } 
      }
      if( len != p1 - ibuf){
        fprintf(stderr, "ERROR: CONT FORMAT\n");
	return 20;
      } 
     
    //} else  {  // 次のエリアまでコピー
    // MDPRで logical-fileinfoの場合の処理
    } else {  // 次のエリアまでコピー
      // ファイルポインタを戻す
      if(fseek(fr, 0- sizeof(phead), SEEK_CUR)) { return 12; }
      tmp = bswap_32(phead.size);  
      while(tmp > 0)
      {
        if(tmp > BUFSIZE){
          if(fread(ibuf, BUFSIZE,1, fr) != 1){
            printf(ERR001, argv[1] , "FILE READ ERROR");
            return 12;
          }
          tmp -= BUFSIZE;
        } else {
          if(fread(ibuf, tmp,1, fr) != 1){
            printf(ERR001, argv[1] , "FILE READ ERROR");
            return 12;
          }
          tmp = 0;
        }
      }

    }

  }  
  fclose(fr);
 
  if(flg&FLG_UNKW){
    printf(ERR001, argv[1] , "ENCODE ERROR");
  } 
  else{
    if(p2 != obuf){ p2--; *p2 = 0;} 
    if(flg & FLG_SJIS) 
      printf(SUCCESS, argv[1], obuf, "encode SJIS to UTF8"); 
    else if( flg & FLG_COPY)
      printf(SUCCESS, argv[1], obuf, "change copyright symble"); 
    else printf(SUCCESS, argv[1], obuf, "NONE CHANGE");
  }

  return 0;
}

// 自動エンコード
int AutoEnc(iconv_t cd, char **inbuf, int *inlen, char **outbuf, int *outlen)
{
  int res = 0;
  char *ibuf = *inbuf;
  char *obuf = *outbuf;
  int i;
  char *p1;

  // 文字コードの判断 * SJISの場合はiconvに任せる
  p1 = ibuf;
  for(i = 0; i < *inlen; i++){
    if      (*p1 == 0x00)    { res |= FLG_0PAD; } // NULLパディング
    else if (res & FLG_0PAD) { res |= FLG_UNKW; } // NULLパディングエラー
    else if (*p1 <  0x20)    { res |= FLG_UNKW; } // 制御文字エラー    
    else if (*p1 <  0x7f)    {}                   // ASCII文字
    else if (*p1 <= 0x9f)    { res |= FLG_SJIS; break; } // SJIS 
    else if (*p1 == 0xa9)    { res |= FLG_COPY; } // (c)マーク SJIS以外
    else if (*p1 <= 0xdf)    { res |= FLG_UNKW; } // 半角カナ エラーとする
    else if (*p1 <= 0xef)    { res |= FLG_SJIS; break; } // SJIS
    else                     { res |= FLG_UNKW; }
    p1++;
  }

  // 変換不可能な場合は何もしない
  if(res&FLG_UNKW){ return res; }
  // SJISの場合
  if(res&FLG_SJIS){
    i = iconv(cd, inbuf, inlen, outbuf, outlen);
    if(i < 0) { res != FLG_UNKW; }
    return res;
  }
  if(res&FLG_COPY || 0 == res || FLG_0PAD == res){
    while(*inlen){
      if(**inbuf == 0x00) break; 
      if(**inbuf == 0xa9){ **outbuf = 0xc2; *outbuf = *outbuf + 1; *outlen--; }
      **outbuf = **inbuf; *outbuf=*outbuf+1;*inbuf=*inbuf+1;*inlen--; *outlen--;
    }
    return res;
  }

  return res;
}

// ヘルプ表示
void view_help(){
  fprintf(stderr, "rmview の使い方\n");
  fprintf(stderr, "rmview [入力ファイル名] \n");
  fprintf(stderr, "\n");
  fprintf(stderr, " 出力は ファイル名,変換方式,タイトル,作成者,著作権,コメント,プログラム内メッセージ \n");
}
