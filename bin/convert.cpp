// demo.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <speex/speex.h>
#include <stdio.h>
#include <ostream>

#include <speex\speex_preprocess.h>
#include <speex\speex_echo.h>
#pragma comment(lib,"libspeexdsp.lib")
#define FRAME_SIZE 160
int main(int argc, char **argv)
{

        char *inFile;

        FILE *fin,*fout1,*fout2,*fout3;

        short in[FRAME_SIZE];
        short out[FRAME_SIZE];
        float input[FRAME_SIZE];
        float output[FRAME_SIZE];
        char cbits[200];

        int nbBytes;

        /*保存编码的状态*/

        void *stateEncode;
        void *stateDecode;

        /*保存字节因此他们可以被speex常规读写*/



        SpeexBits bitsEncode;
        SpeexBits bitsDecode;
        int i, tmp;

        //新建一个新的编码状态在窄宽(narrowband)模式下

        stateEncode = speex_encoder_init(&speex_nb_mode);
        stateDecode = speex_decoder_init(&speex_nb_mode);
        //设置质量为8(15kbps)

        tmp=0;
        speex_encoder_ctl(stateEncode, SPEEX_SET_VBR, &tmp);
        float q=4;
        speex_encoder_ctl(stateEncode, SPEEX_SET_VBR_QUALITY, &q);
        speex_encoder_ctl(stateEncode, SPEEX_SET_QUALITY, &tmp);

        inFile = argv[1];

        fin = fopen("c:/demo.pcm", "rb");
        fout1 = fopen("c:/demo_speex.raw", "wb");
        fout2 = fopen("c:/demo1.pcm", "wb");
        fout3 = fopen("c:/demo_slience.pcm", "wb");
        //初始化结构使他们保存数据

        speex_bits_init(&bitsEncode);
        speex_bits_init(&bitsDecode);
        int ret;
        int j=0;
        SpeexPreprocessState * m_st;
        SpeexEchoState *echo_state;
        m_st=speex_preprocess_state_init(160, 8000);
//        echo_state = speex_echo_state_init(160, 8000);
        int denoise = 1;
        int noiseSuppress = -25;
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_DENOISE, &denoise); //降噪
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress); //设置噪声的dB


        int agc = 1;
        q=24000;
        //actually default is 8000(0,32768),here make it louder for voice is not loudy enough by default. 8000
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC, &agc);//增益
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC_LEVEL,&q);
        int vad = 1;
        int vadProbStart = 80;
        int vadProbContinue = 65;
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_VAD, &vad); //静音检测
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_PROB_START , &vadProbStart); //Set probability required for the VAD to go from silence to voice
        speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vadProbContinue); //Set probability required for the VAD to stay in the voice state (integer percent)


        while (1)
        {
                memset(out,0,FRAME_SIZE*sizeof(short));
                //读入一帧16bits的声音
                j++;
                int r=fread(in, sizeof(short), FRAME_SIZE, fin);

                if (r<FRAME_SIZE)
                        break;

                //把16bits的值转化为float,以便speex库可以在上面工作
        spx_int16_t * ptr=(spx_int16_t *)in;

        if (speex_preprocess_run(m_st, ptr))//预处理 打开了静音检测和降噪
        {
                printf("speech,");
                fwrite(in, sizeof(short), FRAME_SIZE, fout3);
        }
        else
        {
                printf("slience,");
                fwrite(out, sizeof(short), FRAME_SIZE, fout3);
        }
                for (i=0;i<FRAME_SIZE;i++)

                        input[i]=in[i];

                //清空这个结构体里所有的字节,以便我们可以编码一个新的帧

                speex_bits_reset(&bitsEncode);

                //对帧进行编码

                ret=speex_encode(stateEncode, input, &bitsEncode);
                //把bits拷贝到一个利用写出的char型数组
                nbBytes = speex_bits_write(&bitsEncode, cbits, 200);
                fwrite(cbits, sizeof(char), nbBytes, fout1);
                printf("%02d,",nbBytes);


                //清空这个结构体里所有的字节,以便我们可以编码一个新的帧
                speex_bits_reset(&bitsDecode);
                //将编码数据如读入bits
                speex_bits_read_from(&bitsDecode, cbits, nbBytes);
                //对帧进行解码
                ret = speex_decode(stateDecode, &bitsDecode,output);
                for (i=0;i<FRAME_SIZE;i++)
                        out[i]=output[i];
                fwrite(out, sizeof(short), FRAME_SIZE, fout2);
        }

        //释放编码器状态量

        speex_encoder_destroy(stateEncode);

        //释放bit_packing结构

        speex_bits_destroy(&bitsEncode);
        speex_decoder_destroy(stateDecode);

        //释放bit_packing结构

        speex_bits_destroy(&bitsDecode);
        fclose(fin);
        fclose(fout1);
        fclose(fout2);
        fclose(fout3);
 return 0;

}
