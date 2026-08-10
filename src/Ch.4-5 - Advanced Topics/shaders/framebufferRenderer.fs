#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D framebufferTexture;
uniform sampler2D bloomTexture;

// 0 = normal, 1 = invert,  2 = grayscale, 3 = sharpen, 4 = blur, 5 = edge-detection, 6 = bright, 7 = bright gaussian blur, 9 = shadow-depth (not in this shader)
uniform int postProcessingEffect;

// sharpen kernel
const float sharpenKernel[9] = float[](
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
);

// blur kernel
const float blurKernel[9] = float[](
    1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f,
    2.0f/16.0f, 4.0f/16.0f, 2.0f/16.0f,
    1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f
);
        
// edge kernel
const float edgeKernel[9] = float[](
    1,  1,  1,
    1, -8,  1,
    1,  1,  1
);

// gaussian blur variables
uniform bool horizontal;
const float weight[11] = float[] (
    0.23028146286326628,
    0.19494258146609367,
    0.11826327789981353,
    0.05141485055427349,
    0.01601853187213253,
    0.0035764512352154977,
    0.0005722388934480861,
    0.0000656142097928419,
    0.000005391553261103389,
    3.1748657295068635e-7,
    1.3397763040667093e-8);
uniform bool bloomReady;

// displays framebufferTexture onto quad covering screen
void main()
{             
    vec3 result = vec3(0.0f);

    
    // normal
    if (postProcessingEffect == 0)
    {
        result = vec3(texture(framebufferTexture, TexCoords));
    }
    
    // inverted
    if (postProcessingEffect == 1)
    {
        result = vec3(1.0 - texture(framebufferTexture, TexCoords));
    }
    
    // grayscale
    else if (postProcessingEffect == 2)
    {
        vec4 texColor = texture(framebufferTexture, TexCoords);
        float average = 0.2126 * texColor.r + 0.7152 * texColor.g + 0.0722 * texColor.b;
        result = vec3(average, average, average);
    }
    
    // kernel effects
    else if (postProcessingEffect == 3 || postProcessingEffect == 4 || postProcessingEffect == 5)
    {
        // since OpenGL texture coordinates are in range [0,1], normalize 1 pixel offset based on dimensions
        vec2 offset = 1.0f / textureSize(framebufferTexture, 0);

        vec2 offsets[9] = vec2[](
            vec2(-offset.x, offset.y),  // top-left
            vec2(0.0f,      offset.y),  // top-center
            vec2(offset.x,  offset.y),  // top-right
            vec2(-offset.x, 0.0f),      // center-left
            vec2(0.0f,      0.0f),      // center
            vec2(offset.x,  0.0f),      // center-right
            vec2(-offset.x, -offset.y), // bottom-left
            vec2(0.0f,      -offset.y), // bottom-center
            vec2(offset.x,  -offset.y)  // bottom-right
        );
        
        vec3 sampleTex[9];
        for (int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(framebufferTexture, TexCoords.st + offsets[i]));
        }

        for (int i = 0; i < 9; i++)
        {
            if (postProcessingEffect == 3)
            {
                result += sampleTex[i] * sharpenKernel[i];
            }
            else if (postProcessingEffect == 4)
            {
                result += sampleTex[i] * blurKernel[i];
            }
            else if (postProcessingEffect == 5)
            {
                result += sampleTex[i] * edgeKernel[i];
            }
        }
    }
    
    // bright spots only
    else if (postProcessingEffect == 6)
    {
        float brightness = dot(texture(framebufferTexture, TexCoords).rgb, vec3(0.2126f, 0.7152f, 0.0722f));
        if (brightness > 0.25f)
        {
            result = texture(framebufferTexture, TexCoords).rgb;
        }
    }
    
    // bloom
    else if (postProcessingEffect == 7)
    {
        if (!bloomReady)
        {
            // since OpenGL texture coordinates are in range [0,1], normalize 1 pixel offset based on dimensions
            vec2 offset = 1.0f / textureSize(framebufferTexture, 0);

            result = texture(framebufferTexture, TexCoords).rgb * weight[0];

            if (horizontal)
            {
                for (int i = 1; i < weight.length(); i++)
                {
                    result += texture(framebufferTexture, TexCoords + vec2(offset.x * i, 0.0)).rgb * weight[i];
                    result += texture(framebufferTexture, TexCoords - vec2(offset.x * i, 0.0)).rgb * weight[i];
                }
            }
            else
            {
                for(int i = 1; i < weight.length(); ++i)
                {
                    result += texture(framebufferTexture, TexCoords + vec2(0.0, offset.y * i)).rgb * weight[i];
                    result += texture(framebufferTexture, TexCoords - vec2(0.0, offset.y * i)).rgb * weight[i];
                }
            }
        }
        else
        {
            float gamma = 2.2f;
            vec3 color = texture(framebufferTexture, TexCoords).rgb;
            vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
            result = color + bloomColor;
        }
    }
    
    FragColor = vec4(result, 1.0f);

}
