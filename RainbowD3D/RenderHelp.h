#ifndef _RENDER_HELP_
#define _RENDER_HELP_



#include "Vector.h";
#include "Matrix.h";
#include <map>
#include <functional>
#include "Bitmap.h"






// matrix set to zero
inline static Mat4x4f matrix_set_zero() {
	Mat4x4f m;
	m.m[0][0] = m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][1] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][2] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = m.m[3][3] = 0.0f;
	return m;
}

// set to identity
inline static Mat4x4f matrix_set_identity() {
	Mat4x4f m;
	m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
	m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	return m;
}

// 平移变换
inline static Mat4x4f matrix_set_translate(float x, float y, float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[3][0] = x;
	m.m[3][1] = y;
	m.m[3][2] = z;
	return m;
}

// 缩放变换
inline static Mat4x4f matrix_set_scale(float x, float y, float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[0][0] = x;
	m.m[1][1] = y;
	m.m[2][2] = z;
	return m;
}

// Local2World矩阵
inline static Mat4x4f Model2World_Matrix(const Vec3f& pos,const Vec3f& xAxis, const Vec3f& yAxis, const Vec3f& zAxis) {
	Mat4x4f m;
	m.SetRow(0, Vec4f(xAxis.x, xAxis.y, xAxis.z, 0));
	m.SetRow(1, Vec4f(yAxis.x, yAxis.y, yAxis.z, 0));
	m.SetRow(2, Vec4f(zAxis.x, zAxis.y, zAxis.z, 0));
	m.SetRow(3, Vec4f(pos.x, pos.y, pos.z, 1.0f));
	return m;
}

// 摄影机变换矩阵：eye/视点位置，at/看向哪里，up/指向上方的矢量  
//Camera2World矩阵
inline static Mat4x4f matrix_set_lookat(const Vec3f& eye, const Vec3f& at, const Vec3f& up) {
	Vec3f zaxis = vector_normalize(eye-at);
	Vec3f xaxis = vector_normalize(vector_cross(up, zaxis));
	Vec3f yaxis = vector_cross(xaxis, zaxis);
	Mat4x4f m;
	m.SetRow(0, Vec4f(xaxis.x, xaxis.y, xaxis.z, 0));
	m.SetRow(1, Vec4f(yaxis.x, yaxis.y, yaxis.z, 0));
	m.SetRow(2, Vec4f(zaxis.x, zaxis.y, zaxis.z, 0));
	m.SetRow(3, Vec4f(eye.x,eye.y,eye.z, 1.0f));
	return m;
}


//View矩阵  (右乘)
inline static Mat4x4f World2Camera_Matrix(const Vec3f& eye, const Vec3f& at, const Vec3f& up)
{
	Mat4x4f m = matrix_set_lookat(eye, at, up);
	return matrix_invert(m);
}


/// <summary>
/// 投影矩阵
/// </summary>
/// <param name="fovy">视域:thera角</param>
/// <param name="zn"></param>
/// <param name="zf"></param>
/// <returns></returns>
inline static Mat4x4f Projection_Matrix(float fovy, float zn, float zf) {
	float scale = 1.0f / (float)tan(fovy * 0.5f);
	Mat4x4f m = matrix_set_zero();
	m.m[0][0] = scale;
	m.m[1][1] = scale;
	m.m[2][2] = -zf / (zf - zn);
	m.m[3][2] = -zf * zn / (zf - zn);
	m.m[2][3] = -1;//set w = -z 
	return m;
}

//---------------------------------------------------------------------
// 着色器定义
//---------------------------------------------------------------------

// 着色器上下文，由 VS 设置，再由渲染器按像素逐点插值后，供 PS 读取
struct ShaderContext {
	std::map<int, float> varying_float;    // 浮点数 varying 列表
	std::map<int, Vec2f> varying_vec2f;    // 二维矢量 varying 列表
	std::map<int, Vec3f> varying_vec3f;    // 三维矢量 varying 列表
	std::map<int, Vec4f> varying_vec4f;    // 四维矢量 varying 列表
};


// 顶点着色器：因为是 C++ 编写，无需传递 attribute，传个 0-2 的顶点序号
// 着色器函数直接在外层根据序号读取相应数据即可，最后需要返回一个坐标 pos
// 各项 varying 设置到 output 里，由渲染器插值后传递给 PS 
typedef std::function<Vec4f(int index, ShaderContext& output)> VertexShader;


// 像素着色器：输入 ShaderContext，需要返回 Vec4f 类型的颜色
// 三角形内每个点的 input 具体值会根据前面三个顶点的 output 插值得到
typedef std::function<Vec4f(ShaderContext& input)> PixelShader;   //函数指针



