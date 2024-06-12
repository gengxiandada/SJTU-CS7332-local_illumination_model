#version 330 core

out vec4 FragColor;// 片元颜色

in vec3 FragPos;// 片元位置
in vec3 Normal;// 片元法向量

uniform vec3 lightPos[LIGHT_NUM];// 光源的位置
uniform vec3 lightColor[LIGHT_NUM];// 光源的颜色
uniform vec3 viewPos;// 观察者位置，即摄像机位置
uniform vec3 objectColor;// 物体的颜色
uniform float alpha;// 半透明物体的透明度
uniform bool flag;// 片元是否位于半透明物体上

// 材质属性
uniform float ambientStrength;// Ambient light coefficient.
uniform float specularStrength;// Specular light coefficient.
uniform float diffuseStrength;// Diffuse coefficient.

// 光照衰减参数
uniform float att_a;// 衰减参数 a
uniform float att_b;// 衰减参数 b
uniform float att_c;// 衰减参数 c

uniform int n;// 幂次

// 阴影相关
uniform sampler2D opShadowMap[LIGHT_NUM];// 不透明物体的阴影贴图
uniform sampler2D transShadowMap[LIGHT_NUM];// 半透明物体的阴影贴图
uniform mat4 lightSpaceMatrix[LIGHT_NUM];// 光照空间变换矩阵

float ShadowCalculation(vec3 fragPos, int index) {
    vec4 fragPosLightSpace = lightSpaceMatrix[index] * vec4(fragPos, 1.0);// 将片元位置转换到光空间坐标
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;// 透视除法，转换到标准化设备坐标系 (NDC)
    projCoords = projCoords * 0.5 + 0.5;// 将坐标从[-1, 1]范围变换到[0, 1]范围，以便用于采样阴影贴图

    // 如果片元在光空间坐标系的z值大于1.0，表示片元在光源视锥体外，无需阴影计算
    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float shadow = 0.0;// 初始化阴影因子
    float bias = max(0.05 * (1.0 - dot(Normal, lightPos[index] - FragPos)), 0.005);// 计算偏差值，防止阴影失真（阴影彼得潘效应）
    // 计算阴影贴图的纹理尺寸
    vec2 opTexelSize = 1.0 / textureSize(opShadowMap[index], 0);
    vec2 tranTexelSize = 1.0 / textureSize(transShadowMap[index], 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            // 从阴影贴图采样深度值
            float opDepth = texture(opShadowMap[index], projCoords.xy + vec2(x, y) * opTexelSize).r;
            float transDepth = texture(transShadowMap[index], projCoords.xy + vec2(x, y) * tranTexelSize).r;

            if (projCoords.z - bias > opDepth){ // 如果当前深度值大于不透明物体采样的深度值，且在偏差范围内，则认为被不透明物体遮挡
                shadow += 1.0;
            } else if (projCoords.z - bias > transDepth){ // 如果当前深度值大于半透明物体采样的深度值，且在偏差范围内，则认为被半透明物体遮挡
                shadow += 1.0 * alpha;
            } else { // 不被遮挡
                shadow = 0.0;
            }
        }
    }

    // 计算平均阴影因子
    shadow /= 9.0;

    return shadow;
}

void main() {
    vec3 norm = normalize(Normal);// 归一化法向量
    vec3 viewDir = normalize(viewPos - FragPos);// 观察方向，即从片元指向观察者的方向

    vec3 totalDiffuse = vec3(0.0);// 总的漫反射光
    vec3 totalSpecular = vec3(0.0);// 总的镜面反射光
    vec3 totalAmbient = vec3(0.0);// 总的环境光
    float shadow = 0.0;// 总的阴影

    for (int i = 0; i < LIGHT_NUM; i++) {
        vec3 lightDir = normalize(lightPos[i] - FragPos);// 光源到片元的方向
        float diff = max(dot(norm, lightDir), 0.0);// 漫反射强度

        vec3 reflectDir = reflect(-lightDir, norm);// 反射方向
        float spec = max(pow(dot(viewDir, reflectDir), n), 0.0);// 镜面反射强度

        float distance = length(lightPos[i] - FragPos);// 光源到片元的距离
        float attenuation = 1.0 / (att_a + att_b * distance + att_c * pow(distance, 2));// 衰减因子

        vec3 diffuse = diffuseStrength * lightColor[i] * diff * attenuation;// 漫反射分量
        vec3 specular = specularStrength * lightColor[i] * spec * attenuation;// 镜面反射分量
        vec3 ambient = ambientStrength * lightColor[i];// 环境光分量

        totalDiffuse += diffuse;// 累加漫反射光
        totalSpecular += specular;// 累加镜面反射光
        totalAmbient += ambient;// 累加环境光

        shadow += ShadowCalculation(FragPos, i);// 累加阴影
    }

    vec3 result = (totalAmbient + (1.0 - shadow) * (totalDiffuse + totalSpecular)) * objectColor;// 计算最终颜色

    // 设置片元颜色
    if(flag){
        FragColor = vec4(result, alpha);
    } else{
        FragColor = vec4(result, 1.0);
    }
}