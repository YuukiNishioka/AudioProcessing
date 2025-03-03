#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wavelib.h"

int main(void)
{
    struct RIFF RIFF1;
    struct fmt fmt1;
    struct data data1;

    printf("===== 音声データにディレイをかけて，音を減衰させながら繰り返す ====\n");

    /* wave ファイルの読み込み */
    printf("元の音声データを読み込みます．\n");
    load_wave_data( &RIFF1, &fmt1, &data1, "../data/org1.wav" );

    // パラメータの入力
    int delay_count;          // ディレイ回数
    double delay_interval;    // ディレイ時間間隔（秒）
    int attenuation_type;     // 減衰方法（1: 等差減少, 2: 等比減少）
    double attenuation_value; // 公差または公比

    printf("加える減衰信号の回数 : ");
    scanf("%d", &delay_count);

    printf("減衰信号の時間間隔（秒） : ");
    scanf("%lf", &delay_interval);

    printf("減衰係数 (1: 等差減少, 2: 等比減少) : ");
    scanf("%d", &attenuation_type);

    if (attenuation_type == 1)
    {
        printf("等差減少の公差 : ");
    }
    else if (attenuation_type == 2)
    {
        printf("等比減少の公比 : ");
    }
    else
    {
        printf("無効な選択肢です。プログラムを終了します。\n");
        return 1;
    }
    scanf("%lf", &attenuation_value);

    int sample_delay = (int)(fmt1.SamplesPerSec * delay_interval);                     // 減衰信号の時間間隔をサンプル数に変換
    int newsize = data1.size_of_sounds + delay_count * sample_delay * fmt1.BlockSize; // ディレイ効果を加えた後の音声データの最大サイズ

    // 出力用データを準備
    unsigned char *new_sounds = (unsigned char *)calloc(newsize, sizeof(unsigned char)); // sizeof(unsigned char) = 1バイト
    if (new_sounds == NULL)
    {
        printf("メモリが確保失敗\n");
        exit(1);
    }

    // 元の音声データをコピー
    memcpy(new_sounds, data1.sounds, data1.size_of_sounds); // new_sounds に data1.sounds をコピー

    // 未使用領域の初期化 (8ビット音声の場合のみ)
    if (fmt1.BitsPerSample == 8)
    {
        for (int i = data1.size_of_sounds; i < newsize; i++)
        {
            new_sounds[i] = 128; // 8ビット音声では基準値128
        }
    }

    // ディレイ効果を加える
    for (int d = 0; d < delay_count; d++)
    {
        double attenuation; // 減衰率
        if (attenuation_type == 1)
        {
            attenuation = 1.0 - d * attenuation_value; // 等差減少
            if (attenuation < 0.0)
                attenuation = 0.0; // 負の値を防ぐ
        }
        else
        {
            attenuation = pow(attenuation_value, d + 1); // 等比減少
        }

        int offset = (d + 1) * sample_delay; // ディレイ効果を加える位置(サンプル数)

        // 元の音声データ（data1.sounds）の各サンプルに対して、ディレイ信号を順次加算
        for (int i = 0; i < data1.size_of_sounds; i++)
        {
            int index = i + offset; // 元データの位置（i）を基準に、ディレイ信号が加えられる位置を計算
            // ディレイ信号が音声データの範囲外の場合はスキップ
            if (index >= newsize || index < 0)
                continue;

            if (fmt1.BitsPerSample == 8) // 8ビット音声データにディレイ信号を加える処理
            {
                int original = new_sounds[index] - 128;              // 符号なし整数（0～255）を符号付き整数（-128～127）に変換(既存のデータの値)
                int delayed = (data1.sounds[i] - 128) * attenuation; // 符号なし整数（0～255）を符号付き整数（-128～127）に変換(ディレイ信号の値)
                int result = original + delayed;                     // 既存のデータとディレイ信号を加算

                // オーバーフロー処理
                if (result > 127)
                {
                    result = 127;
                }
                else if (result < -128)
                {
                    result = -128;
                }
                new_sounds[index] = result + 128; // 符号付き整数（-128～127）を符号なし整数（0～255）に変換
            }
            else if (fmt1.BitsPerSample == 16) // 16ビット音声データにディレイ信号を加える処理
            {
                // 16ビット音声データの場合は、符号付き整数（-32768～32767）で計算
                short int *new_sounds_16 = (short int *)new_sounds; 
                short int *data1_16 = (short int *)data1.sounds;

                int result = new_sounds_16[index] + (int)(data1_16[i] * attenuation); // 既存のデータとディレイ信号を加算

                // オーバーフロー処理
                if (result > 32767)
                {
                    result = 32767;
                }
                else if (result < -32768)
                {
                    result = -32768;
                }
                new_sounds_16[index] = result;
            }

        }
    }

    RIFF1.SIZE = RIFF1.SIZE - data1.size_of_sounds + newsize;
    data1.size_of_sounds = newsize;
    data1.sounds = new_sounds;

    printf("\n生成した音声を保存します．\n");
    save_wave_data(&RIFF1, &fmt1, &data1, "out1.wav");

    free(new_sounds);

    return 0;
}