class RenderHelp
{
public:

	inline virtual ~RenderHelp() { Reset(); }



	//默认填充内部像素，不描线框
	inline RenderHelp() {
		_frame_buffer = NULL;
		_depth_buffer = NULL;
		_render_frame = false;
		_render_pixel = true;
	}

	inline RenderHelp(int width, int height) {
		_frame_buffer = NULL;
		_depth_buffer = NULL;
		_render_frame = false;
		_render_pixel = true;
		Init(width, height);
	}

public:

	// 复位状态
	inline void Reset() {
		_vertex_shader = NULL;
		_pixel_shader = NULL;
		if (_frame_buffer) delete _frame_buffer;
		_frame_buffer = NULL;
		if (_depth_buffer) {
			for (int j = 0; j < _fb_height; j++) {
				if (_depth_buffer[j]) delete[]_depth_buffer[j];  //释放帧缓存和深度缓存
				_depth_buffer[j] = NULL;
			}
			delete[]_depth_buffer;
			_depth_buffer = NULL;
		}
		_color_fg = 0xffffffff;
		_color_bg = 0xff191970;
	}


	// 初始化 FrameBuffer，渲染前需要先调用
	inline void Init(int width, int height) {
		Reset();
		_frame_buffer = new Bitmap(width, height);
		_fb_width = width;   //设置图片的宽度像素
		_fb_height = height;   //高度像素
		_depth_buffer = new float* [height];  //创建深度缓存的动态内存
		for (int j = 0; j < height; j++) {
			_depth_buffer[j] = new float[width];
		}
		Clear();  //全部置零
	}

	// 清空 FrameBuffer 和深度缓存
	inline void Clear() {
		if (_frame_buffer) {
			_frame_buffer->Fill(_color_bg);
		}
		if (_depth_buffer) {
			for (int j = 0; j < _fb_height; j++) {
				for (int i = 0; i < _fb_width; i++)
					_depth_buffer[j][i] = 0.0f;
			}
		}
	}

	// 设置 VS/PS 着色器函数
	inline void SetVertexShader(VertexShader vs) { _vertex_shader = vs; }
	inline void SetPixelShader(PixelShader ps) { _pixel_shader = ps; }

	// 保存 FrameBuffer 到 BMP 文件
	inline void SaveFile(FILE* fp,const char* filename) { if (_frame_buffer) _frame_buffer->SaveFile(fp,filename); }

	// 设置背景/前景色
	inline void SetBGColor(uint32_t color) { _color_bg = color; }
	inline void SetFGColor(uint32_t color) { _color_fg = color; }

	// FrameBuffer 里画点
	inline void SetPixel(int x, int y, uint32_t cc) { if (_frame_buffer) _frame_buffer->SetPixel(x, y, cc); }
	inline void SetPixel(int x, int y, const Vec4f& cc) { SetPixel(x, y, Bitmap::vector_to_color(cc)); }
	inline void SetPixel(int x, int y, const Vec3f& cc) { SetPixel(x, y, Bitmap::vector_to_color(cc)); }


	// FrameBuffer 里画线
	inline void DrawLine(int x1, int y1, int x2, int y2) {
		if (_frame_buffer) _frame_buffer->DrawLine(x1, y1, x2, y2, _color_fg);
	}

	// 设置渲染状态，是否显示线框图，是否填充三角形
	inline void SetRenderState(bool frame, bool pixel) {
		_render_frame = frame;
		_render_pixel = pixel;
	}

	// 判断一条边是不是三角形的左上边 (Top-Left Edge)
	inline bool IsTopLeft(const Vec2i& a, const Vec2i& b) {
		return ((a.y == b.y) && (a.x < b.x)) || (a.y > b.y);
	}


	inline bool edgeFunction(const Vec2f& p, const Vec2i v0, const Vec2i v1)
	{
		return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x) <= 0;
	}


