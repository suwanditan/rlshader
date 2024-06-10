#version 330

// Input uniform values
uniform vec4 color;

// Output fragment color
out vec4 finalColor;
uniform float currentTime;

// NOTE: Add here your custom variables

void main()
{
    // Each point is drawn as a screen space square of gl_PointSize size. gl_PointCoord contains where we are inside of
    // it. (0, 0) is the top left, (1, 1) the bottom right corner.
    // Draw each point as a colored circle with alpha 1.0 in the center and 0.0 at the outer edges.
    
    vec3 ctmp = vec3(0.);
    ctmp = vec3(1.0, 0.5, abs(sin(currentTime)));

    //gl_FragColor = vec4(color.rgb,1.0);
    finalColor = vec4(ctmp.rgb, length(gl_PointCoord.xy - vec2(0.5)) );
    
    //finalColor = vec4(color.rgb, color.a * (1 - length(gl_PointCoord.xy - vec2(0.5))*2));
}
