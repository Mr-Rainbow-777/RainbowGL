#ifndef _BIT_MAP_H_
#define _BIT_MAP_H_


#include <stddef.h>
#include <stdint.h>
#include "Vector.h"





//---------------------------------------------------------------------
// 工具函数
//---------------------------------------------------------------------
template<typename T> inline T Abs(T x) { return (x < 0) ? (-x) : x; }
template<typename T> inline T Max(T x, T y) { return (x < y) ? y : x; }
template<typename T> inline T Min(T x, T y) { return (x > y) ? y : x; }

template<typename T> inline bool NearEqual(T x, T y, T error) {
	return (Abs(x - y) < error);
}

template<typename T> inline T Between(T xmin, T xmax, T x) {
	return Min(Max(xmin, x), xmax);
}

// 截取 [0, 1] 的范围
template<typename T> inline T Saturate(T x) {
	return Between<T>(0, 1, x);
}

// 类型别名
typedef Vector<2, float>  Vec2f;
typedef Vector<2, double> Vec2d;
typedef Vector<2, int>    Vec2i;
typedef Vector<3, float>  Vec3f;
typedef Vector<3, double> Vec3d;
typedef Vector<3, int>    Vec3i;
typedef Vector<4, float>  Vec4f;
typedef Vector<4, double> Vec4d;
typedef Vector<4, int>    Vec4i;

typedef Matrix<4, 4, float> Mat4x4f;
typedef Matrix<3, 3, float> Mat3x3f;
typedef Matrix<4, 3, float> Mat4x3f;
typedef Matrix<3, 4, float> Mat3x4f;



 

//---------------------------------------------------------------------
// 位图库：用于加载/保存图片，画点，画线，颜色读取
//---------------------------------------------------------------------
class Bitmap
{
public:
	inline virtual ~Bitmap() { if (_bits) delete[]_bits; _bits = NULL; }
	inline Bitmap(int width, int height) : _w(width), _h(height) {
		_pitch = width * 4;  
		_bits = new uint8_t[_pitch * _h];
		Fill(0);
	}

	inline Bitmap(const Bitmap& src) : _w(src._w), _h(src._h), _pitch(src._pitch) {
		_bits = new uint8_t[_pitch * _h];
		memcpy(_bits, src._bits, _pitch * _h);
	}



public:
	inline int GetW() const { return _w; }
	inline int GetH() const { return _h; }
	inline int GetPitch() const { return _pitch; }
	inline uint8_t* GetBits() { return _bits; }
	inline const uint8_t* GetBits() const { return _bits; }
	inline uint8_t* GetLine(int y) { return _bits + _pitch * y; }
	inline const uint8_t* GetLine(int y) const { return _bits + _pitch * y; }

public:

	inline void Fill(uint32_t color) {
		for (int j = 0; j < _h; j++) {
			uint32_t* row = (uint32_t*)(_bits + j * _pitch);  //拿到第j行的指针
			for (int i = 0; i < _w; i++, row++)
				memcpy(row, &color, sizeof(uint32_t));   //往第【j，i】这个像素点拷贝颜色缓冲值，一共4个字节
		}
	}

	//设置像素点
	inline void SetPixel(int x, int y, uint32_t color) {
		if (x >= 0 && x < _w && y >= 0 && y < _h) {
			memcpy(_bits + y * _pitch + x * 4, &color, sizeof(uint32_t));   //内存拷贝
		}
	}

	inline uint32_t GetPixel(int x, int y) const {
		uint32_t color = 0;
		if (x >= 0 && x < _w && y >= 0 && y < _h) {
			memcpy(&color, _bits + y * _pitch + x * 4, sizeof(uint32_t));
		}
		return color;
	}

	inline void DrawLine(int x1, int y1, int x2, int y2, uint32_t color) {
		int x, y;
		if (x1 == x2 && y1 == y2) {
			SetPixel(x1, y1, color);
			return;
		}
		else if (x1 == x2) {
			int inc = (y1 <= y2) ? 1 : -1;
			for (y = y1; y != y2; y += inc) SetPixel(x1, y, color);
			SetPixel(x2, y2, color);
		}
		else if (y1 == y2) {
			int inc = (x1 <= x2) ? 1 : -1;
			for (x = x1; x != x2; x += inc) SetPixel(x, y1, color);
			SetPixel(x2, y2, color);
		}
		else {
			int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
			int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
			int rem = 0;
			if (dx >= dy) {   //斜率小于1
				if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
				for (x = x1, y = y1; x <= x2; x++) {
					SetPixel(x, y, color);
					rem += dy;
					if (rem >= dx) { rem -= dx; y += (y2 >= y1) ? 1 : -1; SetPixel(x, y, color); }
				}
				SetPixel(x2, y2, color);
			}
			else {
				if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
				for (x = x1, y = y1; y <= y2; y++) {
					SetPixel(x, y, color);
					rem += dx;
					if (rem >= dy) { rem -= dy; x += (x2 >= x1) ? 1 : -1; SetPixel(x, y, color); }
				}
				SetPixel(x2, y2, color);
			}
		}
	}