public:
	// 绘制一个三角形，必须先设定好着色器函数
	inline bool DrawPrimitive()
	{
		//如果
		if (_frame_buffer == NULL || _vertex_shader == NULL)  
			return false;

		// 顶点初始化
		for (size_t i = 0; i < 3; ++i)
		{
			Vertex& vertex = _vertex[i];

			// 在对顶点进行外部赋值时需要进行重置   清空上下文 varying 列表
			vertex.context.varying_float.clear();
			vertex.context.varying_vec2f.clear();
			vertex.context.varying_vec3f.clear();
			vertex.context.varying_vec4f.clear();

			//此时处在裁剪空间下  顶点已经经过MVP矩阵变换
			// 运行顶点着色程序，返回顶点坐标
			vertex.pos = _vertex_shader(i, vertex.context);

			// 简单裁剪，任何一个顶点超过 CVV 就剔除
			float w = vertex.pos.w;

			// 这里图简单，当一个点越界，立马放弃整个三角形，更精细的做法是
			// 如果越界了就在齐次空间内进行裁剪，拆分为 0-2 个三角形然后继续
			if (w <= 0.0f) return false;
			if (vertex.pos.y > w||vertex.pos.y<-w) return false;
			if (vertex.pos.x > w || vertex.pos.x < -w) return false;

			vertex.rhw = 1 / vertex.pos.w;   //此时的w是-z  我们将它倒置然后取反，得到的是线性的深度值，避免了ZFighting

			// 齐次坐标空间 /w 归一化到单位体积 cvv     转换到NDC坐标
			vertex.pos *= vertex.rhw;

			// 计算屏幕坐标
			vertex.spf.x = (vertex.pos.x + 1.0f) * _fb_width * 0.5f;
			vertex.spf.y = (1.0f - vertex.pos.y) * _fb_height * 0.5f;

			// 整数屏幕坐标：加 0.5 的偏移取屏幕像素方格中心对齐
			vertex.spi.x = (int)(vertex.spf.x + 0.5f);
			vertex.spi.y = (int)(vertex.spf.y + 0.5f);


			// 更新外接矩形范围（三角形的最大外接矩形）
			if (i == 0) {
				_min_x = _max_x = Between(0, _fb_width - 1, vertex.spi.x);
				_min_y = _max_y = Between(0, _fb_height - 1, vertex.spi.y);
			}
			else {
				_min_x = Between(0, _fb_width - 1, Min(_min_x, vertex.spi.x));
				_max_x = Between(0, _fb_width - 1, Max(_max_x, vertex.spi.x));
				_min_y = Between(0, _fb_height - 1, Min(_min_y, vertex.spi.y));
				_max_y = Between(0, _fb_height - 1, Max(_max_y, vertex.spi.y));
			}
		}
			// 如果不填充像素就退出
			if (_render_pixel == false) return false;


			// 判断三角形朝向
			Vec4f v01 = _vertex[1].pos - _vertex[0].pos;
			Vec4f v02 = _vertex[2].pos - _vertex[0].pos;
			Vec4f normal = vector_cross(v01, v02);

			// 使用 vtx 访问三个顶点，而不直接用 _vertex 访问，因为可能会调整顺序
			Vertex* vtx[3] = { &_vertex[0], &_vertex[1], &_vertex[2] };

			// 如果背向视点，则交换顶点，保证 edge equation 判断的符号为正  
			if (normal.z > 0.0f) {
				vtx[1] = &_vertex[2];
				vtx[2] = &_vertex[1];
			}
			else if (normal.z == 0.0f) {
				return false;
			}

			// 保存三个端点位置
			Vec2i v0 = vtx[0]->spi;
			Vec2i v1 = vtx[1]->spi;
			Vec2i v2 = vtx[2]->spi;

			// 计算面积，为零就退出
			float s = Abs(vector_cross(v1 - v0, v2 - v0));
			if (s <= 0) return false;

			for (size_t cy = _min_y; cy < _max_y; cy++)
			{
				for (size_t cx = _min_x; cx < _max_x; cx++)
				{
					Vec2f p = { (float)cx + 0.5f, (float)cy + 0.5f };
					bool inside=true;
					inside &= edgeFunction(p, v0, v1);
					inside &= edgeFunction(p, v1, v2);
					inside &= edgeFunction(p, v2, v0);

					if (!inside) { continue; }

					// 三个端点到当前点的矢量
					Vec2f s0 = vtx[0]->spf - p;
					Vec2f s1 = vtx[1]->spf - p;
					Vec2f s2 = vtx[2]->spf - p;


					// 重心坐标系：计算内部子三角形面积 a / b / c
					float a = Abs(vector_cross(s1, s2));    // 子三角形 P-v1-v2 面积
					float b = Abs(vector_cross(s2, s0));    // 子三角形 P-v2-v0 面积
					float c = Abs(vector_cross(s0, s1));    // 子三角形 P-v0-v1 面积
					float s = a + b + c;                    // 大三角形 P0-P1-P2 面积

					if (s == 0.0f) continue;

					// 除以总面积，以保证：a + b + c = 1，方便用作插值系数
					a = a * (1.0f / s);
					b = b * (1.0f / s);
					c = c * (1.0f / s);

					// 计算当前点的 1/w，因 1/w 和屏幕空间呈线性关系，故直接重心插值
					float rhw = vtx[0]->rhw * a + vtx[1]->rhw * b + vtx[2]->rhw * c;

					// 进行深度测试
					if (rhw < _depth_buffer[cy][cx]) continue;
					_depth_buffer[cy][cx] = rhw;   // 记录 1/w 到深度缓存

					// 还原当前像素的 w
					float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

					// 计算三个顶点插值 varying 的系数
					// 先除以各自顶点的 w 然后进行屏幕空间插值然后再乘以当前 w
					float c0 = vtx[0]->rhw * a * w;
					float c1 = vtx[1]->rhw * b * w;
					float c2 = vtx[2]->rhw * c * w;


					// 准备为当前像素的各项 varying 进行插值  ，这个插值完的像素点数据需要传给片元着色器，进行像素着色处理
					ShaderContext input;

					//拿到每个顶点的待处理数据，坐标  比如：法线，uv坐标等等
					ShaderContext& i0 = vtx[0]->context;
					ShaderContext& i1 = vtx[1]->context;
					ShaderContext& i2 = vtx[2]->context;

					// 插值各项 varying
					for (auto const& it : i0.varying_float) {
						int key = it.first;
						float f0 = i0.varying_float[key];
						float f1 = i1.varying_float[key];
						float f2 = i2.varying_float[key];
						input.varying_float[key] = c0 * f0 + c1 * f1 + c2 * f2;
					}

					for (auto const& it : i0.varying_vec2f) {
						int key = it.first;
						const Vec2f& f0 = i0.varying_vec2f[key];
						const Vec2f& f1 = i1.varying_vec2f[key];
						const Vec2f& f2 = i2.varying_vec2f[key];
						input.varying_vec2f[key] = c0 * f0 + c1 * f1 + c2 * f2;
					}

					for (auto const& it : i0.varying_vec3f) {
						int key = it.first;
						const Vec3f& f0 = i0.varying_vec3f[key];
						const Vec3f& f1 = i1.varying_vec3f[key];
						const Vec3f& f2 = i2.varying_vec3f[key];
						input.varying_vec3f[key] = c0 * f0 + c1 * f1 + c2 * f2;
					}

					for (auto const& it : i0.varying_vec4f) {
						int key = it.first;
						const Vec4f& f0 = i0.varying_vec4f[key];
						const Vec4f& f1 = i1.varying_vec4f[key];
						const Vec4f& f2 = i2.varying_vec4f[key];
						input.varying_vec4f[key] = c0 * f0 + c1 * f1 + c2 * f2;
					}

					// 执行像素着色器
					Vec4f color = { 0.0f, 0.0f, 0.0f, 0.0f };

					if (_pixel_shader != NULL) {
						color = _pixel_shader(input);
					}

				// 绘制到 framebuffer 上，这里可以加判断，如果 PS 返回的颜色 alpha 分量
				// 小于等于零则放弃绘制，不过这样的话要把前面的更新深度缓存的代码挪下来，
				// 只有需要渲染的时候才更新深度。
					_frame_buffer->SetPixel(cx, cy, color);

					// 绘制线框，再画一次避免覆盖
					if (_render_frame) {
						DrawLine(_vertex[0].spi.x, _vertex[0].spi.y, _vertex[1].spi.x, _vertex[1].spi.y);
						DrawLine(_vertex[0].spi.x, _vertex[0].spi.y, _vertex[2].spi.x, _vertex[2].spi.y);
						DrawLine(_vertex[2].spi.x, _vertex[2].spi.y, _vertex[1].spi.x, _vertex[1].spi.y);
					}
				}
			}
			return true;





		}



	
	




protected:

	// 顶点结构体
	struct Vertex {
		ShaderContext context;    // 上下文
		float rhw;                // w 的倒数
		Vec4f pos;                // 坐标
		Vec2f spf;                // 浮点数屏幕坐标
		Vec2i spi;                // 整数屏幕坐标
	};

protected:
	Bitmap* _frame_buffer;    // 像素缓存
	float** _depth_buffer;    // 深度缓存   双指针=》二维数组  保存的是深度信息Z

	int _fb_width;            // frame buffer 宽度
	int _fb_height;           // frame buffer 高度

	uint32_t _color_fg;       // 前景色：画线时候用
	uint32_t _color_bg;       // 背景色：Clear 时候用

	Vertex _vertex[3];        // 三角形的三个顶点

	int _min_x;               // 三角形外接矩形
	int _min_y;
	int _max_x;
	int _max_y;

	bool _render_frame;       // 是否绘制线框
	bool _render_pixel;       // 是否填充像素

	VertexShader _vertex_shader;   //顶点数据处理函数的接口函数 
	PixelShader _pixel_shader;     //像素处理函数的接口函数


};

#endif // !_RENDER_HELP_

