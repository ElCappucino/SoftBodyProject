#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDir;

void main()
{
    // normalize normal
    vec3 norm = normalize(Normal);

    // diffuse lighting
    float diff = max(dot(norm, -lightDir), 0.0);

    // ambient strength
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse color
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}