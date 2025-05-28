#version 330 core
out vec4 FragColor; 

in vec3 color; 
in vec2 uv;
in mat3 tbn;
in vec3 worldPosition; // Position in world space

uniform sampler2D mainTex;
uniform sampler2D normalTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform vec3 objectColor;
void main()
{
    // normal map
    vec3 normal = texture(normalTex, uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal.rg = normal.rg * .1;
    normal = normalize(normal);
    normal = tbn * normal;

    vec3 lightDirection = normalize(worldPosition - lightPosition);

    // specular data
    vec3 viewDir = normalize(worldPosition - cameraPosition);
    vec3 reflDir = normalize(reflect(lightDirection, normal));

    // lighting
    float lightIntensity = 2.0;
    float specularIntensity = 2.0;
    float lightvalue = max(dot(normal, lightDirection), 0.0) * lightIntensity;
    float specular = pow(max(-dot(reflDir, viewDir), 0.0), 128) * specularIntensity;

    // separate RGB and RGBA calculations
    vec4 output = vec4(objectColor, 1.0f) * texture(mainTex, uv);
    output.rgb = output.rgb * min(lightvalue + 0.1, 1.0) + (specular * output.rgb);

    // Boost overall brightness
    output.rgb *= 1.9;
    // ...
    
    FragColor = output;
}