	struct BITMAPINFOHEADER { // bmih  
		uint32_t	biSize;
		uint32_t	biWidth;
		int32_t		biHeight;
		uint16_t	biPlanes;
		uint16_t	biBitCount;
		uint32_t	biCompression;
		uint32_t	biSizeImage;
		uint32_t	biXPelsPerMeter;
		uint32_t	biYPelsPerMeter;
		uint32_t	biClrUsed;
		uint32_t	biClrImportant;
	};

	// 读取 BMP 图片，支持 24/32 位两种格式
	inline static Bitmap* LoadFile(FILE* fp,const char* filename) {
		errno_t err = fopen_s(&fp,filename, "rb");
		if (fp == NULL) return NULL;
		BITMAPINFOHEADER info;
		uint8_t header[14];    //8位一个字节    以一个字节的offset来读取14个内存块
		int hr = (int)fread(header, 1, 14, fp);  //读取流式数据
		if (hr != 14) { fclose(fp); return NULL; }
		if (header[0] != 0x42 || header[1] != 0x4d) { fclose(fp); return NULL; }
		hr = (int)fread(&info, 1, sizeof(info), fp);
		if (hr != 40) { fclose(fp); return NULL; }
		if (info.biBitCount != 24 && info.biBitCount != 32) { fclose(fp); return NULL; }
		Bitmap* bmp = new Bitmap(info.biWidth, info.biHeight);
		uint32_t offset;
		memcpy(&offset, header + 10, sizeof(uint32_t));
		fseek(fp, offset, SEEK_SET);
		uint32_t pixelsize = (info.biBitCount + 7) / 8;
		uint32_t pitch = (pixelsize * info.biWidth + 3) & (~3);
		for (int y = 0; y < (int)info.biHeight; y++) {
			uint8_t* line = bmp->GetLine(info.biHeight - 1 - y);
			for (int x = 0; x < (int)info.biWidth; x++, line += 4) {
				line[3] = 255;
				fread(line, pixelsize, 1, fp);
			}
			fseek(fp, pitch - info.biWidth * pixelsize, SEEK_CUR);
		}
		fclose(fp);
		return bmp;
	}

	// 保存 BMP 图片
	inline bool SaveFile(FILE* fp,const char* filename, bool withAlpha = false) const {
		errno_t err = fopen_s(&fp,filename, "wb");
		if (fp == NULL) return false;
		BITMAPINFOHEADER info;
		uint32_t pixelsize = (withAlpha) ? 4 : 3;
		uint32_t pitch = (GetW() * pixelsize + 3) & (~3);
		info.biSizeImage = pitch * GetH();
		uint32_t bfSize = 54 + info.biSizeImage;
		uint32_t zero = 0, offset = 54;
		fputc(0x42, fp);
		fputc(0x4d, fp);
		fwrite(&bfSize, 4, 1, fp);
		fwrite(&zero, 4, 1, fp);
		fwrite(&offset, 4, 1, fp);
		info.biSize = 40;
		info.biWidth = GetW();
		info.biHeight = GetH();
		info.biPlanes = 1;
		info.biBitCount = (withAlpha) ? 32 : 24;
		info.biCompression = 0;
		info.biXPelsPerMeter = 0xb12;
		info.biYPelsPerMeter = 0xb12;
		info.biClrUsed = 0;
		info.biClrImportant = 0;
		fwrite(&info, sizeof(info), 1, fp);
		// printf("pitch=%d %d\n", (int)pitch, info.biSizeImage);
		for (int y = 0; y < GetH(); y++) {
			const uint8_t* line = GetLine(info.biHeight - 1 - y);
			uint32_t padding = pitch - GetW() * pixelsize;
			for (int x = 0; x < GetW(); x++, line += 4) {
				fwrite(line, pixelsize, 1, fp);
			}
			for (int i = 0; i < (int)padding; i++) fputc(0, fp);
		}
		fclose(fp);
		return true;
	}

