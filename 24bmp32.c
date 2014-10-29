#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef long LONG;


typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER, *PBITMAPINFOHEADER;



int main(int argc, char *argv[])
{  
    int data_size;
    unsigned char *img_buf, *img_buf_in;
    BITMAPFILEHEADER file_head;
    BITMAPINFOHEADER info_head;

    BITMAPFILEHEADER file_head_in;
    BITMAPINFOHEADER info_head_in;

    if (argc != 3)
    {
        printf("usage %s <in> <out>\n", argv[0]);
        return -1;
    }        

    /*open files*/
    int in = open(argv[1], O_RDONLY);
    int out = open(argv[2], O_WRONLY | O_CREAT, 0644);
    if ((in == -1) || (out == -1)) {
        perror("open image");
        return -1;
    }

    read(in, &file_head_in, sizeof(file_head_in));
    read(in, &info_head_in, sizeof(info_head_in));

    printf("in %ld x %ld x %d\n", info_head_in.biWidth, info_head_in.biHeight, info_head_in.biBitCount);

    data_size = 4 * info_head_in.biHeight * info_head_in.biWidth;
    /*initialize bmp structs*/
    file_head.bfType = 0x4d42;
    file_head.bfSize = sizeof(file_head) + sizeof(info_head) + data_size;
    file_head.bfReserved1 = file_head.bfReserved2 = 0;
    file_head.bfOffBits = sizeof(file_head) + sizeof(info_head);

    info_head.biSize = sizeof(info_head);
    info_head.biWidth = info_head_in.biWidth;
    info_head.biHeight = info_head_in.biHeight;
    info_head.biPlanes = 1;
    info_head.biBitCount = 32;
    info_head.biCompression = 0;
    info_head.biSizeImage = data_size;
    info_head.biXPelsPerMeter = 0;
    info_head.biYPelsPerMeter = 0;
    info_head.biClrUsed = 0;
    info_head.biClrImportant = 0;

    int bpl_in = info_head_in.biWidth * 3 + (4 - info_head_in.biWidth * 3 % 4);
    img_buf_in = (unsigned char *)malloc(info_head_in.biHeight * bpl_in);
    img_buf = (unsigned char *)malloc(data_size);
    if (img_buf_in == NULL || img_buf == NULL)
    {
        printf("malloc failed!\n");
        return -1;
    }

    /*read img data and */
    int len = read(in, img_buf_in, data_size);
    printf("read %d / %d bytes\n", len, data_size);

    /* convert 24 to 32 bpp */
    for (int i = 0; i < info_head.biHeight; i++) {
        for (int j = 0; j < info_head.biWidth; j++) {
            int *pout = (int *)img_buf + i * info_head.biWidth + j;
            unsigned char *pin = img_buf_in + i * bpl_in + j * 3;
            *pout = *(pin + 2) << 16
                | *(pin + 1) << 8 
                | *(pin + 0) << 0 
                | 0xff000000;
        }
    }


    /* save file */
    write(out, &file_head, sizeof(file_head));
    write(out, &info_head, sizeof(info_head));

    for (int i = 0; i < info_head.biHeight; i++)
    {
        /* revese img and write to file */
        write(out, img_buf + info_head.biWidth * i *4, info_head.biWidth * 4);
        // write(out, img_buf + info_head.biWidth * (info_head.biHeight-i-1)*4, info_head.biWidth * 4);
    }

    close(in);
    close(out);
    return 0;
}

