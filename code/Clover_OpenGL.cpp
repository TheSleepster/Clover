/* ========================================================================
   $File: Clover_OpenGL.cpp $
   $Date: Mon, 25 Nov 24: 08:PM $
   $Revision: $
   $Creator: Justin Lewis $
=================================== */

constexpr uint32 MAX_LAYER_BITS      = 21;
constexpr uint32 MAX_ACTIVE_TEXTURES = 32;
constexpr uint32 MAX_LAYERS          = 32;

struct clover_texture
{
    string Filepath;
    time_t LastWriteTime;

    ivec3  TextureChannelData;
    char  *RawData;

    GLuint TextureID;
};

uint32 QuadCounter = 3;
uint32 TransparentQuadCounter = 4;

struct gl_vertex
{
    vec4   Position;
    vec4   Color;
    vec2   TexCoords;
    
    int32  TextureIndex;
};

struct render_quad
{
    union
    {
        gl_vertex Elements[4];
        struct
        {
            gl_vertex TopLeft;
            gl_vertex TopRight;
            gl_vertex BottomLeft;
            gl_vertex BottomRight;
        };
    };
    real32 Rotation;
    int32  ZLayer;
    bool32 IsTransparent;

    mat4   xForm;
    vec4   QuadColor;
    int32  SortingTransparency;

    GLuint BoundTextureID;
};

enum gl_shader_work_type
{
    SHADER_WORK_TYPE_NULL,
    SHADER_WORK_TYPE_RENDERING,
    SHADER_WORK_TYPE_COMPUTATION,
};

struct gl_shader
{
    GLuint VertexShaderSourceID;
    GLuint FragmentShaderSourceID;
    GLuint ComputeShaderSourceID;

    GLuint ShaderID;

    string Filepath;
    time_t LastWriteTime;

    gl_shader_work_type WorkType;
};

struct gl_render_info
{
    GLuint PrimaryVAOID;
    GLuint PrimaryVBOID;
    GLuint PrimaryEBOID;

    GLuint ProjectionMatrixUID;
    GLuint ViewMatrixUID;

    GLuint IndirectDrawingCommandBufferID;

    // DEBUG
    GLuint ComputeFramebuffer;
    GLuint TestComputeTexture;
    clover_texture Testure;
};

struct draw_elements_indirect_command
{
    GLuint IndexCount;
    GLuint InstanceCount;
    GLuint FirstIndex;
    GLuint FirstVertex;
    GLuint BaseInstance;
};

constexpr uint32 IndirectDrawCommandBufferSize   = (MAX_LAYERS * 2) * sizeof(draw_elements_indirect_command);

struct render_layer_info
{
    int32 BaseIndex;
    int32 EndingIndex;

    int32 LayerOpaqueQuadCount;
    int32 LayerTransparentQuadCount;
};

struct gl_draw_frame_data
{
    GLuint         ActiveTextures[MAX_ACTIVE_TEXTURES];
    int32          ActiveTextureCount;

    mat4      	   ProjectionMatrix;
    mat4      	   ViewMatrix;

    render_quad   *QuadBuffer;	
    render_quad   *QuadSortingBuffer;
    gl_vertex     *glVertexBuffer;

    int32          QuadCounter;

    bool32         ZSorting;
    render_quad    NullQuad;

    int32          ActiveZLayer;

    gl_shader 	   Shader;
    gl_shader      TestComputeShader;

    uint32            ActiveLayerCounter;
    render_layer_info Layers[MAX_LAYERS];
    draw_elements_indirect_command IndirectRenderCommandBuffer[MAX_LAYERS * 2];
};

internal render_quad *DrawQuad(gl_draw_frame_data *DrawFrame, vec2 Position, vec2 Size, vec4 Color);
internal render_quad *DrawQuadTextured(gl_draw_frame_data *DrawFrame, vec2 Position, vec2 Size, vec4 Color, clover_texture *Texture);
internal render_quad *DrawImageXForm(gl_draw_frame_data *DrawFrame, mat4 XForm, vec2 Size, clover_texture *Texture, vec4 Color);
internal render_quad *DrawQuadXForm(gl_draw_frame_data *DrawFrame, mat4 XForm, vec2 Size, vec4 Color);

