#version 330 core
out vec4 FragColor;

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_SPOT_LIGHTS 2

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    // Phase 1: Ambient
    // vec3(0.05, 0.05, 0.1) to give a bluish tint to simulate nighttime. R = 0.05, G = 0.05, B = 0.1
    vec3 ambient = vec3(0.05, 0.05, 0.1) * texture(texture_diffuse1, TexCoords).rgb;
    result += ambient;

    // Phase 2: Directional light
    vec3 lightDir = normalize(vec3(-1.0f, -0.1f, -1.0f));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 moonlight = vec3(0.6f, 0.6f, 0.7f) * diff * texture(texture_diffuse1, TexCoords).rgb;
    result += moonlight;

    // Phase 3: Spot lights
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, TexCoords);
    }

    FragColor = vec4(result, 1.0);
    // DEBUG 
    // FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb, 1.0);
    // FragColor = vec4(cos(radians(12.5)), cos(radians(17.5)), 0.0, 1.0);
    // float theta = dot(lightDir, normalize(-spotLights[0].direction));
    // FragColor = vec4(theta);
    // float d = length(spotLights[0].position - FragPos);
    // FragColor = vec4(vec3(d / 10.0), 1.0);
    // FragColor = vec4(spotLights[0].linear, spotLights[0].quadratic, 0.0, 1.0);
    // float debug = spotLights[0].constant + spotLights[0].linear + spotLights[0].quadratic * 0.0;
    // FragColor = vec4(spotLights[0].linear, spotLights[0].quadratic, 0.0, 1.0);
    // FragColor = vec4(attenuation, 1.0);
}

// Calculate the color when using a spot light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords) {
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);  // 32.0 = shininess. hard coded for now

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // DEBUG
    // float attenuation = 0.0;
    // float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    // float attenuation = 1.0 / (0.0 + 0 * distance + 0 * (distance * distance));
    // float attenuation = 1.0 / (attenuation.x + attenuation.y * distance + attenuation.z * (distance * distance));

    // Spot light intesity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // DEBUG
    // float theta = dot(lightDir, normalize(-light.direction));
    // float cutOff = cos(radians(12.5));
    // float outerCutOff = cos(radians(17.5)); 
    // float epsilon = cutOff - outerCutOff;
    // float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    // Combine results
    vec3 ambient = light.ambient * texture(texture_diffuse1, texCoords).rgb;
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, texCoords).rgb;
    vec3 specular = light.specular * spec * texture(texture_specular1, texCoords).rgb;

    // ambient *= intensity;
    // diffuse *= intensity;
    // specular *= intensity;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
    // DEBUG
    // return vec3(intensity);
}