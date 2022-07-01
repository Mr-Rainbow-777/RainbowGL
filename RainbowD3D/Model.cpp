#include <iostream>
#include "Vector.h"
#include "RenderHelp.h"
#include "Model.h"



int main()
{
	RenderHelp rh(600, 800);

	// 加载模型
	Model model("Model/box_stack.obj");

	Vec3f eye_pos = { 0, 2, -10 };
	Vec3f eye_at = { 0, 0, 1 };
	Vec3f eye_up = { 0, -1, 0 };
	//Vec3f light_dir = { 1, 1, 0.85 };			// 光的方向
	float perspective = 3.1415926f * 0.5f;

	Mat4x4f mat_model = matrix_set_scale(1, 1, 1);
	Mat4x4f mat_view = World2Camera_Matrix(eye_pos, eye_at, eye_up);
	Mat4x4f mat_proj = Projection_Matrix(3.1415f / 2, 1.0, 500.0f);
	Mat4x4f mat_mvp = mat_model * mat_view * mat_proj;

	// 用于将法向量从模型坐标系变换到世界坐标系
	Mat4x4f mat_model_it = matrix_invert(mat_model).Transpose();

	// 顶点属性
	struct { Vec3f pos; Vec3f normal; Vec2f uv; } vs_input[3];

	const int VARYING_UV = 0;
	const int VARYING_NORMAL = 1;
	const int VARYING_COLOR = 0;

	rh.SetVertexShader([&](int index, ShaderContext& output) -> Vec4f {
		Vec4f pos = vs_input[index].pos.xyz1() * mat_mvp;
		Vec4f normal = (vs_input[index].normal.xyz1() * mat_model_it);
		output.varying_vec2f[VARYING_UV] = vs_input[index].uv;
		output.varying_vec3f[VARYING_NORMAL] = normal.xyz(); // 转化为三维
		output.varying_vec4f[VARYING_COLOR] = { 1, 0, 0, 1 };
		return pos;
		});

	rh.SetPixelShader([&](ShaderContext& input) -> Vec4f {
		return input.varying_vec4f[VARYING_COLOR];
		});

	// 迭代模型每一个面
	for (int i = 0; i < model.nfaces(); i++) {
		// 设置三个顶点的输入，供 VS 读取
		for (int j = 0; j < 3; j++) {
			vs_input[j].pos = model.vert(i, j);
			vs_input[j].uv = model.uv(i, j);
			vs_input[j].normal = model.normal(i, j);
		}
		// 绘制三角形
		rh.DrawPrimitive();
	}
	FILE* fp = nullptr;
	rh.SaveFile(fp,"output.bmp");

#if defined(WIN32) || defined(_WIN32)
	system("mspaint output.bmp");
#endif

	return 0;
}
