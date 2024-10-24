
internal vec2
GJK_AveragePoint(entity *Entity) 
{
    vec2 Average = {};
    
    for(int Index = 0; 
        Index < Entity->VertexCount; 
        ++Index) 
    {
        Average.x += Entity->Vertex[Index].x;
        Average.y += Entity->Vertex[Index].y;
    }
    Average.x /= Entity->VertexCount;
    Average.y /= Entity->VertexCount;
    
    return(Average);
}

internal bool32
GJK_FurthestPoint(entity *Entity, vec2 Direction) 
{ 
    int BestIndex = 0;
    real32 MaxProduct = v2Dot(Entity->Vertex[0], Direction);
    vec2 Result = {};
    for(int Point = 1; 
        Point < Entity->VertexCount; 
        ++Point) 
    { 
        real32 Product = v2Dot(Entity->Vertex[Point], Direction); 
        if(Product > MaxProduct) 
        {
            MaxProduct = Product;
            BestIndex = Point;
        }
    }
    return(BestIndex);
}

internal vec2
GJK_Support(entity *A, entity *B, vec2 Direction) 
{
    int AMax = GJK_FurthestPoint(A, Direction);
    int BMax = GJK_FurthestPoint(B, -Direction);
    
    return(A->Vertex[AMax] - B->Vertex[BMax]);
}

internal bool32
GJK(entity *A, entity *B) 
{
    vec2 Simplex[32] = {};
    vec2 Position1 = GJK_AveragePoint(A);
    vec2 Position2 = GJK_AveragePoint(B);
    vec2 Direction = Position1 - Position2; // Set the direction to the point in the Minkowski Difference
    
    vec2 RegionA;
    vec2 RegionB;
    vec2 RegionC;
    vec2 AO;
    vec2 AB;
    vec2 AC;
    vec2 ACPerp;
    vec2 ABPerp;
    
    // If Zero set to any arbitray Axis
    if((Direction.x == 0) && (Direction.y == 0)) 
    {
        Direction.x = 1.0f;
    }
    
    // Set the First Support as the new point of the Simplex
    Simplex[0] = GJK_Support(A, B, Direction);
    if(v2Dot(Simplex[0], Direction) <= 0) 
    {
        return(0);
        // Collision not possible
    }
    
    // Invert the vector so the next search direction is flipped towards the origin
    Direction = v2Inverse(Direction);
    
    int Index = 0;
    for(;;) 
    {
        RegionA = Simplex[++Index] = GJK_Support(A, B, Direction);
        if(v2Dot(RegionA, Direction) <= 0) 
        {
            return(0);
            // Collision not possible
        }
        
        AO = v2Inverse(RegionA); // From point A to Origin is likely just negative A
        // Simplex has 2 points, it is not a triangle yet.
        if(Index < 2) 
        {
            RegionB = Simplex[0];
            AB = RegionB - RegionA;
            Direction = v2TripleProduct(AB, AO, AB);
            if(v2LengthSq(Direction) == 0) 
            {
                Direction = v2Perpendicular(AB);
            }
            continue;
        } 
        
        RegionB = Simplex[1];
        RegionC = Simplex[0];
        
        AB = RegionB - RegionA; // Normal from AB to the origin
        AC = RegionC - RegionA;
        
        ACPerp = v2TripleProduct(AB, AC, AC);
        if(v2Dot(ACPerp, AO) >= 0) 
        { 
            Direction = ACPerp;
        }
        else 
        {
            ABPerp = v2TripleProduct(AC, AB, AB);
            if(v2Dot(ABPerp, AO) < 0) 
            {
                return(1);
                //Collision
            }
            
            Simplex[0] = Simplex[1]; // Swap the First Element (C);
            Direction = ABPerp; // New Direction is normal to AB -> Origin
        }
        
        Simplex[1] = Simplex[2]; // Swap Elements
        --Index;
    }
}


























