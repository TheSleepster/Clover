/* ========================================================================
   $File: Clover_DrawingInterface.cpp $
   $Date: Mon, 25 Nov 24: 02:58PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
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
        Assert(DrawFrame->ActiveTextureCount + 1 < MAX_ACTIVE_TEXTURES);
        DrawFrame->ActiveTextures[DrawFrame->ActiveTextureCount++] = TextureID;
        return(true);
    }

    Assert(DrawFrame->ActiveTextureCount + 1 < MAX_ACTIVE_TEXTURES);
    return(false);
}

internal render_quad
CreateRenderQuad(gl_draw_frame_data *DrawFrame,
                 vec2                WorldPosition,
                 vec2                RenderSize,
                 clover_texture     *Texture,
                 vec2                AtlasOffset,
                 vec2                SpriteSize,
                 int32               TextureIndex,
                 vec4                Color,
                 int32               Layer,
                 uint32              RenderingOptions = 0,
                 real32              LitFactor = 1.0f)
{
    const real32 Top    = WorldPosition.Y;
    const real32 Bottom = WorldPosition.Y + RenderSize.Y;
    const real32 Left   = WorldPosition.X;
    const real32 Right  = WorldPosition.X + RenderSize.X;

    render_quad Quad  = {};
    Quad.TopLeft.WorldPosition     = v2Expand(vec2{Left, Top}, 0, 1);
    Quad.TopRight.WorldPosition    = v2Expand(vec2{Right, Top}, 0, 1);
    Quad.BottomLeft.WorldPosition  = v2Expand(vec2{Left, Bottom}, 0, 1);
    Quad.BottomRight.WorldPosition = v2Expand(vec2{Right, Bottom}, 0, 1);

    if(Texture)
    {
        Quad.BoundTextureID = Texture->TextureID;

        Quad.BottomLeft.TexCoords  = {AtlasOffset.X               , AtlasOffset.Y + SpriteSize.Y};
        Quad.BottomRight.TexCoords = {AtlasOffset.X + SpriteSize.X, AtlasOffset.Y + SpriteSize.Y};
        Quad.TopLeft.TexCoords     = {AtlasOffset.X               , AtlasOffset.Y};
        Quad.TopRight.TexCoords    = {AtlasOffset.X + SpriteSize.X, AtlasOffset.Y};
    }
    Quad.QuadColor = Color;
    Quad.QuadColor.A >= 1.0f ? Quad.SortingTransparency = 0 : Quad.SortingTransparency = 1;
    Quad.RenderingOptions = RenderingOptions;

    if((Quad.RenderingOptions & RENDERING_OPTION_FONT) != 0)
    {
        Quad.SortingTransparency = 1;
    }

    Quad.ZLayer    = Layer;
    for(uint32 Index = 0;
        Index < 4;
        ++Index)
    {
        Quad.Elements[Index].Color = Quad.QuadColor;
        Quad.Elements[Index].TextureIndex = TextureIndex;
        Quad.Elements[Index].RenderingOptions = RenderingOptions;
        Quad.Elements[Index].Normals = vec3{0.0, 0.0, 1};
        Quad.Elements[Index].LitFactor = LitFactor;
    }

    return(Quad);
}

internal inline void
AddQuadToLayer(gl_draw_frame_data *DrawFrame, render_quad *Quad)
{
    if(Quad->QuadColor.A == 1.0f && (Quad->RenderingOptions & RENDERING_OPTION_FONT) == 0)
        DrawFrame->Layers[Quad->ZLayer].LayerOpaqueQuadCount++;
    else
        DrawFrame->Layers[Quad->ZLayer].LayerTransparentQuadCount++;
}

// IMPORTANT(Sleepster): The quad's vertex Coords are in NDC (normalized (-1, 1)) 
internal render_quad*
DrawQuadProjected(gl_draw_frame_data *DrawFrame, render_quad Quad, mat4 WorldToClip)
{
    Assert(DrawFrame->QuadCounter < MAX_QUADS);
    
	Quad.BottomLeft.NDCPosition  = mat4Transform(WorldToClip, Quad.BottomLeft.WorldPosition);
	Quad.BottomRight.NDCPosition = mat4Transform(WorldToClip, Quad.BottomRight.WorldPosition);
	Quad.TopLeft.NDCPosition     = mat4Transform(WorldToClip, Quad.TopLeft.WorldPosition);
	Quad.TopRight.NDCPosition    = mat4Transform(WorldToClip, Quad.TopRight.WorldPosition);

    bool32 ShouldCull =
        (Quad.BottomLeft.NDCPosition.X < -1 && Quad.TopLeft.NDCPosition.X < -1 && Quad.BottomRight.NDCPosition.X < -1 && Quad.TopRight.NDCPosition.X < -1 ||
         Quad.BottomLeft.NDCPosition.X >  1 && Quad.TopLeft.NDCPosition.X >  1 && Quad.BottomRight.NDCPosition.X >  1 && Quad.TopRight.NDCPosition.X >  1 ||
         Quad.BottomLeft.NDCPosition.Y < -1 && Quad.TopLeft.NDCPosition.Y < -1 && Quad.BottomRight.NDCPosition.Y < -1 && Quad.TopRight.NDCPosition.Y < -1 ||
         Quad.BottomLeft.NDCPosition.Y >  1 && Quad.TopLeft.NDCPosition.Y >  1 && Quad.BottomRight.NDCPosition.Y >  1 && Quad.TopRight.NDCPosition.Y >  1);
    if(ShouldCull)
    {
        return(&DrawFrame->NullQuad);
    }

    AddQuadToLayer(DrawFrame, &Quad);
    render_quad *RenderQuad = &DrawFrame->QuadBuffer[DrawFrame->QuadCounter++];
    *RenderQuad = Quad;
    
    return(&Quad);
}

// NOTE(Sleepster): The DrawFrame Matrices can just be modified before calling a "Draw" function.
//                  This would allow you to set what space you want the vertices to be in without a seperate drawcall
internal render_quad*
DrawQuadInView(gl_draw_frame_data *DrawFrame, render_quad Quad)
{
    // TODO(Sleepster): This may be expensive and unnecessary 
	mat4 WorldToClip = mat4Multiply(DrawFrame->ProjectionMatrix, DrawFrame->ViewMatrix);
    return(DrawQuadProjected(DrawFrame, Quad, WorldToClip));
}

internal render_quad*
DrawQuad(gl_draw_frame_data *DrawFrame, vec2 Position, vec2 Size, vec4 Color, uint32 RenderingOptions, real32 LitFactor)
{
    render_quad Quad = CreateRenderQuad(DrawFrame, Position, Size, 0, {0, 0}, {16, 16}, -1, Color, DrawFrame->ActiveZLayer, RenderingOptions, LitFactor);
    return(DrawQuadInView(DrawFrame, Quad));
}

internal render_quad*
DrawQuadTextured(gl_draw_frame_data *DrawFrame,
                 vec2                Position,
                 vec2                RenderSize,
                 clover_texture     *Texture,
                 ivec2               AtlasOffset,
                 ivec2               SpriteSize,
                 vec4                Color,
                 uint32              RenderingOptions,
                 real32              LitFactor)
{
    AddTextureToBoundList(DrawFrame, Texture->TextureID);
    render_quad Quad = CreateRenderQuad(DrawFrame,
                                        Position,
                                        RenderSize,
                                        Texture,
                                        v2Cast(AtlasOffset),
                                        v2Cast(SpriteSize),
                                        DrawFrame->ActiveTextureCount,
                                        Color,
                                        DrawFrame->ActiveZLayer,
                                        RenderingOptions,
                                        LitFactor);
    return(DrawQuadInView(DrawFrame, Quad));
}


// XFORM FUNCTIONS
internal render_quad *
DrawQuadXFormInFrame(gl_draw_frame_data *DrawFrame, render_quad Quad, mat4 XForm)
{
    mat4 WorldToClip = mat4Identity(1.0f);
    WorldToClip = mat4Multiply(WorldToClip, DrawFrame->ProjectionMatrix);
    WorldToClip = mat4Multiply(WorldToClip, DrawFrame->ViewMatrix);
    WorldToClip = mat4Multiply(WorldToClip, XForm);

    return(DrawQuadProjected(DrawFrame, Quad, WorldToClip));
}

internal render_quad*
DrawQuadXForm(gl_draw_frame_data *DrawFrame, mat4 XForm, vec2 RenderSize, vec4 Color, uint32 RenderingOptions, real32 LitFactor)
{
    vec4 WorldPosition = PositionFromMat4(XForm);
    render_quad Quad = CreateRenderQuad(DrawFrame, {WorldPosition.X, WorldPosition.Y}, RenderSize, 0, {0, 0}, {16, 16}, -1, Color, DrawFrame->ActiveZLayer, RenderingOptions, LitFactor);
    return(DrawQuadXFormInFrame(DrawFrame, Quad, XForm));
}

// TODO(Sleepster): Make this an ID Retreival 
internal render_quad*
DrawTextureXForm(gl_draw_frame_data *DrawFrame,
                 mat4                XForm,
                 vec2                RenderSize,
                 clover_texture     *Texture,
                 ivec2               AtlasOffset,
                 ivec2               SpriteSize,
                 vec4                Color,
                 uint32              RenderingOptions,
                 real32              LitFactor)
{
    vec4 WorldPosition = PositionFromMat4(XForm);
    render_quad Quad = CreateRenderQuad(DrawFrame,
                                        {WorldPosition.X, WorldPosition.Y},
                                        RenderSize,
                                        Texture,
                                        v2Cast(AtlasOffset),
                                        v2Cast(SpriteSize),
                                        DrawFrame->ActiveTextureCount,
                                        Color,
                                        DrawFrame->ActiveZLayer,
                                        RenderingOptions,
                                        LitFactor);
    return(DrawQuadXFormInFrame(DrawFrame, Quad, XForm));
}

internal void
DisplayText(gl_draw_frame_data *DrawFrame,
            string              TextToRender,
            vec2                Position,
            uint32              FontSize,
            vec4                Color,
            clover_font_data   *FontID,
            uint32              RenderingOptions,
            real32              LitFactor)
            //font_id             FontID = GF_UbuntuMono,
{
    clover_font_data *Font = FontID;
    Assert(Font->SizeOnLoad != 0);

    //real32 ScaleFactor = (real32)FontSize / 100;
    real32 ScaleFactor = (real32)FontSize / (real32)Font->SizeOnLoad;

    if(Font)
    {
        vec2   TextOrigin = Position;
        for(uint32 StringIndex = 0;
            StringIndex < TextToRender.Length;
            ++StringIndex)
        {
            if(TextToRender.Data[StringIndex] == '\n')
            {
                Position.Y += Font->FontHeight * ScaleFactor;
                Position.X  = TextOrigin.X;
                continue;
            }

            char              Character     = TextToRender.Data[StringIndex];
            clover_font_glyph Glyph         = Font->Glyphs[Character];
            vec2              GlyphPosition = {Position.X + (Glyph.Offset.X * ScaleFactor), Position.Y - ((Glyph.Size.Y - Glyph.Offset.Y) * ScaleFactor)};

            vec2  GlyphRenderSize = v2Cast(Glyph.Size) * (ScaleFactor * 2);
            ivec2 AtlasOffset     = Glyph.UVs;

            DrawQuadTextured(DrawFrame, GlyphPosition, GlyphRenderSize, &Font->FontAtlas, AtlasOffset, Glyph.Size, Color, RenderingOptions, LitFactor);
            Position.X += Glyph.Advance.X * (ScaleFactor * 2);
        }
    }
    else
    {
        cl_Error("Failed to find font!\n");
    }
}

internal point_light*
CreatePointLight(gl_draw_frame_data *DrawFrame,
                 vec2                Position, // THIS IS IN WORLDSPACE POSITION
                 real32              Strength,
                 real32              Radius,   // PIXELS
                 attenuation_data   *Attenuation,
                 vec4                Color)
{   
    point_light *Light    = &DrawFrame->PointLights[DrawFrame->PointLightCounter++];
    mat4 WorldToClip = mat4Multiply(DrawFrame->ProjectionMatrix, DrawFrame->ViewMatrix);
    vec4 NDCPosition = mat4Transform(WorldToClip, v2Expand(Position, 0, 1));
    
    Light->NDCPosition    = NDCPosition.XYZ;
    Light->WorldPosition  = v2Expand(Position, 0);
    Light->Radius         = Radius;
    Light->Strength       = Strength;
    Light->LightColor     = Color;
    Light->Attenuation    = *Attenuation;

    return(Light);
}

internal inline void
PushZLayer(gl_draw_frame_data *DrawFrame, int Layer)
{
    DrawFrame->ActiveZLayer = Layer;
}

internal inline void
PopZLayer(gl_draw_frame_data *DrawFrame)
{
    DrawFrame->ActiveZLayer = 0;
}
