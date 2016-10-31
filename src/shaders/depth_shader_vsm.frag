#version 330 core

in vec4 pos;
out vec4 color;

void main()
{
	float depth = pos.z;  // pos.w;

	float mom1 = depth;
	float mom2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);

	mom2 += 0.25 * (dx * dx + dy * dy);

	color = vec4(mom1, mom2, 0, 1.0);
}