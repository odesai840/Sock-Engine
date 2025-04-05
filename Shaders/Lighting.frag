#version 460 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

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

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform DirLight dirLight;
//uniform PointLight pointLight;
//uniform SpotLight spotLight;
uniform Material material;
uniform bool debugNormals;
uniform bool debugSpec;
uniform sampler2D shadowMap;
uniform float shadowBias;

// calculates shadow factor
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get current depth value
    float currentDepth = projCoords.z;
    // check if fragment is in shadow
    float shadow = 0.0;
    // use bias to prevent shadow acne
    float bias = max(shadowBias * (1.0 - dot(normalize(TBN[2]), normalize(-dirLight.direction))), shadowBias * 0.5);

    // PCF (Percentage Closer Filtering) for softer shadows
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep shadow at 0.0 when outside the light's far plane
    if(projCoords.z > 1.0) {
        shadow = 0.0;
    }

    return shadow;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec4 diffuseMap, vec4 specularMap, vec3 normal, vec3 viewDir, out float specOut)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(diffuseMap);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseMap);
    vec3 specular = light.specular * spec * vec3(specularMap);
    specOut += spec;

    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);

    // apply shadow to diffuse and specular components only
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec4 diffuseMap, vec4 specularMap, vec3 normal, vec3 viewDir, out float specOut)
{
    vec3 lightDir = normalize(light.position - FragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(diffuseMap);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseMap);
    vec3 specular = light.specular * spec * vec3(specularMap);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    specOut += spec;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec4 diffuseMap, vec4 specularMap, vec3 normal, vec3 viewDir, out float specOut)
{
    vec3 lightDir = normalize(light.position - FragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(diffuseMap);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseMap);
    vec3 specular = light.specular * spec * vec3(specularMap);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    specOut += spec;
    return (ambient + diffuse + specular);
}

void main()
{
    vec4 diffuseMap = texture(material.texture_diffuse1, TexCoords);

    // discard fragments with low alpha for transparency
    if (diffuseMap.a < 0.1){
        discard;
    }

    vec4 specularMap = texture(material.texture_specular1, TexCoords);

    // properties
    vec3 normal = texture(material.texture_normal1, TexCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    float specOut = 0.0;

    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, diffuseMap, specularMap, normal, viewDir, specOut);
    // phase 2: point light
    //result += CalcPointLight(pointLight, diffuseMap, specularMap, normal, viewDir, specOut);
    // phase 3: spot light
    //result += CalcSpotLight(spotLight, diffuseMap, specularMap, normal, viewDir, specOut);

    if(debugNormals) {
        FragColor = vec4(normal, 1.0);
    }
    else if(debugSpec) {
        FragColor = vec4(vec3(specOut), 1.0);
    }
    else {
        // output the color with calculated transparency
        FragColor = vec4(result, diffuseMap.a);
    }
}