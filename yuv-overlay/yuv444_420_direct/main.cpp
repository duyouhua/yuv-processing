#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char* const SRC_FILE = "titles_444.yuv";           //yuv420p
const char* const BACK_FILE = "background_420.yuv";      //yuv420p
const char* const OUT_FILE = "out_420.yuv";              //yuv420p

//yuv444p直接叠加到yuv420p上，不做转换
int main()
{
    FILE* pfSrc = fopen(SRC_FILE, "r");
    if (pfSrc == 0)
    {
        printf("open file %s error\n", SRC_FILE);
        return 1;
    }

    FILE* pfBack = fopen(BACK_FILE, "r");
    if (pfBack == 0)
    {
        printf("open file %s error\n", BACK_FILE);
        return 1;
    }

    FILE* pfOut = fopen(OUT_FILE, "w");
    if (pfOut == 0)
    {
        printf("open file %s error\n", OUT_FILE);
        return 1;
    }

    const int w = 480;
    const int h = 272;
    const int bufsize_444 = w*h*3;
    const int bufsize_420 = w*h*3/2;

    unsigned char* const psrc = new unsigned char[bufsize_444];
    unsigned char* const pback = new unsigned char[bufsize_420];
    unsigned char* const pout = new unsigned char[bufsize_420];

    //将原图及背景图yuv数据全部读取出来
    fread(psrc, 1, bufsize_444, pfSrc);
    fread(pback, 1, bufsize_420, pfBack);

    unsigned char* psrcY = psrc;
    unsigned char* psrcU = psrcY + w*h;
    unsigned char* psrcV = psrcU + w*h;

    unsigned char* pbackY = pback;
    unsigned char* pbackU = pbackY + w*h;
    unsigned char* pbackV = pbackU + w*h/4;

    unsigned char* poutY = pout;
    unsigned char* poutU = poutY + w*h;
    unsigned char* poutV = poutU + w*h/4;

    //黑色的yuv分量值，将原图中的黑色变成透明
    const unsigned char Y_BLACK = 16;
    const unsigned char U_BLACK = 128;
    const unsigned char V_BLACK = 128;

    //开始yuv叠加处理
    for (int m = 0; m < h; m++)
    {
        for (int n = 0; n < w; n++)
        {
            int offset = m*w + n;
            unsigned char Y = psrcY[offset];
            unsigned char U = psrcU[offset];
            unsigned char V = psrcV[offset];

            //背景为420p格式，背景的uv分量偏移
            int offset_back_uv = (m/2) * (w/2) + (n/2);
            
            //420p中，周围的4个y分量共用1个uv分量。当与444p叠加时，4个y分量拥有同一个uv分量。
            //但当需要用444p的样点yuv值替换420p的样点yuv值时，只有左上角的样点能改变uv值，
            //其它样点不改变uv值，只改变y值。相当于uv值属于左上角的样点。
            bool bUVOwner = false;
            if ((m % 2 == 0) && (n % 2 == 0))
            {
                bUVOwner = true;
            }

            if (Y == Y_BLACK && U == U_BLACK && V == V_BLACK)
            {
                poutY[offset] = pbackY[offset];
                if (bUVOwner)
                {
                    poutU[offset_back_uv] = pbackU[offset_back_uv];
                    poutV[offset_back_uv] = pbackV[offset_back_uv];
                }
            }
            else
            {
                poutY[offset] = Y;
                if (bUVOwner)
                {
                    poutU[offset_back_uv] = U; 
                    poutV[offset_back_uv] = V;
                }
            }
        }
    }

    //叠加后的yuv保存到输出文件
    fwrite(pout, 1, bufsize_420, pfOut);

    fclose(pfSrc);
    fclose(pfBack);
    fclose(pfOut);

    delete[] psrc;
    delete[] pback;
    delete[] pout;

    return 0;
}
