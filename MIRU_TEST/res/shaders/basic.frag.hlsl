#include "msc_common.h"

struct PS_IN
{
    MIRU_LOCATION(0, float4, position, SV_POSITION);
    MIRU_LOCATION(1, float3, texCoords, TEXCOORD1);
};
    
struct PS_OUT
{
    MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 2, float4, colour);

PS_OUT main(PS_IN IN)
{
    PS_OUT OUT;
	OUT.colour = colour_image_cis.Sample(colour_sampler_cis, IN.texCoords);
    return OUT;
}