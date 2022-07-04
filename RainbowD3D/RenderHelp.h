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

// ƽ�Ʊ任
inline static Mat4x4f matrix_set_translate(float x, float y, float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[3][0] = x;
	m.m[3][1] = y;
	m.m[3][2] = z;
	return m;
}

// ���ű任
inline static Mat4x4f matrix_set_scale(float x, float y, float z) {
	Mat4x4f m = matrix_set_identity();
	m.m[0][0] = x;
	m.m[1][1] = y;
	m.m[2][2] = z;
	return m;
}

// Local2World����
inline static Mat4x4f Model2World_Matrix(const Vec3f& pos,const Vec3f& xAxis, const Vec3f& yAxis, const Vec3f& zAxis) {
	Mat4x4f m;
	m.SetRow(0, Vec4f(xAxis.x, xAxis.y, xAxis.z, 0));
	m.SetRow(1, Vec4f(yAxis.x, yAxis.y, yAxis.z, 0));
	m.SetRow(2, Vec4f(zAxis.x, zAxis.y, zAxis.z, 0));
	m.SetRow(3, Vec4f(pos.x, pos.y, pos.z, 1.0f));
	return m;
}

// ��Ӱ���任����eye/�ӵ�λ�ã�at/�������up/ָ���Ϸ���ʸ��  
//Camera2World����
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


//View����  (�ҳ�)
inline static Mat4x4f World2Camera_Matrix(const Vec3f& eye, const Vec3f& at, const Vec3f& up)
{
	Mat4x4f m = matrix_set_lookat(eye, at, up);
	return matrix_invert(m);
}


/// <summary>
/// ͶӰ����
/// </summary>
/// <param name="fovy">����:thera��</param>
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
// ��ɫ������
//---------------------------------------------------------------------

// ��ɫ�������ģ��� VS ���ã�������Ⱦ������������ֵ�󣬹� PS ��ȡ
struct ShaderContext {
	std::map<int, float> varying_float;    // ������ varying �б�
	std::map<int, Vec2f> varying_vec2f;    // ��άʸ�� varying �б�
	std::map<int, Vec3f> varying_vec3f;    // ��άʸ�� varying �б�
	std::map<int, Vec4f> varying_vec4f;    // ��άʸ�� varying �б�
};


// ������ɫ������Ϊ�� C++ ��д�����贫�� attribute������ 0-2 �Ķ������
// ��ɫ������ֱ������������Ŷ�ȡ��Ӧ���ݼ��ɣ������Ҫ����һ������ pos
// ���� varying ���õ� output �����Ⱦ����ֵ�󴫵ݸ� PS 
typedef std::function<Vec4f(int index, ShaderContext& output)> VertexShader;


// ������ɫ�������� ShaderContext����Ҫ���� Vec4f ���͵���ɫ
// ��������ÿ����� input ����ֵ�����ǰ����������� output ��ֵ�õ�
typedef std::function<Vec4f(ShaderContext& input)> PixelShader;   //����ָ��



class RenderHelp
{
public:

	inline virtual ~RenderHelp() { Reset(); }



	//Ĭ������ڲ����أ������߿�
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

	// ��λ״̬
	inline void Reset() {
		_vertex_shader = NULL;
		_pixel_shader = NULL;
		if (_frame_buffer) delete _frame_buffer;
		_frame_buffer = NULL;
		if (_depth_buffer) {
			for (int j = 0; j < _fb_height; j++) {
				if (_depth_buffer[j]) delete[]_depth_buffer[j];  //�ͷ�֡�������Ȼ���
				_depth_buffer[j] = NULL;
			}
			delete[]_depth_buffer;
			_depth_buffer = NULL;
		}
		_color_fg = 0xffffffff;
		_color_bg = 0xff191970;
	}


	// ��ʼ�� FrameBuffer����Ⱦǰ��Ҫ�ȵ���
	inline void Init(int width, int height) {
		Reset();
		_frame_buffer = new Bitmap(width, height);
		_fb_width = width;   //����ͼƬ�Ŀ������
		_fb_height = height;   //�߶�����
		_depth_buffer = new float* [height];  //������Ȼ���Ķ�̬�ڴ�
		for (int j = 0; j < height; j++) {
			_depth_buffer[j] = new float[width];
		}
		Clear();  //ȫ������
	}

	// ��� FrameBuffer ����Ȼ���
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

	// ���� VS/PS ��ɫ������
	inline void SetVertexShader(VertexShader vs) { _vertex_shader = vs; }
	inline void SetPixelShader(PixelShader ps) { _pixel_shader = ps; }

