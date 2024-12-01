/* ========================================================================
   $File: Clover_DrawingInterface.cpp $
   $Date: Mon, 25 Nov 24: 02:58PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

internal render_quad
CreateRenderQuad(vec2            Position,
                 vec2            Size,
                 vec4            Color,
                 clover_texture *Texture,
                 int32           TextureIndex,
                 int32           Layer)
{
    const real32 Top    = Position.Y;
    const real32 Bottom = Position.Y + Size.Y;
    const real32 Left   = Position.X;
    const real32 Right  = Position.X + Size.X;

    render_quad Quad  = {};
    Quad.TopLeft.Position     = v2Expand(vec2{Left, Top}, 0, 1);
    Quad.TopRight.Position    = v2Expand(vec2{Right, Top}, 0, 1);
    Quad.BottomLeft.Position  = v2Expand(vec2{Left, Bottom}, 0, 1);
    Quad.BottomRight.Position = v2Expand(vec2{Right, Bottom}, 0, 1);

    if(Texture)
    {
        Quad.BoundTextureID = Texture->TextureID;

        Quad.BottomLeft.TexCoords  = {0, 0};
        Quad.BottomRight.TexCoords = {1, 0};
        Quad.TopLeft.TexCoords     = {0, 1};
        Quad.TopRight.TexCoords    = {1, 1};
    }
    Quad.QuadColor = Color;
    Quad.QuadColor.A >= 1.0f ? Quad.SortingTransparency = 0 : Quad.SortingTransparency = 1;

    Quad.ZLayer    = Layer;
    for(uint32 Index = 0;
        Index < 4;
        ++Index)
    {
        Quad.Elements[Index].Color = Quad.QuadColor;
        Quad.Elements[Index].TextureIndex = TextureIndex;
    }

    return(Quad);
}

internal inline void
AddQuadToLayer(gl_draw_frame_data *DrawFrame, render_quad *Quad)
{
    if(Quad->QuadColor.A == 1.0f)
        DrawFrame->Layers[Quad->ZLayer].LayerOpaqueQuadCount++;
    else
        DrawFrame->Layers[Quad->ZLayer].LayerTransparentQuadCount++;
}

// IMPORTANT(Sleepster): The quad's vertex Coords are in NDC (normalized (-1, 1)) 
internal render_quad*
DrawQuadProjected(gl_draw_frame_data *DrawFrame, render_quad Quad, mat4 WorldToClip)
{
    Assert(DrawFrame->QuadCounter < MAX_QUADS);
    
	Quad.BottomLeft.Position  = mat4Transform(WorldToClip, Quad.BottomLeft.Position);
	Quad.BottomRight.Position = mat4Transform(WorldToClip, Quad.BottomRight.Position);
	Quad.TopLeft.Position     = mat4Transform(WorldToClip, Quad.TopLeft.Position);
	Quad.TopRight.Position    = mat4Transform(WorldToClip, Quad.TopRight.Position);

    bool32 ShouldCull =
        (Quad.BottomLeft.Position.X < -1 && Quad.TopLeft.Position.X < -1 && Quad.BottomRight.Position.X < -1 && Quad.TopRight.Position.X < -1 ||
         Quad.BottomLeft.Position.X >  1 && Quad.TopLeft.Position.X >  1 && Quad.BottomRight.Position.X >  1 && Quad.TopRight.Position.X >  1 ||
         Quad.BottomLeft.Position.Y < -1 && Quad.TopLeft.Position.Y < -1 && Quad.BottomRight.Position.Y < -1 && Quad.TopRight.Position.Y < -1 ||
         Quad.BottomLeft.Position.Y >  1 && Quad.TopLeft.Position.Y >  1 && Quad.BottomRight.Position.Y >  1 && Quad.TopRight.Position.Y >  1);
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
DrawQuad(gl_draw_frame_data *DrawFrame, vec2 Position, vec2 Size, vec4 Color)
{
    render_quad Quad = CreateRenderQuad(Position, Size, Color, 0, -1, DrawFrame->ActiveZLayer);
    return(DrawQuadInView(DrawFrame, Quad));
}

internal render_quad*
DrawQuadTextured(gl_draw_frame_data *DrawFrame,
                 vec2 Position, vec2 Size, vec4 Color,
                 clover_texture *Texture)
{
    render_quad Quad = CreateRenderQuad(Position, Size, Color, Texture, DrawFrame->ActiveTextureCount, DrawFrame->ActiveZLayer);
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
DrawQuadXForm(gl_draw_frame_data *DrawFrame, mat4 XForm, vec2 Size, vec4 Color)
{
    render_quad Quad = CreateRenderQuad({0, 0}, Size, Color, 0, -1, DrawFrame->ActiveZLayer);
    return(DrawQuadXFormInFrame(DrawFrame, Quad, XForm));
}

internal render_quad*
DrawImageXForm(gl_draw_frame_data *DrawFrame,
               mat4                XForm,
               vec2                Size,
               clover_texture     *Texture,
               vec4                Color)
{
    render_quad Quad = CreateRenderQuad({0, 0}, Size, Color, Texture, DrawFrame->ActiveTextureCount, DrawFrame->ActiveZLayer);
    return(DrawQuadXFormInFrame(DrawFrame, Quad, XForm));
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
