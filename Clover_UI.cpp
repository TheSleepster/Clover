/* ========================================================================
   $File: Clover_UI.cpp $
   $Date: September 20 2024 06:21 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#include "Intrinsics.h"

// UTILS
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

// CLOVER HEADERS
#include "Clover.h"
#include "Clover_Globals.h"
#include "Clover_Renderer.h"
#include "Clover_Input.h"
#include "Clover_UI.h"

// NOTE(Sleepster): Layer Starts at index 0
internal void
CloverUIPushLayer(clover_ui_context *Context, int32 Layer)
{
    Context->ActiveLayer = ui_layer(1 << Layer);
}

internal ui_element *
CloverUICreateElement(clover_ui_context *Context, ui_type Type)
{
    ui_element *Result = {};
    
    for(uint32 ElementIndex = 1;
        ElementIndex < MAX_UI_ELEMENTS;
        ElementIndex++)
    {
        ui_element *Found = &Context->UIElements[ElementIndex]; 
        if(!Found->IsValid)
        {
            Result = Found;
            
            Result->IsValid = true;
            Result->UIID.ID = ElementIndex;
            Result->UIID.LayerIdx = Context->ActiveLayer;
            Result->Type = Type;
            break;
        }
    }
    Assert(Result);
    
    Context->CreatedElementCounter++;
    return(Result);
}

internal void
CloverUIWidgetMakeXForm(ui_element *Widget)
{
    mat4 Identity    = mat4Identity(1.0f);
    mat4 Translation = mat4Multiply(Identity, mat4Translate(v2Expand(Widget->Position, 0.0f)));
    mat4 Scale       = mat4Multiply(Identity, mat4MakeScale(v2Expand(Widget->Size,     1.0f)));
    
    Widget->XForm = Translation * Scale;
} 

internal bool 
CloverUIIsHot(clover_ui_context *Context, ui_element *Widget)
{
    vec2 TransformedMousePos = TransformMouseCoords(Context->UICameraViewMatrix,
                                                    Context->UICameraProjectionMatrix, 
                                                    Context->GameInput->Keyboard.CurrentMouse,
                                                    SizeData);
    if(IsRangeWithinBounds(TransformedMousePos, Widget->OccupiedRange))
    {
        Widget->IsHot = true;
        return(true);
    }
    return(false);
}

internal ui_element_state
CloverUIButton(clover_ui_context *Context, 
               string             Name, 
               vec2               Position, 
               vec2               Size, 
               static_sprite_data Sprite, 
               vec4               Color) 
{
    ui_element *Widget = CloverUICreateElement(Context, UI_Button);
    
    Widget->ElementName = Name;
    Widget->Sprite      = Sprite;
    Widget->Position    = Position;
    Widget->Size        = Size;
    Widget->DrawColor   = Color;
    Widget->HasText     = true;
    Widget->OccupiedRange = CreateRange(vec2{Widget->Position.X - (Widget->Size.X * 0.5f), Widget->Position.Y - (Widget->Size.Y * 0.5f)}, 
                                        vec2{Widget->Position.X + (Widget->Size.X * 0.5f), Widget->Position.Y + (Widget->Size.Y * 0.5f)});
    ui_element_state ButtonState = {};
    if(CloverUIIsHot(Context, Widget))
    {
        ButtonState.IsHot = true;
        if(IsGameKeyPressed(ATTACK, Context->GameInput))
        {
            Widget->IsActive = true;
            ButtonState.IsReleased = false;
            ButtonState.IsPressed = true;
        }
        if(IsGameKeyReleased(ATTACK, Context->GameInput))
        {
            Widget->IsActive = false;
            ButtonState.IsReleased = true;
            ButtonState.IsPressed = false;
        }
    }
    ButtonState.UIID  = Widget->UIID;
    return(ButtonState);
} 


internal ui_element_state
CloverUIHoldButton(clover_ui_context *Context, 
                   string             Name, 
                   vec2               Position, 
                   vec2               Size, 
                   static_sprite_data Sprite, 
                   vec4               Color) 
{
    ui_element *Widget = CloverUICreateElement(Context, UI_Button);
    
    Widget->ElementName = Name;
    Widget->Sprite      = Sprite;
    Widget->Position    = Position;
    Widget->Size        = Size;
    Widget->DrawColor   = Color;
    Widget->HasText     = true;
    Widget->OccupiedRange = CreateRange(vec2{Widget->Position.X - (Widget->Size.X * 0.5f), Widget->Position.Y - (Widget->Size.Y * 0.5f)}, 
                                        vec2{Widget->Position.X + (Widget->Size.X * 0.5f), Widget->Position.Y + (Widget->Size.Y * 0.5f)});
    ui_element_state ButtonState = {};
    if(CloverUIIsHot(Context, Widget))
    {
        ButtonState.IsHot = true;
        if(IsGameKeyDown(ATTACK, Context->GameInput))
        {
            Widget->IsActive = true;
            ButtonState.IsPressed = true;
        }
    }
    ButtonState.UIID  = Widget->UIID;
    return(ButtonState);
} 


internal ui_element*
CloverUIMakeTextElement(clover_ui_context *Context,
                        string             FormattedText,
                        vec2               Position,
                        real32             FontScaleFactor,
                        ui_text_alignment  Alignment, 
                        vec4               DrawColor)
{
    ui_element *TextElement = CloverUICreateElement(Context, UI_Text);
    
    vec2 TextOrigin = Position;
    real32 TrueScale = FontScaleFactor / 100.0f;
    
    vec2 TotalTextSize   = {};
    TotalTextSize.Y = Context->ActiveFont->FontHeight * TrueScale; 
    for(uint32 StringIndex = 0;
        StringIndex < FormattedText.Length;
        StringIndex++)
    {
        if(FormattedText.Data[StringIndex] == '\n') 
        {
            TotalTextSize.Y += Context->ActiveFont->FontHeight * TrueScale; 
            continue; 
        }
        
        char Character = (char)FormattedText.Data[StringIndex];
        font_glyph Glyph = Context->ActiveFont->Glyphs[Character];
        TotalTextSize.X += Glyph.GlyphAdvance.X * TrueScale;
    }
    
    switch(Alignment)
    {
        case TEXT_ALIGNMENT_Left:
        {
            TextOrigin.X -= TotalTextSize.X;
        }break;
        case TEXT_ALIGNMENT_Center:
        {
            TextOrigin.X -= TotalTextSize.X * 0.5f;
        }break;
        default: break;
    }
    
    TextElement->Position    = TextOrigin;
    TextElement->TextOrigin  = TextOrigin;
    TextElement->Size        = TotalTextSize;
    TextElement->FontScale   = FontScaleFactor;
    TextElement->ElementText = FormattedText;
    TextElement->TextDrawColor   = DrawColor;
    return(TextElement);
}

internal ui_element_state
CloverUITextBox(clover_ui_context *Context, 
                string             Name, 
                string             FormattedText, 
                vec2               BoxPosition, 
                vec2               BoxSize,
                vec2               TextOrigin,
                real32             FontScaleFactor, 
                static_sprite_data Sprite,
                vec4               BoxColor,
                vec4               TextColor, 
                ui_text_alignment  ContentAlignment)
{
    ui_element *TextData     = CloverUIMakeTextElement(Context, FormattedText, TextOrigin, FontScaleFactor, ContentAlignment, TextColor);
    ui_element *Container    = CloverUICreateElement(Context, UI_TextBox);
    
    Container->ElementName   = Name;
    Container->ElementText   = FormattedText;
    Container->TextOrigin    = TextData->Position;
    Container->FontScale     = FontScaleFactor;
    
    Container->Position      = BoxPosition;
    Container->Size          = TextData->Size + BoxSize;
    Container->Sprite        = Sprite;
    Container->DrawColor     = BoxColor;
    Container->TextDrawColor = TextColor; 
    
    ui_element_state BoxState = {};
    BoxState.IsHot = CloverUIIsHot(Context, Container);
    BoxState.UIID  = Container->UIID;
    return(BoxState);
}

internal ui_element_state
CloverUISpriteElement(clover_ui_context *Context, vec2 Position, vec2 Size, mat4 XForm, static_sprite_data Sprite, vec4 Color)
{
    ui_element *SpriteElement = CloverUICreateElement(Context, UI_ItemSprite);
    SpriteElement->Position = Position;
    SpriteElement->Size = Size;
    SpriteElement->Sprite = Sprite;
    SpriteElement->DrawColor = Color;
    if(XForm != NULLMATRIX)
    {
        SpriteElement->XForm = XForm;
    }
    
    ui_element_state SpriteState = {};
    SpriteState.UIID = SpriteElement->UIID;
    return(SpriteState);
}

internal int32
CloverUISortElementsByLayer(const void *A, const void *B)
{
    const ui_element *ElementA = (const ui_element*)A;
    const ui_element *ElementB = (const ui_element*)B;
    
    bool IsOpaqueA = (ElementA->DrawColor.A == 1.0f);
    bool IsOpaqueB = (ElementB->DrawColor.A == 1.0f);
    
    return((ElementA->UIID.LayerIdx > ElementB->UIID.LayerIdx) ?  1 :
           (ElementA->UIID.LayerIdx < ElementB->UIID.LayerIdx) ? -1 : 
           (IsOpaqueA && !IsOpaqueB) ?  1 :
           (!IsOpaqueA && IsOpaqueB) ? -1 : 0);
}

internal void
CloverUIDrawWidgets(gl_render_data *RenderData, clover_ui_context *Context)
{
    qsort(Context->UIElements, Context->CreatedElementCounter, sizeof(struct ui_element), CloverUISortElementsByLayer);
    for(int32 WidgetIndex = 0;
        WidgetIndex <= Context->CreatedElementCounter;
        WidgetIndex++)
    {
        ui_element *Widget = &Context->UIElements[WidgetIndex];
        if(Widget)
        {
            // NOTE(Sleepster): Generating an XForm, It's required 
            if(Widget->XForm == NULLMATRIX)
            {
                CloverUIWidgetMakeXForm(Widget);
            }
            switch(Widget->Type)
            {
                case UI_Button:
                {
                    quad WidgetQuad = CreateDrawQuad(RenderData, 
                                                     Widget->Position, 
                                                     Widget->Size,
                                                     Widget->Sprite.SpriteSize,
                                                     Widget->Sprite.AtlasOffset,
                                                     0,
                                                     Widget->DrawColor,
                                                     1);
                    DrawUIQuadXForm(RenderData, &WidgetQuad, &Widget->XForm, 0);
                }break;
                case UI_TextBox:
                {
                    quad WidgetQuad = CreateDrawQuad(RenderData, 
                                                     Widget->Position, 
                                                     Widget->Size,
                                                     Widget->Sprite.SpriteSize,
                                                     Widget->Sprite.AtlasOffset,
                                                     0,
                                                     Widget->DrawColor,
                                                     1);
                    DrawUIQuadXForm(RenderData, &WidgetQuad, &Widget->XForm, 0);
                    DrawUIText(RenderData, Widget->ElementText, Widget->TextOrigin, Widget->FontScale, Context->ActiveFontIndex, Widget->TextDrawColor);
                }break;
                case UI_ItemSprite:
                {
                    quad WidgetQuad = CreateDrawQuad(RenderData, 
                                                     Widget->Position, 
                                                     Widget->Size,
                                                     Widget->Sprite.SpriteSize,
                                                     Widget->Sprite.AtlasOffset,
                                                     0,
                                                     Widget->DrawColor,
                                                     1);
                    DrawUIQuadXForm(RenderData, &WidgetQuad, &Widget->XForm, 0);
                }break;
                case UI_Text:
                {
                    DrawUIText(RenderData, Widget->ElementText, Widget->TextOrigin, Widget->FontScale, Context->ActiveFontIndex, Widget->TextDrawColor);
                }
                case UI_Nil:{}break;
                default: {InvalidCodePath;}break;
            }
        }
    }
}

internal void
CloverUIResetState(clover_ui_context *Context)
{
    Context->CreatedElementCounter = 0;
    Context->ActiveLayer = UI_LAYER_0;
    memset(&Context->UIElements, 0, sizeof(Context->UIElements));
}