	// ���� FrameBuffer �� BMP �ļ�
	inline void SaveFile(FILE* fp,const char* filename) { if (_frame_buffer) _frame_buffer->SaveFile(fp,filename); }

	// ���ñ���/ǰ��ɫ
	inline void SetBGColor(uint32_t color) { _color_bg = color; }
	inline void SetFGColor(uint32_t color) { _color_fg = color; }

	// FrameBuffer �ﻭ��
	inline void SetPixel(int x, int y, uint32_t cc) { if (_frame_buffer) _frame_buffer->SetPixel(x, y, cc); }
	inline void SetPixel(int x, int y, const Vec4f& cc) { SetPixel(x, y, Bitmap::vector_to_color(cc)); }
	inline void SetPixel(int x, int y, const Vec3f& cc) { SetPixel(x, y, Bitmap::vector_to_color(cc)); }


	// FrameBuffer �ﻭ��
	inline void DrawLine(int x1, int y1, int x2, int y2) {
		if (_frame_buffer) _frame_buffer->DrawLine(x1, y1, x2, y2, _color_fg);
	}

	// ������Ⱦ״̬���Ƿ���ʾ�߿�ͼ���Ƿ����������
	inline void SetRenderState(bool frame, bool pixel) {
		_render_frame = frame;
		_render_pixel = pixel;
	}

	// �ж�һ�����ǲ��������ε����ϱ� (Top-Left Edge)
	inline bool IsTopLeft(const Vec2i& a, const Vec2i& b) {
		return ((a.y == b.y) && (a.x < b.x)) || (a.y > b.y);
	}


	inline bool edgeFunction(const Vec2f& p, const Vec2i v0, const Vec2i v1)
	{
		return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x) <= 0;
	}