	// 双线性插值
	inline uint32_t SampleBilinear(float x, float y) const {
		int32_t fx = (int32_t)(x * 0x10000);
		int32_t fy = (int32_t)(y * 0x10000);
		int32_t x1 = Between(0, _w - 1, fx >> 16);
		int32_t y1 = Between(0, _h - 1, fy >> 16);
		int32_t x2 = Between(0, _w - 1, x1 + 1);
		int32_t y2 = Between(0, _h - 1, y1 + 1);
		int32_t dx = (fx >> 8) & 0xff;
		int32_t dy = (fy >> 8) & 0xff;
		if (_w <= 0 || _h <= 0) return 0;
		uint32_t c00 = GetPixel(x1, y1);
		uint32_t c01 = GetPixel(x2, y1);
		uint32_t c10 = GetPixel(x1, y2);
		uint32_t c11 = GetPixel(x2, y2);
		return BilinearInterp(c00, c01, c10, c11, dx, dy);
	}

	// 矢量转整数颜色
	inline static uint32_t vector_to_color(const Vec4f& color) {
		uint32_t r = (uint32_t)Between(0, 255, (int)(color.r * 255.0f));
		uint32_t g = (uint32_t)Between(0, 255, (int)(color.g * 255.0f));
		uint32_t b = (uint32_t)Between(0, 255, (int)(color.b * 255.0f));
		uint32_t a = (uint32_t)Between(0, 255, (int)(color.a * 255.0f));
		return (r << 16) | (g << 8) | b | (a << 24);
	}

	// 矢量转换整数颜色
	inline static uint32_t vector_to_color(const Vec3f& color) {
		return vector_to_color(color.xyz1());
	}

	// 整数颜色到矢量
	inline static Vec4f vector_from_color(uint32_t rgba) {
		Vec4f out;
		out.r = ((rgba >> 16) & 0xff) / 255.0f;
		out.g = ((rgba >> 8) & 0xff) / 255.0f;
		out.b = ((rgba >> 0) & 0xff) / 255.0f;
		out.a = ((rgba >> 24) & 0xff) / 255.0f;
		return out;
	}

	// 纹理采样
	inline Vec4f Sample2D(float u, float v) const {
		uint32_t rgba = SampleBilinear(u * _w + 0.5f, v * _h + 0.5f);
		return vector_from_color(rgba);
	}

	// 纹理采样：直接传入 Vec2f
	inline Vec4f Sample2D(const Vec2f& uv) const {
		return Sample2D(uv.x, uv.y);
	}

	// 按照 Vec4f 画点
	inline void SetPixel(int x, int y, const Vec4f& color) {
		SetPixel(x, y, vector_to_color(color));
	}

	// 上下反转
	inline void FlipVertical() {
		uint8_t* buffer = new uint8_t[_pitch];
		for (int i = 0, j = _h - 1; i < j; i++, j--) {
			memcpy(buffer, GetLine(i), _pitch);
			memcpy(GetLine(i), GetLine(j), _pitch);
			memcpy(GetLine(j), buffer, _pitch);
		}
		delete[]buffer;
	}

	// 水平反转
	inline void FlipHorizontal() {
		for (int y = 0; y < _h; y++) {
			for (int i = 0, j = _w - 1; i < j; i++, j--) {
				uint32_t c1 = GetPixel(i, y);
				uint32_t c2 = GetPixel(j, y);
				SetPixel(i, y, c2);
				SetPixel(j, y, c1);
			}
		}
	}

protected:

	// 双线性插值计算：给出四个点的颜色，以及坐标偏移，计算结果
	inline static uint32_t BilinearInterp(uint32_t tl, uint32_t tr,
		uint32_t bl, uint32_t br, int32_t distx, int32_t disty) {
		uint32_t f, r;
		int32_t distxy = distx * disty;
		int32_t distxiy = (distx << 8) - distxy;  /* distx * (256 - disty) */
		int32_t distixy = (disty << 8) - distxy;  /* disty * (256 - distx) */
		int32_t distixiy = 256 * 256 - (disty << 8) - (distx << 8) + distxy;
		r = (tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy
			+ (bl & 0x000000ff) * distixy + (br & 0x000000ff) * distxy;
		f = (tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy
			+ (bl & 0x0000ff00) * distixy + (br & 0x0000ff00) * distxy;
		r |= f & 0xff000000;
		tl >>= 16; tr >>= 16; bl >>= 16; br >>= 16; r >>= 16;
		f = (tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy
			+ (bl & 0x000000ff) * distixy + (br & 0x000000ff) * distxy;
		r |= f & 0x00ff0000;
		f = (tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy
			+ (bl & 0x0000ff00) * distixy + (br & 0x0000ff00) * distxy;
		r |= f & 0xff000000;
		return r;
	}

protected:
	int32_t _w;  //宽
	int32_t _h;    //高
	int32_t _pitch;  // 图片宽度所占字节数 （每个像素占用32位，RGBA）
	uint8_t* _bits;   //整幅图的字节数
};






#endif // !_BIT_MAP_H_

