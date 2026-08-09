#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D framebufferTexture;

uniform float SCR_WIDTH;
uniform float SCR_HEIGHT;

// 0 = normal, 1 = invert,  2 = grayscale, 3 = sharpen, 4 = blur, 5 = edge-detection, 9 = shadow-depth (not in this shader),
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

// displays framebufferTexture onto quad covering screen
void main()
{             
    vec3 result = vec3(0.0f);

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
        float xOffset = 1.0f / SCR_WIDTH;
        float yOffset = 1.0f / SCR_HEIGHT;

        vec2 offsets[9] = vec2[](
            vec2(-xOffset, yOffset),  // top-left
            vec2(0.0f,     yOffset),  // top-center
            vec2(xOffset,  yOffset),  // top-right
            vec2(-xOffset, 0.0f),     // center-left
            vec2(0.0f,     0.0f),     // center
            vec2(xOffset,  0.0f),     // center-right
            vec2(-xOffset, -yOffset), // bottom-left
            vec2(0.0f,     -yOffset), // bottom-center
            vec2(xOffset,  -yOffset)  // bottom-right
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
    
    FragColor = vec4(result, 1.0f);

}