public:
	// ����һ�������Σ��������趨����ɫ������
	inline bool DrawPrimitive()
	{
		//���
		if (_frame_buffer == NULL || _vertex_shader == NULL)  
			return false;

		// �����ʼ��
		for (size_t i = 0; i < 3; ++i)
		{
			Vertex& vertex = _vertex[i];

			// �ڶԶ�������ⲿ��ֵʱ��Ҫ��������   ��������� varying �б�
			vertex.context.varying_float.clear();
			vertex.context.varying_vec2f.clear();
			vertex.context.varying_vec3f.clear();
			vertex.context.varying_vec4f.clear();

			//��ʱ���ڲü��ռ���  �����Ѿ�����MVP����任
			// ���ж�����ɫ���򣬷��ض�������
			vertex.pos = _vertex_shader(i, vertex.context);

			// �򵥲ü����κ�һ�����㳬�� CVV ���޳�
			float w = vertex.pos.w;

			// ����ͼ�򵥣���һ����Խ�磬����������������Σ�����ϸ��������
			// ���Խ���˾�����οռ��ڽ��вü������Ϊ 0-2 ��������Ȼ�����
			if (w <= 0.0f) return false;
			if (vertex.pos.y > w||vertex.pos.y<-w) return false;
			if (vertex.pos.x > w || vertex.pos.x < -w) return false;

			vertex.rhw = 1 / vertex.pos.w;   //��ʱ��w��-z  ���ǽ�������Ȼ��ȡ�����õ��������Ե����ֵ��������ZFighting

			// �������ռ� /w ��һ������λ��� cvv     ת����NDC����
			vertex.pos *= vertex.rhw;

			// ������Ļ����
			vertex.spf.x = (vertex.pos.x + 1.0f) * _fb_width * 0.5f;
			vertex.spf.y = (1.0f - vertex.pos.y) * _fb_height * 0.5f;

			// ������Ļ���꣺�� 0.5 ��ƫ��ȡ��Ļ���ط������Ķ���
			vertex.spi.x = (int)(vertex.spf.x + 0.5f);
			vertex.spi.y = (int)(vertex.spf.y + 0.5f);


			// ������Ӿ��η�Χ�������ε������Ӿ��Σ�
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
			// �����������ؾ��˳�
			if (_render_pixel == false) return false;


			// �ж������γ���
			Vec4f v01 = _vertex[1].pos - _vertex[0].pos;
			Vec4f v02 = _vertex[2].pos - _vertex[0].pos;
			Vec4f normal = vector_cross(v01, v02);

			// ʹ�� vtx �����������㣬����ֱ���� _vertex ���ʣ���Ϊ���ܻ����˳��
			Vertex* vtx[3] = { &_vertex[0], &_vertex[1], &_vertex[2] };

			// ��������ӵ㣬�򽻻����㣬��֤ edge equation �жϵķ���Ϊ��  
			if (normal.z > 0.0f) {
				vtx[1] = &_vertex[2];
				vtx[2] = &_vertex[1];
			}
			else if (normal.z == 0.0f) {
				return false;
			}

			// ���������˵�λ��
			Vec2i v0 = vtx[0]->spi;
			Vec2i v1 = vtx[1]->spi;
			Vec2i v2 = vtx[2]->spi;

			// ���������Ϊ����˳�
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

					// �����˵㵽��ǰ���ʸ��
					Vec2f s0 = vtx[0]->spf - p;
					Vec2f s1 = vtx[1]->spf - p;
					Vec2f s2 = vtx[2]->spf - p;


					// ��������ϵ�������ڲ������������ a / b / c
					float a = Abs(vector_cross(s1, s2));    // �������� P-v1-v2 ���
					float b = Abs(vector_cross(s2, s0));    // �������� P-v2-v0 ���
					float c = Abs(vector_cross(s0, s1));    // �������� P-v0-v1 ���
					float s = a + b + c;                    // �������� P0-P1-P2 ���

					if (s == 0.0f) continue;

					// ������������Ա�֤��a + b + c = 1������������ֵϵ��
					a = a * (1.0f / s);
					b = b * (1.0f / s);
					c = c * (1.0f / s);

					// ���㵱ǰ��� 1/w���� 1/w ����Ļ�ռ�����Թ�ϵ����ֱ�����Ĳ�ֵ
					float rhw = vtx[0]->rhw * a + vtx[1]->rhw * b + vtx[2]->rhw * c;

					// ������Ȳ���
					if (rhw < _depth_buffer[cy][cx]) continue;
					_depth_buffer[cy][cx] = rhw;   // ��¼ 1/w ����Ȼ���

					// ��ԭ��ǰ���ص� w
					float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

					// �������������ֵ varying ��ϵ��
					// �ȳ��Ը��Զ���� w Ȼ�������Ļ�ռ��ֵȻ���ٳ��Ե�ǰ w
					float c0 = vtx[0]->rhw * a * w;
					float c1 = vtx[1]->rhw * b * w;
					float c2 = vtx[2]->rhw * c * w;


					// ׼��Ϊ��ǰ���صĸ��� varying ���в�ֵ  �������ֵ������ص�������Ҫ����ƬԪ��ɫ��������������ɫ����
					ShaderContext input;

					//�õ�ÿ������Ĵ��������ݣ�����  ���磺���ߣ�uv����ȵ�
					ShaderContext& i0 = vtx[0]->context;
					ShaderContext& i1 = vtx[1]->context;
					ShaderContext& i2 = vtx[2]->context;

					// ��ֵ���� varying
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

					// ִ��������ɫ��
					Vec4f color = { 0.0f, 0.0f, 0.0f, 0.0f };

					if (_pixel_shader != NULL) {
						color = _pixel_shader(input);
					}

				// ���Ƶ� framebuffer �ϣ�������Լ��жϣ���� PS ���ص���ɫ alpha ����
				// С�ڵ�������������ƣ����������Ļ�Ҫ��ǰ��ĸ�����Ȼ���Ĵ���Ų������
				// ֻ����Ҫ��Ⱦ��ʱ��Ÿ�����ȡ�
					_frame_buffer->SetPixel(cx, cy, color);

					// �����߿��ٻ�һ�α��⸲��
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

	// ����ṹ��
	struct Vertex {
		ShaderContext context;    // ������
		float rhw;                // w �ĵ���
		Vec4f pos;                // ����
		Vec2f spf;                // ��������Ļ����
		Vec2i spi;                // ������Ļ����
	};

protected:
	Bitmap* _frame_buffer;    // ���ػ���
	float** _depth_buffer;    // ��Ȼ���   ˫ָ��=����ά����  ������������ϢZ

	int _fb_width;            // frame buffer ���
	int _fb_height;           // frame buffer �߶�

	uint32_t _color_fg;       // ǰ��ɫ������ʱ����
	uint32_t _color_bg;       // ����ɫ��Clear ʱ����

	Vertex _vertex[3];        // �����ε���������

	int _min_x;               // ��������Ӿ���
	int _min_y;
	int _max_x;
	int _max_y;

	bool _render_frame;       // �Ƿ�����߿�
	bool _render_pixel;       // �Ƿ��������

	VertexShader _vertex_shader;   //�������ݴ������Ľӿں��� 
	PixelShader _pixel_shader;     //���ش������Ľӿں���


};

#endif // !_RENDER_HELP_

