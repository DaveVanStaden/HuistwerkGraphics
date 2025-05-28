#version 330 core
out vec4 FragColor; 

in vec3 worldPosition; // Position in world space

uniform vec3 lightDirection;
uniform vec3 cameraPosition;
in vec3 vDirection;
vec3 lerp(vec3 a, vec3 b, float t){
        return a + (b - a) * t;
    }

void main()
{
    vec3 topColor = vec3(68.0f/255.0f, 118.0f/255.0f, 189.0f/255.0f);
    vec3 botColor = vec3(188.0f/255.0f, 214.0f/255.0f, 189.0f/231.0f);
    // specular data
    vec3 sunColor = vec3(1.0, 200/255.0, 50/255.0);
    
    //calculate view
    vec3 viewDir = normalize(vDirection);

    float sun = max(pow(dot(-viewDir, lightDirection), 32.0), 0.0); // Lower exponent for a bigger sun
    vec3 skyColor = lerp(botColor, topColor, abs(viewDir.y));
    skyColor += sun * sunColor * 2.0; // Boost sun intensity

    FragColor = vec4(skyColor, 1.0f);
}