internal inline void PushZLayer(gl_draw_frame_data *DrawFrame, int Layer);
internal inline void PopZLayer(gl_draw_frame_data *DrawFrame);

internal void APIENTRY
OpenGLDebugMessageCallback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity,
                           GLsizei Length, const GLchar *Message, const void *UserParam)
{
    if(Severity == GL_DEBUG_SEVERITY_LOW||
       Severity == GL_DEBUG_SEVERITY_MEDIUM||
       Severity == GL_DEBUG_SEVERITY_HIGH)
    {
        printlm("Error: %s\n", Message);
        Assert(false);
    }
    else
    {
        // NOTE(Sleepster): We don't have Tracing here because ImGui will BLAST my output window otherwise 
    }
}

void
CloverTestShader(GLuint TestID, GLuint Type)
{
    bool32 Success = {};
    char ShaderLog[2048] = {};
    
    switch(Type)
    {
        case GL_VERTEX_SHADER:
        case GL_FRAGMENT_SHADER:
        case GL_COMPUTE_SHADER:
        {
            glGetShaderiv(TestID, GL_COMPILE_STATUS, &Success);
        }break;
        case GL_PROGRAM:
        {
            glGetProgramiv(TestID, GL_LINK_STATUS, &Success);
        }break;
    }
    
    if(!Success)
    {
        glGetShaderInfoLog(TestID, 2048, 0, ShaderLog);
        printm("ERROR ON SHADER COMPILATION: %s\n", ShaderLog);
        Assert(false);
    }
}

internal gl_shader
CloverLoadShader(memory_arena *Arena, string Filepath, gl_shader_work_type WorkType)
{
	gl_shader Result = {};
    
    uint32 Size = 0;
    string ShaderSource = ReadEntireFileMA(Arena, Filepath, &Size);
    if(ShaderSource.Data)
    {
        Result.Filepath = Filepath;
        Result.LastWriteTime = FileGetLastWriteTime(Filepath);

        Result.WorkType = WorkType;
        switch(WorkType)
        {
            case SHADER_WORK_TYPE_RENDERING:
            {
                memory_index VertexHeaderSize   = strlen("#VERTEX");
                memory_index FragmentHeaderSize = strlen("#FRAGMENT");

                string VertexShaderStart   = STR(strstr(CSTR(ShaderSource), "#VERTEX"));
                string FragmentShaderSource = STR(strstr(CSTR(ShaderSource), "#FRAGMENT"));

                FragmentShaderSource.Data   += FragmentHeaderSize;
                FragmentShaderSource.Length -= FragmentHeaderSize;

                VertexShaderStart.Data   += VertexHeaderSize;
                VertexShaderStart.Length -= VertexHeaderSize + (FragmentShaderSource.Length + FragmentHeaderSize);

                string VertexShaderSource = StringCopy(VertexShaderStart, Arena); 

                if(VertexShaderSource.Data && FragmentShaderSource.Data)
                {
                    Result.VertexShaderSourceID   = glCreateShader(GL_VERTEX_SHADER);
                    Result.FragmentShaderSourceID = glCreateShader(GL_FRAGMENT_SHADER);

                    glShaderSource(Result.VertexShaderSourceID, 1, &CSTR(VertexShaderSource), 0);
                    glCompileShader(Result.VertexShaderSourceID);
                    CloverTestShader(Result.VertexShaderSourceID, GL_VERTEX_SHADER);

                    glShaderSource(Result.FragmentShaderSourceID, 1, &CSTR(FragmentShaderSource), 0);
                    glCompileShader(Result.FragmentShaderSourceID);
                    CloverTestShader(Result.FragmentShaderSourceID, GL_FRAGMENT_SHADER);

                    Result.ShaderID = glCreateProgram();
                    glAttachShader(Result.ShaderID, Result.VertexShaderSourceID);
                    glAttachShader(Result.ShaderID, Result.FragmentShaderSourceID);
                    glLinkProgram(Result.ShaderID);
                    CloverTestShader(Result.ShaderID, GL_PROGRAM);

                    glDetachShader(Result.ShaderID, Result.VertexShaderSourceID);
                    glDetachShader(Result.ShaderID, Result.FragmentShaderSourceID);
                    glDeleteShader(Result.VertexShaderSourceID);
                    glDeleteShader(Result.FragmentShaderSourceID);
                }
                else
                {
                    cl_Error("Failure to get the shader data from the file, verify that #VERTEX and #FRAGMENT Exist\n");
                }
            }break;
            case SHADER_WORK_TYPE_COMPUTATION:
            {
                Result.ComputeShaderSourceID = glCreateShader(GL_COMPUTE_SHADER);

                glShaderSource(Result.ComputeShaderSourceID, 1, &CSTR(ShaderSource), 0);
                glCompileShader(Result.ComputeShaderSourceID);
                CloverTestShader(Result.ComputeShaderSourceID, GL_COMPUTE_SHADER);

                Result.ShaderID = glCreateProgram();
                glAttachShader(Result.ShaderID, Result.ComputeShaderSourceID);
                glLinkProgram(Result.ShaderID);
                CloverTestShader(Result.ShaderID, GL_PROGRAM);

                glDetachShader(Result.ShaderID, Result.ComputeShaderSourceID);
                glDeleteShader(Result.ComputeShaderSourceID);
            }break;
            default:
            {
                InvalidCodePath;
            }break;
        }
    }
    else
    {
        cl_Error("Failure to load the filepath specified!\n");
    }

    return(Result);
}

