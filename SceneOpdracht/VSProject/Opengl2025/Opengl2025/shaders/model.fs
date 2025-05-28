#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

uniform vec3 cameraPosition;

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void main()
{
    vec3 lightDir = normalize(vec3(-1, -0.5, -1));
    
    vec4 diffuse = texture(texture_diffuse1, TexCoords);
    vec4 specTex = texture(texture_specular1, TexCoords);
    float ambientOcclusion = texture(texture_ao1, TexCoords).r;
    float roughness = texture(texture_roughness1, TexCoords).r;
    // Lighting calculation
    vec3 normalSample = texture(texture_normal1, TexCoords).rgb;
    vec3 normal = normalize(normalSample * 2.0 - 1.0);

    float light = max(dot(-lightDir, normal), 0.0);

    vec3 viewDir = normalize(cameraPosition - FragPos.xyz);
    vec3 refl = reflect(lightDir, normal);


    float spec = pow(max(dot(viewDir, refl), 0.0), lerp(1, 128, roughness));
    vec3 specular = spec * specTex.rgb;

    vec3 color = diffuse.rgb * max(light * ambientOcclusion, 0.5) + specular;
    FragColor = vec4(color, diffuse.a);
}