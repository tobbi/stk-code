#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};
#endif

in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;

#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;
#endif

out vec3 nor;
out vec2 uv;
out vec4 color;

mat4 getWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);
mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation, Scale) * InverseViewMatrix);
    gl_Position = ProjectionMatrix * ViewMatrix *  ModelMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
    color = Color.zyxw;
}