internal void 
CloverLoadTextureData(gl_render_info *RenderInfo, clover_texture *Texture, string Filepath)
{
    if(Texture)
    {
        glGenTextures(1, &Texture->TextureID);
        glBindTexture(GL_TEXTURE_2D, Texture->TextureID);

        Texture->Filepath = Filepath;
        Texture->LastWriteTime = FileGetLastWriteTime(Filepath);
        Texture->RawData = (char *)stbi_load(CSTR(Filepath),
                                             &Texture->TextureChannelData.Width,
                                             &Texture->TextureChannelData.Height,
                                             &Texture->TextureChannelData.Channels,
                                             4);
        if(Texture->RawData)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 
                         Texture->TextureChannelData.Width, Texture->TextureChannelData.Height, 0, 
                         GL_RGBA, GL_UNSIGNED_BYTE, Texture->RawData);
        }
        else
        {
            cl_Error("Could not extract image data. Image may be invalid.\n");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    	stbi_image_free(Texture->RawData);
    }
    else
    {
        cl_Error("Texture memory Invalid\n");
    }
}








// DEBUG
GLuint TestTimeLocation;
gl_shader TestQuadShader;
float ShaderCounter;









internal void
CloverInitializeOpenGLRenderer(gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, transient_state *TransientState)
{
    uint32 Indices[MAX_INDICES] = {};
    uint32 Offset               = 0;
    for(uint32 Index = 0;
        Index < MAX_INDICES;
        Index += 6)
    {
        Indices[Index + 0] = Offset + 0;
        Indices[Index + 1] = Offset + 1;
        Indices[Index + 2] = Offset + 2;
        Indices[Index + 3] = Offset + 2;
        Indices[Index + 4] = Offset + 3;
        Indices[Index + 5] = Offset + 0;
        Offset += 4;
    }

    // STATE INITIALIZATION
    {
        glDebugMessageCallback(&OpenGLDebugMessageCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
        
        glEnable(GL_BLEND);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_FRAMEBUFFER_SRGB);
        glDisable(0x809D); // Disabling multisampling
    }

    // NOTE(Sleepster): PRIMARY BUFFER SETUP 
    {
        // PRIMARY VERTEX ARRAY
        glGenVertexArrays(1, &RenderInfo->PrimaryVAOID);
        glBindVertexArray(RenderInfo->PrimaryVAOID);

        // PRIMARY VERTEX BUFFER
        glGenBuffers(1, &RenderInfo->PrimaryVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, RenderInfo->PrimaryVBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gl_vertex) * MAX_VERTICES, 0, GL_DYNAMIC_DRAW);
        
        // PRIMARY INDEX BUFFER
        glGenBuffers(1, &RenderInfo->PrimaryEBOID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderInfo->PrimaryEBOID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, Position));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, Color));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gl_vertex), (void *)offsetof(gl_vertex, TexCoords));

        glVertexAttribIPointer(3, 1, GL_INT, sizeof(gl_vertex), (void *)offsetof(gl_vertex, TextureIndex));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
    }

    // DEBUG
    {
        glGenTextures(1, &RenderInfo->TestComputeTexture);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->TestComputeTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SizeData.Width, SizeData.Height, 0, GL_RGBA, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // DRAW COMMAND BUFFER
    {
        glGenBuffers(1, &RenderInfo->IndirectDrawingCommandBufferID);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RenderInfo->IndirectDrawingCommandBufferID);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, (MAX_LAYERS * 2) * sizeof(draw_elements_indirect_command), 0, GL_DYNAMIC_DRAW);
    }

    {
        DrawFrame->Shader =
            CloverLoadShader(&TransientState->Garbage, STR("../code/shader/Test.glsl"), SHADER_WORK_TYPE_RENDERING);

        DrawFrame->TestComputeShader =
            CloverLoadShader(&TransientState->Garbage, STR("../code/shader/Test.comp"), SHADER_WORK_TYPE_COMPUTATION);

        TestQuadShader =
            CloverLoadShader(&TransientState->Garbage, STR("../code/shader/Quad.glsl"), SHADER_WORK_TYPE_RENDERING);

        TestTimeLocation = glGetUniformLocation(DrawFrame->TestComputeShader.ShaderID, "Time");
    }

    CloverLoadTextureData(RenderInfo, &RenderInfo->Testure, STR("../data/res/textures/test.png"));
}

