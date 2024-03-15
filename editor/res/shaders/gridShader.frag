#version 430 core
layout (location = 0) out vec4 fragColor;
uniform vec2 u_resolution;

void stroke(float dist, vec4 color, inout vec4 fragColor, float thickness, float aa)
{
    float alpha = smoothstep(0.5 * (thickness + aa), 0.5 * (thickness - aa), abs(dist));
    fragColor = mix(fragColor, color, alpha);
}

void renderGrid(vec2 pos, out vec4 fragColor) {
    vec3 background = vec3(0.23);
    vec3 axes = vec3(0.4);
    vec3 lines = vec3(0.7);
    vec4 sublines = vec4(0.42, 0.42, 0.42, 1);
    vec4 sublines1 = vec4(1, 1, 1, 0.5);

    float subdiv = 8.0; // default 8.0
    float subdiv1 = 64.0; // default 8.0
    float thickness = 0.001; // default 0.003

    fragColor.rgb = background;
    fragColor.a = 0.0;

    float aa = length(fwidth(pos));
    vec2 toSubGrid = pos - round(pos*subdiv)/subdiv;
    stroke(min(abs(toSubGrid.x), abs(toSubGrid.y)), sublines, fragColor, thickness, aa);
    vec2 toSubGrid1 = pos - round(pos * subdiv1) / subdiv1;
    stroke(min(abs(toSubGrid1.x), abs(toSubGrid1.y)), sublines1, fragColor, 0.001, aa);

    vec2 toGrid = pos - round(pos);

    // stroke(min(abs(toGrid.x), abs(toGrid.y)), lines, fragColor, thickness, aa);
    // stroke(min(abs(pos.x), abs(pos.y)), axes, fragColor, thickness, aa);
}

void main() {
    float aspect = u_resolution.x / u_resolution.y;
    vec2 pos = ((gl_FragCoord.xy / u_resolution.xy) - 0.5) * vec2(aspect, 1.0) * 1.2;
    renderGrid(pos, fragColor);
}