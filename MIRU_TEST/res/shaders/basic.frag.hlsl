#include "msc_common.h"

struct PS_IN
{
    MIRU_LOCATION(0, float4, position, SV_POSITION);
    MIRU_LOCATION(1, float2, texCoords, TEXCOORD);
};
    
struct PS_OUT
{
    MIRU_LOCATION(0, float4, colour, SV_TARGET);
};

PS_OUT main(PS_IN IN)
{
    PS_OUT OUT;
    OUT.colour = float4(IN.texCoords, 0.0, 1.0);
    return OUT;
}