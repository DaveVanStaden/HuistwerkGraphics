#version 330 core
out vec4 FragColor; 

in vec2 uv;
in vec3 worldPosition; // Position in world space

uniform sampler2D mainTex;
uniform sampler2D normalTex;

uniform sampler2D dirt, sand, grass, rock, snow;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t){
    return a + (b - a) * t;
}
void main()
{
    // normal map
    vec3 normal = texture(normalTex, uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal.gb = normal.bg;
    normal.r = -normal.r;
    normal.b = -normal.b;
    //vec3 lightDirection = normalize(worldPosition - lightPosition);

    // specular data
    //vec3 viewDir = normalize(worldPosition - cameraPosition);
    //vec3 reflDir = normalize(reflect(lightDirection, normal));

    // lighting
    float lightIntensity = 1.0;
    float specularIntensity = 1.0;
    float lightvalue = max(-dot(normal, lightDirection), 0.0) * lightIntensity;
    //float specular = pow(max(-dot(reflDir, viewDir), 0.0), 128) * specularIntensity;
    float dist = length(worldPosition.xyz - cameraPosition);

    //build color!
    float h = worldPosition.y;

// Define height thresholds for each layer
   float dirtLevel  = 10.0;
   float sandLevel  = 15.0;
   float grassLevel = 25.0;
   float rockLevel  = 35.0;
   float snowLevel  = 40.0;

// Compute blend factors using smoothstep for soft transitions
float dirtAmount  = 1.0 - smoothstep(dirtLevel, sandLevel, h);
float sandAmount  = smoothstep(dirtLevel, sandLevel, h) - smoothstep(sandLevel, grassLevel, h);
float grassAmount = smoothstep(sandLevel, grassLevel, h) - smoothstep(grassLevel, rockLevel, h);
float rockAmount  = smoothstep(grassLevel, rockLevel, h) - smoothstep(rockLevel, snowLevel, h);
float snowAmount  = smoothstep(rockLevel, snowLevel, h);

// Sample textures
vec3 dirtColor  = texture(dirt, uv * 10).rgb;
vec3 sandColor  = texture(sand, uv * 10).rgb;
vec3 grassColor = texture(grass, uv * 10).rgb;
vec3 rockColor  = texture(rock, uv * 10).rgb;
vec3 snowColor  = texture(snow, uv * 10).rgb;

float fog = pow(clamp((dist - 250) / 1000, 0, 1),2);

// Blend all layers
float total = dirtAmount + sandAmount + grassAmount + rockAmount + snowAmount;
if (total > 0.0) {
    dirtAmount  /= total;
    sandAmount  /= total;
    grassAmount /= total;
    rockAmount  /= total;
    snowAmount  /= total;
}
vec3 diffuse = 
    dirtColor  * dirtAmount +
    sandColor  * sandAmount +
    grassColor * grassAmount +
    rockColor  * rockAmount +
    snowColor  * snowAmount;

// Lighting as before
vec4 output = vec4(lerp(diffuse * min(lightvalue + 0.1, 1.0), vec3(1,1,1), fog), 1.0);
FragColor = output;
}