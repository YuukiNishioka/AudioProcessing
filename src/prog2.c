#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wavelib.h"

int main(void)
{
    struct RIFF RIFF1;
    struct fmt fmt1;
    struct data data1;

    printf("===== 男性の声に変換するプログラム ====\n");

    /* wave ファイルの読み込み */
    printf("元の音声データを読み込みます．\n");
    load_wave_data(&RIFF1, &fmt1, &data1, "../data/org2.wav");

    // ピッチを下げる(男性の声に変換)
    double pitch = 0.7; // ピッチ変更係数（<1で低下、>1で上昇）
    int newsize = (int)(data1.size_of_sounds / pitch);

    // 出力用データを準備
    unsigned char *new_sounds = (unsigned char *)calloc(newsize, sizeof(unsigned char)); // sizeof(unsigned char) = 1バイト
    if (new_sounds == NULL)
    {
        printf("メモリが確保失敗\n");
        exit(1);
    }

    // 元の波形データを間引いてピッチを変更
    for (int i = 0; i < newsize; i++)
    {
        int index = (int)(i * pitch); // ピッチ変更後の位置(少数の場合は四捨五入)
        if (index < data1.size_of_sounds)
        {
            new_sounds[i] = data1.sounds[index]; // ピッチ変更後の位置に元のデータをコピー
        }
    }

    RIFF1.SIZE = RIFF1.SIZE - data1.size_of_sounds + newsize;
    data1.size_of_sounds = newsize;
    data1.sounds = new_sounds;

    printf("\n生成した音声を保存します．\n");
    save_wave_data(&RIFF1, &fmt1, &data1, "out2.wav");

    free(new_sounds);

    return 0;
}