internal bool32
AddTextureToBoundList(gl_draw_frame_data *DrawFrame, GLuint TextureID)
{
    for(int32 TextureIndex = 0;
        TextureIndex < DrawFrame->ActiveTextureCount;
        ++TextureIndex)
    {
        if(DrawFrame->ActiveTextures[TextureIndex] == TextureID)
        {
            return(false);
        }
    }
    if(DrawFrame->ActiveTextureCount < MAX_ACTIVE_TEXTURES)
    {
        DrawFrame->ActiveTextures[DrawFrame->ActiveTextureCount++] = TextureID;
        return(true);
    }

    Assert(DrawFrame->ActiveTextureCount + 1 < MAX_ACTIVE_TEXTURES);
    return(false);
}

internal void
ResetLayerData(gl_draw_frame_data *DrawFrame)
{
    for(uint32 LayerIndex = 0;
        LayerIndex < MAX_LAYERS;
        ++LayerIndex)
    {
        DrawFrame->Layers[LayerIndex] = {};
    }
}

internal void
CloverOpenGLRender(gl_render_info *RenderInfo, gl_draw_frame_data *DrawFrame, time_data Time)
{

    DrawFrame->QuadCounter = 0;
    DrawFrame->ActiveTextureCount = 0;

    DrawFrame->ZSorting = true;
    DrawFrame->ActiveZLayer = 32;

    ResetLayerData(DrawFrame);

    // TODO(Sleepster): Change this ViewMatrix so that it zooms in when the screen gets larger  
	DrawFrame->ViewMatrix 		= mat4Scale(mat4Identity(1.0f), vec3{real32(SizeData.Width / 160), real32(SizeData.Width / 160), 1.0});
    DrawFrame->ProjectionMatrix = mat4RHGLOrtho((real32)-SizeData.Width,
                                                (real32)SizeData.Width,
                                                (real32)-SizeData.Height,
                                                (real32)SizeData.Height,
                                                -1.0f,
                                                 1.0f);
#if 1
    mat4 XForm = mat4Identity(1.0f);
    XForm = mat4Translation(XForm, vec3{0, 32});

    PushZLayer(DrawFrame, 14);
    DrawQuadXForm(DrawFrame, XForm, {16, 16}, RED);

    XForm = mat4Identity(1.0f);
    XForm = mat4Translation(XForm, vec3{8, 32});

    PushZLayer(DrawFrame, 3);
    DrawQuad(DrawFrame, {-16, 0}, {16, 16}, BLUE);

    PushZLayer(DrawFrame, 4);
    DrawQuadTextured(DrawFrame, {32, 0}, {16, 16}, WHITE, &RenderInfo->Testure);
    DrawQuad(DrawFrame, {-32, 0}, {16, 16}, WHITE);

    PushZLayer(DrawFrame, 5);
    DrawQuad(DrawFrame, {-42, 0}, {16, 16}, {0.0f, 0.2f, 0.0f, 0.3f});

    PushZLayer(DrawFrame, 3);
    DrawQuad(DrawFrame, {-36, -10}, {16, 16}, {0.4f, 0.0f, 0.0f, 0.6f});


    PushZLayer(DrawFrame, 15);
    DrawImageXForm(DrawFrame, XForm, {16, 16}, &RenderInfo->Testure, WHITE);


    PushZLayer(DrawFrame, 0);
    ivec2  TileRadius   = {32, 32};
    for(int32 TileX = 0;
        TileX < TileRadius.X;
        ++TileX)
    {
        for(int32 TileY = 0;
            TileY < TileRadius.Y;
            ++TileY)
        {
            if((TileX + (TileY % 2 == 0)) % 2 == 0)
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuad(DrawFrame, {X - (TILE_SIZE * 16), (Y - TILE_SIZE) - (TILE_SIZE * 12)}, {16, 16}, DARK_GRAY);
            }
            else
            {
                real32 X = TileX * TILE_SIZE;
                real32 Y = TileY * TILE_SIZE;
                DrawQuad(DrawFrame, {X - (TILE_SIZE * 16), (Y - TILE_SIZE) - (TILE_SIZE * 12)}, {16, 16}, DARKER_GRAY);
            }
        }
    }
    PopZLayer(DrawFrame);

    if(DrawFrame->QuadCounter > 0)
    {
        gl_vertex *VertexBufferptr = DrawFrame->glVertexBuffer;
        if(DrawFrame->ZSorting)
        {
            RadixSort((void *)DrawFrame->QuadBuffer,
                        (void *)DrawFrame->QuadSortingBuffer,
                        DrawFrame->QuadCounter,
                        sizeof(render_quad),
                        offsetof(render_quad, ZLayer),
                        MAX_LAYER_BITS);

            // NOTE(Sleepster): This is so that we know where the Transparent quads are in the buffer
            // So that we can just start at the opaque index and render them and not worry about it
            int32 QuadBufferLayerOffset = 0;
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];
                CurrentWorkingLayer->BaseIndex   = QuadBufferLayerOffset;
                     
                if(CurrentWorkingLayer->LayerOpaqueQuadCount > 0)
                {
                    CurrentWorkingLayer->EndingIndex = QuadBufferLayerOffset + (CurrentWorkingLayer->LayerOpaqueQuadCount); 
                    QuadBufferLayerOffset += CurrentWorkingLayer->LayerOpaqueQuadCount;
                }

                if(CurrentWorkingLayer->LayerTransparentQuadCount > 0)
                {
                    CurrentWorkingLayer->EndingIndex = QuadBufferLayerOffset + (CurrentWorkingLayer->LayerTransparentQuadCount - 1); 
                    QuadBufferLayerOffset += CurrentWorkingLayer->LayerTransparentQuadCount;
                }
            }

            // NOTE(Sleepster): Sort by transparency
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];
                if(CurrentWorkingLayer->LayerTransparentQuadCount)
                {
                    RadixSort((void *)(DrawFrame->QuadBuffer + (CurrentWorkingLayer->BaseIndex)),
                              (void *)DrawFrame->QuadSortingBuffer,
                              CurrentWorkingLayer->LayerTransparentQuadCount + CurrentWorkingLayer->LayerOpaqueQuadCount,
                              sizeof(render_quad),
                              offsetof(render_quad, SortingTransparency),
                              MAX_LAYER_BITS);
                }
            }

            // NOTE(Sleepster): Then we set up the indirect draw command buffer 
            DrawFrame->ActiveLayerCounter = 0;
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];
                if(CurrentWorkingLayer->LayerOpaqueQuadCount > 0)
                {
                    DrawFrame->IndirectRenderCommandBuffer[DrawFrame->ActiveLayerCounter] =
                    {
                        .IndexCount = uint32(CurrentWorkingLayer->LayerOpaqueQuadCount * 6),
                        .InstanceCount = 1,
                        .FirstIndex = uint32(CurrentWorkingLayer->BaseIndex * 6),
                        .FirstVertex = 0,
                        .BaseInstance = 0,
                    };
                }

                if(CurrentWorkingLayer->LayerTransparentQuadCount > 0)
                {
                    uint32 TransparentLayerIndex = int32(MAX_LAYERS) + DrawFrame->ActiveLayerCounter;
                    DrawFrame->IndirectRenderCommandBuffer[TransparentLayerIndex] =
                    {
                        .IndexCount = uint32(CurrentWorkingLayer->LayerTransparentQuadCount * 6),
                        .InstanceCount = 1,
                        .FirstIndex = uint32((CurrentWorkingLayer->BaseIndex + CurrentWorkingLayer->LayerOpaqueQuadCount) * 6),
                        .FirstVertex = 0,
                        .BaseInstance = 0,
                    };
                }

                if(CurrentWorkingLayer->LayerOpaqueQuadCount || CurrentWorkingLayer->LayerTransparentQuadCount) DrawFrame->ActiveLayerCounter++;
            }

    #if 0
            for(uint32 LayerIndex = 0;
                LayerIndex < MAX_LAYERS;
                ++LayerIndex)
            {
                render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];

                cl_Info("Layer[%i]: CurrentWorkingLayer->BaseIndex = %d\n",      LayerIndex, CurrentWorkingLayer->BaseIndex);
                cl_Info("Layer[%i]: CurrentWorkingLayer->EndingIndex = %d\n",    LayerIndex, CurrentWorkingLayer->EndingIndex);
                cl_Info("Layer[%i]: CurrentWorkingLayer->LayerQuadCount = %d\n", LayerIndex, CurrentWorkingLayer->LayerQuadCount);
            }
    #endif
        }

        for(int32 QuadIndex = 0;
            QuadIndex < DrawFrame->QuadCounter;
            ++QuadIndex)
        {
            render_quad *Quad = DrawFrame->QuadBuffer + QuadIndex;

            gl_vertex *BottomLeft  = VertexBufferptr + 0;
            gl_vertex *TopLeft     = VertexBufferptr + 1;
            gl_vertex *TopRight    = VertexBufferptr + 2;
            gl_vertex *BottomRight = VertexBufferptr + 3;
            VertexBufferptr += 4;

            BottomLeft->Position  = {Quad->BottomLeft.Position.X,  Quad->BottomLeft.Position.Y,  0, 1};
            TopLeft->Position     = {Quad->TopLeft.Position.X,     Quad->TopLeft.Position.Y,     0, 1};
            TopRight->Position    = {Quad->TopRight.Position.X,    Quad->TopRight.Position.Y,    0, 1};
            BottomRight->Position = {Quad->BottomRight.Position.X, Quad->BottomRight.Position.Y, 0, 1};

            BottomLeft->Color  = Quad->QuadColor;
            TopLeft->Color     = Quad->QuadColor;
            TopRight->Color    = Quad->QuadColor;
            BottomRight->Color = Quad->QuadColor;

            BottomLeft->TexCoords  = Quad->BottomLeft.TexCoords;
            TopLeft->TexCoords 	   = Quad->TopLeft.TexCoords;
            TopRight->TexCoords	   = Quad->TopRight.TexCoords;
            BottomRight->TexCoords = Quad->BottomRight.TexCoords;

            BottomLeft->TextureIndex  = Quad->BottomLeft.TextureIndex;
            TopLeft->TextureIndex 	  = Quad->TopLeft.TextureIndex;
            TopRight->TextureIndex	  = Quad->TopRight.TextureIndex;
            BottomRight->TextureIndex = Quad->BottomRight.TextureIndex;

            if(Quad->BoundTextureID != 0)
            {
                AddTextureToBoundList(DrawFrame, Quad->BoundTextureID);
            }
        }
    }

    #if 0
    // OPAQUE
    {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
        glDepthMask(GL_TRUE);

        glBindBuffer(GL_ARRAY_BUFFER, RenderInfo->PrimaryVBOID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (DrawFrame->QuadCounter * 4) * sizeof(gl_vertex), DrawFrame->glVertexBuffer);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RenderInfo->IndirectDrawingCommandBufferID);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, (DrawFrame->ActiveLayerCounter) * sizeof(draw_elements_indirect_command), &DrawFrame->IndirectRenderCommandBuffer[0]);

        glUseProgram(DrawFrame->Shader.ShaderID);
        for(int32 ActiveTextureIndex = 0;
            ActiveTextureIndex < DrawFrame->ActiveTextureCount;
            ++ActiveTextureIndex)
        {
            glActiveTexture(GL_TEXTURE0 + (ActiveTextureIndex));
            glBindTexture(GL_TEXTURE_2D, DrawFrame->ActiveTextures[ActiveTextureIndex]);
        }

        glBindVertexArray(RenderInfo->PrimaryVAOID);
        glMultiDrawElementsIndirect(GL_TRIANGLES,
                                    GL_UNSIGNED_INT,
                                    0, 
                                    DrawFrame->ActiveLayerCounter,
                                    sizeof(draw_elements_indirect_command));
    }

    // TRANSPARENT
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glEnable(GL_BLEND);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RenderInfo->IndirectDrawingCommandBufferID);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER,
                        GLint(MAX_LAYERS) * sizeof(draw_elements_indirect_command),
                        (DrawFrame->ActiveLayerCounter) * sizeof(draw_elements_indirect_command),
                        &DrawFrame->IndirectRenderCommandBuffer[32]);

        glUseProgram(DrawFrame->Shader.ShaderID);
        glMultiDrawElementsIndirect(GL_TRIANGLES,
                                    GL_UNSIGNED_INT,
                                    (void *)(int32((MAX_LAYERS) * sizeof(draw_elements_indirect_command))), 
                                    DrawFrame->ActiveLayerCounter,
                                    sizeof(draw_elements_indirect_command));
    }
    #else



    glBindBuffer(GL_ARRAY_BUFFER, RenderInfo->PrimaryVBOID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (DrawFrame->QuadCounter * 4) * sizeof(gl_vertex), DrawFrame->glVertexBuffer);

    glBindVertexArray(RenderInfo->PrimaryVAOID);
    glUseProgram(DrawFrame->Shader.ShaderID);

    for(int32 ActiveTextureIndex = 0;
        ActiveTextureIndex < DrawFrame->ActiveTextureCount;
        ++ActiveTextureIndex)
    {
        glActiveTexture(GL_TEXTURE0 + (ActiveTextureIndex));
        glBindTexture(GL_TEXTURE_2D, DrawFrame->ActiveTextures[ActiveTextureIndex]);
    }

    for(uint32 LayerIndex = 0;
        LayerIndex < MAX_LAYERS;
        ++LayerIndex)
    {
        render_layer_info *CurrentWorkingLayer = &DrawFrame->Layers[LayerIndex];

        // DRAW OPAQUE
        if(CurrentWorkingLayer->LayerOpaqueQuadCount > 0)
        {
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
             
            glDrawElements(GL_TRIANGLES,
                           CurrentWorkingLayer->LayerOpaqueQuadCount * 6,
                           GL_UNSIGNED_INT,
                           (void *)((CurrentWorkingLayer->BaseIndex * 6) * sizeof(uint32)));
        }

        // DRAW TRANSPARENT
        if(CurrentWorkingLayer->LayerTransparentQuadCount > 0)
        {
            glDisable(GL_DEPTH_TEST);
            
            glEnable(GL_BLEND);        
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
             
            glDrawElements(GL_TRIANGLES,
                           CurrentWorkingLayer->LayerTransparentQuadCount* 6,
                           GL_UNSIGNED_INT,
                           (void *)(((CurrentWorkingLayer->BaseIndex + CurrentWorkingLayer->LayerOpaqueQuadCount) * 6) * sizeof(uint32)));
        }
    }
    #endif




    glFlush();
#else
    {
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glUseProgram(DrawFrame->TestComputeShader.ShaderID);
        glBindImageTexture(0, RenderInfo->TestComputeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        ShaderCounter += (1 * 0.001);
        glUniform1f(TestTimeLocation, ShaderCounter);

        glDispatchCompute((SizeData.Width + 7) / 8, (SizeData.Height + 3) / 4, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(TestQuadShader.ShaderID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, RenderInfo->TestComputeTexture);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
#endif
}
