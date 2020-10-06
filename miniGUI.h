#ifndef _miniGUI_H_
#define _miniGUI_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "IT8951.h" 

typedef struct 
{
  uint16_t X;
  uint16_t Y;
}Point, *pPoint; 

//14byte�ļ�ͷ
typedef struct
{
	uint16_t cfType;//�ļ����ͣ�"BM"(0x4D42)
	uint32_t cfSize;//�ļ���С���ֽڣ�
	uint32_t cfReserved;//������ֵΪ0
	uint32_t cfoffBits;//������������ļ�ͷ��ƫ�������ֽڣ�
}__attribute__((packed)) BITMAPFILEHEADER;
//__attribute__((packed))�������Ǹ��߱�����ȡ���ṹ�ڱ�������е��Ż�����
 
//40byte��Ϣͷ
typedef struct
{
	uint32_t ciSize;//40
	uint32_t ciWidth;//����
	uint32_t ciHeight;//�߶�
	uint16_t ciPlanes;//Ŀ���豸��λƽ������ֵΪ1
	uint16_t ciBitCount;//ÿ�����ص�λ��
	uint32_t ciCompress;//ѹ��˵��
	uint32_t ciSizeImage;//���ֽڱ�ʾ��ͼ���С�������ݱ�����4�ı���
	uint32_t ciXPelsPerMeter;//Ŀ���豸��ˮƽ������/��
	uint32_t ciYPelsPerMeter;//Ŀ���豸�Ĵ�ֱ������/��
	uint32_t ciClrUsed;//λͼʹ�õ�ɫ�����ɫ��
	uint32_t ciClrImportant;//ָ����Ҫ����ɫ�����������ֵ������ɫ��ʱ�����ߵ���0ʱ������ʾ������ɫ��һ����Ҫ
}__attribute__((packed)) BITMAPINFOHEADER;

typedef struct
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t reserved;
}__attribute__((packed)) PIXEL;//��ɫģʽRGB

#define ABS(X)    ((X) > 0 ? (X) : -(X))     

void EPD_Clear(uint8_t Color);
void EPD_DrawPixel(uint16_t x0,uint16_t y0,uint8_t color);
void EPD_DrawPixel90(uint16_t x0, uint16_t y0, uint8_t color);
void EPD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint8_t color);
void EPD_DrawRect(uint16_t Xpos,uint16_t Ypos,uint16_t Width,uint16_t Height,uint8_t color);
void EPD_DrawCircle(uint16_t Xpos,uint16_t Ypos,uint16_t Radius,uint8_t color);
void EPD_DrawPolygon(pPoint Points,uint16_t PointCount,uint8_t color);
void EPD_DrawEllipse(uint16_t Xpos,uint16_t Ypos,uint16_t XRadius,uint16_t YRadius,uint8_t color);
void EPD_FillRect(uint16_t Xpos,uint16_t Ypos,uint16_t Width,uint16_t Height,uint8_t color);
void EPD_FillCircle(uint16_t Xpos,uint16_t Ypos,uint16_t Radius,uint8_t color);
void EPD_PutChar(uint16_t Xpos,uint16_t Ypos,uint8_t ASCI,uint8_t charColor,uint8_t bkColor);
void EPD_Text(uint16_t Xpos,uint16_t Ypos,uint8_t *str,uint8_t Color,uint8_t bkColor);
void EPD_DrawBitmap(uint16_t Xpos, uint16_t Ypos,uint16_t *bmp);
void EPD_DrawMatrix(uint16_t Xpos, uint16_t Ypos,uint16_t Width, uint16_t High,const uint16_t* Matrix);
uint8_t Show_bmp(uint32_t x, uint32_t y,char *path);



#endif
