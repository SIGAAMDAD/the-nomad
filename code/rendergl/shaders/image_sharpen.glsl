// -- Sharpening --

uniform float u_SharpenAmount;
uniform vec2 u_ScreenSize;

#define sharp_clamp 0.000  //[0.000 to 1.000] Limits maximum amount of sharpening a pixel recieves - Default is 0.035

#define offset_bias 6.0  //[0.0 to 6.0] Offset bias adjusts the radius of the sampling pattern.
						 //I designed the pattern for offset_bias 1.0, but feel free to experiment.

//#define CoefLuma vec3( 0.2126, 0.7152, 0.0722 )      // BT.709 & sRBG luma coefficient (Monitors and HD Television)
//#define CoefLuma vec3( 0.299, 0.587, 0.114 )       // BT.601 luma coefficient (SD Television)
#define CoefLuma vec3( 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0 ) // Equal weight coefficient

vec4 sharpenImage( sampler2D tex, vec2 pos )
{
	vec4 colorInput = texture2D( tex, pos );
  	
	vec3 ori = colorInput.rgb;

	// -- Combining the strength and luma multipliers --
	vec3 sharp_strength_luma = CoefLuma * u_SharpenAmount; //I'll be combining even more multipliers with it later on
	
	// -- Gaussian filter --
	//   [ .25, .50, .25]     [ 1 , 2 , 1 ]
	//   [ .50,   1, .50]  =  [ 2 , 4 , 2 ]
 	//   [ .25, .50, .25]     [ 1 , 2 , 1 ]


	float px = 1.0 / u_ScreenSize[0];
	float py = 1.0 / u_ScreenSize[1];

	vec3 blur_ori = texture2D(tex, pos + vec2(px,-py) * 0.5 * offset_bias).rgb // South East
		+ texture2D(tex, pos + vec2(-px,-py) * 0.5 * offset_bias).rgb  // South West
		+ texture2D(tex, pos + vec2(px,py) * 0.5 * offset_bias).rgb // North East
		+ texture2D(tex, pos + vec2(-px,py) * 0.5 * offset_bias).rgb // North West
		* 0.25;  // ( /= 4) Divide by the number of texture fetches

	// -- Calculate the sharpening --
	vec3 sharp = ori - blur_ori;  //Subtracting the blurred image from the original image

	// -- Adjust strength of the sharpening and clamp it--
	vec4 sharp_strength_luma_clamp = vec4(sharp_strength_luma * (0.5 / sharp_clamp),0.5); //Roll part of the clamp into the dot

	float sharp_luma = clamp((dot(vec4(sharp,1.0), sharp_strength_luma_clamp)), 0.0,1.0 ); //Calculate the luma, adjust the strength, scale up and clamp
	sharp_luma = (sharp_clamp * 2.0) * sharp_luma - sharp_clamp; //scale down


	// -- Combining the values to get the final sharpened pixel	--

	colorInput.rgb = colorInput.rgb + sharp_luma;    // Add the sharpening to the input color.
	return clamp(colorInput, 0.0,1.0);
}