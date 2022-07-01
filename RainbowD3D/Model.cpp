#include <iostream>
#include "Vector.h"
#include "RenderHelp.h"
#include "Model.h"



int main()
{
	RenderHelp rh(600, 800);

	// ����ģ��
	Model model("Model/box_stack.obj");

	Vec3f eye_pos = { 0, 2, -10 };
	Vec3f eye_at = { 0, 0, 1 };
	Vec3f eye_up = { 0, -1, 0 };
	//Vec3f light_dir = { 1, 1, 0.85 };			// ��ķ���
	float perspective = 3.1415926f * 0.5f;

	Mat4x4f mat_model = matrix_set_scale(1, 1, 1);
	Mat4x4f mat_view = World2Camera_Matrix(eye_pos, eye_at, eye_up);
	Mat4x4f mat_proj = Projection_Matrix(3.1415f / 2, 1.0, 500.0f);
	Mat4x4f mat_mvp = mat_model * mat_view * mat_proj;

	// ���ڽ���������ģ������ϵ�任����������ϵ
	Mat4x4f mat_model_it = matrix_invert(mat_model).Transpose();

	// ��������
	struct { Vec3f pos; Vec3f normal; Vec2f uv; } vs_input[3];

	const int VARYING_UV = 0;
	const int VARYING_NORMAL = 1;
	const int VARYING_COLOR = 0;

	rh.SetVertexShader([&](int index, ShaderContext& output) -> Vec4f {
		Vec4f pos = vs_input[index].pos.xyz1() * mat_mvp;
		Vec4f normal = (vs_input[index].normal.xyz1() * mat_model_it);
		output.varying_vec2f[VARYING_UV] = vs_input[index].uv;
		output.varying_vec3f[VARYING_NORMAL] = normal.xyz(); // ת��Ϊ��ά
		output.varying_vec4f[VARYING_COLOR] = { 1, 0, 0, 1 };
		return pos;
		});

	rh.SetPixelShader([&](ShaderContext& input) -> Vec4f {
		return input.varying_vec4f[VARYING_COLOR];
		});

	// ����ģ��ÿһ����
	for (int i = 0; i < model.nfaces(); i++) {
		// ����������������룬�� VS ��ȡ
		for (int j = 0; j < 3; j++) {
			vs_input[j].pos = model.vert(i, j);
			vs_input[j].uv = model.uv(i, j);
			vs_input[j].normal = model.normal(i, j);
		}
		// ����������
		rh.DrawPrimitive();
	}
	FILE* fp = nullptr;
	rh.SaveFile(fp,"output.bmp");

#if defined(WIN32) || defined(_WIN32)
	system("mspaint output.bmp");
#endif

	return 0;
}
