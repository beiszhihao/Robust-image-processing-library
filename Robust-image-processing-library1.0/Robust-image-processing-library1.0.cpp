/*Robust图像处理库
*版本：v1.0
*作者：周志豪
*4.17 19：09
*/
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
//类型定义
//8bit  
typedef char UINT_8;
//16bit  
typedef char UINT_16[2];
//22bit  
typedef char UINT_22[3];
//256bit  
typedef char UINT_256[256];
//32bit  
typedef int  UINT_32;
//data  
typedef char* DATA;
//64 fbit  
typedef double FUINT_64;
//16bit int  
typedef short SUINT_16;
//结构体
//图像结构体，防止结构体对齐否则读取文件流时出错，所以用pragma命令强制对齐
#pragma pack(1)  
typedef struct image_struct{
	UINT_16   image_pil;                //文件标识符  
	UINT_32   image_Size;               //文件大小  
	UINT_16   image_Reserved1;          //无需过问  
	UINT_16   image_Reserved2;          //无需过问  
	UINT_32   image_Offbits;            //头文件到图像数据偏移量  
	UINT_32   image_Stsize;             //结构体所需大小  
	UINT_32   image_Width;              //图像宽度  
	UINT_32   image_Height;             //图像高度  
	UINT_16   image_Planes;             //目标设备位面数  
	SUINT_16  image_Bitcount;           //像素点占用多少bit位  
	UINT_32   image_Compression;        //图像压缩类型  
	UINT_32   image_Sizeimage;          //图像的大小  
	UINT_32   image_Xpelspermeter;      //水平分辨率  
	UINT_32   image_Ypelspermeter;      //垂直分辨率  
	UINT_32   image_ClrUsed;            //调色板索引数  
	UINT_32   image_Clrlmportant;       //图像显示重要颜色索引数目  
	DATA      image_Data;               //图像数据 
	UINT_32   image_Data_Size;			//图像数据大小
}image; 
#pragma pack()  
//函数
//加载图像 
int image_load(struct image_struct** im, char *path){
	FILE *image_path_fp;
	image_path_fp = fopen(path, "rb");
	if (image_path_fp == NULL){
		return -1;
	}
	//取文件大小
	fseek(image_path_fp, 0, SEEK_END); //定位到文件末 
	int nFileLen = ftell(image_path_fp); //文件长度
	fseek(image_path_fp, 0, SEEK_SET);	//恢复到文件头，防止从文件尾读取数据
	//读取头信息
	if (fread((*im), (sizeof(struct image_struct) - ((sizeof(DATA/*image_Data*/)+(sizeof(UINT_32)/*image_Data_Size*/)))/*暂不读取数据，无法从头文件中获取数据偏移量，防止数据混乱*/), 1, image_path_fp) == 0){
		return -2;
	}
	//给data变量分配内存
	(*im)->image_Data = (DATA)malloc(nFileLen-(*im)->image_Offbits/*完整的数据大小*/);
	//判断是否分配成功
	if ((*im)->image_Data == NULL){	//如果没有可用堆内存则malloc返回NULL
		return -3;
	}
	//读取数据
	//读取前将文件指针挪移到文件头信息后，找到正确的数据存储区
	fseek(image_path_fp, 0, SEEK_SET);	//恢复到文件头，因为已经fread一次了，所以数据文件指针发生变更
	fseek(image_path_fp, (*im)->image_Offbits, SEEK_CUR);	//忽略头数据
	if (fread((*im)->image_Data, (nFileLen - (*im)->image_Offbits/*file - off = 实际大小*/), 1, image_path_fp) == 0){
		return -4;
	}
	//保存文件大小,方便读写操作
	(*im)->image_Data_Size = (nFileLen - (*im)->image_Offbits/*file - off = 实际大小*/);
	//文件指针释放，防止占用文件内核的临界资源
	fclose(image_path_fp);
	image_path_fp == NULL;
	return 0;
}
//给图像数据分配内存
int image_malloc(struct image_struct** im){
	*im = (struct image_struct*)malloc(sizeof(struct image_struct));
	if (*im == NULL){
		return -1;
	}
	return 0;
}
//将图像转换成反向图_该方法只能用于真彩图
int image_reverse_rgb(struct image_struct** im){
	if ((*im) == NULL){	//判断传递进来的图像指针是否为空
		return -1;
	}
	//转换成反向图很简单只需要将每个图像里的像素点转换成负的就可以了,注意在一个24位的图像文件中一个字节对应一个颜色值三个字节则为一个完整的像素点，所以我们一个一个像素点的转换就可以了
	//算法公式为：S=-R-G-B
	for (int i = 0; i < (*im)->image_Data_Size; ++i){
			if ((*im)->image_Data[i] == (int)0){	//如果等于0则不处理
				continue;	//开始下一次循环
			}//i*(*im)->image_Width + j
			(*im)->image_Data[i] = -(*im)->image_Data[i];	//调用宏函数转换
	}
	return 0;
}
//保存图像数据到文件
int image_save_file(struct image_struct** im, char *path){
	FILE* file_fp = fopen(path, "wb+");	//以二进制可读写方式打开
	if (file_fp == NULL){	//判断文件指针是否为空
		return -1;
	}
	//写入头信息
	fwrite((*im), (*im)->image_Offbits/*直接写入头文件到数据的偏移量大小即可*/, 1, file_fp);
	//写入文件数据
	fwrite((*im)->image_Data, (*im)->image_Data_Size, 1, file_fp);
	return 0;

}
int main(){
	image *imga;
	image_malloc(&imga);
	image_load(&imga, "test.bmp");
	image_reverse_rgb(&imga);
	image_save_file(&imga, "test1.bmp");
	printf("转换完成");
	getchar();
}