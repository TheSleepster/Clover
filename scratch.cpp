/* ========================================================================
   $File: scratch.cpp $
   $Date: September 20 2024 05:46 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

internal void
UIResetState(game_state *State)
{
    State->GameData.UIState.ElementCount = 0;
    memset(State->GameData.UIState.UIElements, 0, (sizeof(struct ui_element) * MAX_UI_ELEMENTS));
}

internal ui_element*
WidgetCreate(game_state *State, ui_type ElementType)
{
    ui_element *Element = {};

    for(uint32 ElementIndex = 1;
        ElementIndex < MAX_UI_ELEMENTS;
        ++ElementIndex)
    {
        ui_element *Found = &State->GameData.UIState.UIElements[ElementIndex];
        if(!Found->Captured)
        {
            Element = Found;

            Element->Captured      = true;
            Element->IsDisplayed   = true;
            Element->Type          = ElementType;
            Element->UIID.ID       = ElementIndex;
            Element->UIID.LayerIdx = UI_LAYER_0;
            Element->XForm         = mat4Identity(1.0f);
            break;
        }
    }
    Assert(Element);

    State->GameData.UIState.ElementCount++;
    return(Element);
}

internal inline void
WidgetAddSprite(ui_element *Widget, static_sprite_data Sprite)
{
    Widget->Sprite = Sprite;
}

internal inline void
WidgetAddText(ui_element *Widget, string FormattedText)
{
    Widget->ElementText = FormattedText;
}

internal inline void
WidgetSetXForm(ui_element *Widget, mat4 XForm)
{
    Widget->XForm = XForm;
}

internal inline void
WidgetSetColor(ui_element *Widget, vec4 Color)
{
    Widget->DrawColor = Color;
}

internal inline void
WidgetSetPosition(ui_element *Widget, vec2 Position)
{
    Widget->Position = Position;
    Widget->XForm = mat4Multiply(Widget->XForm, mat4Translate(v2Expand(Position, 0.0f)));
}

internal inline void
WidgetSetSize(ui_element *Widget, vec2 Size)
{
    Widget->Size = Size;
    Widget->XForm = mat4Multiply(Widget->XForm, mat4MakeScale(v2Expand(Size, 1.0f)));
}

internal inline void
WidgetApplyXForm(ui_element *Widget)
{
    mat4 Identity    = mat4Identity(1.0f);
    mat4 Translation = mat4Multiply(Identity, mat4Translate(v2Expand(Widget->Position, 0.0f)));
    mat4 Scale       = mat4Multiply(Identity, mat4MakeScale(v2Expand(Widget->Size,     0.0f)));

    Widget->XForm = Translation * Scale;
}

internal inline void
WidgetCalculateRange(ui_element *Widget)
{
    Widget->OccupiedRange = CreateRange(vec2{Widget->Position.X - (Widget->Size.X * 0.5f), Widget->Position.Y - (Widget->Size.Y * 0.5f)}, 
                                        vec2{Widget->Position.X + (Widget->Size.X * 0.5f), Widget->Position.Y + (Widget->Size.Y * 0.5f)});
}

internal inline void
WidgetPushLayer(ui_element *Widget, int32 Layer)
{
    Widget->UIID.LayerIdx = (ui_layer)(1 << Layer);
}

internal bool
WidgetIsHot(gl_render_data *RenderData, game_state *State, ui_element *Widget)
{
    vec2 MousePos = ConvertMouseToUIPos(RenderData, State->GameInput.Keyboard.CurrentMouse, SizeData);

    if(IsRangeWithinBounds(Widget->OccupiedRange, MousePos))
    {
        State->GameData.UIState.HotLastFrame = Widget->UIID;
        Widget->IsHot = true;
    }

    return(Widget->IsHot);
}

internal bool
WidgetDoButton(gl_render_data *RenderData, game_state *State, ui_element *Widget)
{
    Widget->OccupiedRange = CreateRange(vec2{Widget->Position.X - (Widget->Size.X * 0.5f), Widget->Position.Y - (Widget->Size.Y * 0.5f)}, 
                                        vec2{Widget->Position.X + (Widget->Size.X * 0.5f), Widget->Position.Y + (Widget->Size.Y * 0.5f)});
    if(WidgetIsHot(RenderData, State, Widget))
    {
        if(IsGameKeyDown(ATTACK, &State->GameInput))
        {
            Widget->IsActive = true;
            return(true);
        }
    }
    else
    {
        if(Widget->UIID.ID == State->GameData.UIState.HotLastFrame.ID)
        {
            State->GameData.UIState.HotLastFrame = State->GameData.UIState.UIElements[0].UIID;
        }
    }

    return(false);
}

internal void
WidgetSetupButton(game_state *State, 
                  ui_element *Widget,
                  vec2        Position,
                  vec2        Size, 
                  sprite_type Sprite,
                  vec4        Color)
{
        WidgetSetPosition(Widget, Position);
        WidgetSetSize(Widget, Size);
        WidgetAddSprite(Widget, GetSprite(State, Sprite));
        WidgetCalculateRange(Widget);
        WidgetSetColor(Widget, Color);
}

internal void
WidgetSetupDescBox(game_state *State, 
                   ui_element *Widget,
                   vec2        Position,
                   vec2        Size,
                   sprite_type Sprite,
                   string      FormattedText,
                   vec4        Color)
{
        WidgetSetPosition(Widget, Position);
        WidgetSetSize(Widget, Size);
        WidgetAddSprite(Widget, GetSprite(State, Sprite));
        WidgetAddText(Widget, FormattedText);
        WidgetCalculateRange(Widget);
        WidgetSetColor(Widget, Color);
}

internal void
UIUpdateState(gl_render_data *RenderData, game_state *State)
{
    for(uint32 WidgetIndex = 0;
        WidgetIndex < State->GameData.UIState.ElementCount;
        WidgetIndex++)
    {
        ui_element *Widget = &State->GameData.UIState.UIElements[WidgetIndex];
        if(Widget->Captured)
        {
            if(Widget->XForm == NULLMATRIX)
            {
                WidgetApplyXForm(Widget);
            }
            Widget->OccupiedRange = CreateRange(vec2{Widget->Position.X - (Widget->Size.X * 0.5f), Widget->Position.Y - (Widget->Size.Y * 0.5f)}, 
                    vec2{Widget->Position.X + (Widget->Size.X * 0.5f), Widget->Position.Y + (Widget->Size.Y * 0.5f)});
        }
    }
}

internal void
UIDrawElements(gl_render_data *RenderData, game_state *State)
{
    for(uint32 ElementIndex = 0;
        ElementIndex <= State->GameData.UIState.ElementCount;
        ElementIndex++)
    {
        ui_element *Widget = &State->GameData.UIState.UIElements[ElementIndex];
        if(Widget->IsDisplayed)
        {
            quad Element = CreateDrawQuad(RenderData, 
                    Widget->Position, 
                    Widget->Size, 
                    Widget->Sprite.SpriteSize, 
                    Widget->Sprite.AtlasOffset, 
                    0, 
                    Widget->DrawColor, 
                    1);
            DrawUIQuadXForm(RenderData, &Element, &Widget->XForm);
        }
    }
}



// NOTE(Sleepster): LGIHT
        float AmbientStrength = 0.3;
        vec3  GlobalAmbientColor = vec3(0.2);

        for(int LightIndex = 0; LightIndex < PointLightCount; ++LightIndex)
        {
            point_light PointLight = PointLights[LightIndex];

            vec3 LightDir = normalize(PointLight.Position - vFragPos);
            float LightDist = length(PointLight.Position - vFragPos);

            float Attenuation = 1.0 / (PointLight.Attenuation.Constant 
                                 + PointLight.Attenuation.Linear * LightDist 
                                 + PointLight.Attenuation.Quadratic * (LightDist * LightDist));

            vec3 Norm = normalize(vNormals);
            vec3 AmbientLight = AmbientStrength * GlobalAmbientColor;

            // float Diff = max(dot(LightDir, Norm), 0.0);
            float Diff = 1.0;
            vec3 Diffuse = Diff * PointLight.LightColor.rgb * Attenuation;

            vec4 TextureColor = texelFetch(GameAtlas, ivec2(vTextureUVs), 0);
            if (TextureColor.a <= 0.1)
            {
                discard;
            }

            vec4 ObjectColor = vec4(TextureColor.rgb * vMatColor.rgb, TextureColor.a);
            FragColor = vec4((AmbientLight + Diffuse) * PointLight.Strength, 1.0) * ObjectColor;































































































int main(int argc, char *Argv[])
{
}
