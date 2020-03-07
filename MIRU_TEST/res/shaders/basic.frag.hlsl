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

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 0, float4, colour);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 0, float4, colour2);

PS_OUT main(PS_IN IN)
{
    PS_OUT OUT;
	OUT.colour = colour_image_cis.Sample(colour_sampler_cis, IN.texCoords) + colour2_image_cis.Sample(colour2_sampler_cis, IN.texCoords);
    return OUT;
}